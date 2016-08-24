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

#include <bootloader-private.h>
#include <bootloader-hex.h>
#include <hardware.h>

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

// read/write RTC without using SDK functions
static uint32 rtc_mem(int32 addr, void *buff, int32 length, uint8 mode) {
    int32 blocks;
    if (addr < 64)
    	return 0;
    if (buff == 0)
    	return 0;
    if (((uint32)buff & 0x3) != 0)
    	return 0;
    if ((length & 0x3) != 0)
    	return 0;
    if (length > (0x300 - (addr * 4)))
    	return 0;
    for (blocks = (length >> 2) - 1; blocks >= 0; blocks--) {
        volatile uint32 *ram = ((uint32*)buff) + blocks;
        volatile uint32 *rtc = ((uint32*)0x60001100) + addr + blocks;
		if (mode == RTC_WRITE) {
			*rtc = *ram;
		} else {
			*ram = *rtc;
		}
    }
    return 1;
}

static uint32 check_image(uint8 slot) {
	uint32 readpos = slotaddr(slot);

	uint8 buffer[BUFFER_SIZE];
	uint8 sectcount;
	uint8 sectcurrent;
	uint8 *writepos;
	uint8 chksum = CHKSUM_INIT;
	uint32 loop;
	uint32 remaining;
	uint32 romaddr;

	rom_header_new *header = (rom_header_new*)buffer;
	section_header *section = (section_header*)buffer;

	if (readpos == 0 || readpos == 0xffffffff) {
		return 0;
	}

	// read rom header
	if (SPIRead(readpos, header, sizeof(rom_header_new)) != 0) {
		return 0;
	}

	// check header type
	if (header->magic == ROM_MAGIC) {
		// old type, no extra header or irom section to skip over
		romaddr = readpos;
		readpos += sizeof(rom_header);
		sectcount = header->count;
	} else if (header->magic == ROM_MAGIC_NEW1 && header->count == ROM_MAGIC_NEW2) {
		// new type, has extra header and irom section first
		romaddr = readpos + header->len + sizeof(rom_header_new);
		// we will set the real section count later, when we read the header
		sectcount = 0xff;
		// just skip the first part of the header
		// rest is processed for the chksum
		readpos += sizeof(rom_header);
	} else {
		return 0;
	}

	// test each section
	for (sectcurrent = 0; sectcurrent < sectcount; sectcurrent++) {

		// read section header
		if (SPIRead(readpos, section, sizeof(section_header)) != 0) {
			return 0;
		}
		readpos += sizeof(section_header);

		// get section address and length
		writepos = section->address;
		remaining = section->length;

		while (remaining > 0) {
			// work out how much to read, up to BUFFER_SIZE
			uint32 readlen = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
			// read the block
			if (SPIRead(readpos, buffer, readlen) != 0) {
				return 0;
			}
			// increment next read and write positions
			readpos += readlen;
			writepos += readlen;
			// decrement remaining count
			remaining -= readlen;
			// add to chksum
			for (loop = 0; loop < readlen; loop++) {
				chksum ^= buffer[loop];
			}
		}

		if (sectcount == 0xff) {
			// just processed the irom section, now
			// read the normal header that follows
			if (SPIRead(readpos, header, sizeof(rom_header)) != 0) {
				return 0;
			}
			sectcount = header->count + 1;
			readpos += sizeof(rom_header);
		}
	}

	// round up to next 16 and get checksum
	readpos = readpos | 0x0f;
	if (SPIRead(readpos, buffer, 1) != 0) {
		return 0;
	}

	// compare calculated and stored checksums
	if (buffer[0] != chksum) {
		return 0;
	}

	return romaddr;
}

