#include "scheduling.h"


int32_t top_process[3] = {-1,-1,-1}; //-1: no process, else pid of top process 
int32_t schedule_flag = 0;
int32_t cur_sched_terminal = 0; //sched has to be 2, currently scheduled terminal
uint32_t cur_user_terminal = 0; // the visible / displayed terminal
uint32_t counter = 0; // for first time bootup of 3 shells
uint32_t target_terminal = 0; // terminal to switch to

/*
 * get_cur_user_terminal
 *   DESCRIPTION: get the number 0-2 of the terminal the user can currently see
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: number 0-2 of the terminal the user can currently see
 *   SIDE EFFECTS: none
 */
uint32_t get_cur_user_terminal(){
    return cur_user_terminal;
}


/*
 * get_cur_sched_terminal
 *   DESCRIPTION: get the number 0-2 of the terminal that has the currently scheduled process
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: number 0-2 of the terminal that has the currently scheduled process
 *   SIDE EFFECTS: none
 */
uint32_t get_cur_sched_terminal(){
    return cur_sched_terminal;
}

/*
 * set_target_terminal
 *   DESCRIPTION: sets the variable target_terminal
 *   INPUTS: terminal_num -- number 0-2 corresponding to the target terminal we want to switch to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: updates target_terminal
 */
void set_target_terminal(uint32_t terminal_num){
    target_terminal = terminal_num;
}

/*
 * user_switch_handler
 *   DESCRIPTION: Remaps and copies memory to and from storage / video memory when the user switches the visible terminal in the keyboard handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: updates target_terminal
 */
void user_switch_handler(){

    if(cur_user_terminal == target_terminal){ // do nothing if user is already viewing the terminal to switch to
        return;
    }
    // otherwise store whats in video memory  currently into terminal storage, and put what was in terminal
    // storage into video memory
    buffer_swap(cur_user_terminal, target_terminal);

    //switching into sched terminal
    if(target_terminal == cur_sched_terminal){
        cur_user_terminal = target_terminal; 
        vidmap_helper(USER_VID_MEM); //map user virtual video memory to actual video memory 
        set_video_mem((unsigned char *)VIDMEM); //change terminal write to write to video mem
        return;
    }

    //switching out of sched terminal
    if(cur_user_terminal == cur_sched_terminal){
        cur_user_terminal = target_terminal;
        vidmap_change(USER_VID_MEM, cur_sched_terminal); // map user vid mem to correct terminal buffer
        set_video_mem((unsigned char *)(VIDMEM + FOUR_KiB*cur_sched_terminal + FOUR_KiB)); //change terminal write to write to cur_sched_terminal buffer
        return;
    }
    cur_user_terminal = target_terminal;
}

/*
 * schedule
 *   DESCRIPTION: changes scheduled process
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
uint32_t schedule(){
    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & PCB_STACK);

    register uint32_t cur_ebp asm("ebp");
    pcb_address->scheduler_ebp = cur_ebp; //save value of ebp to pcb

    // to load 3 base shells on boot up
    if(counter < 2){
        cur_sched_terminal+=1;
        counter +=1;
        uint8_t cmd[6] = "shell";
        if(counter ==1 ){
            vidmap_change(USER_VID_MEM, 1);
            
            // map terminal write to terminal buffer 1
            set_video_mem((unsigned char *)(VIDMEM + FOUR_KB*1 + FOUR_KB));
            terminal_t * terminal = get_terminal(1);
            set_screen_x(&(terminal->screen_x));
            set_screen_y(&(terminal->screen_y));
        }
        if(counter ==2){
            vidmap_change(USER_VID_MEM, 2);

            // map terminal write to terminal buffer
            set_video_mem((unsigned char *)(VIDMEM + FOUR_KB*2 + FOUR_KB));
            terminal_t * terminal = get_terminal(2);
            set_screen_x(&(terminal->screen_x));
            set_screen_y(&(terminal->screen_y));
        }
        
        send_eoi(0);
        execute(cmd);
    }

    //only reach here on the third PIT interrupt and after
    //increment terminal number to get next terminal number
    cur_sched_terminal = (cur_sched_terminal + 1) % 3; // round robin, move to next terminal
    
    int32_t next_pid = top_process[cur_sched_terminal];
    pcb_t * next_process_pcb = (pcb_t *)get_pcb_address(next_pid);
    uint32_t next_process_ebp = next_process_pcb->scheduler_ebp; //get next process ebp
    //change state
    //user vid mapping, terminal write mapping, buffer switching
    if(cur_user_terminal == cur_sched_terminal){ //switching into a terminal user is on
        vidmap_helper(USER_VID_MEM); //1. map user process vid mem to video memory
        terminal_t * terminal = get_terminal(cur_sched_terminal);
        set_screen_x(&(terminal->screen_x)); // Putc should update the x and y position for this terminal
        set_screen_y(&(terminal->screen_y));
        update_cursor(terminal->screen_x, terminal->screen_y); //update cursor using active termial's screen_x y
        set_video_mem((unsigned char *)VIDMEM); // 2. Map terminal write to video memory
    

    }else{ //switching to a terminal that user is not on
        //1. map user process vid mem to terminal buffer
        vidmap_change(USER_VID_MEM, cur_sched_terminal);
        terminal_t * terminal = get_terminal(cur_sched_terminal);
        set_screen_x(&(terminal->screen_x)); // Putc should update x and y position for this terminal
        set_screen_y(&(terminal->screen_y));
        set_video_mem((unsigned char *)(VIDMEM + FOUR_KB*cur_sched_terminal + FOUR_KB)); //2. map terminal write to terminal buffer
    }

    tss.esp0 = EIGHT_MB - next_pid*EIGHT_KB - UINT_BYTES;
    tss.ss0 = KERNEL_DS;
    
    map_helper(next_pid); //remap program memory for process

    send_eoi(0);
    asm volatile ("             \n\
        movl %0, %%ebp          \n\
        leave                   \n\
        ret                     \n\
        "
        :
        : "r"(next_process_ebp)
        : "memory"
    );

    return 0;
}

/*
 * bshell_count
 *   DESCRIPTION: returns the number of base shells
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: number of base shells
 *   SIDE EFFECTS: none
 */
uint32_t bshell_count(){
    uint32_t count = 0;
    int i;
    for(i = 0; i<3; i++){
        if(top_process[i] != -1){
            count+=1;
        }
    }
    
    return count;
}

/*
 * set_top_process
 *   DESCRIPTION: set the top process to be equal to some pid, or -1 for no process
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_top_process(int32_t terminal, int32_t pid){
    top_process[terminal] = pid;
}


