#ifndef TESTS_H
#define TESTS_H

typedef enum {
	IDT_TEST,
	PAGE_FAULT_TEST,
    PAGE_ACCESS_TEST,
	DIVIDE_ZERO_TEST
} test_t;

// test launcher
void launch_tests(test_t test_num);

#endif /* TESTS_H */
