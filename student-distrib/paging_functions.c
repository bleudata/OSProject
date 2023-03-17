
#include "paging.h"



//create page directory for kernel
page_directory_entry page_directory[1024] __attribute__((aligned(4096)));


//create page table for video memory
page_table_entry first_page_table[1024] __attribute__((aligned(4096))); 


/*initializes page directory and page table for the kernel*/
void init_paging() {
    
    //initialize page directory to all not present
    int i;
    for(i = 0; i < 1024; i++){
        page_directory[i].entry = 0x00000000;

    }

    //initialize first page table to all not present
    for(i = 0; i< 1024; i++){
        
        first_page_table[i].entry = 0x00000000;

    }

    // 0000 0000 0100 0000  0000 0000 0000 0000 (4mb)
    // 0x0040 0000 (virtual) -> 0x0040 0000 (physical) (for kernel) (4MB page)
    // 0x000B 8000 (virtual) -> 0x000B 8000 (physical) (for video memory) (4KB page)

    //initialize 4MB page for kernel
    //page_directory[1].entry = 0x00400083;
    page_directory[1].fourmb.present = 1;
    page_directory[1].fourmb.read_write = 1;
    page_directory[1].fourmb.page_size = 1;
    page_directory[1].fourmb.page_address = 1;


    //initialize 4KB page for video memory
    page_directory[0].entry = (uint32_t)(first_page_table) | 3;
    //first_page_table[184].entry = 0x000B8003; 
    //0xB8000 = 753664, 753664/4096 = 184 or 0xb8 = 184
    first_page_table[184].pt_fields.present = 1;
    first_page_table[184].pt_fields.read_write = 1;
    first_page_table[184].pt_fields.page_address = 184;
    

    load_page_dir(page_directory);
    enable_mixed_size();
    enable_paging();
    
    
    
    
}


