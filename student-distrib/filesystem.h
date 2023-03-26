#ifndef FILESYS
#define FILESYS

#include "lib.h"

#define FILENAME_LENGTH 32

//64 bytes size
typedef struct __attribute__ ((packed)){
    int8_t filename[FILENAME_LENGTH]; 
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[24];
} d_entry;

//4KB size
typedef struct __attribute__ ((packed)){
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_block_count;
    int8_t reserved[52];
    d_entry dir_entries[63];
}boot_b_struct;

//4KB size
typedef struct __attribute__ ((packed)){
    int32_t length;
    int32_t data_block_num[1023];
}inode_struct;

//4KB size
typedef struct __attribute__ ((packed)){
    uint8_t data[4096];
}data_struct;

//4KB size
// typedef union{
//     boot_block_struct boot_type;
//     inode_block_struct inode_type;
//     uint8_t data[4096];
// }file_sys_block;



//file_init -> just initialize global variables(pointers to stuff inside the filesystem)
extern void filesys_init(uint32_t* fileimg_address);


//fopen (arguments are same as system call) (design choice)
int32_t file_open(const uint8_t* filename,  d_entry* dentry);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, void* buf, int32_t nbytes);

int32_t dir_open(const uint8_t* filename, d_entry* dentry);
int32_t dir_close(int32_t fd);
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, void* buf, int32_t nbytes);

//read_dentry_by_name
int32_t read_dentry_by_name(const uint8_t* fname, d_entry* dentry);

//read_dentry_by_index
int32_t read_dentry_by_index(uint32_t index, d_entry* dentry);

//read_data, offset = no. of bytes offset to start reading from in the file
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

uint32_t get_file_length(int32_t inode_num);

#endif
