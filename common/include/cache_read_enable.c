#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>
#include <bootloader.h>

// plain sdk defaults to iram
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

extern void Cache_Read_Enable(uint32, uint32, uint32);

uint8 mmap_1 = 0xff;
uint8 mmap_2 = 0xff;

void IRAM_ATTR Cache_Read_Enable_New(void) {
	if (mmap_1 == 0xff) {
		uint32 val;
		bootloader_status rtc;
		uint8 off = (uint8*)&rtc.current_rom - (uint8*)&rtc;
		volatile uint32 *rtcd = (uint32*)(0x60001100 + (RTCADDR_BOOT*4) + (off & ~3));
		val = *rtcd;
		val = ((uint8*)&val)[off & 3];
		val = ((val - 1) * 0x100000) + 0x1000;
		val /= 0x100000;
		mmap_2 = val / 2;
		mmap_1 = val % 2;
		ets_printf("cre map %d / %d\r\n", mmap_1, mmap_2);
	}
	Cache_Read_Enable(mmap_1, mmap_2, 1);
}

#ifdef __cplusplus
}
#endif
