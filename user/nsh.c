#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_LINE 1024

typedef enum {EXEC, REDIR, PIPE} cmd_t;
char buffer[MAX_LINE];

struct cmd{
    cmd_t type;
};

struct execcmd{
    cmd_t type;
    char *uargv[10];
}execcmd[MAX_LINE];

struct redircmd{
    cmd_t type;
    struct cmd *cmd;
    char *filename;
    int fd;
}redircmd[MAX_LINE];

struct pipecmd{
    cmd_t type;
    struct cmd *left;
    struct cmd *right;
}pipecmd[MAX_LINE];

int iexeccmd = 0, iredircmd = 0, ipipecmd = 0;

int handle(char **, char*, struct cmd **);
char token(char **, char *, char **, char **);
void runcmd(struct cmd *);

int main(int argc, char **argv){

    while(1){
        struct cmd *cmd;
        write(1, "@ ", 2);
        gets(buffer, MAX_LINE);

        if(buffer[0] == 0) exit(0);
        if(buffer[0] == '\n') continue;

        char *start = buffer;
        char *end = start + strlen(buffer);
        if(!handle(&start, end, &cmd)) continue;

        if(fork()) wait(0);
        else runcmd(cmd);
    }

    exit(0);
}

int handle(char **start, char *end, struct cmd **cmd){
    char *tstart, *tend;
    int i=0;

    struct execcmd *ecmd = &execcmd[iexeccmd++]; //NO MALLOC
    ecmd->type = EXEC;
    *cmd = (struct cmd *) ecmd;

    while(*start < end){
        char t = token(start, end, &tstart, &tend);

        if(t == '*') ecmd->uargv[i++] = tstart;
        else if(t == '<' || t == '>'){
            struct redircmd * rcmd;
            rcmd = &redircmd[iredircmd++];
            rcmd->cmd = *cmd;
            rcmd->type = REDIR;
            rcmd->fd = (t == '>' ? 1 : 0);

            t = token(start, end, &tstart, &tend);

            if(t != '*'){
                fprintf(2, "missing file name\n");
                return 0;
            }
            else rcmd->filename = tstart;

            *cmd = (struct cmd *) rcmd;
        }
        else if(t == '|'){
            struct pipecmd *pcmd = &pipecmd[ipipecmd++];
            pcmd->type = PIPE;
            pcmd->left = *cmd;

            struct cmd *rcmd;

            if(!handle(start, end, &rcmd)){
                fprintf(2, "wrong pipe command\n");
                return 0;
            }
            else{
                pcmd->right = rcmd;
                *cmd = (struct cmd*) pcmd;
            }
            break;
        }
        else{
            fprintf(2, "error\n");
            return 0;
        }

    }
    return 1;
}

char token(char **start, char *end, char **tstart, char **tend){
    char *s = *start;
    char t;

    if(*start == end) return 'x';

    while(s < end && *s == ' ') s++;

    *tstart = s;

    switch(*s){
        case '<':
        case '>':
            t = *s;
            s++;
            break;
        case '|':
            t = *s;
            s++;
            break;
        default:
            t = '*';
            while(s < end && *s != ' ' && *s != '\n') s++;
            *s = '\0';
            *tend = s;
            s++;
            break;
    }

    while(s < end && *s == ' ') s++;
    *start = s;

    return t;
}

void runcmd(struct cmd *cmd){
    struct execcmd *ecmd;
    struct redircmd *rcmd;
    struct pipecmd *pcmd;

    switch(cmd->type){
        case EXEC:
            ecmd = (struct execcmd*)cmd;
            exec(ecmd->uargv[0], ecmd->uargv);
            fprintf(2, "exec %s failed\n", ecmd->uargv[0]);
            break;

        case REDIR:
            rcmd = (struct redircmd *) cmd;
            close(rcmd->fd);
            open(rcmd->filename, O_CREATE | O_RDWR);
            runcmd(rcmd->cmd);
            break;

        case PIPE:
            pcmd = (struct pipecmd *) cmd;
            int fd[2];
            pipe(fd);
            if (fork() == 0) {
            close(1);
            dup(fd[1]);
            close(fd[0]);
            close(fd[1]);
            runcmd(pcmd->left);
            }
            if (fork() == 0) {
            close(0);
            dup(fd[0]);
            close(fd[0]);
            close(fd[1]);
            runcmd(pcmd->right);
            }
            close(fd[0]);
            close(fd[1]);
            wait(0);
            wait(0);
            break;
        
        default:
            fprintf(2, "unknown cmd type\n");
            break;
    }
    exit(0);
}
