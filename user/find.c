#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char* fmtname(char *path){
    static char buf[DIRSIZ+1];
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

void find(char* path, char* name){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    char tmp[DIRSIZ+1];

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
        case T_FILE:
            memmove(tmp, name, strlen(name));
            memset(tmp+strlen(name), ' ', DIRSIZ-strlen(name));
            if(strcmp(tmp, fmtname(path)) == 0)
                printf("%s\n", path);
            break;

        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            if((de.name[0]=='.' && strlen(de.name)==1) || (de.name[0]=='.' && de.name[1]=='.' && strlen(de.name)==2))
                continue;
            find(buf, name);
        }
        break;
    }
    close(fd);
}

int main(int argc, char **argv){
    if(argc < 3){
        fprintf(2, "Not enough args\n");
        exit();
    }

    find(argv[1], argv[2]);

    exit();
}