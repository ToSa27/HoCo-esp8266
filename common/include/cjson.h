#ifndef __HOCO_CJSON_H__
#define __HOCO_CJSON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

char * ICACHE_FLASH_ATTR JsonGet(char *data, char *name);
uint32 ICACHE_FLASH_ATTR JsonGetUInt(char *data, char *name);
void ICACHE_FLASH_ATTR JsonGetStr(char *data, char *name, char *dest, int len);
void ICACHE_FLASH_ATTR JsonGetIP(char *data, char *name, uint8 *dest);

#ifdef __cplusplus
}
#endif

#endif
