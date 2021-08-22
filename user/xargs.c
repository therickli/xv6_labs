#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[])
{
    // printf("!!!\n");

    char *para[MAXARG];
    int paraIndex;
    if(argc>=MAXARG - 1){
        fprintf(2, "xarg: too many parameter\n");
        exit(1);
    }
    // printf("%d!!!\n", argc);
    for(paraIndex=0;paraIndex+1<argc;paraIndex++){
        para[paraIndex] = argv[paraIndex+1];
    }
    char buf[512], c;
    int idx=0;
    // printf("%s!!!\n",argv[0]);

    while(read(0, &c, 1)!=0){
        // printf("%c\n", c);
        if(c!='\n'&&c!='\0'){
            buf[idx]=c;
            idx++;
        }else{
            buf[idx]=0;
            para[paraIndex] = malloc(idx+1);
            strcpy(para[paraIndex],buf);
            para[paraIndex+1] = 0;
            if(fork()==0){
                exec(para[0], para);
                fprintf(2, "xarg: %s exec failed\n", para[0]);
            }
            wait(0);
            idx=0;
        }
        if(c=='\0'){
            break;
        }
    }
    free(para[paraIndex]);
    para[paraIndex]=0;
    exit(0);
}
