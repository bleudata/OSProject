
#include "syscalls.h"

/* Index into function operation pointers */
#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3

pcb_t temp;

static void (*rtc_fops[])(void) = {rtc_open, rtc_read, rtc_write, rtc_close};
static void (*dir_fops[])(void) = {dir_open, dir_read, dir_write, dir_close};
static void (*file_fops[])(void) = {file_open, file_read, file_write, file_close};
static void (*stdin[])(void) = {terminal_open, terminal_read, invalid_write, terminal_close};
static void (*stdout[])(void) = {terminal_open, invalid_read, terminal_write, terminal_close};

uint32_t process_count = 0;
uint32_t pid_array[6] = {0,0,0,0,0,0}; //available pid

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
    if (read_dentry_by_name(filename, &dentry) < 0 )
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
            temp.fd_array[fd].fops_pointer = dir_fops; 
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
    temp.fd_array[fd].fops_pointer[0](); //???? call open
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
    if(process_count >=6 ){
        return -1;
    }
    uint8_t* cmd_args = strcpy(cmd_args, command);
    
    uint8_t* fname;
    uint32_t cmd_ctr = 0;
    
    // First word is filename 
    while(command[cmd_ctr] != " "){
        cmd_ctr++;
    }
    fname = strncpy(fname, command, cmd_ctr);
    
    d_entry dentry;
    if (read_dentry_by_name(fname, &dentry) == -1){
        return -1;
    }
    //setting the cmd ptr to point to the first char after the first space that is after the first word
    cmd_args += (cmd_ctr + 1);

    // rest is sent to new program 
    // File is executable if first 4 Bytes of the file are (0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46)
    uint8_t exe_check[4];
    uint8_t exe[4] = {0x7F, 0x45, 0x4C, 0x46};
    file_read(dentry.inode_num, exe_check, 4);

    uint32_t ctr = 0;//reverse this if its the other way around, but I read it as byte 0 being the LSB
    while(ctr < 4){  //, otherwise if its MSB then it should be: 0x7F454C46
        if(exe_check[ctr] != exe[ctr]){ 
            return -1;
        }
        ctr++;
    }

    /* Set up this programs paging */
    
    //get the current processes physical memory
    // get the entry point into the progam (bytes 24 - 27 of the executable)
    uint8_t* entry_point;
    if (read_data(dentry.inode_num, 24 , entry_point, 4) < 0 ) 
        return -1;
    
    // set up memory map for new process
    map_helper(temp.pid);
    // write the executable file to the page 
    uint32_t file_length = get_file_length(dentry.inode_num);
    // uint8_t file_data_buf[file_length];
    file_read(dentry.inode_num, PROGRAM_START , file_length);
    
    uint32_t new_pid = get_pid(...);
    //fill in new process PCB
    pcb_t * pcb_address = (pcb_t*)get_pcb_address(new_pid);
    pcb_address->pid = new_pid;
    if(process_count == 0){
        pcb_address->parent_id = -1;
    }else{
        pcb_address->parent_id = get_current_pid();
        register uint32_t parent_esp esp;
        pcb_address->parent_esp = parent esp;
        register uint32_t parent_ebp ebp;
        pcb_address->parent_ebp = parent_ebp;
    }
    process_count += 1;

    pcb_address->fd_array[0].fops_pointer; //set stin fopstable to terminal read
    pcb_address->fd_array[1].fops_pointer; //set stoud fopstable to terminal write

    tss->esp0 = EIGHT_MB- new_pid*EIGHT_KB -1;
    // jump to the entry point of the program and begin execution
    
    context_switch(entry_point);

    

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

uint32_t * get_pcb_address(uint32_t pid){
    return EIGHT_MB - EIGHT_KB*(pid + 1);
}
