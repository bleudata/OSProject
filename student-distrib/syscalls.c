
#include "syscalls.h"

/* Index into function operation pointers */
#define OPEN 1
#define READ 1
#define WRITE 1
#define CLOSE 1

pcb_t temp;

static void (*rtc_fops[])(void) = {rtc_open, rtc_read, rtc_write, rtc_close};
static void (*file_fops[])(void) = {file_open, file_read, file_write, file_close};

/*
 * open
 *   DESCRIPTION: Find the file in the file system and assign an unused file descriptor
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if file not found or if fd array is full
 *   SIDE EFFECTS:  Edits PCB
 */
int32_t open(const uint8_t* filename){
    // Based on the file type we set the file descriptor
    // Parameter Validity check is done IN read_dentry_by_name
    int i, fd;
    // STEPS:
    // Find the file in the file system (need the inode and the file type)
    d_entry dentry;
    if (read_dentry_by_name(filename, dentry) < 0 )
        return -1;

    // Allocate an unused file descriptor
    for (i = 2; i < 8; i ++) {
        // check if we have an open spot in array 
        if (temp.fd_array[i].flag == 0) {
            fd = i;
            break;
        }
    }

    // No open spots in array
    if (i == 8) 
        return -1;

    // Set up any data needed to handle the file type
    int type = dentry.filetype;
    switch (type) {
        // RTC
        case 0 :
            temp.fd_array[fd].fops_pointer = rtc_fops;
            temp.fd_array[fd].inode_num = -1;
            break;
        // Directory
        case 1 :
            temp.fd_array[fd].fops_pointer = file_fops; 
            temp.fd_array[fd].inode_num = dentry.inode_num;
            break;
        // File 
        case 2 :
            temp.fd_array[fd].fops_pointer = file_fops; 
            temp.fd_array[fd].inode_num = dentry.inode_num;
            break;

        default :
            return -1;

    }

    temp.fd_array[fd].file_position = 0;
    temp.fd_array[fd].flags = 1;

    return 0;
}


/*
 * close
 *   DESCRIPTION: Close the specified fd
 *   INPUTS: fd -- fd entry to close 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if fd is not valid (tries to close 0 or 1 or out of range 0-7)
 *   SIDE EFFECTS:  Edits PCB
 */
int32_t close(int32_t fd){
    //Check for Valid fd
    if (fd < 2 || fd > 7) // Shouldnt be able to close read and write 
        return -1;

    // Close -> make entry available
    temp.fd_array[fd].flags = 0;

    return 0;
}


/*
 * halt
 *   DESCRIPTION: Doesn't actually do anything, just need to match system call params
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:  none
 */
int32_t halt(uint8_t status){

    return 0;
}


/*
 * execute
 *   DESCRIPTION: Attempts to load and execute a new program
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:  Hands off the processor 
 */
int32_t execute(const uint8_t* command){
    // File Checks (it exists, it is executable)
    uint8_t* cmd_args = strcopy(cmd_args, command);
    
    uint8_t* fname;
    uint32_t cmd_ctr = 0;
    
    // First word is filename 
    while(command[cmd_ctr] != " "){
        cmd_ctr++;
    }
    fname = strncpy(fname, command, cmd_ctr);
    
    d_entry dentry;
    if (read_dentry_by_name(fname, dentry) < 0 )
        return -1;

    //setting the cmd ptr to point to the first char after the first space that is after the first word
    cmd_args += (cmd_ctr + 1);

    // rest is sent to new program 
    // File is executable if first 4 Bytes of the file are (0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46)
    if(dentry.filetype != 0x464C457F){ //reverse this if its the other way around, but I read it as byte 0 being the LSB, otherwise if its MSB then it should be: 0x7F454C46
        return -1;
    }
    // Set up this programs paging
    // init_paging();
    return 0;
}


/*
 * read
 *   DESCRIPTION: calls correct read based on fd
 *   INPUTS: fd -- index into the pcb array
 *           buf -- buffer we want to fill
 *           nbytes -- number of bytes to read into buf
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if fail
 *   SIDE EFFECTS:  none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
    // fd is an index into PCB array 
    return temp.fd_array[fd].fops_pointer[READ](0, buf, nbytes);
}


/*
 * write
 *   DESCRIPTION: Calls the correct write function based on the fd param
 *   INPUTS: fd -- index into the pcb array
 *           buf -- contains what we want to write
 *           nbytes -- number of bytes to write from buf
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if fail
 *   SIDE EFFECTS:  none
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    // fd is an index into PCB array
    return temp.fd_array[fd].fops_pointer[WRITE](1, buf, nbytes);;
}


