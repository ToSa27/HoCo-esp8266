#ifndef FACTORY_USER_INCLUDE_USER_CPP_H_
#define FACTORY_USER_INCLUDE_USER_CPP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>
#include <mem.h>

//#define os_malloc   pvPortMalloc
//#define os_free     vPortFree
//#define os_zalloc   pvPortZalloc

#ifdef __cplusplus
}//extern "C"
#endif

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

void do_global_ctors(void);

#endif
