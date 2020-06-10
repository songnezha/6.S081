#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"

#define MAX_LINE 100

int read_line(int fd, char *line){
  char tmp;
  int count = 0;
  while(read(fd, &tmp, sizeof(tmp)) == sizeof(tmp) && tmp != '\n'){
    *line++ = tmp;
    count++;
  }
  *line++ = '\0';
  return count;
}

int main(int argc, char **argv){
  char line[MAX_LINE];
  memset(line, 0, sizeof(line));
  
  if(argc < 3){
    fprintf(2, "usage: xargs [command]\n");
    exit(0);
  }

  while(read_line(0, line) != 0){
    char *arg[MAXARG];
    memset(arg, 0, MAXARG);
    char **p = arg;
    for(int i=1; i<argc; i++){
      *p++ = argv[i];
    }
    *p++ = line;
    if(fork()==0){
      exec(argv[1], arg);
    }
    else wait(0);
  }

  exit(0);
}