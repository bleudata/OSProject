#ifndef TESTS_H
#define TESTS_H

typedef enum {
	IDT_TEST,
	PAGE_FAULT_TEST,
    PAGE_ACCESS_TEST,
	DIVIDE_ZERO_TEST,
	MULT_EXCEPTIONS_TEST,
	RTC_OPEN,
	RTC_NEW_HZ,
	RTC_HZ_TOO_BIG,
	RTC_HZ_POWER_TWO,
	RTC_HZ_BUFF_OF,
	RTC_TEST_READ
} test_t;

// test launcher
void launch_tests(test_t test_num);

#endif /* TESTS_H */
