#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "syscalls.h"
#include "scheduling.h"
#include "paging.h"

#define RTC_REG_PORT        0x70
#define RTC_RW_PORT         0x71
#define RTC_RATE            0x06

#define RTC_REG_A           0x0A
#define RTC_REG_A_DISABLE   0x8A
#define RTC_REG_B           0x0B
#define RTC_REG_B_DISABLE   0x8B   
#define RTC_REG_C           0x0C
#define RTC_REG_C_DISABLE   0x8C

#define RTC_PER_IRQ         0x40
#define RTC_GLOB_RES_VAR    0
#define RTC_GLOB_RES_RATE   1
#define RTC_FLAG_SET        1

#define RTC_USR_DEFAULT     512

#define RTC_DATA_UPPER_BYTE 0xf0

#define RTC_BUFF_SIZE       4
#define RTC_BYTE_SHIFT      8
#define RTC_BUFF_LAST_BYTE  3
#define RTC_POW_2_DECR      1

#define RTC_PASS            0
#define RTC_FAIL            -1

#define RTC_HZ              1024
#define RTC_HZ_MIN          2

//initialization function of rtc
extern void rtc_init();

//rtc interrupt handler
extern void rtc_irq_handler();

//user rtc initialization function
extern int32_t rtc_open(const uint8_t* filename);

//user rtc closing function
extern int32_t rtc_close(int32_t fd);

//user rtc read function that handles end of interrupts from user perspective
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

//user rtc write function to set the rtc
extern int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes);

#endif /*RTC_H*/
