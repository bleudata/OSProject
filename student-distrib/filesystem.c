#include "filesystem.h"

#define BLOCK_SIZE 4096

// pointers to locations inside the filesystem
boot_b_struct* boot_block;
inode_struct* inode_array;
data_struct* data_array;
uint32_t file_counter;

//dentry struct variable, for cp2
d_entry cp2_dentry;

// if there are memory issues, might be because of this 
/*
 * filesys_init()
 *   DESCRIPTION: initialize the global variables boot_block, inode_array, data_array
 *   INPUTS: fileimg_address: address of start of filesystem
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void filesys_init(uint32_t* fileimg_address){
    boot_block = (boot_b_struct*)fileimg_address;
    inode_array = (inode_struct*)(fileimg_address) + 1;
    data_array = (data_struct*)(fileimg_address) + 1 + boot_block->inode_count;
    file_counter = 0;
}

//goes through dir_entries array in bootblock and finds the dir entry with matching name
/*
 * read_dentry_by_name() - helper function
 *   DESCRIPTION: goes through dir_entries array in bootblock and finds the dentry with matching name
 *                and copies over info to caller's dentry struct
 *                calls read_dentry_by_index helper function
 *   INPUTS: fname: filename of target dentry, dentry: address of caller's dentry struct
 *   OUTPUTS: fills up caller's dentry struct if matching filename is found
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t read_dentry_by_name(const uint8_t* fname, d_entry* dentry){
    
    //null check for name and dentry
    if(fname == NULL || dentry == NULL){
        return -1;
    }
    // invalid filename, over 32 chars
    if(strlen((int8_t*)fname) > MAX_FILE_LENGTH){
        //printf("string over 32 chars \n");
        return -1;
    }

    int num_dir_entries = boot_block->dir_count;
    // uint32_t str_length = strlen((int8_t*)fname);
    int i; //loop over dir_entries array and find dentry with matching filename's index, call read_dentry_by_index
    for(i = 0; i<num_dir_entries ; i++){
        if(strncmp((int8_t*)fname, boot_block->dir_entries[i].filename, strlen(boot_block->dir_entries[i].filename)) == 0){ 
            //filename matches

            // printf("found matching file: ");
            // printf("%s \n", boot_block->dir_entries[i].filename);
            // printf("index is: ");
            // printf("%d \n", i);
            return read_dentry_by_index(i, dentry); 
        }
    }
    return -1;
}

//dentry = &(boot_block->boot_type.dir_entries[index]); WRONG!
/*
 * read_dentry_by_index() - helper function
 *   DESCRIPTION: copies over dentry information at specified index over to caller's dentry struct
 *   INPUTS: index: index of dentry in dir_entries array, dentry: caller's dentry struct address
 *   OUTPUTS: fills up caller's dentry struct
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t read_dentry_by_index(uint32_t index, d_entry* dentry){
    //printf("%d\n", index);
    //index out of bounds check, dentry null check
    int num_dir_entries = boot_block->dir_count;
    //printf("%d\n", num_dir_entries);
    if(index < 0 || index > num_dir_entries-1){ 
        return -1;
    }
    //printf("%d\n", dentry);
    if(dentry == NULL){
        return -1;
    }
    
    //copy dentry info from bootblock to the caller's dentry struct
    memcpy(dentry, &(boot_block->dir_entries[index]), DENTRY_SIZE_BYTES); //64
    return 0;
}

//inode num and datablocknum start from 0 and are indexes to their arrays
/*
 * read_data() - helper function
 *   DESCRIPTION: reads data in file's data blocks, write into caller's buffer
 *   INPUTS: inode: index of inode, offset: num bytes to skip, length: num bytes to read, buf: buffer to store info to
 *   OUTPUTS: fills up buffer with info read from file
 *   RETURN VALUE: number of bytes read
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    
    inode_struct* cur_inode = &(inode_array[inode]); //ptr to inode of file to read from
    //printf("file length: %d \n",cur_inode->length );
    int file_remainder = cur_inode->length - offset; //remaining bytes in file to read
    unsigned int data_block_count = boot_block->data_block_count; //num of data blocks for sanity check

    int bytes_left; //determine actual number of bytes left
    if(file_remainder < length){
        bytes_left = file_remainder;
    }else{
        bytes_left = length;
    }

    unsigned int block_index = offset / BLOCK_SIZE; //number of blocks to skip(start block index)
    unsigned int block_offset = offset % BLOCK_SIZE; // same as "- num_blocks_skip*BLOCK_SIZE", which byte to start reading from 0 indexed
    unsigned int block_remainder = BLOCK_SIZE - block_offset; //remaining bytes to read in block

    unsigned int bytes_read = 0;
    int i, block_number; // loop through bytes left
    for(i = 0; i < bytes_left ; i++){
        block_number = cur_inode->data_block_num[block_index]; //use block index to get correct block number

        if(block_number<0 || block_number >= data_block_count){ //sanity check
            return -1;
        }

        buf[i] = data_array[block_number].data[block_offset]; //copy 1 byte of data to buffer
        bytes_read += 1;
        //buf += 1;
        block_remainder -=1;
        block_offset++;
        if(block_remainder == 0){ //end of block, move to new block
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
int32_t file_open(const uint8_t* filename){
    //d_entry * dentry;
    // filename length check
    if(strlen((int8_t*)filename) > MAX_FILE_LENGTH || filename == NULL){
        //printf("string over 32 chars \n");
        return -1;
    }
    d_entry dentry;
    int32_t dentry_success = read_dentry_by_name(filename, &dentry);

    if(dentry_success == -1){
        return -1;
    }
    
    return 0;
}

/*
 * file_close
 *   DESCRIPTION: does nothing
 *   INPUTS: file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success -1:fail
 */
