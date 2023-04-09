#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesystem.h"
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

	file_close(0);

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

	file_close(0);

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
	if (result != 0) {
		return FAIL;
	}
	return PASS;
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
	if (result != 0) {
		return FAIL;
	}
	return PASS;
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
	int i;
	unsigned char allocated_buf[200];
	
	// /* Check invalid inputs */
	printf("test 1\n");
	result = terminal_read(0, NULL, 128);
	if (result >= 0) {
		printf("Didnt Check for Null Buf arg.");
		return FAIL;
	}
	printf("test2\n");
	result = terminal_read(-1, allocated_buf, 128);
	if (result >= 0) {
		printf("Didnt Check for Invlaid File Descriptor.");
		return FAIL;
	}

	printf("test3\n");
	result = terminal_read(0, allocated_buf, -1);
	if (result >= 0) {
		printf("Didnt Check for Invlaid Byte Number.");
		return FAIL;
	}

	/* Check If Buf is filled correctly */
	// all we did was check if the buffers are the same without the enters
	// clear out the buffer passed in for the test case to prevent random stuff from getting printed to the screen
	memset(allocated_buf, '\0', 200); 
	
	printf("finished tests\n");
	// test to hold mutliple enters "ece\n391\n"
	printf("Testing Reading and Writing Keyboard. \n");
	for (i = 0; i < 1000000000; i ++) ;
	printf("starting read \n");
	result = terminal_read(0, allocated_buf, 200);
	if (result == -1) 
		return FAIL;
	printf("starting write \n");
	result = terminal_write(1, allocated_buf, 200);
	if (result == -1) 
		return FAIL;
	printf("\nstarting read \n");
	result = terminal_read(0, allocated_buf, 200);
	if (result == -1) 
		return FAIL;
	printf("starting write \n");
	result = terminal_write(1, allocated_buf, 200);
	if (result == -1) 
		return FAIL;
	printf("done\n");

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
	unsigned char allocated_buf[128] = "this is the terminal write test \n"; 
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
	result = terminal_write(1, allocated_buf, 128); // 33 is length of non-null portion of allocated buf
	if (result < 0) {
		return FAIL;
	}

	// test cases from demo
	// terminal_write(1, "aaa", 2);
	// terminal_write(1, "aaa", 4);
	// terminal_write(1, "aaa", -1);

	return PASS;
}

static int32_t file = 0;
static uint8_t* filename = 0;
static uint8_t* buff [4];

/* rtc_open_no_errors
 * 
 * Description: Tests to make sure that user RTC init function works fine with no errors
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int rtc_open_no_errors(){
	int result = rtc_open(filename);
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
	int result = rtc_read(file, buff, 4);
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
	rtc_open(filename);
	uint8_t buffer[4] = {0x00, 0x00, 0x00, 0x02};
	int result_write = rtc_write(file, buffer, 4);

	int ctr = 0;
	while(ctr < 10){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}
	
	//setting Hz to 4
	buffer[3] = 0x04;
	result_write = rtc_write(file, buffer, 4);
	ctr = 0;
	while(ctr < 10){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}
	
	//setting Hz to 8
	buffer[3] = 0x08;
	result_write = rtc_write(file, buffer, 4);
	ctr = 0;
	while(ctr < 20){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}

	//setting Hz to 32
	buffer[3] = 0x20;
	result_write = rtc_write(file, buffer, 4);
	ctr = 0;
	while(ctr < 40){
		printf("%x", rtc_test_reading_freq());
		ctr++;
	}


	//setting Hz to 128
	buffer[3] = 0x80;
	result_write = rtc_write(file, buffer, 4);
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
	result_write = rtc_write(file, buffer, 4);
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
	result_write = rtc_write(file, buffer, 4);
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
	rtc_open(filename);
	uint8_t buffer[4] = {0x80, 0x00, 0x00, 0x00};
	int result = rtc_write(file, buffer, 4);
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
	rtc_open(filename);
	uint8_t buffer[4] = {0x00, 0x00, 0x75, 0x13};
	int result = rtc_write(file, buffer, 4);
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
	rtc_open(filename);
	uint8_t buffer[7] = {0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00};
	int result = rtc_write(file, buffer, 7);
	if(result == 0){
		return PASS;
	}
	else{
		return FAIL;
	}
}

/* Checkpoint 3 tests */


int syscall_open_input(){
    uint8_t* filename = NULL;
    int ret_val = open(filename);
    if(ret_val == -1){
        return PASS;
    }
    else{
        return FAIL;
    }
}


int syscall_close_input(){
    int fd = 1;
    int ret_val = close(fd);
    if(ret_val == -1){
        return PASS;
    }
    return FAIL;
}


int syscall_read_input(){
    int fd = 8;
    uint8_t* filename = NULL;
    int ret_val = read(fd, filename, 0);
    if(ret_val == -1){
        return PASS;
    }
    return FAIL;
}


int syscall_write_input(){
    int fd = 8;
    uint8_t data[10];
    int ret_val = write(fd, data, 10);
    if(ret_val == -1){
        return PASS;
    }
    return FAIL;
}


int syscall_execute_input(){
    uint8_t* command = NULL;
    int ret_val = execute(command);
    if(ret_val == -1){
        return PASS;
    }
    return FAIL;
}


int syscall_halt_input(){
    int status = 0;
    int ret_val = halt(status);
    if(ret_val == -1){
        return PASS;
    }
    return FAIL;
}

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
	case TERMINAL_OPEN_TEST:
		TEST_OUTPUT("terminal_open_test", terminal_open_test());
		break;
	case TERMINAL_CLOSE_TEST:
		TEST_OUTPUT("terminal_close_test", terminal_close_test());
		break;
	case TERMINAL_READ_TEST:
		TEST_OUTPUT("terminal_read_test", terminal_read_test());
		break;
	case TERMINAL_WRITE_TEST:
		TEST_OUTPUT("terminal_write_test", terminal_write_test());
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
		break;
	case RTC_HZ_POWER_TWO:
		TEST_OUTPUT("rtc_test_power_two, should fail if requested frequency is not a power of 2", rtc_test_power_two());
		break;
	case RTC_HZ_BUFF_OF:
		TEST_OUTPUT("rtc_test_buff_overflow, FAIL if overflow", rtc_test_buff_overflow());
		break;
	case SYSCALL_OPEN_INP:
        TEST_OUTPUT("bad open input, PASS if handled correctly", syscall_open_input());
        break;
    case SYSCALL_CLOSE_INP:
        TEST_OUTPUT("bad close input, PASS if handled correctly", syscall_close_input());
        break;
    case SYSCALL_READ_INP:
        TEST_OUTPUT("bad read input, PASS if handled correctly", syscall_read_input());
        break;
    case SYSCALL_WRITE_INP:
        TEST_OUTPUT("bad write input, PASS if handled correctly", syscall_write_input());
        break;
    case SYSCALL_EXECUTE_INP:
        TEST_OUTPUT("bad execute input, PASS if handled correctly", syscall_execute_input());
        break;
    case SYSCALL_HALT_INP:
        TEST_OUTPUT("bad halt input, PASS if handled correctly", syscall_halt_input());
        break;
	default:
		printf("bad test number\n");
		break;
	}

}
