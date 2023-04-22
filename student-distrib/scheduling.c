#include "scheduling.h"

//changes6
int32_t top_process[3] = {-1,-1,-1}; //-1: no process, else pid of top process
int32_t schedule_flag = 0;
uint32_t cur_sched_terminal = 2; //sched
uint32_t user_terminal = 0; //keyboard and sched
uint32_t cur_user_terminal = 0; //same as user_terminal for now
uint32_t counter = 0;

void kb_sched_handler(){

    if(cur_user_terminal == target_terminal){
        return;
    }

    buffer_swap(cur_user_terminal, target_terminal);

    //switching into sched terminal
    if(target_terminal == cur_sched_terminal){
        vidmap_helper(USER_VID_MEM);
        //change terminal write to write to video mem
        cur_user_terminal = target_terminal;
        return;
    }

    //switching out of sched terminal
    if(cur_user_terminal == cur_sched_terminal){
        vidmap_change(USER_VID_MEM, cur_sched_terminal);
        //change terminal write to write to cur_sched_terminal buffer
        cur_user_terminal = target_terminal;
        return;
    }

}


uint32_t schedule(){
    // if(schedule_flag == 0){
    //     return -1;
    // }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_ebp & PCB_STACK);

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
        execute("shell");
    }

    //only reach here on the third PIT interrupt and after

    //increment terminal number to get next terminal number
    uint32_t cur_sched_terminal = (cur_sched_terminal + 1) % 3;
    
    int32_t next_pid = top_process[cur_sched_terminal];
    pcb_t * next_process_pcb = (pcb_t *)get_pcb_address(next_pid);
    uint32_t next_process_ebp = next_process_pcb->scheduler_ebp; //get next process ebp

    //change state
    //user vid mapping, terminal write mapping, buffer switching
    if(user_terminal == cur_sched_terminal){
        //1. map user process vid mem to video memory
        vidmap_helper(USER_VID_MEM);
        
        //2. map terminal write to video memory


    }else{
        //1. map user process vid mem to terminal buffer
        vidmap_change(USER_VID_MEM, cur_sched_terminal);

        //2. map terminal write to terminal buffer
    }
    tss->esp0 = EIGHT_MB - next_pid*EIGHT_KB - UINT_BYTES;
    tss.ss0 = KERNEL_DS;
    
    map_helper(next_pid); //remap program memory for process


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
        if(top_process[i] == -1){
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