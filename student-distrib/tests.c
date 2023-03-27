#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal_driver.h"
#include "keyboard_driver.h"
#include "rtc.h"

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
	return FAIL;
}


/* Checkpoint 2 tests */

/* terminal_open_test
 * 
 * try to open terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int terminal_open_test() {
	TEST_HEADER;
	int result; 
	result = terminal_open(0); // 0 is dummy value since param doesn't actually get used
	return result ? PASS: FAIL;
}

/* terminal_close_test
 * 
 * try to open terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int terminal_close_test() {
	TEST_HEADER;
	int result; 
	result = terminal_close(0); 
	return result ? PASS: FAIL;
}

/* terminal_read_test
 * 
 * try to read terminal
 * Compares the keyboard buffer to the buffer that we fill in read
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int terminal_read_test() {
	TEST_HEADER;
	int result;
	unsigned char allocated_buf[128];
	int i;

	/* Check invalid inputs */
	result = terminal_read(0, NULL, 128);
	if (result >= 0) {
		printf("Didnt Check for Null Buf arg.");
		return FAIL;
	}

	result = terminal_read(-1, allocated_buf, 128);
	if (result >= 0) {
		printf("Didnt Check for Invlaid File Descriptor.");
		return FAIL;
	}

	result = terminal_read(0, allocated_buf, -1);
	if (result >= 0) {
		printf("Didnt Check for Invlaid Byte Number.");
		return FAIL;
	}

	/* Check If Buf is filled correctly */
	result = terminal_read(0, allocated_buf, 128);
	if (result < 0) {
		printf("Failed to copy buffer correctly.");
		return FAIL;
	}
	
	unsigned char* keyboard = get_keyboard_buffer();
	for (i = 0; i < 128; i++, keyboard++) {
		if (*keyboard != allocated_buf[i]) {
			printf("Failed to copy buffer correctly");
			return FAIL;
		}
	}
	return PASS;
}

/* terminal_write_test
 * 
 * try to write to the terminal and test bad inputs 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int terminal_write_test() {
	TEST_HEADER;
	int result;
	unsigned char allocated_buf[128] = " terminal write test "; 

	/* Check invalid inputs */
	result = terminal_write(1, NULL, 128);
	if (result >= 0) {
		printf("Didnt Check for Null Buf.");
		return FAIL;
	}
	result = terminal_write(-1, allocated_buf, 128);
	if (result >= 0) {
		printf("Didnt Check for Invalid File Descriptor.");
		return FAIL;
	}

	result = terminal_write(1, allocated_buf, -1);
	if (result >= 0) {
		printf("Didnt Check for Invalid Byte Number.");
		return FAIL;
	}

	/* Should check If it actually writes to screen */
	result = terminal_write(1, allocated_buf, 127); 
	if (result < 0) {
		return FAIL;
	}

	return PASS;
}



/* rtc_open_no_errors
 * 
 * Description: Tests to make sure that user RTC init function works fine with no errors
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int rtc_open_no_errors(){
	int result = rtc_open();
	if(result == 0){
		return PASS;
	}
	else{
		return FAIL;
	}
}

/* rtc_test_reading_freq
 * 
 * Description: Reads from rtc_read(), which returns at every tick from the RTC from the user perspective 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int rtc_test_reading_freq(){
	int result = rtc_read();
	if(result == 0){
		return PASS;
	}
	else{
		return FAIL;
	}
}

/* rtc_test_changing_freq
 * 
 * Description: Tests to see that changing the frequency returns no errors
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: "Changes" the frequency of RTC from the user perspective
 */
int rtc_test_changing_freq(){
	rtc_open();
	uint8_t buffer[4] = {0x00, 0x00, 0x00, 0x02};
	int result_write = rtc_write(buffer, 4);

	int ctr = 0;
	while(ctr < 10){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}
	
	//setting Hz to 4
	buffer[3] = 0x04;
	result_write = rtc_write(buffer, 4);
	ctr = 0;
	while(ctr < 10){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}
	
	//setting Hz to 8
	buffer[3] = 0x08;
	result_write = rtc_write(buffer, 4);
	ctr = 0;
	while(ctr < 20){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}

	//setting Hz to 32
	buffer[3] = 0x20;
	result_write = rtc_write(buffer, 4);
	ctr = 0;
	while(ctr < 40){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}


	//setting Hz to 128
	buffer[3] = 0x80;
	result_write = rtc_write(buffer, 4);
	ctr = 0;
	printf("\n");
	while(ctr < 321){
		printf("%x", rtc_test_reading_freq());
		if(ctr % 80 == 0 && (ctr != 0 && ctr != 320)){
			printf("\n");
		}
		ctr++;
	}

	//setting to 512
	buffer[3] = 0x00;
	buffer[2] = 0x02;
	result_write = rtc_write(buffer, 4);
	ctr = 0;
	printf("\n");
	while(ctr < 321){
		printf("%x", rtc_test_reading_freq());
		if(ctr % 80 == 0 && (ctr != 0 && ctr != 320)){
			printf("\n");
		}
		ctr++;
	}

	//setting to 1024
	buffer[2] = 0x04;
	result_write = rtc_write(buffer, 4);
	ctr = 0;
	printf("\n");
	while(ctr < 641){
		printf("%x", rtc_test_reading_freq());
		if(ctr % 80 == 0 && (ctr != 0)){
			printf("\n");
		}
		ctr++;
	}
	return PASS;
}

/* rtc_test_big_HZ
 * 
 * Description: Tests that too big of a frequency is not written into the RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int rtc_test_big_HZ(){
	rtc_open();
	uint8_t buffer[4] = {0x80, 0x00, 0x00, 0x00};
	int result = rtc_write(buffer, 4);
	if(result == 0){
		return PASS;
	}
	else{
		return FAIL;
	}
}

/* rtc_test_power_two
 * 
 * Description: Tests that a non-power of 2 number is not written into the RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int rtc_test_power_two(){
	rtc_open();
	uint8_t buffer[4] = {0x00, 0x00, 0x75, 0x13};
	int result = rtc_write(buffer, 4);
	if(result == 0){
		return PASS;
	}
	else{
		return FAIL;
	}
}

/* rtc_test_buff_overflow
 * 
 * Description: Tests that only a buffer of size 4 is processed into the RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */

int rtc_test_buff_overflow(){
	rtc_open();
	uint8_t buffer[7] = {0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00};
	int result = rtc_write(buffer, 7);
	if(result == 0){
		return PASS;
	}
	else{
		return FAIL;
	}
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
	case TERMINAL_TEST:
		terminal_open_test();
		terminal_read_test();
		terminal_write_test();
		terminal_close_test();
		break;
	case RTC_OPEN:
		TEST_OUTPUT("rtc_open works", rtc_open_no_errors());
		break;
	case RTC_NEW_HZ:
		TEST_OUTPUT("rtc_write works", rtc_test_changing_freq());
		break;
	case RTC_HZ_TOO_BIG:
		TEST_OUTPUT("rtc_test_big_HZ, should fail if requested frequency is too big ( > 1024)", rtc_test_big_HZ());
		break;
	case RTC_TEST_READ:
		TEST_OUTPUT("rtc_read test intervals", 1);
		while(1){
			printf("%x", rtc_read());
		}
		break;
	case RTC_HZ_POWER_TWO:
		TEST_OUTPUT("rtc_test_power_two, should fail if requested frequency is not a power of 2", rtc_test_power_two());
		break;
	case RTC_HZ_BUFF_OF:
		TEST_OUTPUT("rtc_test_buff_overflow, FAIL if overflow", rtc_test_buff_overflow());
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
