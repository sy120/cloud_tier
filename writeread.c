/*
Cloud Tier System - Song Yang and Qi Xu

Copyright (C) 2016-2017  Song Yang <sy120@duke.edu>

gcc -o test writeread.c

Write and Read Test Program
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BLOCK_SIZE 4096
#define SIZE 1


int my_write(char* buf, size_t size, off_t offset) {

  size_t offset_within_block = (size_t)(offset % BLOCK_SIZE);
  size_t block_number = (size_t) (offset / BLOCK_SIZE);
  printf("offset is %zd\n", offset_within_block);
  printf("block_number is %zd\n", block_number);

  char readpath[256] = "/home/bitnami/demo/hello";
  printf("The write path is %s\n", readpath);
  sprintf(readpath, "/home/bitnami/demo/hello");
  int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
  printf("write fd is %d\n", fd);
  printf("write offset is %zd\n", offset);
  printf("write size is %zd\n", size);
  printf("The write buffer is %s\n", buf);
  ssize_t res = pwrite(fd, buf, size, offset);
  if (res >= 0) printf("small write success!\n");
  else {
    int err = errno;
    printf("small write errno = %d!\n", err);
    printf("error number is %zd\n", res);
  }
  close(fd);
  return size;
}

int my_read(char* buf, size_t size, off_t offset) {
  printf("read offset is %zd\n", offset);
  printf("read size is %zd\n", size);
  char readpath[256] = "/home/bitnami/demo/hello";
  printf("The read path is %s\n", readpath);
  sprintf(readpath, "/home/bitnami/demo/hello");
  int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
  printf("read fd is %d\n", fd);
  printf("read offset is %zd\n", offset);
  printf("read size is %zd\n", size);
  ssize_t res = pread(fd, buf, size, offset);
  if (res >= 0) {
    printf("small read success! %zd \n", res);
    printf("The read buffer is %s\n", buf);
    FILE *fp;
    fp = fopen("/home/bitnami/read_result.txt" , "w");
    printf("size of buf = %zd", strlen(buf));
    fwrite(buf, 1, strlen(buf), fp);
  }
  else {
    int err = errno;
    printf("small read errno = %d!\n", err);
    printf("error number is %zd\n", res);
  }
  close(fd);	
  return size;
}

int main(int argc, char* argv[]) {
  if (argc < 1) { 
  printf("type read or write!\n"); 
      return 1; 
  } 
  off_t offset = 0; 
  char buf[1000000] = {'\0'}; 
  char output[1000000] = {'\0'}; 
  if (strcmp("write", argv[1]) == 0) { 
  	FILE *fp;
	char str[1000000];
	fp = fopen("/home/bitnami/large_input.txt" , "r");
	if (fp == NULL) {
		perror("Error opening file");
		return -1;
	}
	fread(str, sizeof(str), 1, fp);
	puts(str);
	offset = atoi(argv[2]);
	my_write(str, strlen(str), offset);
	fclose(fp);
  } else {  
  printf("type read size and offset!\n"); 
  size_t read_size = atoi(argv[2]); 
  printf("The buf contains %s\n", buf); 
  off_t offset = atoi(argv[3]); 
  my_read(output, read_size, offset);	
  } 
  return 1;
}
