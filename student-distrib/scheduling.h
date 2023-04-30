#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "syscalls.h"
#include "paging.h"

#define PCB_STACK       0xFFFFE000
#define NUM_TERMS       3
#define INIT_TRACKER    2

#define TERM_ZERO       0
#define TERM_ONE        1
#define TERM_TWO        2

extern uint32_t schedule();
uint32_t bshell_count();
void set_target_terminal(uint32_t terminal_num);
extern void user_switch_handler();
void set_top_process(int32_t terminal, int32_t pid);
uint32_t get_cur_sched_terminal();
uint32_t get_cur_user_terminal();

#endif /*SYSCALLS_H*/
