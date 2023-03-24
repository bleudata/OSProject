#ifndef PAGING_H
#define PAGING_H

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
}boot_block;

typedef struct __attribute__ ((packed)){
    int32_t length;
    int32_t data_block_num[1023];
}inode_block;

typedef union{
    boot_block boot_block_type;
    inode_block inode_type;
}file_sys_block;

#endif

//file_init -> just initialize global variables(pointers to stuff inside the filesystem)
//fopen (arguments are same as system call) (design choice)
//fclose
//fwrite
//fread

//d_open
//d_close
//dwrite
//dread

//read_dentry_by_name
//read_dentry_by_index
//read_data