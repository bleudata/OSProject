#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "syscalls.h"

#define BLANK_ENTRY     0x00000000
#define PT_INDEX_MAP    0x003FF000
#define VIDMEM          0x000B8000
#define VIDMEM_ADDRS    4096

#define VMEM_OFFSET     184
#define VMEM_ENTRY_SET  3
#define VIRTMEM_UPPR_20 12
#define SET_ENTRY_BITS  7

#define PHYS_BASE       2

#define DIR_SIZE        1024
#define TABLE_SIZE      1024

#define FOUR_KB         4096

typedef struct __attribute__ ((packed)){
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through: 1;
    uint32_t cache_disabled: 1;
    uint32_t accessed : 1;
    uint32_t reserved : 1;
    uint32_t page_size : 1;
    uint32_t global_page : 1;
    uint32_t available_prog_use : 3; //11-9
    uint32_t pt_address : 20; //page table physical base address, 12-31
}pde_inside_4KB;

typedef struct __attribute__ ((packed)){
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through: 1;
    uint32_t cache_disabled: 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t page_size : 1;
    uint32_t global_page : 1;
    uint32_t available_prog_use : 3; //11-9
    uint32_t pta_index : 1;
    uint32_t bits_13_21 : 9;
    uint32_t page_address : 10; //physical page base address, bit 22-31
}pde_inside_4MB;

//struct for page directory entry
typedef union{
    
    uint32_t entry;
    pde_inside_4KB fourkb;
    pde_inside_4MB fourmb;

}page_directory_entry;

typedef struct __attribute__ ((packed)){
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t write_through: 1;
        uint32_t cache_disabled: 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t pta_index : 1;
        uint32_t global_page : 1;
        uint32_t available_prog_use : 3; //11-9
        uint32_t page_address : 20; //physical page base address
}pt_fields;

//struct for page table entry
typedef union{
    uint32_t entry;

    pt_fields pt_fields;

}page_table_entry;



/*function that calls other initalization functions*/
extern void init_paging();

//assembly function that puts page_directory address into %cr3
extern void load_page_dir(uint32_t * page_dir_ptr);

//set paging bit (bit 31 in the %cr0 register)
extern void enable_paging();

//enables mixed size pages (4kb and 4mb), set bit 4 of %cr4
extern void enable_mixed_size();

// returns physical address associated to the 
extern void map_helper(uint32_t pid);

extern void destroy_mapping();

void vidmap_helper(uint32_t virtual_address);

extern void flush_tlb();

void vidmap_change(uint32_t virtual_address, uint32_t terminal);

void buffer_swap(uint32_t old_terminal, uint32_t new_terminal);

void copy_video_memory(unsigned char * destination, unsigned char * source);

#endif

