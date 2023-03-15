#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

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
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int page_fault_test(){ //page fault test
	TEST_HEADER;
	// choose one location and comment the rest out :_(

	//these locations should all page fault
	char* ptr = 0x000000; //0MB
	*ptr = 0;

	ptr = (char*) 0x0B7FFF; //one location before 0xB8000 before video mem
	*ptr = 0;
	
	ptr = (char*) 0x0B9001; //one location after 0xB9000 after video mem
	*ptr = 0;

	ptr = (char*) 0x3FFFFF; //one location before 0x40 0000 before kernel mem
	*ptr = 0;

	ptr = (char*) 0x800001; // one location after 0x80 0000 after kernel mem
	*ptr = 0;

	return FAIL; //if it gets here = failed
}

/* page access test
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
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

int divide_zero_test(){
	TEST_HEADER;
	int j = 0;
	int i = 10 /j;
	return i ? PASS: FAIL;
}

// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(test_t test_num){
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
