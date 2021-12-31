#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

extern volatile uint8_t pollingFlag;

uint64_t millis();

void timerInit();

void timerStart();

void timerStop();

#endif
