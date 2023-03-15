
#include "paging.h"



//create page directory for kernel
uint32_t page_directory[1024] __attribute__((aligned(4096)));


uint32_t first_page_table[1024] __attribute__((aligned(4096))); //PCB, global var??

//write a regular c program that tests this, create fake table, print to screen


void init_paging() {
    
    //initialize page directory to all not present
    int i;
    for(i = 0; i < 1024; i++){
        page_directory[i] = 0x00000000;

    }

    //initialize first page table to all not present
    for(i = 0; i< 1024; i++){
        //right shift by 12 bits
        first_page_table[i] = 0x00000000;

    }
    // 0000 0000 0100 0000  0000 0000 0000 0000
    // 0x0040 0000 (virtual) -> 0x0040 0000 (physical) (for kernel) (4MB page)
    // 0x000B 8000 (virtual) -> 0x000B 8000 (physical) (for video memory) (4KB page)

    //initialize 4MB page for kernel
    page_directory[1] = 0x00400083;

    //initialize 4KB page for video memory
    page_directory[0] = (uint32_t)(first_page_table) | 3;
    first_page_table[184] = 0x000B8003; //0xB8000 = 753664, 753664/4096 = 184

    load_page_dir(page_directory);
    enable_mixed_size();
    enable_paging();
    
    
    
    
}


