#ifndef __HOCO_HELPER_H__
#define __HOCO_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

int ICACHE_FLASH_ATTR ltoa(long val, char* buffer, int base);
char ICACHE_FLASH_ATTR *dtostrf(double floatVar, int numDigitsAfterDecimal, char *outputBuffer);
double ICACHE_FLASH_ATTR myatof(const char* s);

#ifdef __cplusplus
}
#endif

#endif
