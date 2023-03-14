#ifndef PAGING_H
#define PAGING_H

#include "types.h"

// 0x0040 0000 (virtual) -> 0x0040 0000 (physical) (for kernel) (4MB page)
// 0x000B 8000 (virtual) -> 0x000B 8000 (physical) (for video memory) (4KB page)

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
    uint32_t bits_12_31 : 20; //page table base address
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
    uint32_t bits_22_31 : 10; //physical page base address
}pde_inside_4MB;

typedef union{
    
    uint32_t entry;
    pde_inside_4KB fourkb;
    pde_inside_4MB fourmb;

}page_directory_entry;



typedef union{
    uint32_t entry;

    struct{
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
        uint32_t bits_12_31 : 20; //physical page base address
    } __attribute__ ((packed));

}page_table_entry;




extern void init_paging();

extern void load_page_dir(uint32_t * page_dir_ptr);

extern void enable_paging();

extern void enable_mixed_size();






//populate_page_table
//allocate_page_table
//free_page_table
//edit_page_table_entry
//add_entry
//free_entry

//create_page_directory


#endif
