#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "mydata.h"


int get_index(char * path){
    //return the index of the last "/" to seperate path and file name
    int index1,flage;
    int len = strlen(path);
    flage = 0;
    for(int i =0;i<len;i++){
        if(flage == 1){break;}
        if(path[i]=='/'){
            index1 = i;
            for(int k = i;k<len;k++){
                if(path[k]=='/'){
                    index1 = k;
                    i = k;
                }
                if(k+1==len){
                    flage = 1;
                }
            }
        }
    }
    return index1;
}

struct path* get_path(char * path,int index1){
    int len = strlen(path);
    struct path* path1 = malloc(sizeof(struct path *));
    char * buff = malloc(sizeof(char)*(index1+2));
    char * filename = malloc(sizeof(char)*(len - index1));
    
    if(index1 == 0){
        buff[0] = '/';
        buff[1] = '\0';
    }else{
        strncpy(buff,path,index1);
        buff[index1] = '\0';
    }
    int len_file_name =len - index1-1;
    for(int i = 0;i<len_file_name;i++){
        filename[i] = path[index1+i+1];
    }
    filename[len_file_name] = '\0';
    
    DIR* dir = opendir(buff);
    if(dir){
        closedir(dir);
        path1->path = buff;
        path1->filename = filename;
    }else{
        path1 = NULL;
        
    }
    return path1;
}

struct ext2_dir_entry *find_entry(struct ext2_inode *dir_inode, char *file_name) {
    
    if (file_name == NULL) {
        return NULL;
    }
    
    int name_length = strlen(file_name);
    
    // Iterate over data blocks in search for the matching directory entry
    int n;
    //since lower 12 block are direct blocks
    for (n = 0; n < 12 && (dir_inode->i_block)[n] != 0; n++) {
        
        int block_num = (dir_inode->i_block)[n];
        
        unsigned char *block_start = disk + (block_num * EXT2_BLOCK_SIZE);
        unsigned char *block_end = BLOCK_END(block_start);
        
        // Current position within this block
        unsigned char *pos = block_start;
        
        while (pos != block_end) {
            struct ext2_dir_entry *entry = (struct ext2_dir_entry *) pos;
            
            if (entry->inode != 0) { // dir entry is in use
                
                // Check if length of file names match
                if (name_length == ((int) entry->name_len)) {
                    
                    if (strncmp(name, entry->name, name_length) == 0) {
                        // File names match
                        return entry;
                    }
                }
            }
            
            pos += entry->rec_len;
        }
    }
    return NULL;
}



