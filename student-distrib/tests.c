#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesystem.h"

#define PASS 1
#define FAIL 0



/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 19; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	if ((idt[0x21].offset_15_00 == NULL) && 
			(idt[0x21].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}

	if ((idt[0x28].offset_15_00 == NULL) && 
			(idt[0x28].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}

	if ((idt[0x80].offset_15_00 == NULL) && 
			(idt[0x80].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}

	return result;
}



/* page fault test
 * 
 * try to access unallocated memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int page_fault_test(){ //page fault test
	TEST_HEADER;
	// choose one location and comment the rest out :_(

	//these locations should all page fault
	char* ptr = 0x000000; //0MB
	// *ptr = 0;

	// ptr = (char*) 0x0B7FFF; //one location before 0xB8000 before video mem
	// *ptr = 0;
	
	// ptr = (char*) 0x0B9001; //one location after 0xB9000 after video mem
	// *ptr = 0;

	// ptr = (char*) 0x3FFFFF; //one location before 0x40 0000 before kernel mem
	// *ptr = 0;

	ptr = (char*) 0x800001; // one location after 0x80 0000 after kernel mem
	*ptr = 0;

	return FAIL; //if it gets here = failed
}

/* page access test
 * 
 * try to access accessible mem locations
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int page_access_test(){ 
	TEST_HEADER;

	int i;
	char * ptr = (char*) 0x0B8000; 
	for(i = 0; i < 1024; i++){  //access video memory test
		*(ptr + i) = 0;
	}

	ptr = (char*) 0x400000;
	char read_var;
	for(i = 0; i < 0x400000; i++){  //access kernel memory test
		read_var = *(ptr + i);
	}

	return PASS; //if it gets here = passed
}

/* divide by zero
 * 
 * try to divide number by 0
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int divide_zero_test(){
	TEST_HEADER;
	// printf("Handling Divide by 0 Test..\n");
	// int j = 0;
	// int i = 10 /j;
	asm volatile("int $0");
	return FAIL;
}

/* bound error test 
 * 
 * try to raise bound error 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int bound_error_test(){
	TEST_HEADER;
	// printf("Handling Divide by 0 Test..\n");
	// int j = 0;
	// int i = 10 /j;
	asm volatile("int $5");
	return FAIL;
}

/* invalid opcode test
 * 
 * try to move something into a register that doesnt exist
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */

int invalid_opcode_test(){
	TEST_HEADER;
	asm volatile("mov %cr6, %eax");
	//if it reaches this point its 0
	return FAIL;
}


/* Checkpoint 2 tests */

/* text_file_read()
 * 
 * test opening, reading and closing text file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int text_file_read(){
	TEST_HEADER;
	// frame0.txt  frame1.txt verylargetextwithverylongname.txt 
	//d_entry fish_dentry;
	d_entry* fish_dentry = get_cp2_dentry_address();
	//file_open((uint8_t*)"frame0.txt", &fish_dentry);
	int32_t open_result = file_open((uint8_t*)"frame0.txt");
	if(open_result == -1){
		return FAIL;
	}
	uint32_t file_length = get_file_length(fish_dentry->inode_num);
	uint8_t buf[file_length+1];
	int32_t bytes_read = file_read(fish_dentry->inode_num, buf, file_length);
	bytes_read = bytes_read; // doing nothing
	//printf("file read call done, num_bytes_read: %d \n", bytes_read);
	//printf("%s", buf);
	int i;
	for(i = 0; i< file_length ; i++){
		printf("%c", buf[i]);
	}

	return PASS;
}

/* non_text_file_read()
 * 
 * test opening, reading and closing non text file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int non_text_file_read(){
	TEST_HEADER;
	// cat counter fish grep hello ls pingpong shell syserr sigtest testprint 
	//d_entry dentry;
	d_entry* dentry = get_cp2_dentry_address();
	file_open((uint8_t*)"grep");
	uint32_t file_length = get_file_length(dentry->inode_num);
	uint8_t buf[file_length];
	int32_t bytes_read = file_read(dentry->inode_num, buf, file_length);
	bytes_read = bytes_read; // doing nothing
	//printf("file read call done, num_bytes_read: %d \n", bytes_read);
	printf("the first few bytes: ");
	int i;
	for(i = 0; i< 5 ; i++){
		printf("%c", buf[i]);
		//use terminal write instead of printf();
	}
	printf("\nthe last few bytes: ");
	for(i = file_length-50; i<file_length; i++){
		printf("%c", buf[i]);
	}
	printf("\n ");
	return PASS;
}

/* length_33_filename_test()
 * 
 * tests opening long name file (over 32 characters)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int length_33_filename_test(){
	TEST_HEADER;

	//d_entry dentry;
	if(-1 == file_open((uint8_t*)"verylargetextwithverylongname.txt")){
		return PASS;
	}
	return FAIL;

}

/* text_file_read()
 * 
 * test opening files with less than 32 characters
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int length_32_filename_test(){
	TEST_HEADER;

	//d_entry dentry;
	if(0 == file_open((uint8_t*)"verylargetextwithverylongname.tx")){
		
		return PASS;
	}
	return FAIL;
	
}

/* dir_read_test()
 * 
 * test opening files with less than 32 characters
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int dir_read_test(){
	TEST_HEADER;
	
	uint8_t buf[MAX_FILE_LENGTH_BYTES];
	while(0 != dir_read(0, buf, MAX_FILE_LENGTH_BYTES)){
		int i;
		for(i = 0; i<MAX_FILE_LENGTH_BYTES; i++){
			if(buf[i] != 0){
				printf("%c", buf[i]);
			}
			// printf("%c", buf[i]);
			buf[i] = 0; //reset the buffer after printing 
		}
		printf(" \n");
	}

	return PASS;
}

/* read_data_test()
 * 
 * test opening files with less than 32 characters
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int read_data_test(){
	TEST_HEADER;
	d_entry* dentry = get_cp2_dentry_address();
	file_open((uint8_t*)"frame0.txt");

	uint32_t file_length = get_file_length(dentry->inode_num);
	uint8_t buf[file_length];
	int32_t bytes_read = read_data(dentry->inode_num, 16,buf, 4);
	//printf("read data call done, num_bytes_read: %d \n", bytes_read);

	int i;
	for(i = 0; i< bytes_read ; i++){
		printf("%c", buf[i]);
	}
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(test_t test_num){
	//int* result;
	switch (test_num)
	{
	case IDT_TEST:
		
		TEST_OUTPUT("idt_test", idt_test());
		break;

	case PAGE_FAULT_TEST:
		TEST_OUTPUT("page_fault_test", page_fault_test());
		break;

	case PAGE_ACCESS_TEST:
		TEST_OUTPUT("page_access_test", page_access_test());
		break;
	
	case DIVIDE_ZERO_TEST:
		TEST_OUTPUT("divide_zero_test", divide_zero_test());
		break;
	case MULT_EXCEPTIONS_TEST:
		// divide_zero_test();
		// bound_error_test();
		// invalid_opcode_test();
		break;
	case TEXT_FILE_READ_TEST:
		TEST_OUTPUT("text_file_read_test", text_file_read());
		break;
	
	case NON_TEXT_FILE_READ_TEST:
		TEST_OUTPUT("non_text_file_read_test", non_text_file_read());
		break;
	
	case LENGTH_33_FILENAME_TEST:
		TEST_OUTPUT("length_33_filename_test", length_33_filename_test());
		break;
	
	case LENGTH_32_FILENAME_TEST:
		TEST_OUTPUT("length_32_filename_test", length_32_filename_test());
		break;
	
	case DIR_READ_TEST:
		TEST_OUTPUT("dir_read_test", dir_read_test());
		break;
	
	case READ_DATA_TEST:
		TEST_OUTPUT("read_data_test", read_data_test());
		break;
	

	default:
		printf("bad test number\n");
		break;
	}

	//TEST_OUTPUT("divide zero", divide_zero_test());
	// int i;
	// for(i = 0; ; i++) {
	// 	printf("%d\n", i);
	// }
	// while(1);
	// launch your tests here
}
