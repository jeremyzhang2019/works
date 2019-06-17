#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"



unsigned char *disk;

int main(int argc, char ** argv){
    struct path* path1;
    int index;
    if(argc != 4){
        //check argument
        fprintf(stderr,"%s Usage:<image file name> <path name>\n",argv[0]);
        exit(-1);
    }
    
    
    
    return 0;
}
