#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "syscalls.h"

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

//init paging for individual processes
extern void process_loading(uint32_t pid, d_entry dentry);

//assembly function that puts page_directory address into %cr3
extern void load_page_dir(uint32_t * page_dir_ptr);

//set paging bit (bit 31 in the %cr0 register)
extern void enable_paging();

//enables mixed size pages (4kb and 4mb), set bit 4 of %cr4
extern void enable_mixed_size();

extern void 




//populate_page_table
//allocate_page_table
//free_page_table
//edit_page_table_entry
//add_entry
//free_entry
//create_page_directory


#endif