#if RESET_GPIO_NUM == 16
// sample gpio code for gpio16
#define ETS_UNCACHED_ADDR(addr) (addr)
#define READ_PERI_REG(addr) (*((volatile uint32 *)ETS_UNCACHED_ADDR(addr)))
#define WRITE_PERI_REG(addr, val) (*((volatile uint32 *)ETS_UNCACHED_ADDR(addr))) = (uint32)(val)
#define PERIPHS_RTC_BASEADDR				0x60000700
#define REG_RTC_BASE  PERIPHS_RTC_BASEADDR
#define RTC_GPIO_OUT							(REG_RTC_BASE + 0x068)
#define RTC_GPIO_ENABLE							(REG_RTC_BASE + 0x074)
#define RTC_GPIO_IN_DATA						(REG_RTC_BASE + 0x08C)
#define RTC_GPIO_CONF							(REG_RTC_BASE + 0x090)
#define PAD_XPD_DCDC_CONF						(REG_RTC_BASE + 0x0A0)
static uint8 reset_btn() {
	// set output level to 1
	WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(1));

	// read level
	WRITE_PERI_REG(PAD_XPD_DCDC_CONF, (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1);	// mux configuration for XPD_DCDC and rtc_gpio0 connection
	WRITE_PERI_REG(RTC_GPIO_CONF, (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable
	WRITE_PERI_REG(RTC_GPIO_ENABLE, READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe);	//out disable

	return (uint8)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
}
#endif

#if RESET_GPIO_NUM < 16
// support for "normal" GPIOs (other than 16)
#define REG_GPIO_BASE            0x60000300
#define GPIO_IN_ADDRESS          (REG_GPIO_BASE + 0x18)
#define GPIO_ENABLE_OUT_ADDRESS  (REG_GPIO_BASE + 0x0c)
#define REG_IOMUX_BASE           0x60000800
#define IOMUX_PULLUP_MASK        (1<<7)
#define IOMUX_FUNC_MASK          0x0130
const uint8 IOMUX_REG_OFFS[] = {0x34, 0x18, 0x38, 0x14, 0x3c, 0x40, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x04, 0x08, 0x0c, 0x10};
const uint8 IOMUX_GPIO_FUNC[] = {0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

static uint8 reset_btn() {
	// disable output buffer if set
	uint32 old_out = READ_PERI_REG(GPIO_ENABLE_OUT_ADDRESS);
	uint32 new_out = old_out & ~ (1<<RESET_GPIO_NUM);
	WRITE_PERI_REG(GPIO_ENABLE_OUT_ADDRESS, new_out);

	// set GPIO function, enable soft pullup
	uint32 iomux_reg = REG_IOMUX_BASE + IOMUX_REG_OFFS[RESET_GPIO_NUM];
	uint32 old_iomux = READ_PERI_REG(iomux_reg);
	uint32 gpio_func = IOMUX_GPIO_FUNC[RESET_GPIO_NUM];
	uint32 new_iomux = (old_iomux & ~IOMUX_FUNC_MASK) | gpio_func | IOMUX_PULLUP_MASK;
	WRITE_PERI_REG(iomux_reg, new_iomux);

	// allow soft pullup to take effect if line was floating
	ets_delay_us(10);
	int result = READ_PERI_REG(GPIO_IN_ADDRESS) & (1<<RESET_GPIO_NUM);

	// set iomux & GPIO output mode back to initial values
	WRITE_PERI_REG(iomux_reg, old_iomux);
	WRITE_PERI_REG(GPIO_ENABLE_OUT_ADDRESS, old_out);
	return (result ? 1 : 0);
}
#endif

uint32 __attribute__ ((noinline)) find_image(void) {
	ets_delay_us(2000000);
	ets_printf("\r\nHoCo bootloader v%d.%d-%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);

	if (reset_btn() == 0) {
		ets_printf("Reset to factory defauts...\r\n");
		uint16 sec;
		SPIEraseSector(SECTOR_CONFIG_BOOT);
		for (sec = SECTOR_CONFIG_SDK; sec < SECTOR_CONFIG_SDK + 4; sec++)
			SPIEraseSector(sec);
		for (sec = SECTOR_CONFIG_SDK_CALI; sec < SECTOR_CONFIG_SDK_CALI + 4; sec++)
			SPIEraseSector(sec);
		for (sec = SECTOR_CONFIG_SYS; sec < SECTOR_CONFIG_SYS + 3; sec++)
			SPIEraseSector(sec);
		return 0;
	}

	// read bootloader config
	bootloader_config romconf;
	SPIRead(SECTOR_CONFIG_BOOT * SECTOR_SIZE, &romconf, sizeof(bootloader_config));
	// valid bootloader config?
	if (romconf.magic != BOOT_CONFIG_MAGIC
		|| romconf.chksum != calc_chksum((uint8*)&romconf, (uint8*)&romconf.chksum)) {
		ets_printf("Writing default bootloader config.\r\n");
		ets_memset(&romconf, 0x00, sizeof(bootloader_config));
		romconf.magic = BOOT_CONFIG_MAGIC;
		ets_strcpy(romconf.roms[0].type, ROM_TYPE);
		romconf.roms[0].major = VERSION_MAJOR;
		romconf.roms[0].minor = VERSION_MINOR;
		romconf.roms[0].build = VERSION_BUILD;
		romconf.chksum = calc_chksum((uint8*)&romconf, (uint8*)&romconf.chksum);
		SPIEraseSector(SECTOR_CONFIG_BOOT);
		SPIWrite(SECTOR_CONFIG_BOOT * SECTOR_SIZE, &romconf, sizeof(bootloader_config));
	} else if (romconf.roms[0].major != VERSION_MAJOR
		|| romconf.roms[0].minor != VERSION_MINOR
		|| romconf.roms[0].build != VERSION_BUILD) {
		ets_printf("Writing rom details to bootloader status.\r\n");
		ets_memset(&romconf.roms[0], 0x00, sizeof(rom_details));
		ets_strcpy(romconf.roms[0].type, ROM_TYPE);
		romconf.roms[0].major = VERSION_MAJOR;
		romconf.roms[0].minor = VERSION_MINOR;
		romconf.roms[0].build = VERSION_BUILD;

		// ToDo : potential version specific update of boot config sector added here

		romconf.chksum = calc_chksum((uint8*)&romconf, (uint8*)&romconf.chksum);
		SPIEraseSector(SECTOR_CONFIG_BOOT);
		SPIWrite(SECTOR_CONFIG_BOOT * SECTOR_SIZE, &romconf, sizeof(bootloader_config));
	}

	uint8 latest_factory = find_latest(romconf, "FACTORY");
	uint8 latest_firmware = find_latest(romconf, "FIRMWARE");
	ets_printf("latest factory/firmware: %d/%d\r\n", latest_factory, latest_firmware);

	if (latest_factory == 0 && latest_firmware == 0) {
		latest_factory = 1;
		ets_printf("No valid config - expecting factory at slot 1\r\n");
	}

	uint8 slot = latest_factory;

	bootloader_status status;
	if (rtc_mem(RTCADDR_BOOT, &status, sizeof(bootloader_status), RTC_READ) &&
		(status.magic == BOOT_RTC_MAGIC) &&
		(status.chksum == calc_chksum((uint8*)&status, (uint8*)&status.chksum))) {
		// valid rtc data means no power cycle
		if (status.temp_rom > 0 && status.temp_rom < 4) {
			slot = status.temp_rom;
			ets_printf("Checking temp rom in slot %d\r\n", slot);
		} else if (latest_firmware > 0) {
			slot = latest_firmware;
			ets_printf("Checking firmware rom in slot %d\r\n", slot);
		} else {
			// no valid firmware, boot to factory
			slot = latest_factory;
			ets_printf("Checking factory rom in slot %d\r\n", slot);
		}
	} else {
		// no valid rtc data means potential power cycle
		slot = latest_factory;
		ets_printf("Checking factory rom in slot %d\r\n", slot);
	}

	uint32 runAddr = check_image(slot);
	if (runAddr == 0) {
		ets_printf("Rom in slot %d is bad.\r\n", slot);
		if (slot != latest_factory) {
			slot = latest_factory;
			ets_printf("Checking factory rom in slot %d\r\n", slot);
			runAddr = check_image(slot);
			if (runAddr == 0)
				ets_printf("Rom in slot %d is bad.\r\n", slot);
		}
	}

	if (runAddr == 0) {
		ets_printf("No valid rom found.\r\n");
		return 0;
	}

	status.magic = BOOT_RTC_MAGIC;
	status.current_rom = slot;
	status.temp_rom = 0;
	status.chksum = calc_chksum((uint8*)&status, (uint8*)&status.chksum);
	rtc_mem(RTCADDR_BOOT, &status, sizeof(bootloader_status), RTC_WRITE);

	ets_printf("Booting rom in slot %d.\r\n", slot);

	// copy the loader to top of iram
	ets_memcpy((void*)_text_addr, _text_data, _text_len);
	// return address to load from
	return runAddr;
}

void call_user_start(void) {
	__asm volatile (
		"mov a15, a0\n"          // store return addr, hope nobody wanted a15!
		"call0 find_image\n"     // find a good rom to boot
		"mov a0, a15\n"          // restore return addr
		"bnez a2, 1f\n"          // ?success
		"ret\n"                  // no, return
		"1:\n"                   // yes...
		"movi a3, entry_addr\n"  // get pointer to entry_addr
		"l32i a3, a3, 0\n"       // get value of entry_addr
		"jx a3\n"                // now jump to it
	);
}
