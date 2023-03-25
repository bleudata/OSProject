#include "filesystem.h"

#define BLOCK_SIZE 4096

file_sys_block* boot_block;
file_sys_block* inode_array;
file_sys_block* data_array;

//initialize the global variables boot_block, inode_array, data_array
// if there is memory issues, it is probably because of this :)
void file_init(uint32_t* fileimg_address){
    boot_block = (file_sys_block*)fileimg_address;
    inode_array = (file_sys_block*)(fileimg_address) + 1;
    data_array = (file_sys_block*)(fileimg_address) + 1 + boot_block->boot_type.inode_count;
}

//goes through dir_entries array in bootblock and finds the dir entry with matching name
int32_t read_dentry_by_name(const uint8_t* fname, d_entry* dentry){
    //calls read_dentry_by_index
    //null check for name and dentry
    if(fname == NULL || dentry == NULL){
        return -1;
    }

    int num_dir_entries = boot_block->boot_type.dir_count;
    uint32_t str_length = strlen(fname);
    int i;
    for(i = 0; i<num_dir_entries ; i++){
        if(strncmp(fname, boot_block->boot_type.dir_entries[i].filename, str_length) == 0){ 
            //filename matches
            return read_dentry_by_index(i, dentry); 
            break;
        }
    }
    return 0;
}

//dentry = &(boot_block->boot_type.dir_entries[index]); WRONG!
//read_dentry_by_index
int32_t read_dentry_by_index(uint32_t index, d_entry* dentry){
    
    //index out of bounds check, dentry null check
    if(index < 0 || index > 62){
        return -1;
    }
    if(dentry == NULL){
        return -1;
    }
    
    //copy dentry info from bootblock to the caller's dentry struct
    int str_length = strlen(boot_block->boot_type.dir_entries[index].filename);
    memcpy(dentry, &(boot_block->boot_type.dir_entries[index]), str_length);

    return 0;
}

//read_data, offset = no. of bytes offset to start reading from in the file
//read data from file's data blocks and put them into the buffer
// offset: number of bytes to skip
// length: number of bytes to read
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    /*
    inode_block_struct cur_inode = inode_array[inode];
    unsigned int num_blocks_skip = offset / BLOCK_SIZE;
    unsigned int start_block_offset = offset % BLOCK_SIZE; //- num_blocks_skip*BLOCK_SIZE
    unsigned int start_block_remainder = BLOCK_SIZE - start_block_offset;

    int file_remainder = cur_inode.length - offset;
    int bytes_left;
    if(file_remainder < length){
        bytes_left = file_remainder;
    }else{
        bytes_left = length;
    }
    
    //first block, including first and last edge case

    //middle blocks

    //last block 
    int num_blocks = (cur_inode.length/BLOCK_SIZE) + ;
    for(i = num_blocks_skip; i < num_blocks; i++ ){
        //read from each block

    }
    */
    inode_block_struct cur_inode = inode_array[inode];
    int file_remainder = cur_inode.length - offset;

    int bytes_left;
    if(file_remainder < length){
        bytes_left = file_remainder;
    }else{
        bytes_left = length;
    }

    unsigned int block_index = offset / BLOCK_SIZE; //number of blocks to skip(start block index)
    unsigned int block_offset = offset % BLOCK_SIZE; //- num_blocks_skip*BLOCK_SIZE
    unsigned int block_remainder = BLOCK_SIZE - start_block_offset;

    int i;
    for(i = 0; i < bytes_left ; i++){
        
        memcpy(buf, data_array[block_index].data + block_offset, 1);
        buf += 1;
        block_remainder -=1;
        if(block_remainder == 0){
            block_remainder = BLOCK_SIZE;
            block_index +=1;
        }

    }

}

int32_t f_open(const uint8_t* filename);
int32_t f_close(int32_t fd){
    return 0;
}
int32_t f_read(int32_t fd, void* buf, int32_t nbytes);
int32_t f_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t d_open(const uint8_t* filename);
int32_t d_close(int32_t fd){
    return 0;
}
int32_t d_read(int32_t fd, void* buf, int32_t nbytes);
int32_t d_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}