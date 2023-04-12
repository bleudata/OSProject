
#include "syscalls.h"

static uint8_t* cmd_args;
uint8_t args_buffer[33];
uint8_t args_length;
uint32_t exception_flag = 0; //0 = no exception
uint32_t process_count = 0;
uint32_t pid_array[6] = {0,0,0,0,0,0}; //available pid
uint32_t entry_point;
uint32_t esp_start = ESP_VIRT_START;


/*
 * set_exception_flag
 *   DESCRIPTION: Sets the exception flag to 1
 *   INPUTS: None
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:  edits the exception flag
 */
void set_exception_flag(){
    exception_flag = 1;
}


/*
 * open
 *   DESCRIPTION: Open creates and populates the pcb for the current process. Based on the type of file, we set the fops table.
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: fd of teh file given
 *   SIDE EFFECTS:  Edits physical memory where the PCB is
 */
int32_t open(const uint8_t* filename){
    int i, fd;
    // Invalid parameter check 
    if(filename == NULL){
        return -1;
    }
    // printf(" 42 filename: %s\n", filename);
    // printf(" \ndone with filename \n");
    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & PCB_STACK);
    
    d_entry dentry;
    if (read_dentry_by_name(filename, &dentry) < 0 )
        return -1;

    // Find an unused file descriptor
    for (i = FD_INIT_SIZE; i < FD_OVERFLOW; i ++) {
        if (pcb_address->fd_array[i].flag == 0) {
            fd = i;
            break;
        }
    }

    // No open spots in array
    if (i == FD_OVERFLOW) 
        return -1;

    // Set up any data needed to handle the file type
    int type = dentry.filetype;
    switch (type) {
        // RTC
        case 0 :
            (pcb_address->fd_array[fd]).fops.open = rtc_open; 
            (pcb_address->fd_array[fd]).fops.close = rtc_close;
            (pcb_address->fd_array[fd]).fops.write = rtc_write;
            (pcb_address->fd_array[fd]).fops.read = rtc_read;
            (pcb_address->fd_array[fd]).inode_num = dentry.inode_num;
            break;
        // Directory
        case 1 :
            (pcb_address->fd_array[fd]).fops.open = dir_open; 
            (pcb_address->fd_array[fd]).fops.close = dir_close;
            (pcb_address->fd_array[fd]).fops.write = dir_write;
            (pcb_address->fd_array[fd]).fops.read = dir_read;
            (pcb_address->fd_array[fd]).inode_num = dentry.inode_num;
            break;
        // File 
        case 2 :
            (pcb_address->fd_array[fd]).fops.open = file_open; 
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

    // Dont call respective open: just return the fd
    if ((pcb_address->fd_array[fd]).fops.open(filename) == -1 ) 
        return -1;

    return fd; 
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
    //Check for Valid fd (not 0 or 1 or out of range)
    if (fd < FD_INIT_SIZE || fd > FD_MAX_SIZE){  
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & PCB_STACK);

    // Close -> make entry available
    if(pcb_address->fd_array[fd].flag == 0)
        return -1;

    pcb_address->fd_array[fd].flag = 0;
    (pcb_address->fd_array[fd]).file_position = 0;
    
    return 0; 
}


/*
 * halt
 *   DESCRIPTION: System call terminates a process, returning the specified value to its parent process
 *   INPUTS: status -- return number unless an execption is called
 *   OUTPUTS: None
 *   RETURN VALUE: status number
 *   SIDE EFFECTS:  edits the PCB returns to parent process
 */
