//////////////////////////////////////////////////
//
// Based on:
// rBoot open source boot loader for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
//
// Adjusted for HoCo needs:
// Copyright 2016 Tobias Hoff
// tobias@die-hoffs.net
//
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifndef __HOCO_BOOTLOADER_H__
#define __HOCO_BOOTLOADER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __HOCO_ESP8266_H__
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef int int32;
#define TRUE 1
#define FALSE 0
#endif

#define CHKSUM_INIT			0xef
#define SECTOR_SIZE			0x1000

#define BOOT_CONFIG_MAGIC	0x2334ae68

#define BOOT_RTC_MAGIC		0x2334ae68

typedef struct {
	char type[16];
	uint8 major;
	uint8 minor;
	uint16 build;
} rom_details;

typedef struct {
	uint32 magic;
	rom_details roms[4];	// 0->bootloader, 1..3->slots
	uint8 chksum;
} bootloader_config;

typedef struct {
	uint32 magic;
	uint8 current_rom;
	uint8 temp_rom;			// 0: no temp boot (default), 1..3: boot (once) to the specific rom slot
	uint8 chksum;
} bootloader_status;

static uint32 slotaddr(uint8 slot) {
	if (slot == 0)
		return 0;
	return ((slot - 1) * 0x100000) + 0x1000;
}

static uint8 calc_chksum(uint8 *start, uint8 *end) {
	uint8 chksum = CHKSUM_INIT;
	while(start < end) {
		chksum ^= *start;
		start++;
	}
	return chksum;
}

extern int ets_strcmp(const char *s1, const char *s2);

static uint8 find_latest(bootloader_config romconf, char *type) {
	uint8 latest = 0;
	for (uint8 slot = 1; slot < 4; slot++) {
		if (ets_strcmp(romconf.roms[slot].type, type) == 0) {
			if (latest == 0)
				latest = slot;
			else {
				if (romconf.roms[slot].major > romconf.roms[latest].major)
					latest = slot;
				else if (romconf.roms[slot].major == romconf.roms[latest].major) {
					if (romconf.roms[slot].minor > romconf.roms[latest].minor)
						latest = slot;
					else if (romconf.roms[slot].minor == romconf.roms[latest].minor) {
						if (romconf.roms[slot].build > romconf.roms[latest].build)
							latest = slot;
					}
				}
			}
		}
	}
	return latest;
}

#ifdef __cplusplus
}
#endif

#endif
