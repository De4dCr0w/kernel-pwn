#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/mman.h>
struct flag
{
	char* ptr;
	unsigned long length;
};
int main(int argc,char **argv)
{
	char *buf;
	struct flag *test_flag;
	int fd = open("/dev/baby",O_RDWR);
	if(fd < 0)
	{
		printf("open fail!\n");
//		exit(0);
	}	
	mmap(NULL,0x1000,PROT_NONE,MAP_SHARED|MAP_ANONYMOUS,0,0);
	buf = mmap(NULL,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,0,0);
	mmap(NULL,0x1000,PROT_NONE,MAP_SHARED|MAP_ANONYMOUS,0,0);
	printf("[debug] get a memory:%p\n",buf);
	//strcpy(buf[0x1000-strlen(argv[1])],argv[1]);
	for(int i = 0;i<strlen(argv[1]);i++)
	{
		buf[0x1000-strlen(argv[1])+i] = argv[1][i];
	}
	printf("[debug] ptr:%p,str:%s\n",&buf[0x1000-strlen(argv[1])],argv[1]);
	((struct flag *)buf)->ptr = &buf[0x1000-strlen(argv[1])];
	((struct flag *)buf)->length = 33;
	///write(fd,test_flag);
	ioctl(fd,0x1337,buf);
	close(fd);
	return 0;
}
