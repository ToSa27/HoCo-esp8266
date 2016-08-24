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

#ifndef __HOCO_BOOTLOADER_PRIVATE_H__
#define __HOCO_BOOTLOADER_PRIVATE_H__

#include <bootloader.h>

#define ROM_TYPE		"BOOTLOADER"
#ifndef VERSION_MAJOR
#define VERSION_MAJOR	0
#endif
#ifndef VERSION_MINOR
#define VERSION_MINOR	0
#endif
#ifndef VERSION_BUILD
#define VERSION_BUILD	0
#endif

#define NOINLINE __attribute__ ((noinline))

#define ROM_MAGIC	   	0xe9
#define ROM_MAGIC_NEW1 	0xea
#define ROM_MAGIC_NEW2 	0x04

#define RTC_READ		1
#define RTC_WRITE		0

#define BUFFER_SIZE		0x100

extern uint32 SPIRead(uint32 addr, void *outptr, uint32 len);
extern uint32 SPIEraseSector(int);
extern uint32 SPIWrite(uint32 addr, void *inptr, uint32 len);
extern void ets_printf(char*, ...);

extern void ets_delay_us(int);
extern void ets_memset(void*, uint8, uint32);
extern void ets_memcpy(void*, const void*, uint32);
extern int ets_strcmp(const char *s1, const char *s2);
extern char *ets_strcpy(char *dest, const char *src);

typedef void stage(uint32);
typedef void usercode(void);

// standard rom header
typedef struct {
	// general rom header
	uint8 magic;
	uint8 count;
	uint8 flags1;
	uint8 flags2;
	usercode* entry;
} rom_header;

typedef struct {
	uint8* address;
	uint32 length;
} section_header;

// new rom header (irom section first) there is
// another 8 byte header straight afterward the
// standard header
typedef struct {
	// general rom header
	uint8 magic;
	uint8 count; // second magic for new header
	uint8 flags1;
	uint8 flags2;
	uint32 entry;
	// new type rom, lib header
	uint32 add; // zero
	uint32 len; // length of irom section
} rom_header_new;

#endif
