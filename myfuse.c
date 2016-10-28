/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall myfuse.c `pkg-config fuse --cflags --libs` -o myfuse
*/
#define BLOCK_SIZE 4096
#define FUSE_USE_VERSION 26
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif
#include <math.h>
#include <unistd.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char* my_path = "/mybd";

char * block_path(int Block_num, char* ret) {
  char* cb = (char *)malloc(sizeof(*cb) * 50);//figure out the targe block path
  strcpy(cb, "/home/bitnami/myfuse_test/");
  char* x = (char *)malloc(sizeof(*x) * 50);
  sprintf(x, "%d", Block_num);
  strcat(cb,x);
  ret = (char*)malloc(sizeof(*ret) * 50);
  strcpy(ret, cb);
  free(x);
  free(cb);
  return ret;
}

void log_param(const char* para, const char* description) {
  int fdd = open("/home/bitnami/log.fuse", O_WRONLY | O_APPEND);
  char * str = (char *)malloc(sizeof(*str) * 50);
  strcpy(str, description);
  strcat(str, para);
  // TODO : check if the path is valid in the VDB or not
  strcat(str,"\n");
  ssize_t ret = pwrite(fdd, str, strlen(str), 0);
  if (ret == -1) {
    ret = -errno;
  }
  free(str);
  close(fdd);
}

static int my_getattr(const char *path, struct stat *stbuf) {
  int res = 0;
  log_param(path, "getattr path = ");
  /*int lstat(const char* path, struct stat* buf)
    get the file states and fill into buf
    On success, 0 returned; Otherwise, -1;
   */
  //res = lstat("/home/bitnami/log.fuse", stbuf);
  memset(stbuf,0,sizeof(struct stat));
  if(strcmp(path,"/") == 0){
    stbuf->st_mode = S_IFDIR | 0777;
    stbuf->st_nlink = 2;
  }
  else if(strcmp(path,my_path) == 0){
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = 2048000;
  }
  else{
    res = -ENOENT;
  }
  return res;
}


static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi) {
  log_param(path, "readdir path = ");
  /* DIR *dp; */
  /* struct dirent *de; */
  /* (void) offset; */
  /* (void) fi; */

  /* dp = opendir(path); */
  /* if (dp == NULL) */
  /*   return -errno; */

  /* while ((de = readdir(dp)) != NULL) { */
  /*   struct stat st; */
  /*   memset(&st, 0, sizeof(st)); */
  /*   st.st_ino = de->d_ino; */
  /*   st.st_mode = de->d_type << 12; */
  /*   if (filler(buf, de->d_name, &st, 0)) */
  /*     break; */
  /* } */

  /* closedir(dp); */
  (void) offset;
  (void) fi;
  if(strcmp(path,"/") != 0){
    return -ENOENT;
  }
  filler(buf,my_path+1,NULL,0);
  return 0;
}


static int my_open(const char *path, struct fuse_file_info *fi) {
  log_param(path, "open path = ");
  //if(strcmp(path,my_path) != 0){
  //return -ENOENT;
  //}

  // int fd = open(path, O_RDWR);
  // if((fi->flags & 3) != O_RDONLY){
  // return -EACCES;
  // }
  //return fd;

  /* int res; */
  /* res = open("/mybd", fi->flags); */
  /* if (res == -1) */
  /*   return -errno; */
  /* close(res); */

  /* if (strcmp(path, "/mybd") != 0) */
  /*   return -ENOENT; */

  return 0;
}

