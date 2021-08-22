#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void find(char *path, char *target)
{
    int fd;
    if((fd = open(path, O_RDONLY))<0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    struct stat st;
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if(st.type!=T_DIR){
        fprintf(2, "find: path is not a directory %s\n", path);
        close(fd);
    }

    char buf[512], *p;
    struct dirent de;
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("find: path too long\n");
        return;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p = '/';
    p++;
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;
        if(strcmp(de.name, ".")==0||strcmp(de.name, "..")==0)
            continue;
        
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf("find: cannot stat %s\n", buf);
            continue;
        }
        if(st.type==T_DIR){
            find(buf, target);
        }else if(st.type==T_FILE){
            if(strcmp(de.name, target)==0){
                printf("%s\n", buf);
            }
        }
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if(argc != 3){
        fprintf(2,"invalid para\n");
        exit(1);
    }
    
    find(argv[1], argv[2]);
    exit(0);
}