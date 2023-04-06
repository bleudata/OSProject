#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "lib.h"
#include "filesystem.h"
#include "rtc.h"
#include "paging.h"

// MOVE TO CORRECT PLACE
// PCB STRUCTURES

extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);


#endif /*SYSCALLS_H*/