static int my_truncate(const char *path, off_t size) {
  return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi) {
  int fd;
  printf("Enter read\n");
  if(strcmp(path,my_path) != 0){
    return -ENOENT;
  }
  log_param(path, "read path = ");
  (void) fi;
  // | --- | --- | --- | --- | --- |
  //    0     1     2     3     4
  // offset = 5128, size = 8789
  int Block_num_sentinel = (int)ceil((double)((int)offset/(int)BLOCK_SIZE));
  int Offset_within_block = (int)offset%(int)BLOCK_SIZE;
  int cursor = 0;
  int offset_in_buf = 0;
  int Block_num;
  int cnt = 0;
  int res;
  int ans = 0;
  for (cursor = 0; cursor < (int)size; cursor++) {
    cnt++;
    Block_num = (int)ceil((double) ( ((int)offset+cursor)/(int)BLOCK_SIZE) );
    //if larger than one block write, then batch them and write
    if(Block_num == Block_num_sentinel && cnt <= BLOCK_SIZE){
      continue;
    } else {
      cnt=0;
      Block_num_sentinel++;
      Block_num--;
      printf("offset = %d, cursor = %d, Offset_within_block = %d\n", (int)offset, cursor, Offset_within_block);
      int batch_size = ((int)offset+cursor-1)%(int)BLOCK_SIZE + 1 - Offset_within_block;
      //batch_write to block
      char* blockPath = NULL;
      blockPath = block_path(Block_num, blockPath);
      printf("blockPath = %s\n", blockPath);
      fd = open(blockPath, O_RDONLY);
      if (fd == -1) {
        return -errno;
      }
      /* the file offset is not change */
      //printf("Offset_within_block = %d\n", Offset_within_block);
      printf("\033[32mbatch_size = %d\033[0m\n", batch_size);
      printf("Offset_within_block = %d\n", Offset_within_block);
      //printf("offset_in_buf = %d\n", offset_in_buf);
      //  printf("After realloc, before pread\n");
      res = pread(fd, buf+offset_in_buf, (size_t)batch_size, (off_t)Offset_within_block);
      ans += res;
      //printf("%s\n",buf);
      offset_in_buf += batch_size;
      if (res == -1) {
        res = -errno;
      } else {
        printf("pread succeeded.\n");
      }
      close(fd);
      Offset_within_block = ((int)offset+cursor)%(int)BLOCK_SIZE;//update new start of another block
      Block_num = (int)ceil((double) ( ((int)offset+cursor)/(int)BLOCK_SIZE) );
      cnt = 0;
    }
  }
  if(Block_num == Block_num_sentinel) { //deal with last block write
    int batch_size = ((int)offset+cursor-1)%(int)BLOCK_SIZE + 1 - Offset_within_block;
    //batch_write to block
    char * blockPath = NULL;
    blockPath = block_path(Block_num, blockPath);
    printf("blockPath = %s\n", blockPath);
    fd = open(blockPath, O_RDONLY);
    if (fd == -1) {
      return -errno;
    }
    /* write up to size bytes from buffer starting at buf to the file descriptor fd at offset */
    /* the file offset is not change */
    printf("offset = %d, cursor = %d\n", (int)offset, cursor);
    //    printf("offset_in_buf = %d\n", offset_in_buf);
    printf("\033[32mbatch_size = %d\033[0m\n",batch_size);
    res = pread(fd, buf+offset_in_buf, (size_t)batch_size, (off_t)Offset_within_block);
    if (res == -1){
      res = -errno;
    } else {
      printf("pread succeeded.\n");
    }
    ans += res;
    close(fd);
  }
  return ans;
}

