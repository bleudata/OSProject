#include "lib.h"

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