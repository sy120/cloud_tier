/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
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

#define BLOCK_SIZE 4
#define SIZE 1

//static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
//char *readpath = "/home/bitnami/files";
//off_t filesize;

void logging(const char* param, const char* describ) {
  int fd = open("/home/bitnami/logging", O_WRONLY | O_APPEND);
  char temp[50] = {'\0'};
  strcpy(temp, describ);
  strcat(temp, param);
  strcat(temp, "\n");
  write(fd, temp, 50);
  close(fd);
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
  int res = 0;
  logging(path, "getattr path = ");
  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0777;
    stbuf->st_nlink = 2;
  } else if (strcmp(path, hello_path) == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    //stbuf->st_size = strlen(hello_str);
    //int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
    //off_t size = lseek(fd, 0, SEEK_END);
    stbuf->st_size = 500 * BLOCK_SIZE;
    //filesize = ;
    //close(fd);
  } else
    res = -ENOENT;
  
  return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
       off_t offset, struct fuse_file_info *fi)
{
  (void) offset;
  (void) fi;
  logging(path, "readdir path = ");
  if (strcmp(path, "/") != 0)
    return -ENOENT;
  
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  filler(buf, hello_path + 1, NULL, 0);
  //filler(buf, "test.txt", NULL, 0);
  return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
  logging(path, "open path = ");
  if (strcmp(path, hello_path) != 0)
    return -ENOENT;
  
  //  if ((fi->flags & 3) != O_RDWR)
  //  return -EACCES;
  //int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
  //FD = fd;
  
  return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
  logging(path, "************ read path = ");
  off_t len;
  (void) fi;
  char readpath[256];
  if(strcmp(path, hello_path) != 0)
    return -ENOENT;
  int cursor = 0;
  char tempbuf[50];
  sprintf(tempbuf, "%zd", size);
  logging(tempbuf, "* read size = ");
    sprintf(tempbuf, "%zd", offset);
  logging(tempbuf, "* read offset = ");
  //for (; cursor < size; cursor++) {
    size_t realoffset = offset + cursor;
    // sprintf(tempbuf, "%d", BLOCK_SIZE);
    // logging(tempbuf, "* read BLOCK_SIZE = ");
    // int block_number = (int) (realoffset / BLOCK_SIZE);
    // printf(tempbuf, "%zd", realoffset);
    // logging(tempbuf, "* read realoffset = ");
    // sprintf(tempbuf, "%zd", size);
    // logging(tempbuf, "* read block_number = ");
    // size_t block_offset = realoffset % BLOCK_SIZE;
    sprintf(readpath, "/home/bitnami/files/%d", 2);
    logging(readpath, "** readpath = ");
    int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
    len = 4096;
    if (realoffset < len) {
      lseek(fd, realoffset, SEEK_SET); 
      if (realoffset + size > len) {
        size = len - realoffset;
      }
      read(fd, buf + realoffset, size);
      close(fd);
    } else {
      size = 0;
    }
  //}
  return size;
  // sprintf(readpath, "/home/bitnami/files/%d", 1);
  // int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
  
  // len = lseek(fd, 0, SEEK_END);
  // if (offset < len) {
  //   lseek(fd, offset, SEEK_SET);
  //   if (offset + size > len)
  //     size = len - offset;
  //   read(fd, buf + offset, size);
  //   close(fd);
  //   //memcpy(buf, hello_str + offset, size);
  // } else
  //   size = 0;
  
  // return size;
}

static int hello_write(const char *path, const char* buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  logging(path, "write path = ");
  logging(buf, "write buf = ");
  char tempbuf[50];
  sprintf(tempbuf, "%zd", size);
  logging(tempbuf, "write size = ");
  sprintf(tempbuf, "%zd", offset);
  logging(tempbuf, "write offset = ");
  size_t len;
  (void)fi;
  size_t offset_within_block = (size_t)(offset % BLOCK_SIZE);
  size_t block_number = (size_t) (offset / BLOCK_SIZE);

  len = BLOCK_SIZE;
  sprintf(tempbuf, "%zd", len);
  logging(tempbuf, "write len = ");
  // offset + size <= 4096
  // size = 4096 offset = 0
  // size = 4096 offset = 4096
  // size = 573  offset = 8192
  // offset = 0, size = 5
  

  if (offset < len) {
    if (offset + size > len) 
      size = len - offset;

    char readpath[256];
    sprintf(readpath, "/home/bitnami/files/%d", 0);
    logging(readpath, "write readpath = ");
    int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
    ssize_t writeBytes = write(fd, buf + offset, size);
    
    if (writeBytes < size) {
      if (writeBytes == -1) {
         int err = errno;
         sprintf(tempbuf, "%d", err);
         logging(tempbuf, "write write errno = ");
      } 
      else {
         sprintf(tempbuf, "%zd", writeBytes);
         logging(tempbuf, "write less bytes, num  = ");
      }
    } 
    else {
      logging("!", "small write success");
    }
    close(fd);
  } 
  else {
    int i = block_number;
    size_t offset_b = 0;
    logging("!", "large write success entry");
    int bb = (int)(size/BLOCK_SIZE);
    for (; i < bb+1; i++) {
      char readpath[256];
      sprintf(readpath, "/home/bitnami/files/%d", i);
      logging(readpath, "write_loop readpath = ");
      int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
      ssize_t writeBytes = write(fd, buf + offset_b, BLOCK_SIZE);
      offset_b = offset_b+BLOCK_SIZE;
    }
    if (offset_b - BLOCK_SIZE < size) {
      char readpath[256];
      sprintf(readpath, "/home/bitnami/files/%d", i);
      logging(readpath, "write_last_loop readpath = ");
      int fd = open(readpath, O_RDWR, S_IRUSR | S_IWUSR);
      offset_b = offset_b-BLOCK_SIZE;
      ssize_t writeBytes = write(fd, buf + offset_b, (int)(size%BLOCK_SIZE));  
      logging("!", "large write success complete");
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
  .getattr  = hello_getattr,
  .readdir  = hello_readdir,
  .open   = hello_open,
  .read         = hello_read,
  .write        = hello_write,
  .truncate = hello_truncate,
};

int main(int argc, char *argv[]) {
  return fuse_main(argc, argv, &hello_oper, NULL);
}