static int my_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi) {
  char tempbuf[50];
  log_param(path, "write path = ");
  sprintf(tempbuf, "%zd", size);
  log_param(tempbuf, "my_write size = ");
  sprintf(tempbuf, "%zd", offset);
  log_param(tempbuf, "my_write offset = ");
  log_param("!", "Enter write");
  if(strcmp(path,my_path) != 0){
   return -ENOENT;
  }
  int fd;
  int res;
  int ans = 0;
  (void) fi;
  // | --- | --- | --- | --- | --- |
  //    0     1     2     3     4
  // offset = 5128, size = 8789
  int Block_num_sentinel = (int)ceil((double)((int)offset/(int)BLOCK_SIZE));
  int Offset_within_block = (int)offset%(int)BLOCK_SIZE;
  int cursor = 0;
  int offset_in_buf = 0;
  int Block_num;
  int cnt = 0;
  for (cursor = 0; cursor < (int)size; cursor++) {
    cnt++;
    Block_num = (int)ceil((double) ( ((int)offset+cursor)/(int)BLOCK_SIZE) );
    //if larger than one block write, then batch them and write
    if(Block_num == Block_num_sentinel && cnt <= BLOCK_SIZE){
      continue;
    } else {
      cnt=0;
      Block_num_sentinel++;
      Block_num--;
      printf("offset = %d, cursor = %d, Offset_within_block = %d\n", (int)offset, cursor, Offset_within_block);
      int batch_size = ((int)offset+cursor-1)%(int)BLOCK_SIZE + 1 - Offset_within_block;
      //batch_write to block
      char* blockPath = NULL;
      blockPath = block_path(Block_num, blockPath);
      log_param(blockPath,"mywrite blockPath = ");
      printf("blockPath = %s\n", blockPath);
      fd = open(blockPath, O_WRONLY);
      if (fd == -1) {
	sprintf(tempbuf, "%d", errno);
	log_param(tempbuf, "mywrite open file errno = ");
        return -errno;
      }
      /* the file offset is not change */
      //printf("Offset_within_block = %d\n", Offset_within_block);
      printf("\033[32mbatch_size = %d\033[0m\n", batch_size);
      printf("Offset_within_block = %d\n", Offset_within_block);
      printf("offset_in_buf = %d\n", offset_in_buf);
      res = pwrite(fd, buf + offset_in_buf, (size_t)batch_size, (off_t)Offset_within_block);
      offset_in_buf += batch_size;
      if (res == -1) {
	sprintf(tempbuf, "%d", errno);
	log_param(tempbuf, "mywrite pwrite write file errno = ");
        res = -errno;
      } else {
        printf("pwrite succeeded.\n");
      }
      ans += res;
      close(fd);
      Offset_within_block = ((int)offset+cursor)%(int)BLOCK_SIZE;//update new start of another block
      Block_num = (int)ceil((double) ( ((int)offset+cursor)/(int)BLOCK_SIZE) );
      cnt = 0;
    }
  }
  if(Block_num == Block_num_sentinel) { //deal with last block write
    int batch_size = ((int)offset+cursor-1)%(int)BLOCK_SIZE + 1 - Offset_within_block;
    //batch_write to block
    char * blockPath = NULL;
    blockPath = block_path(Block_num, blockPath);
    log_param(blockPath, "mywrite last block path = ");
    fd = open(blockPath, O_WRONLY);
    if (fd == -1) {
      sprintf(tempbuf, "%d", errno);
      log_param(tempbuf, "mywrite pwrite open last block file errno = ");
      return -errno;
    }
    /* write up to size bytes from buffer starting at buf to the file descriptor fd at offset */
    /* the file offset is not change */
    printf("offset = %d, cursor = %d\n", (int)offset, cursor);
    printf("offset_in_buf = %d\n", offset_in_buf);
    printf("\033[32mbatch_size = %d\033[0m\n",batch_size);
    res = pwrite(fd, buf + offset_in_buf, (size_t)batch_size, (off_t)Offset_within_block);
    if (res == -1){
      sprintf(tempbuf, "%d", errno);
      log_param(tempbuf, "mywrite pwrite write last block file errno = ");
      res = -errno;
    } else {
      printf("pwrite succeeded.\n");
    }
    ans += res;
    close(fd);
  }
  return ans;
}

static struct fuse_operations xmp_oper = {
  .getattr	= my_getattr,
  .readdir	= my_readdir,
  .truncate     = my_truncate,
  .open		= my_open,
  .read		= my_read,
  .write	= my_write,
};

int main(int argc, char *argv[]) {
  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);
}

