
#include "syscalls.h"

/* Index into function operation pointers */
#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3

#define ESP_VIRT_START 0x083FFFFC
#define BYTE_SHIFT     8


uint32_t exception_flag = 0; //0 = no exception
uint32_t process_count = 0;
uint32_t pid_array[6] = {0,0,0,0,0,0}; //available pid
uint32_t entry_point;
uint32_t esp_start = ESP_VIRT_START;
// uint32_t esp_start = PROGRAM_END;


/*
 * set_exception_flag
 *   DESCRIPTION: Find the file in the file system and assign an unused file descriptor
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if file not found or if fd array is full
 *   SIDE EFFECTS:  Edits PCB
 */
void set_exception_flag(){
    exception_flag = 1;
}


/*
 * invalid_function
 *   DESCRIPTION: Find the file in the file system and assign an unused file descriptor
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if file not found or if fd array is full
 *   SIDE EFFECTS:  Edits PCB
 */
uint32_t invalid_function() {
    return 0;
}

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
    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & 0xFFFFE000);
    

    d_entry dentry;
    if (read_dentry_by_name(filename, &dentry) < 0 )
        return -1;

    // Allocate an unused file descriptor
    for (i = 2; i < 8; i ++) {
        // check if we have an open spot in array 
        if (pcb_address->fd_array[i].flag == 0) {
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
            (pcb_address->fd_array[fd]).fops.open = rtc_open; //set stin fopstable to terminal read
            (pcb_address->fd_array[fd]).fops.close = rtc_close;
            (pcb_address->fd_array[fd]).fops.write = rtc_write;
            (pcb_address->fd_array[fd]).fops.read = rtc_read;
            (pcb_address->fd_array[fd]).inode_num = dentry.inode_num;
            break;
        // Directory
        case 1 :
            (pcb_address->fd_array[fd]).fops.open = dir_open; //set stin fopstable to terminal read
            (pcb_address->fd_array[fd]).fops.close = dir_close;
            (pcb_address->fd_array[fd]).fops.write = dir_write;
            (pcb_address->fd_array[fd]).fops.read = dir_read;
            (pcb_address->fd_array[fd]).inode_num = dentry.inode_num;
            break;
        // File 
        case 2 :
            (pcb_address->fd_array[fd]).fops.open = file_open; //set stin fopstable to terminal read
            (pcb_address->fd_array[fd]).fops.close = file_close;
            (pcb_address->fd_array[fd]).fops.write = file_write;
            (pcb_address->fd_array[fd]).fops.read = file_read;
            (pcb_address->fd_array[fd]).inode_num = dentry.inode_num;
            break;

        default :
            return -1;

    }
    (pcb_address->fd_array[fd]).file_position = 0;
    (pcb_address->fd_array[fd]).flag = 1;

    return fd; //basically find fd, return fd..(pcb_address->fd_array[fd]).fops.open(filename)
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
    if (fd < 2 || fd > 7){ // Shouldnt be able to close read and write 
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & 0xFFFFE000);

    // Close -> make entry available
    if(pcb_address->fd_array[fd].flag == 0){
        
        return -1;
    }
    
    pcb_address->fd_array[fd].flag = 0;
    
    return 0; //(pcb_address->fd_array)[fd].fops.close(fd)
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
    uint32_t ret_status = status;

    //close all files
    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & 0xFFFFE000);

    int32_t parent_pid = pcb_address->parent_pid;
    int fd;
    for(fd=0; fd <8; fd++){
        
        if(pcb_address->fd_array[fd].flag == 1){
            close(fd);
        }
        
        pcb_address->fd_array[fd].flag = 0;
    }
    //mark child pcb as non-active
    pcb_address->active = 0;
    pid_array[pcb_address->pid] = 0;
    process_count-=1;
    //check if main shell
    if(parent_pid == -1){
        tss.esp0 = EIGHT_MB- 0*EIGHT_KB - 4; //confirm this
        tss.ss0 = KERNEL_DS;
        //maybe have to close all pcbs
        destroy_mapping();
        //call have to call execute from kernel
        execute("shell");

    }else{
        tss.esp0 = EIGHT_MB- (parent_pid)*EIGHT_KB - 4;
        tss.ss0 = KERNEL_DS;
        map_helper(parent_pid);
        get_pcb_address(parent_pid)->active = 1;// ???
        
    }

    uint32_t parent_esp = pcb_address->parent_esp;
    uint32_t parent_ebp = pcb_address->parent_ebp;
    
    if(exception_flag == 1){
        ret_status = 256;
        exception_flag =0; // or we could use the exception handlers
        //clear exception flag
    }
    
    //jump to execute return
    //does iret mess with eax
    asm volatile ("             \n\
        movl %0, %%esp          \n\
        movl %1, %%ebp          \n\
        movl %2, %%eax          \n\
        leave                   \n\
        ret                     \n\
        "
        :
        : "r"(parent_esp), "r"(parent_ebp), "r"(ret_status)
        : "memory"
    );

    // leave                   \n\
    //     ret                     \n\

    // //we dont reach here right
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
    int i;
    if(process_count >= 5){ // maybe should be 6 but check again bro
        return 0;
    }

    uint8_t* cmd_args;
    uint8_t fname[33];
    memset(fname, '\0', 33);
    uint32_t cmd_ctr = 0;

    // printf(" cmd: %s \n ", command);
    
    // First word is filename 
    while( command[cmd_ctr] != ' '  && command[cmd_ctr] != '\0'  && command[cmd_ctr] != '\n'){
        cmd_ctr++;
    }

    strncpy((int8_t*)fname, (int8_t*)command, cmd_ctr);


    d_entry dentry;
    if (read_dentry_by_name(fname, &dentry) == -1){
        return -1;
    }
    //setting the cmd ptr to point to the first char after the first space that is after the first word
    cmd_args = (uint8_t*)(command + cmd_ctr + 1);

    // rest is sent to new program 
    // File is executable if first 4 Bytes of the file are (0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46)
    uint8_t exe_check[4];
    uint8_t exe[4] = {0x7F, 0x45, 0x4C, 0x46};
    //file_read(dentry.inode_num, exe_check, 4);
    
    read_data(dentry.inode_num, 0, exe_check, 4);
    
    if(strncmp((int8_t*)exe_check, (int8_t*)exe, 4) != 0){
        return -1;
    }
    
    /* Set up this programs paging */
    int32_t ep_storage;
    uint32_t* ep = &ep_storage;
    //get the current processes physical memory
    // get the entry point into the progam (bytes 24 - 27 of the executable)
    if (read_data(dentry.inode_num, 24 , ep, 4) < 0 ){  
        return -1;
    }
    entry_point = *ep;
    uint32_t new_pid = get_pid();
    // set up memory map for new process
    map_helper(new_pid);
    // write the executable file to the page 
    int32_t file_length = get_file_length(dentry.inode_num);
    // uint8_t file_data_buf[file_length];
  
    if(read_data(dentry.inode_num, 0,  PROGRAM_START , file_length) == -1) {//(uint8_t*)or (uint32_t*) // PAGE FAULT HERE
        return -1;
    }
   
    register uint32_t cur_esp asm("esp");
    pcb_t * parent_pcb = (pcb_t*)(cur_esp & 0xFFFFE000);

    //fill in new process PCB
    pcb_t * pcb_address = get_pcb_address(new_pid);
    pcb_address->pid = new_pid;
    if(process_count == 0){
        pcb_address->parent_pid = -1;
    }else{
        pcb_address->parent_pid = parent_pcb->pid;
        register uint32_t parent_esp asm("esp");
        pcb_address->parent_esp = parent_esp; // technically not needed
        register uint32_t parent_ebp asm("ebp");
        pcb_address->parent_ebp = parent_ebp;
    }
    process_count += 1;

    pcb_address->fd_array[0].fops.open = terminal_open; //set stin fopstable to terminal read
    pcb_address->fd_array[0].fops.close = terminal_close;
    pcb_address->fd_array[0].fops.write = NULL;
    pcb_address->fd_array[0].fops.read = terminal_read;
    pcb_address->fd_array[0].flag = 1;
    
    pcb_address->fd_array[1].fops.open = terminal_open;  //set stoud fopstable to terminal write
    pcb_address->fd_array[1].fops.close = terminal_close;
    pcb_address->fd_array[1].fops.write = terminal_write;
    pcb_address->fd_array[1].fops.read = NULL;
    pcb_address->fd_array[1].flag = 1;
   
    tss.esp0 = EIGHT_MB - new_pid*EIGHT_KB - 4;
    tss.ss0 = KERNEL_DS;
    //do i set the active field of parent to 0 here?

    // jump to the entry point of the program and begin execution
    asm volatile (" \n\
            pushl $0x002B           \n\
            pushl %0                \n\
            pushfl                  \n\
            pushl $0x0023           \n\
            pushl %1                \n\
            iret                    \n\
            "
            :
            : "r"(esp_start), "r"(entry_point)
            : "memory"
    );

            // movw  $0x2B, %%ax       \n\
            // movw %%ax, %%ds         \n\
    // context_switch();
    // Inline assembly
    // we never reach here right
    return 0;
}

uint32_t get_pid(){
    int i;
    for(i = 0; i< 5; i++){
        if(pid_array[i] == 0){
            pid_array[i] = 1;
            return i;
        }
    }
    return -1;
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
    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & 0xFFFFE000);
    return (pcb_address->fd_array[fd]).fops.read(fd, buf, nbytes);
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
    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & 0xFFFFE000);
    return (pcb_address->fd_array[fd]).fops.write(fd, buf, nbytes);
}

pcb_t * get_pcb_address(uint32_t pid){
    return (pcb_t*)(EIGHT_MB - EIGHT_KB*(pid + 1));
}


extern int32_t getargs(uint8_t* buf, int32_t nbytes) {
    return 0;
}
extern int32_t vidmap(uint8_t** screen_start) {
    return 0;
}
extern int32_t set_handler(int32_t signum, void* handler_address) {
    return 0;
}
extern int32_t sigreturn(void) {
    return 0;
}
