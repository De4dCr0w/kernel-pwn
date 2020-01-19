#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/auxv.h> 

 #include <sys/mman.h>
int main(){

	unsigned long sysinfo_ehdr = getauxval(AT_SYSINFO_EHDR);
	if (sysinfo_ehdr!=0){
		for (int i=0;i<0x2000;i+=1){
			printf("%02x ",*(unsigned char *)(sysinfo_ehdr+i));
		}
	}

}

