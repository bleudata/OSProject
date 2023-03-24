#include "filesystem.h"

file_sys_block* boot_block;
file_sys_block* inode_array;
file_sys_block* data_array;

//initialize the global variables boot_block, inode_array, data_array
void file_init(file_sys_block* fileimg_address){
    boot_block = fileimg_address;
    inode_array = fileimg_address + 1;
    data_array = fileimg_address + 1 + boot_block->boot_type.inode_count;
}

//goes through dir_entries array in bootblock and finds the dir entry with matching name
int32_t read_dentry_by_name(const uint8_t* fname, d_entry* dentry){
    //calls read_dentry_by_index
    int i;
    for(i = 0; i<63 ; i++){
        if(strncmp(fname, boot_block->boot_type.dir_entries[i].filename, 4) == 0){ 
            //filename matches
            read_dentry_by_index(i, dentry); 
            break;
        }
    }
}

//read_dentry_by_index
int32_t read_dentry_by_index(uint32_t index, d_entry* dentry){
    dentry = &(boot_block->boot_type.dir_entries[index]);
}

//read_data, offset = no. of bytes offset to start reading from in the file
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){

}