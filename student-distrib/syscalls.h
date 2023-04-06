#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "lib.h"
#include "filesystem.h"
#include "rtc.h"
#include "paging.h"

// MOVE TO CORRECT PLACE
// PCB STRUCTURES
typedef struct __attribute__ ((packed)){
    // function operations table pointer
    uint32_t fops_pointer;
    int32_t inode_num;
    uint32_t file_position;
    uint32_t flags; // 0 is free, 1 is taken 
} fd_entry;

typedef struct __attribute__ ((packed)){
    uint32_t pid; 
    uint32_t parent_id;
    // file descriptors: we need the actual array of file descriptors
    fd_entry fd_array[8];
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint8_t active;
} pcb_t;

extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);


#endif /*SYSCALLS_H*/