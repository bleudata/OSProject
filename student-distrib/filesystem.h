#ifndef PAGING_H
#define PAGING_H

#include "lib.h"

#define FILENAME_LENGTH 4

typedef struct __attribute__ ((packed)){
    int8_t filename[FILENAME_LENGTH];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[24]
} d_entry;

typedef struct __attribute__ ((packed)){
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[52];
    d_entry dir_entries[63];
}boot_block_struct;

typedef struct __attribute__ ((packed)){
    int32_t length;
    int32_t data_block_num[1023];
}inode_block_struct;

typedef union{
    boot_block_struct boot_type;
    inode_block_struct inode_type;
}file_sys_block;



//file_init -> just initialize global variables(pointers to stuff inside the filesystem)
void filesys_init(file_sys_block* fileimg_address);


//fopen (arguments are same as system call) (design choice)
//fclose
//fwrite
//fread

//d_open
//d_close
//dwrite
//dread

//read_dentry_by_name
int32_t read_dentry_by_name(const uint8_t* fname, d_entry* dentry);

//read_dentry_by_index
int32_t read_dentry_by_index(uint32_t index, d_entry* dentry);

//read_data, offset = no. of bytes offset to start reading from in the file
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


#endif