/*
Cloud Tier System - Song Yang and Qi Xu

Copyright (C) 2016-2017  Song Yang <sy120@duke.edu>

gcc -Wall  hello_single.c `pkg-config fuse --cflags --libs` -o hello_single -lm

Fuse C Program
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK_SIZE 4096
#define SIZE 1

static const char *hello_path = "/hello";

void logging(const char* param, const char* describ) {
  int fd = open("/home/bitnami/logging", O_WRONLY | O_APPEND);
  char temp[50] = {'\0'};
  strcpy(temp, describ);
  strcat(temp, param);
  strcat(temp, "\n");
  write(fd, temp, 50);
  close(fd);
}

static int hello_getattr(const char *path, struct stat *stbuf) {
  int res = 0;
  logging(path, "getattr path = ");
  memset(stbuf, 0, sizeof(struct stat));
  
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0777;
    stbuf->st_nlink = 2;
  } 
  else if (strcmp(path, hello_path) == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = 500 * BLOCK_SIZE;
  } 
  else {
    res = -ENOENT;
  }
  return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi) {
  (void) offset;
  (void) fi;
  logging(path, "readdir path = ");
  if (strcmp(path, "/") != 0)
    return -ENOENT;
  
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  filler(buf, hello_path + 1, NULL, 0);
  return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi) {
  logging(path, "open path = ");
  if (strcmp(path, hello_path) != 0) {
    return -ENOENT;
  }
  return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi) {
  /*
    real_offset:    offset within a particular block
    current_block:  current block has been read
    block_number:   current block to be read from
    block_offset:   current block offset
    read_size:      size to be read in this block 
  */

  //baseline for each block
  off_t len = 4096;
  //useless
  (void) fi;
  //readpath container:  readpath may change
  char readpath[256];
  //print container: print to logging
  char tempbuf[50];

  //check path
  if(strcmp(path, hello_path) != 0) return -ENOENT;

  //initilize cursor
	int cursor = 0;
  int current_block = (int) offset / BLOCK_SIZE - 1;
  sprintf(tempbuf, "%zd", size);
  logging(tempbuf, "*** pread size = ");
	for (; cursor < size; cursor++) {
	   size_t real_offset = offset + cursor;
	   int block_number = (int) (real_offset / BLOCK_SIZE);
     if (current_block == block_number) continue;
     else {
        current_block = block_number;
        size_t block_offset = real_offset % BLOCK_SIZE;
        sprintf(readpath, "/home/bitnami/files/%d", block_number);
	      int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
        //check realoffset should be < len
        int read_size = len;
	      if (block_offset < len) {
	         if (block_offset + size - cursor < len) {
              read_size = size - cursor;
           }
	         else {
              read_size = len - block_offset; 
           }
           //pread: fd, buf, size, offset
	         ssize_t res = pread(fd, buf+cursor, read_size, block_offset);
           if (res == -1) {
              int err = errno;
              sprintf(tempbuf, "%d", err);
              logging(tempbuf, "*** pread errno = ");
           }
           else {
              sprintf(tempbuf, "%d", cursor);
              logging(tempbuf, "*** pread cursor = ");
              logging(tempbuf, "*** pread success !");
           }
        }	  
        else {
	       size = 0;
	      }
     }
   }
   return size;
}

static int hello_write(const char *path, const char* buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  /*
    real_offset:    offset within a particular block
    current_block:  current block has been read
    block_number:   current block to be read from
    block_offset:   current block offset
    write_size:     size to be written in this block 
  */
  //baseline for each block
  off_t len = 4096;
  //useless
  (void) fi;
  //readpath container:  readpath may change
  char readpath[256];
  //print container: print to logging
  char tempbuf[50];
  
  /* test input parameter for function hello_write */
  logging(path, "write path = ");
  sprintf(tempbuf, "%zd", size);
  logging(tempbuf, "write size = ");
  sprintf(tempbuf, "%zd", offset);
  logging(tempbuf, "write offset = ");
  
  int cursor = 0;
//initilize cursor
  int current_block = (int) offset / BLOCK_SIZE - 1;
  for (; cursor < size; cursor++) {
     size_t real_offset = offset + cursor;
     int block_number = (int) (real_offset / BLOCK_SIZE);
     if (current_block == block_number) continue;
     else {
        current_block = block_number;
        size_t block_offset = real_offset % BLOCK_SIZE;
        sprintf(readpath, "/home/bitnami/files/%d", block_number);
        int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
        //check realoffset should be < len
        int write_size = len;
        if (block_offset < len) {
           if (block_offset + size - cursor < len) {
              write_size = size - cursor;
           }
           else {
              write_size = len - block_offset; 
           }
           //pread: fd, buf, size, offset
           sprintf(tempbuf, "%d", write_size);
           logging(tempbuf, "write size = ");
           sprintf(tempbuf, "%zd", block_offset);
           logging(tempbuf, "write offset = ");
           ssize_t res = pwrite(fd, buf+cursor, write_size, block_offset);
           if (res == -1) {
              int err = errno;  
              sprintf(tempbuf, "%d", err);
              logging(tempbuf, "*** pwrite errno = ");
           }
           else {
              logging(tempbuf, "*** pwrite success !");
           }
        }   
        else {
         size = 0;
        }
     }
   }
   return size;
}

static int hello_truncate(const char* path, off_t size) {
  logging(path, "truncate path = ");
  int res = truncate(path, size);
  if (res == -1)
       return -errno;
  return 0;  
}

static struct fuse_operations hello_oper = {
  .getattr	= hello_getattr,
  .readdir	= hello_readdir,
  .open		= hello_open,
  .read         = hello_read,
  .write        = hello_write,
  .truncate	= hello_truncate,
};

int main(int argc, char *argv[]) {
	return fuse_main(argc, argv, &hello_oper, NULL);
}



