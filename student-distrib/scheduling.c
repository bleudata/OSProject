#include "scheduling.h"

//changes6
uint32_t top_process[3] = {-1,-1,-1}; //-1: no process, else pid of top process
int32_t schedule_flag = 0;
uint32_t next_terminal = 0;
uint32_t user_terminal = 0;

uint32_t schedule(){
    if(schedule_flag == 0){
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_ebp & PCB_STACK);

    register uint32_t cur_ebp asm("ebp");
    pcb_address->scheduler_ebp = cur_ebp; //save value of ebp to pcb

    

    int i; //execute all base shells
    for(i = 0; i < 3; i++){
        if(top_process[i] == -1){
            execute("shell"); // have to edit execute to make this work
        }
    }

    
    int32_t next_pid = top_process[next_terminal];
    pcb_t * next_process_pcb = (pcb_t *)get_pcb_address(next_pid);
    uint32_t next_process_ebp = next_process_pcb->scheduler_ebp; //get next process ebp

    //change state
    //user vid mapping, terminal write mapping, buffer switching
    if(user_terminal == next_terminal){
        //1. map user process vid mem to video memory
        vidmap_helper(USER_VID_MEM);
        
        //2. map terminal write to video memory


    }else{
        //1. map user process vid mem to terminal buffer
        vidmap_change(USER_VID_MEM, next_terminal);

        //2. map terminal write to terminal buffer
    }
    tss->esp0 = EIGHT_MB - next_pid*EIGHT_KB - UINT_BYTES;
    tss.ss0 = KERNEL_DS;
    
    map_helper(next_pid); //remap program memory for process




    //increment terminal number
    uint32_t next_terminal = (next_terminal + 1) % 3;

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

uint32_t bshell_count(){
    uint32_t count = 0;
    int i;
    for(i = 0; i<3; i++){
        if(top_process[i] == -1){
            count+=1;
        }
    }
    
    return count;
}