int32_t halt(uint8_t status){
    uint32_t ret_status = status;

    //close all files
    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & PCB_STACK);

    int32_t parent_pid = pcb_address->parent_pid;
    int fd;
    for(fd=0; fd < FD_OVERFLOW; fd++){
        
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
        tss.esp0 = EIGHT_MB - EIGHT_KB - UINT_BYTES; 
        tss.ss0 = KERNEL_DS;
        // Unmap and call shell again
        destroy_mapping();
        uint8_t cmd[SHELL_SIZE] = "shell";
        execute(cmd);

    }else{
        tss.esp0 = EIGHT_MB- (parent_pid)*EIGHT_KB - UINT_BYTES;
        tss.ss0 = KERNEL_DS;
        map_helper(parent_pid);
        get_pcb_address(parent_pid)->active = 1;
        
    }

    uint32_t parent_esp = pcb_address->parent_esp;
    uint32_t parent_ebp = pcb_address->parent_ebp;
    
    if(exception_flag == 1){
        ret_status = EXCEPT_STATUS;
        exception_flag = 0; 
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
    // Parameter check
    if(process_count >= MAX_PROC_CNT){
        return -1;
    }

    if(command == NULL){
        return -1;
    }

    uint8_t fname[FNAME_MAX_SIZE];
    memset(fname, '\0', FNAME_MAX_SIZE);
    uint32_t cmd_ctr = 0;
    
    // Get first word which is the fname
    while( command[cmd_ctr] != ' '  && command[cmd_ctr] != '\0'  && command[cmd_ctr] != '\n'){
        cmd_ctr++;
    }
    strncpy((int8_t*)fname, (int8_t*)command, cmd_ctr);

    d_entry dentry;
    if (read_dentry_by_name(fname, &dentry) == -1){
        return -1;
    }
    //setting the cmd ptr to point to the first char after the first space that is after the first word
    int k = 0;
    if (command[cmd_ctr] == ' ' ) {
        // terminal_write(1, " in setting cmd args \n command: ", 22);
        cmd_args = (uint8_t*)(command + cmd_ctr + 1);
        memset(args_buffer, '\0', 33);
        // fill the actual characters
        while((cmd_args[k] != '\0')) { 
            args_buffer[k] = cmd_args[k];
            k++;
        }

        args_length = k+1; // add +1 because need to include a null terminator 
        // puts(cmd_args);
    }
    // printf(" argument: %s", cmd_args);
    
    // File is executable if first 4 Bytes of the file are (0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46)
    uint8_t exe_check[EXE_BUF];
    uint8_t exe[EXE_BUF] = {EXE_BYTE0, EXE_BYTE1, EXE_BYTE2, EXE_BYTE3};
    
    read_data(dentry.inode_num, 0, exe_check, EXE_BUF);
    
    if(strncmp((int8_t*)exe_check, (int8_t*)exe, EXE_BUF) != 0){
        return -1;
    }
    
    /* Set up this programs paging */
    // Entry point into the progam (bytes 24 - 27 of the executable)
    if (read_data(dentry.inode_num, 24 , (uint8_t*)&entry_point, UINT_BYTES) < 0 ){  
        return -1;
    }
    uint32_t new_pid = get_pid();
    map_helper(new_pid); // set up memory map for new process
    // if (command != "shell") {
    //     // puts(" \n before the file lentgh \n ");
    //     // puts(cmd_args);
    // }
    // puts(cmd_args);
    // puts("\n");
    int32_t file_length = get_file_length(dentry.inode_num);
    // puts(cmd_args);
    // if (command[cmd_ctr] == ' ' ) {
    //     puts(cmd_args);
    // }
    uint32_t * program_start = (uint32_t*)PROGRAM_START;

    if(read_data(dentry.inode_num, 0,  (uint8_t*)program_start , file_length) == -1)  // write the executable file to the page
        return -1;
   
    register uint32_t cur_esp asm("esp");
    pcb_t * parent_pcb = (pcb_t*)(cur_esp & PCB_STACK);

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

    //Set stdin fops to correct terminal
    pcb_address->fd_array[STDIN_FD].fops.open = terminal_open; 
    pcb_address->fd_array[STDIN_FD].fops.close = terminal_close;
    pcb_address->fd_array[STDIN_FD].fops.write = NULL;
    pcb_address->fd_array[STDIN_FD].fops.read = terminal_read;
    pcb_address->fd_array[STDIN_FD].flag = 1;
    pcb_address->active = 1;
    
    //Set stdout fops to correct terminal
    pcb_address->fd_array[STDOUT_FD].fops.open = terminal_open;  
    pcb_address->fd_array[STDOUT_FD].fops.close = terminal_close;
    pcb_address->fd_array[STDOUT_FD].fops.write = terminal_write;
    pcb_address->fd_array[STDOUT_FD].fops.read = NULL;
    pcb_address->fd_array[STDOUT_FD].flag = 1;
    pcb_address->active = 1;

    //setting the new TSS ESP0 and SS0
    tss.esp0 = EIGHT_MB - new_pid*EIGHT_KB - UINT_BYTES;
    tss.ss0 = KERNEL_DS;
    
    // jump to the entry point of the program and begin execution

    //line 1: This value is USER DS
    //line 2: This pushes the ESP
    //line 3: This pushes the flags
    //line 4: This value is USER CS
    //line 5: This pushes the EIP

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
  
    return 0;
}

