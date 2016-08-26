#ifndef __FACTORY_USER_HTTPD_REQUESTS_H__
#define __FACTORY_USER_HTTPD_REQUESTS_H__

#include <esp8266.h>

int ICACHE_FLASH_ATTR httpd_handle_request(char *reqdata, int reqlen, char *resdata, int reslen);

#endif
