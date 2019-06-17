#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include "ext2.h"
#include "mydata.h"
#ifndef ENOENT
#define ENOENT -1
#endif
#ifndef EEXIST
#define EEXIST -1
#endif

unsigned char *disk;
extern struct ext2_super_block *sb;
extern struct ext2_group_desc *gd;
extern struct ext2_inode * id;
extern char *bm;
extern char *im;





int main(int argc, char ** argv){
    struct path* path1;
    int index;
    if(argc != 3){
        //check argument
        fprintf(stderr,"%s Usage:<image file name> <path name>\n",argv[0]);
        exit(-1);
    }
   
    if(strcmp(argv[2],"/")==0){
       printf("Root directory already exits\n"); 
       return EEXIST;       
    }  




    int fd = open(argv[1],O_RDWR);
        //check validation of the image file
    if(fd == -1){
        perror("Error: Fail To Open The Path\n");
        exit(-1);
    }
    
    disk = mmap(NULL,128*1024, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    if(disk == MAP_FAILED){
        perror("Error: Fail To MMAP\n");
        exit(-1);
    }
    //initialize pointers
    sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE); //sb start from 1
    gd = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE*2);//gd start from 2
    id = (struct ext2_inode *)(gd->bg_inode_table);//inode table from gd
    bm = disk + gd->bg_block_bitmap;//block bitmap
    im = disk + gd->bg_inode_bitmap;//inode bitmap
    




    
    //check validation of the file path
    //helper functions : get_index-> return index of the last "/"
    //                   get_path-> return struct path which contain
    //                               full path and file name
    //                               return NULL if the path is invalid
    index = get_index(argv[2]);
    path1 = get_path(argv[2],index);
    
      




    if(path1 == NULL){
        fprintf(stderr,"Invalid Address\n");
        return ENOENT;}
    
//    }else{
//        //since the component on the path is valid then check the exitence of the file
//        DIR* dir = opendir(argv[2]);
//        if(dir){
//            //the directory already exsited then return EEXIST
//            closedir(dir);
//            return EEXIST;
//        }
//    }

//since it passed the validation evaluation then initialize the inode in the inode table and update its parent's imformations 

//initialize the entry









    
    return 0;
}
