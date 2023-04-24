#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "syscalls.h"
#include "paging.h"

#define PCB_STACK       0xFFFFE000

extern uint32_t schedule();
uint32_t bshell_count();
void set_target_terminal(uint32_t terminal_num);
void user_switch_hanlder();
void set_top_process(int32_t terminal, int32_t pid);
uint32_t get_cur_sched_terminal();
// extern int32_t top_process[3]; could do this

#endif /*SYSCALLS_H*/
