#ifndef __FACTORY_USER_HTTPD_H__
#define __FACTORY_USER_HTTPD_H__

#include <esp8266.h>

//int ICACHE_FLASH_ATTR handleRequest(char *reqdata, int reqlen, char *resdata, int reslen);
void ICACHE_FLASH_ATTR httpd_init(bool capt);

#endif
