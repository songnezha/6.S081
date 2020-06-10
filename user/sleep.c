#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int main(int argc, char **argv){
    int time;

    if(argc!=2){
        printf("Need one argument\n");
        exit(0);
    }
    time = atoi(argv[1]);
    
    sleep(time*10); //second

    printf("%d\n", uptime());

    exit(0);
}