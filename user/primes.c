#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void prime(int fd)
{
    char c, pN;
    if(read(fd,&pN, 1)==0){
        return;
    }
    printf("prime %d\n", pN);

    int p[2];
    pipe(p);
    if(fork()==0){
        close(p[1]);
        prime(p[0]);
        close(p[0]);
        exit(0);
    }
    close(p[0]);
    while(read(fd,&c, 1)!=0){
        if(c%pN!=0){
            write(p[1], &c, 1);
        }
    }
    close(p[1]);
    // printf("test1 %d\n", pN);
    wait(0); //!!! important
    // printf("test2 %d\n", pN);
}

int main(int argc, char *argv[]){
    int p[2];
    pipe(p);
    char c;
    if(fork()==0){
        close(p[1]);
        prime(p[0]);
        close(p[0]);
        exit(0);
    }
    close(p[0]);
    for(int i=2;i<=35;i++){
        c = i;
        write(p[1], &c, 1);
    }
    close(p[1]);
    // printf("test3 \n");
    wait(0);
    // printf("test4 \n");
    exit(0);
}

// test read and write
// int main(int argc, char *argv[]){
//     int p[2];
//     pipe(p);
//     char c;
//     c = 1;
//     write(p[1], &c, 1);
//     c = 2;
//     write(p[1], &c, 1);
//     read(p[0], &c, 1);
//     printf("%d!\n",c);
//     if(read(p[0], &c, 1)==0){
//         printf("error\n");
//     }else{
//         printf("%d!\n",c);
//     }
//     read(p[0], &c, 1);
//     wait(0);
//     exit(0);
// }