
#include "paging.h"



//create page directory for kernel
page_directory_entry page_directory[1024] __attribute__((aligned(4096)));
page_directory_entry page_process_dir[6][1024] __attribute__((aligned(4096)));

//create page table for video memory
page_table_entry first_page_table[1024] __attribute__((aligned(4096))); 
page_table_entry vmem_page_tables[6][1024] __attribute__((aligned(4096))); 

/*
 * init_paging()
 *   DESCRIPTION: initializes kernel's page dir and page table, calls page initalization functions
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes paging for kernel :)
 */
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

    // 0000 0000 0100 0000  0000 0000 0000 0000 (4mb) 2^22
    // 0x0040 0000 (virtual) -> 0x0040 0000 (physical) (for kernel) (4MB page)
    // 0x000B 8000 (virtual) -> 0x000B 8000 (physical) (for video memory) (4KB page) 2^12

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
    
    //load page dir to %cr3, enable mixed size pages and turn on paging
    load_page_dir((uint32_t *)page_directory);
    enable_mixed_size();
    enable_paging();

}

//just copied daniel's function here; added the [pid] index to get the correct paging structs from the global arrays and also changed first_page_table to vmem_page_tables and page_dir to page_process_dir
void process_loading(uint32_t pid, d_entry dentry){
    uint32_t* proc_mem = PHYS_MEM_BASE + (pid * PHYS_MEM_OFF);

    //initialize page directory to all not present
    int i;
    for(i = 0; i < 1024; i++){
        page_process_dir[pid][i].entry = 0x00000000;

    }

    //initialize first page table to all not present
    for(i = 0; i< 1024; i++){
        vmem_page_tables[pid][i].entry = 0x00000000;

    }

    // 0000 0000 0100 0000  0000 0000 0000 0000 (4mb) 2^22
    // 0x0040 0000 (virtual) -> 0x0040 0000 (physical) (for kernel) (4MB page)
    // 0x000B 8000 (virtual) -> 0x000B 8000 (physical) (for video memory) (4KB page) 2^12

    //initialize 4MB page for kernel
    //page_directory[1].entry = 0x00400083;
    page_process_dir[pid][1].fourmb.present = 1;
    page_process_dir[pid][1].fourmb.read_write = 1;
    page_process_dir[pid][1].fourmb.page_size = 1;
    page_process_dir[pid][1].fourmb.page_address = 1;

    //initializing 4MB page for process image 
    page_process_dir[pid][2].fourmb.present = 1;
    page_process_dir[pid][2].fourmb.read_write = 1;
    page_process_dir[pid][2].fourmb.page_size = 1;
    page_process_dir[pid][2].fourmb.page_address = 1;

    //initialize 4KB page for video memory
    page_process_dir[pid][0].entry = (uint32_t)(vmem_page_tables[pid]) | 3;
    //first_page_table[184].entry = 0x000B8003; 
    //0xB8000 = 753664, 753664/4096 = 184 or 0xb8 = 184
    vmem_page_tables[pid][184].pt_fields.present = 1;
    vmem_page_tables[pid][184].pt_fields.read_write = 1;
    vmem_page_tables[pid][184].pt_fields.page_address = 184;
    
    //load page dir to %cr3, enable mixed size pages and turn on paging
    load_page_dir((uint32_t *)page_process_dir[pid]);
    enable_mixed_size();
    enable_paging();

}