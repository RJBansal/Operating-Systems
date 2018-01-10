#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "restart.h"
#include "fileio.h"

#if 1
#define VERBOSE(p) (p)
#else
#define VERBOSE(p) (0)
#endif

int file_read(char *path, int offset, void *buffer, size_t bufbytes)
{
  if(path == NULL || buffer == NULL || bufbytes < 1 || offset < 0){
    return IOERR_INVALID_ARGS;
  }
  int file = open(path, O_RDONLY);
  if(file == -1){
    return IOERR_INVALID_PATH;
  }
  int seek = lseek(file,offset,SEEK_CUR);
  if(seek == -1){
    return IOERR_POSIX;
  }
  int r = read(file, buffer, bufbytes);
  if(r == -1){
    return IOERR_POSIX;
  }
  return r;
}

int file_info(char *path, void *buffer, size_t bufbytes)
{
    if (!path || !buffer || bufbytes < 1) {
	   return IOERR_INVALID_ARGS;
    }
    struct stat info;

    if(stat(path, &info) == -1){
      return IOERR_POSIX;
    }
    int sz = info.st_size;
    long accessed = info.st_atime;
    long modified = info.st_mtime;
    char s;
    switch(info.st_mode & S_IFMT){
      case S_IFREG: s = 'f'; break;
      case S_IFDIR: s = 'd'; break;
    }

    sprintf(buffer, "Size:%d Accessed:%ld Modified:%ld Type %c", sz, accessed, modified, s);

    return 0;
}

int file_write(char *path, int offset, void *buffer, size_t bufbytes)
{
    int read;
    if (path == NULL || buffer == NULL || bufbytes < 1 || offset < 0){
      return IOERR_INVALID_ARGS;
    }

    int file = open(path, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR);
    if (file == -1){
      return IOERR_INVALID_PATH;
    }
    int seek = lseek(file, offset, SEEK_CUR);
    if (seek == -1){
      return IOERR_POSIX;
    }

    read = r_write(file,buffer,bufbytes);
    close(file);

    return read;
}

int file_create(char *path, char *pattern, int repeatcount)
{
  if(path == NULL || pattern == NULL) {
    return IOERR_INVALID_ARGS;
  }

  if(repeatcount <= 0) {
    return IOERR_INVALID_ARGS;
  }
  FILE *f = fopen(path, "w");

  if(f == NULL) {
    return IOERR_INVALID_PATH;
  }
  for(int i = 0; i < repeatcount; i++) {
    fwrite(pattern, sizeof(char), strlen(pattern), f);
  }
  fclose(file);
  return 0;
}

int file_remove(char *path)
{
    if(path == NULL){
      return IOERR_INVALID_ARGS;
    }
    int rem = remove(path);
    if(rem == -1){
      return IOERR_INVALID_PATH;
    }
    return rem;
}

int dir_create(char *path)
{
  if(path == NULL){
    return IOERR_INVALID_ARGS;
  }
  int make = mkdir(path, S_IRUSR||S_IWUSR||S_IXUSR);
  if(make == -1){
    return IOERR_INVALID_PATH;
  }

  return 0;
}

int dir_list(char *path, void *buffer, size_t bufbytes)
{
    if (path == NULL || buffer == NULL || bufbytes < 1 ){
      return IOERR_INVALID_ARGS;
    }

    DIR *dir= opendir(path);

    if (dir == NULL){
      return IOERR_INVALID_PATH;
    }
    if (sizeof(buffer) > bufbytes){
      return IOERR_BUFFER_TOO_SMALL;
    }

    struct dirent *dstr;
    while ((dstr = readdir (dir)) != NULL){
      sprintf(buffer+strlen(buffer), "%s\n", dstr->d_name);
    }
    closedir (dir);
    return 0;

}


int file_checksum(char *path)
{
  if(path == NULL){
    return IOERR_INVALID_ARGS;
  }
  char *buffer[11];
  int check = file_read(path, 0, buffer, 11);
  if(check == -1){
    return IOERR_POSIX;
  }
  return checksum(buffer, strlen(buffer), 0);

}


