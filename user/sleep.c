#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc<=1){
        fprintf(2, "sleep: you must pass at lease one parameter\n");
        exit(1);
    }
    for(int i=1;i<argc;i++){
        int time = atoi(argv[i]);
        sleep(time);
    }
    exit(0);
}