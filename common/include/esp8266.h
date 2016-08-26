#ifndef __HOCO_ESP8266_H__
#define __HOCO_ESP8266_H__

#ifdef ICACHE_IRAM
#define ICACHE_IRAM_ATTR __attribute__((section(".text")))
#else
#define ICACHE_IRAM_ATTR
#endif /* ICACHE_IRAM */

#ifdef __cplusplus
extern "C" {
#endif

#define SECTOR_SIZE		0x1000

#include <stddef.h>
#include <ctype.h>
#include <c_types.h>
#include <ets_sys.h>

#define bool			uint8
typedef bool 			BOOL;

extern void *pvPortMalloc(size_t xWantedSize, const char *file, int line);
extern void *pvPortZalloc(size_t, const char *file, int line);
extern void vPortFree(void *ptr, const char *file, int line);
extern void *vPortMalloc(size_t xWantedSize, const char *file, int line);
extern void pvPortFree(void *ptr, const char *file, int line);

extern int atoi(const char *nptr);

extern void ets_delay_us(uint32_t us);

extern void ets_timer_arm_new(ETSTimer *a, int b, int c, int isMstimer);
extern void ets_timer_disarm(ETSTimer *a);
extern void ets_timer_setfn(ETSTimer *t, ETSTimerFunc *fn, void *parg);

extern void *ets_memset(void *s, int c, size_t n);
extern void *ets_memcpy(void *dest, const void *src, size_t n);

extern size_t ets_strlen(const char *s);
extern int ets_strcmp(const char *s1, const char *s2);
extern int ets_strncmp(const char *s1, const char *s2, int len);
extern char *ets_strcpy(char *dest, const char *src);
extern char *ets_strncpy(char *dest, const char *src, size_t n);
extern char *ets_strstr(const char *haystack, const char *needle);
extern void ets_printf(const char*, ...);
extern int ets_sprintf(char *str, const char *format, ...)  __attribute__ ((format (printf, 2, 3)));

void ets_isr_attach(int intr, void *handler, void *arg);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);
extern void ets_intr_lock();
extern void ets_intr_unlock();

int base64_decode(size_t in_len, const char *in, size_t out_len, unsigned char *out);
int base64_encode(size_t in_len, const unsigned char *in, size_t out_len, char *out);

#ifdef __cplusplus
}
#endif

#endif