int32_t file_close(int32_t fd){
    terminal_write(1, "in fil \n", 9);
    if(fd<2 || fd >7){
        return -1;
    }
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

    //sanity check
    if(buf == NULL){
        return -1;
    }
    if(fd<0 || fd >7){
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    uint32_t * pcb_address = (uint32_t*)(cur_esp & 0xFFFFE000); 

    int32_t inode_num = ((pcb_t*)pcb_address)->fd_array[fd].inode_num;

    return read_data(inode_num, ZERO_OFFSET , buf , nbytes); //fd -> inode num only for cp2
}

/*
 * file_write
 *   DESCRIPTION: does nothing
 *   INPUTS: fd: file descriptor, buf: input buffer, nbytes: no. bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    if(fd<0 || fd >7){
        return -1;
    }
    if(buf == NULL){
        return -1;
    }
    return -1;
}

/*
 * dir_open
 *   DESCRIPTION: initializes caller's dentry struct with "." dir info
 *   INPUTS: filename, dentry struct ptr
 *   OUTPUTS: none
 *   RETURN VALUE: fd of dentry/file on sucess, -1:fail
 */
int32_t dir_open(const uint8_t* filename){
    //printf("in dir open\n");
    if(filename == NULL){
        return -1;
    }

    d_entry dentry;
    int32_t dentry_success = read_dentry_by_name(filename, &dentry);
    if(dentry_success == -1){
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & 0xFFFFE000);

    int32_t inode_num = dentry.inode_num;
    int32_t fd = 2;
    
    while(fd < 8){
        if(inode_num == ((pcb_t*)pcb_address)->fd_array[fd].inode_num){
            //printf("%d\n", fd);
            return fd;
        }
        fd++;
    }
    //if it reaches here it failed
    return -1;
    
}

/*
 * dir_close
 *   DESCRIPTION: does nothing
 *   INPUTS: fd: file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t dir_close(int32_t fd){
    terminal_write(1, "in dir \n", 9);
    if(fd<0 || fd >7){
        return -1;
    }
    return 0;
}

// print/read directory "." and the rest of the files
// read one filename, also keep track of which file number you are on
/*
 * dir_read
 *   DESCRIPTION: reads one filename at a time into buf, updates file counter
 *   INPUTS: fd:file descriptor , buf: output buffer, nbytes - to tell the function how long the buf provided is
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
    //printf("in dir read\n");
    //fd -> inode num only for cp2, not used in cp2
    if(buf == NULL){ //sanity check
        return -1;
    }
    if(fd<0 || fd >7){
        return -1;
    }
    int num_dir_entries = boot_block->dir_count;

    if(file_counter >= num_dir_entries){ //dont read non_existing file
        return 0;
    }
    int num_bytes_read;
    if(nbytes < MAX_FILE_LENGTH){
        num_bytes_read = nbytes;
    }else{
        num_bytes_read = MAX_FILE_LENGTH;
    }
    //copy filename to buffer (min of nbytes and 32)
    memcpy(buf, boot_block->dir_entries[file_counter].filename, num_bytes_read);
    file_counter +=1;
    
    // if(file_counter == num_dir_entries){
    //     file_counter = 0;
    // } resetting shouldnt an automatic thing, why??
    return num_bytes_read;
}

/*
 * dir_write
 *   DESCRIPTION: does nothing
 *   INPUTS: fd:file descriptor, buf: buffer, nbytes: num bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: 0:success, -1:fail
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes){
    if(fd<0 || fd >7){
        return -1;
    }
    if(buf == NULL){
        return -1;
    }
    return -1;
}

/*
 * get_file_length()
 *   DESCRIPTION: gets file length of file
 *   INPUTS: inode num of file
 *   OUTPUTS: none
 *   RETURN VALUE: length of file in bytes
 */
uint32_t get_file_length(int32_t inode_num){
    //printf("INODE NUM: %d \n", inode_num);
    return inode_array[inode_num].length;
}

/*
 * get_cp2_dentry_address
 *   DESCRIPTION: return address of cp2_dentry
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: dentry address
 */
d_entry * get_cp2_dentry_address(){
    return &cp2_dentry;
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


