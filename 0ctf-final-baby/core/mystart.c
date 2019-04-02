#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
	char order[50];
	char *str = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%%&\\'()*+,-./:;<=>?@[]^_`{|}~";
	for(int i = 0;i < strlen(str);i++)
	{
		sprintf(order,"./mypwn flag{THI%c",str[i]);
		system(order);
		printf("cmd:%s\n",order);
	}
	return 0;
}
