#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "lib.h"
#include "filesystem.h"
#include "rtc.h"
#include "paging.h"
#include "x86_desc.h"
#include "terminal_driver.h"
#include "../syscalls/ece391sysnum.h" 
#include "keyboard_driver.h"

#define PHYS_MEM_BASE   0x800000 //8MB 
#define PHYS_MEM_OFF    0x400000 //4MB
#define PROGRAM_START   0x08048000  
#define EIGHT_MB        0x800000
#define EIGHT_KB        0x2000
#define FOUR_MB         0x400000 //4MB
#define PROGRAM_END     PROGRAM_START + FOUR_MB - 1
#define ESP_VIRT_START  0x083FFFFC
#define BYTE_SHIFT      8
#define PCB_STACK       0xFFFFE000
#define VIRT_MEM_PAGE   0x08000000
#define VIRT_MEM_SHIFT  22
#define USER_VID_MEM    0xDBBA0000

#define FD_OVERFLOW     8
#define FD_INIT_SIZE    2
#define FD_MAX_SIZE     7

#define UINT_BYTES      4
#define SHELL_SIZE      6
#define EXE_BUF         4
#define FNAME_MAX_SIZE  33

#define EXCEPT_STATUS   256

#define MAX_PROC_CNT    6
#define PROC_CNT        5

#define EXE_BYTE0       0x7F
#define EXE_BYTE1       0x45
#define EXE_BYTE2       0x4C
#define EXE_BYTE3       0x46

#define STDIN_FD        0
#define STDOUT_FD       1

// Function pointer struct 
typedef struct __attribute__ ((packed)){
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
    
} fops_table;

// MOVE TO CORRECT PLACE
// PCB STRUCTURES
typedef struct __attribute__ ((packed)){
    // function operations table pointer
    fops_table fops;
    int32_t inode_num; 
    uint32_t file_position;
    uint32_t flag; // 0 is free, 1 is taken 
} fd_entry;

typedef struct __attribute__ ((packed)){
    uint32_t pid; // 0-5
    int32_t parent_pid;
    fd_entry fd_array[8]; // file descriptors: we need the actual array of file descriptors
    uint32_t parent_esp;
    uint32_t parent_ebp;
    uint32_t scheduler_ebp; //ebp for scheduling
    uint8_t active;
    uint8_t args_data[KEYBOARD_BUF_SIZE];
    uint8_t args_length;
    uint32_t terminal;

} pcb_t;



uint32_t get_pid();

extern void set_exception_flag();

extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn(void);

pcb_t * get_pcb_address(uint32_t pid);

// handler for system calls
extern void system_call_handler_lnk();

#endif /*SYSCALLS_H*/

