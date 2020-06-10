#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int main(){
    char buffer[20];
    int parent_fd[2], child_fd[2];
    int tmp1 = pipe(parent_fd);
    int tmp2 = pipe(child_fd);
    
    if(tmp1 == -1 || tmp2 == -1){
        printf("error\n");
        exit(0);
    }

    if(fork() == 0){
        read(parent_fd[0], buffer, 20);
        printf("%d: %s\n", getpid(), buffer);
        write(child_fd[1],"received pong", 20);
        close(child_fd[1]);
        close(parent_fd[0]);
        exit(0);
    }
    else{
        write(parent_fd[1],"received ping", 20);
        read(child_fd[0], buffer, 20);
        printf("%d: %s\n", getpid(), buffer);
        wait(0);
        close(parent_fd[1]);
        close(child_fd[0]);
        exit(0);
    }

    exit(0);
}