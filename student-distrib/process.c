#include "process.h"

uint8_t processes_open = 0x00;
uint8_t curr_process = 0x00; //ranges from 0 to 7, but only need 0-5 for mp3. BUT, only need 0-1 for mp3.3

