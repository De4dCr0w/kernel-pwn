#include <stdio.h>

int main(){
	char *ch = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%%&\\'()*+,-./:;<=>?@[]^_`{|}~";
	//char input[34]= {0};
	//char order[0x100];
	char order2[0x100];
	//FILE * fd = fopen("save.txt","a+");
	//fscanf(fd,"%s",input);
	for(int i = 1;i<strlen(ch);i++){
		if (ch[i] == '\"' ||  ch[i] == '\\' || ch[i] == '`' ){
			//sprintf(order,"echo \"%s\\%c\" > save.txt",input,ch[i]);
			sprintf(order2,"./pwn \\%c",ch[i] );
		}
		else{
		//sprintf(order,"echo \"%s%c\" > save.txt",input,ch[i]);
		sprintf(order2,"./pwn flag{TH%c",ch[i] );
		}
		printf("%s\n",order2);
		//system(order);
		system(order2);

		//strcpy(order,"echo \"");
		//strcpy
		//system("")
	}

}
