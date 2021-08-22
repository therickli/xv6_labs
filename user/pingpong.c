#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// int main(int argc, char *argv[])
// {
//     if(argc>1){
//         fprintf(2, "pingpong: no parameter needed\n");
//         exit(1);
//     }
//     char buf[2];
//     int p[2];
//     pipe(p);
//     if(fork()==0){
//         read(p[0], buf, 2);
//         printf("%d: received ping\n", getpid());
//         write(p[1], " ", 1);
//         close(p[1]);
//     } else {
//         write(p[1], " ", 1);
//         close(p[1]);
//         wait(0);
//         read(p[0], buf, 2);
//         printf("%d: received pong\n", getpid());
//     }
//     exit(0);
// }
int main(int argc, char *argv[])
{
    if(argc>1){
        fprintf(2, "pingpong: no parameter needed\n");
        exit(1);
    }
    char buf[2];
    int p1[2],p2[2];
    pipe(p1);
    pipe(p2);

    if(fork()==0){
        close(p1[1]);
        while(read(p1[0],buf,2)>0){
            printf("%d: received ping\n", getpid());
        }
        write(p2[1]," ", 1);
        close(p2[1]);
    } else {
        close(p2[1]);
        write(p1[1], " ", 1);
        close(p1[1]);
        while(read(p2[0],buf,2)>0){
            printf("%d: received pong\n", getpid());
        }
    }
    exit(0);
}