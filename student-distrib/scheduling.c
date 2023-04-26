#include "scheduling.h"


//changes6
int32_t top_process[3] = {-1,-1,-1}; //-1: no process, else pid of top process
int32_t schedule_flag = 0;
uint32_t cur_sched_terminal = 2; //sched
//uint32_t user_terminal = 0; //keyboard and sched
uint32_t cur_user_terminal = 0; //same as user_terminal for now
uint32_t counter = 0;
uint32_t target_terminal = 0;
uint32_t special_counter = 0;

//setter function for cur_user_terminal maybe
uint32_t get_cur_user_terminal(){
    return cur_user_terminal;
}
uint32_t get_cur_sched_terminal(){
    return cur_sched_terminal;
}
void set_target_terminal(uint32_t terminal_num){
    target_terminal = terminal_num;
}

// this is for the keyboard handler
void user_switch_handler(){

    if(cur_user_terminal == target_terminal){
        return;
    }

    buffer_swap(cur_user_terminal, target_terminal);
    

    //switching into sched terminal
    if(target_terminal == cur_sched_terminal){
        vidmap_helper(USER_VID_MEM); //map user virtual video memory to actual video memory

        //TODO change terminal write to write to video mem
        set_video_mem((unsigned char *)VIDMEM);

        cur_user_terminal = target_terminal;
        return;
    }

    //switching out of sched terminal
    if(cur_user_terminal == cur_sched_terminal){
        vidmap_change(USER_VID_MEM, cur_sched_terminal); // map user vid mem to correct terminal buffer

        //TODO change terminal write to write to cur_sched_terminal buffer
        set_video_mem((unsigned char *)(VIDMEM + FOUR_KB*cur_sched_terminal + FOUR_KB));

        cur_user_terminal = target_terminal;
        return;
    }
    cur_user_terminal = target_terminal;
}


uint32_t schedule(){
    // if(schedule_flag == 0){
    //     return -1;
    // }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & PCB_STACK);

    register uint32_t cur_ebp asm("ebp");
    pcb_address->scheduler_ebp = cur_ebp; //save value of ebp to pcb

    

    // int i; //execute all base shells
    // for(i = 1; i < 3; i++){
    //     if(top_process[i] == -1){
    //         execute("shell"); // have to edit execute to make this work
    //     }
    // }

    //^ or this
    if(counter < 2){
        counter +=1;
        uint8_t cmd[6] = "shell";
        //printf("first two PIT\n");
        if(counter ==1 ){
            vidmap_change(USER_VID_MEM, 1);
            
            //TODO 2. map terminal write to terminal buffer 1
            set_video_mem((unsigned char *)(VIDMEM + FOUR_KB*1 + FOUR_KB));
            terminal_t * terminal = first_get_terminal();
            set_screen_x(&(terminal->screen_x));
            set_screen_y(&(terminal->screen_y));
        }
        if(counter ==2){
            vidmap_change(USER_VID_MEM, 2);

            //TODO 2. map terminal write to terminal buffer
            set_video_mem((unsigned char *)(VIDMEM + FOUR_KB*2 + FOUR_KB));
            terminal_t * terminal = second_get_terminal();
            set_screen_x(&(terminal->screen_x));
            set_screen_y(&(terminal->screen_y));
        }
        
        send_eoi(0);
        execute(cmd);
    }//else{
    //     send_eoi(0);
    //     return;
    // }
    // if(special_counter ==0){
    //     send_eoi(0);
    //     return;
    // }
    // special_counter +=1;

    //only reach here on the third PIT interrupt and after

    //increment terminal number to get next terminal number
    cur_sched_terminal = (cur_sched_terminal + 1) % 3;
    
    int32_t next_pid = top_process[cur_sched_terminal];
    pcb_t * next_process_pcb = (pcb_t *)get_pcb_address(next_pid);
    uint32_t next_process_ebp = next_process_pcb->scheduler_ebp; //get next process ebp
    //printf("next_pid : %d\n", next_pid);
    //change state
    //user vid mapping, terminal write mapping, buffer switching
    if(cur_user_terminal == cur_sched_terminal){ //switching into a terminal user is on
        //1. map user process vid mem to video memory
        vidmap_helper(USER_VID_MEM);

        // update cursor
        // get tje 
        
        terminal_t * terminal = get_terminal(cur_sched_terminal);
        set_screen_x(&(terminal->screen_x));
        set_screen_y(&(terminal->screen_y));
        update_cursor(terminal->screen_x, terminal->screen_y); //update cursor using active termial's screen_x y
    

        //TODO 2. map terminal write to video memory
        set_video_mem((unsigned char *)VIDMEM);
    

    }else{ //switching to a terminal that user is not on
        //1. map user process vid mem to terminal buffer
        vidmap_change(USER_VID_MEM, cur_sched_terminal);
        terminal_t * terminal = get_terminal(cur_sched_terminal);
        set_screen_x(&(terminal->screen_x));
        set_screen_y(&(terminal->screen_y));

        //TODO 2. map terminal write to terminal buffer
        set_video_mem((unsigned char *)(VIDMEM + FOUR_KB*cur_sched_terminal + FOUR_KB));
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

//returns number of base shells
uint32_t bshell_count(){
    uint32_t count = 0;
    int i;
    for(i = 0; i<3; i++){
        if(top_process[i] != -1){
            count+=1;
        }
    }
    
    return count;
    //or return counter+1;
}

//set top process to be equal to some pid, or -1 for no process
void set_top_process(int32_t terminal, int32_t pid){
    top_process[terminal] = pid;
}


