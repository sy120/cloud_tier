#include <stdio.h>
#include <stdlib.h>
int main() {
	FILE *fp = fopen("/home/bitnami/larget_input.txt" , "r");
    	char buf[1000000];
	printf("size of buf = %zd\n", sizeof(buf));
    	fread(buf,sizeof(buf),1,fp);
	//FILE *fd = fopen("/home/bitnami/wf_output.txt" , "w");
	//ssize_t write_byte = fwrite(buf, 1, sizeof(buf), fd);
	//printf("write_byte is %zd\n", write_byte);\
	fclose(fp);
	//fclose(fd);
}
