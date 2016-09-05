#ifndef __HOCO_ARDUINO_H__
#define __HOCO_ARDUINO_H__

#include <esp8266.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <ets_sys.h>
#include <osapi.h>
#include <pin.h>
#include <tick.h>

#ifdef __cplusplus
}
#endif

#define ESP8266_REG(addr) *((volatile uint32_t *)(0x60000000+(addr)))
#define GPO    ESP8266_REG(0x300) //GPIO_OUT R/W (Output Level)
#define GPOS   ESP8266_REG(0x304) //GPIO_OUT_SET WO
#define GPOC   ESP8266_REG(0x308) //GPIO_OUT_CLR WO
#define GPE    ESP8266_REG(0x30C) //GPIO_ENABLE R/W (Enable)
#define GPI    ESP8266_REG(0x318) //GPIO_IN RO (Read Input Level)

#define PROGMEM     ICACHE_RODATA_ATTR
#define PGM_P  		const char *
#define PGM_VOID_P  const void *
#define PSTR(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))

#define pgm_read_byte(addr) 		                                           \
(__extension__({                                                               \
    PGM_P __local = (PGM_P)(addr);  /* isolate varible for macro expansion */         \
    ptrdiff_t __offset = ((uint32_t)__local & 0x00000003); /* byte aligned mask */            \
    const uint32_t* __addr32 = (const uint32_t*)((const uint8_t*)(__local)-__offset); \
    uint8_t __result = ((*__addr32) >> (__offset * 8));                        \
    __result;                                                                  \
}))

#define pgm_read_word(addr) 		                                           \
(__extension__({                                                               \
    PGM_P __local = (PGM_P)(addr); /* isolate varible for macro expansion */          \
    ptrdiff_t __offset = ((uint32_t)__local & 0x00000002);   /* word aligned mask */          \
    const uint32_t* __addr32 = (const uint32_t*)((const uint8_t*)(__local) - __offset); \
    uint16_t __result = ((*__addr32) >> (__offset * 8));                       \
    __result;                                                                  \
}))

#endif
