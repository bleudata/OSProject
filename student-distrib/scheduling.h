#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "syscalls.h"

#define PCB_STACK       0xFFFFE000

uint32_t schedule();
uint32_t bshell_count();
// extern int32_t top_process[3]; could do this

#endif /*SYSCALLS_H*/