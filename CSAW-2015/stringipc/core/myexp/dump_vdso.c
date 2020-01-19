/*************************************************************************
	> File Name: dump_vdso.c
	> Author:De4dCr0w 
	> Mail:test@gmail.com 
	> Created Time: Tue 14 Jan 2020 11:21:09 PM EST
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/auxv.h> 

 #include <sys/mman.h>
int main(){
    int test;
    size_t result=0;
    unsigned long sysinfo_ehdr = getauxval(AT_SYSINFO_EHDR);
    result=memmem(sysinfo_ehdr,0x1000,"gettimeofday",12);
    printf("[+]VDSO : %p\n",sysinfo_ehdr);
    printf("[+]The offset of gettimeofday is : %x\n",result-sysinfo_ehdr);
    scanf("Wait! %d", test);  
    /* 
    gdb break point at 0x400A36
    and then dump memory
    why only dump 0x1000 ???
    
    if (sysinfo_ehdr!=0){
        for (int i=0;i<0x2000;i+=1){
            printf("%02x ",*(unsigned char *)(sysinfo_ehdr+i));
        }
    }
*/
	return 0;
}
