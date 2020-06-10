#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void mydup(int n, int* fd){
    close(n);
    dup(fd[n]);
    close(fd[0]);
    close(fd[1]);
}

void check(int n){
    int num;
    while(read(0, &num, sizeof(num))){
        if(num % n != 0)
            write(1, &num, sizeof(num));
    }
}

void recurse(){
    int fd[2];
    int num;
    if(read(0, &num, sizeof(num))){
        printf("prime %d\n", num);
        if(pipe(fd) == -1){
            printf("error\n");
            exit(0);
        }
        if(fork()){
            mydup(0,fd);
            recurse();
            exit(0);
        }else{
            mydup(1, fd);
            check(num);
            exit(0);
        }
    }
}

int main(){
    int fd[2];
    if(pipe(fd) == -1){
        printf("error\n");
        exit(0);
    }

    if(fork()){
        mydup(0, fd);
        recurse();
        exit(0);
    }else{
        mydup(1, fd);
        for(int i=2; i<=35; i++)
            write(1, &i, sizeof(i));
        wait(0);
    }
    exit(0);
}