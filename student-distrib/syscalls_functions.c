
#include "syscalls.h"

/* Index into function operation pointers */
#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3

#define ESP_VIRT_START 0x083FFFFC
#define BYTE_SHIFT     8

static fops_table rtc_fops;
static fops_table dir_fops;
static fops_table file_fops;
static fops_table stdin_fops;
static fops_table stdout_fops;

uint32_t process_count = 0;
uint32_t pid_array[6] = {0,0,0,0,0,0}; //available pid
uint32_t entry_point;
uint32_t esp_start = ESP_VIRT_START;
// uint32_t esp_start = PROGRAM_END;


/*
 * fops_init
 *   DESCRIPTION: Find the file in the file system and assign an unused file descriptor
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, -1 if file not found or if fd array is full
 *   SIDE EFFECTS:  Edits PCB
 */
void fops_init(){
    rtc_fops.open = &rtc_open;
    rtc_fops.read = &rtc_read;
    rtc_fops.write = &rtc_write;
    rtc_fops.close = &rtc_close;

    dir_fops.open = &dir_open;
    dir_fops.read = &dir_read;
    dir_fops.write = &dir_write;
    dir_fops.close = &dir_close;

    file_fops.open = &file_open;
    file_fops.read = &file_read;
    file_fops.write = &file_write;
    file_fops.close = &file_close;

    stdin_fops.open = &terminal_open;
    stdin_fops.read = &terminal_read;
    stdin_fops.write = NULL;
    stdin_fops.close = &terminal_close;

    stdout_fops.open = &terminal_open;
    stdout_fops.read = NULL;
    stdout_fops.write = &terminal_write;
    stdout_fops.close = &terminal_close;

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
            (pcb_address->fd_array[fd]).fops.open = rtc_fops.open;
            (pcb_address->fd_array[fd]).fops.read = rtc_fops.read;
            (pcb_address->fd_array[fd]).fops.write = rtc_fops.write;
            (pcb_address->fd_array[fd]).fops.close = rtc_fops.close;
            (pcb_address->fd_array[fd]).inode_num = -1;
            break;
        // Directory
        case 1 :
            // (pcb_address->fd_array[fd]).fops = dir_fops; 
            (pcb_address->fd_array[fd]).inode_num = dentry.inode_num;
            break;
        // File 
        case 2 :
            // (pcb_address->fd_array[fd]).fops = file_fops; 
            (pcb_address->fd_array[fd]).inode_num = dentry.inode_num;
            break;

        default :
            return -1;

    }

    (pcb_address->fd_array[fd]).file_position = 0;
    (pcb_address->fd_array[fd]).flag = 1;

    return (pcb_address->fd_array[fd]).fops.open(filename); //???? call open
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
    pcb_address->fd_array[fd].flag = 0;
    return pcb_address->fd_array[fd].fops.close(fd);
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

    uint8_t* cmd_args;
    //uint8_t* fname;
    uint8_t fname[9];
    uint8_t cmd_ctr = 0;
    
    // First word is filename 
    while( command[cmd_ctr] != ' '){
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
    // uint32_t ctr = 0;//reverse this if its the other way around, but I read it as byte 0 being the LSB
    // while(ctr < 4){  //, otherwise if its MSB then it should be: 0x7F454C46
    //     if(exe_check[ctr] != exe[ctr]){ 
    //         return -1;
    //     }
    //     ctr++;
    // }
    

    /* Set up this programs paging */
    
    uint32_t* ep;
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
    uint32_t file_length = get_file_length(dentry.inode_num);
    // uint8_t file_data_buf[file_length];
    if(read_data(dentry.inode_num, 0,  PROGRAM_START , file_length) == -1) {//(uint8_t*)or (uint32_t*)
        return -1;
    }
    
    register uint32_t cur_esp asm("esp");
    pcb_t * parent_pcb = (pcb_t*)(cur_esp & 0xFFFFE000);

    //fill in new process PCB
    pcb_t * pcb_address = get_pcb_address(new_pid);
    pcb_address->pid = new_pid;
    if(process_count == 0){
        pcb_address->parent_id = -1;
    }else{
        pcb_address->parent_id = parent_pcb->pid;
        register uint32_t parent_esp asm("esp");
        pcb_address->parent_esp = parent_esp; // technically not needed
        register uint32_t parent_ebp asm("ebp");
        pcb_address->parent_ebp = parent_ebp;
    }
    process_count += 1;

    pcb_address->fd_array[0].fops.open = stdin_fops.open; //set stin fopstable to terminal read
    pcb_address->fd_array[0].fops.close = stdin_fops.close;
    pcb_address->fd_array[0].fops.write = stdin_fops.write;
    pcb_address->fd_array[0].fops.read = stdin_fops.read;
    pcb_address->fd_array[0].flag = 1;
    
    pcb_address->fd_array[1].fops.open = stdout_fops.open;  //set stoud fopstable to terminal write
    pcb_address->fd_array[1].fops.close = stdout_fops.close;
    pcb_address->fd_array[1].fops.write = stdout_fops.write;
    pcb_address->fd_array[1].fops.read = stdout_fops.read;
    pcb_address->fd_array[1].flag = 1;
   

    tss.esp0 = EIGHT_MB- new_pid*EIGHT_KB - 4;
    tss.ss0 = KERNEL_DS;
    // jump to the entry point of the program and begin execution
    asm volatile (" \n\
            pushl $0x002B           \n\
            pushl %0                \n\
            pushfl                  \n\
            pushl $0x0023           \n\
            pushl %1                \n\
            movw  $0x2B, %%ax       \n\
            movw %%ax, %%ds         \n\
            iret                    \n\
            "
            :
            : "r"(esp_start), "r"(entry_point)
            : "memory"
        );


    // context_switch();
    // Inline assembly

    return 0;
}

uint32_t get_pid(){
    int i;
    for(i = 0; i< 5; i++){
        if(pid_array[i] == 0){
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
    return pcb_address->fd_array[fd].fops.read(0, buf, nbytes);
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
    return pcb_address->fd_array[fd].fops.write(1, buf, nbytes);
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
