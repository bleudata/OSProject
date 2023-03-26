#include "filesystem.h"

#define BLOCK_SIZE 4096

boot_b_struct* boot_block;
inode_struct* inode_array;
data_struct* data_array;
uint32_t file_counter;

//initialize the global variables boot_block, inode_array, data_array
// if there is memory issues, it is probably because of this :)
void filesys_init(uint32_t* fileimg_address){
    boot_block = (boot_b_struct*)fileimg_address;
    inode_array = (inode_struct*)(fileimg_address) + 1;
    data_array = (data_struct*)(fileimg_address) + 1 + boot_block->inode_count;
    file_counter = 0;
}

//goes through dir_entries array in bootblock and finds the dir entry with matching name
int32_t read_dentry_by_name(const uint8_t* fname, d_entry* dentry){
    //calls read_dentry_by_index
    //null check for name and dentry
    if(fname == NULL || dentry == NULL){
        return -1;
    }

    if(strlen(fname) > 32){
        printf("string over 32 chars \n");
        return -1;
    }

    int num_dir_entries = boot_block->dir_count;
    uint32_t str_length = strlen((int8_t*)fname);
    int i;
    for(i = 0; i<num_dir_entries ; i++){
        if(strncmp((int8_t*)fname, boot_block->dir_entries[i].filename, str_length) == 0){ 
            //filename matches
            printf("found matching file: ");
            printf("%s \n", boot_block->dir_entries[i].filename);
            printf("index is: ");
            printf("%d \n", i);
            return read_dentry_by_index(i, dentry); 
        }
    }
    return -1;
}

//dentry = &(boot_block->boot_type.dir_entries[index]); WRONG!
//read_dentry_by_index
int32_t read_dentry_by_index(uint32_t index, d_entry* dentry){
    
    //index out of bounds check, dentry null check
    int num_dir_entries = boot_block->dir_count;
    if(index < 0 || index > num_dir_entries-1){ 
        return -1;
    }
    if(dentry == NULL){
        return -1;
    }
    
    //copy dentry info from bootblock to the caller's dentry struct
    memcpy(dentry, &(boot_block->dir_entries[index]), 64);
    return 0;
}

//read_data, offset = no. of bytes offset to start reading from in the file
//read data from file's data blocks and put them into the buffer
// offset: number of bytes to skip
// length: number of bytes to read
// return number of bytes read
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    
    inode_struct* cur_inode = &(inode_array[inode]);
    printf("file length: %d \n",cur_inode->length );
    int file_remainder = cur_inode->length - offset;
    unsigned int data_block_count = boot_block->data_block_count;

    int bytes_left;
    if(file_remainder < length){
        bytes_left = file_remainder;
    }else{
        bytes_left = length;
    }

    unsigned int block_index = offset / BLOCK_SIZE; //number of blocks to skip(start block index)
    unsigned int block_offset = offset % BLOCK_SIZE; //- num_blocks_skip*BLOCK_SIZE
    unsigned int block_remainder = BLOCK_SIZE - block_offset;

    unsigned int bytes_read = 0;
    int i, block_number;
    for(i = 0; i < bytes_left ; i++){
        block_number = cur_inode->data_block_num[block_index];

        if(block_number<0 || block_number >= data_block_count){ //sanity check
            return -1;
        }

        buf[i] = data_array[block_number].data[block_offset];
        bytes_read += 1;
        //buf += 1;
        block_remainder -=1;
        block_offset++;
        if(block_remainder == 0){
            block_remainder = BLOCK_SIZE;
            block_index +=1;
            block_offset = 0;
        }

    }
    return bytes_read;
}

/*
 * file_open
 *   DESCRIPTION: initializes caller's dentry struct
 *   INPUTS: filename, dentry struct ptr
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t file_open(const uint8_t* filename, d_entry * dentry){
    //d_entry * dentry;
    if(strlen(filename) > 32){
        printf("string over 32 chars \n");
        return -1;
    }
    return read_dentry_by_name(filename, dentry);
}

/*
 * file_close
 *   DESCRIPTION: does nothing
 *   INPUTS: file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success
 */
int32_t file_close(int32_t fd){
    return 0;
}


/*
 * file_read
 *   DESCRIPTION: read nybte bytes of one file into input buffer
 *   INPUTS: fd: file descriptor, buf: input buffer, nbytes: no. bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){

    if(buf == NULL){
        return -1;
    }

    return read_data(fd, 0 , buf , nbytes); //fd -> inode num only for cp2
}

/*
 * file_write
 *   DESCRIPTION: does nothing
 *   INPUTS: fd: file descriptor, buf: input buffer, nbytes: no. bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t file_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/*
 * dir_open
 *   DESCRIPTION: initializes caller's dentry struct with dir info
 *   INPUTS: filename, dentry struct ptr
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t dir_open(const uint8_t* filename, d_entry* dentry){
    return read_dentry_by_index(0, dentry);
}

int32_t dir_close(int32_t fd){
    return 0;
}

// print/read directory "." and the rest of the files
// read one filename, also keep track of which file number you are on
/*
 * dir_read
 *   DESCRIPTION: reads one filename at a time, updates file counter
 *   INPUTS: fd:file descriptor??? , buf: output buffer, nbytes - to tell the function how long the buf provided is
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
    //fd -> inode num only for cp2
    if(buf == NULL){
        return -1;
    }
    int num_dir_entries = boot_block->dir_count;

    if(file_counter >= num_dir_entries){
        return 0;
    }
    int num_bytes_read;
    if(nbytes < 32){
        num_bytes_read = nbytes;
    }else{
        num_bytes_read = 32;
    }
    //copy filename to buffer (min of nbytes and 32)
    memcpy(buf, boot_block->dir_entries[file_counter].filename, num_bytes_read);
    file_counter +=1;
    
    // if(file_counter == num_dir_entries){
    //     file_counter = 0;
    // } resetting shouldnt an automatic thing, why??
    return num_bytes_read;
}


int32_t dir_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

//inode and block number start at zero right???
uint32_t get_file_length(int32_t inode_num){
    printf("INODE NUM: %d \n", inode_num);
    return inode_array[inode_num].length;
}

//fish frame 0
// read non text 
// read largefilename.txt should not work
// read largefilename.tx should work
// dir test  . file1 file2 ..
// offset test

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