/*
 * get_pid
 *   DESCRIPTION: Returns the current pid
 *   INPUTS: 
 *   OUTPUTS: none
 *   RETURN VALUE: the current pid, -1 if fail
 *   SIDE EFFECTS:  none
 */
uint32_t get_pid(){
    int i;
    for(i = 0; i< PROC_CNT; i++){
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
    // Invalid parameter
    int32_t bytes_read;
    if(fd < 0 || fd > FD_MAX_SIZE){
        return -1;
    }
    if(buf == NULL){
        return -1;
    }
    if(nbytes < 0){
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & PCB_STACK);

    if((pcb_address->fd_array[fd]).flag == 0){
        return -1;
    }

    //check if its terminal read for stdin aka if its NULL, and if it is return -1, otherwise do a normal file's read
    if((pcb_address->fd_array[fd]).fops.read != NULL){
        // read the file and update the file position based on number of bytes succesfully read
        bytes_read = (pcb_address->fd_array[fd]).fops.read(fd, buf, nbytes);
        (pcb_address->fd_array[fd]).file_position += bytes_read;
        return bytes_read;
    }
    else{
        return -1;
    }
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
    // Invalid Parameter check
    if(fd < 0 || fd > FD_MAX_SIZE){
        return -1;
    }
    if(buf == NULL){
        return -1;
    }
    if(nbytes < 0){
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & PCB_STACK);

    //check if its terminal write for stdout aka if its NULL, and if it is return -1, otherwise do a normal file's write
    if((pcb_address->fd_array[fd]).fops.write != NULL){
        return (pcb_address->fd_array[fd]).fops.write(fd, buf, nbytes);
    }
    else{
        return -1;
    }
}

/*
 * get_pcb_address
 *   DESCRIPTION: Returns the current processes PCB
 *   INPUTS: pid- Current process ID (0-5)
 *   OUTPUTS: none
 *   RETURN VALUE: returns the PCB address
 *   SIDE EFFECTS:  None
 */
pcb_t * get_pcb_address(uint32_t pid){
    return (pcb_t*)(EIGHT_MB - EIGHT_KB*(pid + 1));
}

/*
 * getarg
 *   DESCRIPTION: Call reads the program’s command line arguments into a user-level buffer
 *   INPUTS: buf --
 *           nbytes -- 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if fail
 *   SIDE EFFECTS:  none
 */
extern int32_t getargs(uint8_t* buf, int32_t nbytes) {
    // if no args OR args and terminal null dont fit into BUF then return -1
    // puts("cmd value : ");
    // puts(cmd_args);
    int32_t num_bytes = args_length;
    // printf(" \n byts: %d", num_bytes);
    int i;

    // invalid null buffer
    if(buf == NULL) {
        return -1;
    }
    if ( num_bytes == 0) {
        // terminal_write(1, " first -1 ", 11);
        buf = NULL;
        return -1;
    }
    if( nbytes < 0){
        // terminal_write(1, " sec -1 ", 9);
        return -1;
    }

    int32_t bytes_to_read = 0;
    memset(buf, '\0', nbytes); // clear out the buffer for safety?? maybe don't need this

    if(nbytes >= num_bytes){
        bytes_to_read = num_bytes;
    }
    else{
        bytes_to_read = nbytes;
    }

    for(i = 0; i < bytes_to_read; i++){
        buf[i] = args_buffer[i];
    }
    
    // args_flag = 0;
    return 0;
}

/*
 * vidmap
 *   DESCRIPTION: call maps the text-mode video memory into user space at a pre-set virtual address
 *   INPUTS: buf -- 
 *           nbytes -- 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if fail
 *   SIDE EFFECTS:  none
 */
extern int32_t vidmap(uint8_t** screen_start) {
    return -1;
}

/*
 * set_handler
 *   DESCRIPTION: Calls are related to signal handling and are discussed in the section
 *   INPUTS: signum -- 
 *           handler_address -- 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if fail
 *   SIDE EFFECTS:  none
 */
extern int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}

/*
 * sigreturn
 *   DESCRIPTION: Calls are related to signal handling and are discussed in the section 
 *   INPUTS: void
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if fail
 *   SIDE EFFECTS:  none
 */
extern int32_t sigreturn(void) {
    return -1;
}
