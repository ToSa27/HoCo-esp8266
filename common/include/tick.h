#ifndef __HOCO_TICK_H__
#define __HOCO_TICK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

extern unsigned long mymillis();
unsigned long mymicros();

void delay(uint32_t ms);
#define delayMilliseconds(ms) delay(ms)
#define delayMicroseconds(us) ets_delay_us(us)

#ifdef __cplusplus
}
#endif

#endif
