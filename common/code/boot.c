#include <boot.h>
#include <debug.h>
#include <user_interface.h>
#include <spi_flash.h>
#include <mem.h>
#include <hw_config.h>

void ICACHE_FLASH_ATTR boot_init() {
	system_set_os_print(0);
	ets_printf("\r\nHoCo %s v%d.%d-%d\r\n", ROM_TYPE, VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
	wifi_station_set_auto_connect(false);
	wifi_softap_dhcps_stop();
	if (!boot_validate())
		boot_reboot();
}

bool ICACHE_FLASH_ATTR boot_validate() {
	DEBUG("boot_validate");
	bootloader_status bs;
	if (!boot_get_status(&bs)) {
		FATAL("Invalid bootloader status.");
		return false;
	}
	bootloader_config bc;
	if (!boot_get_config(&bc)) {
		FATAL("Invalid bootloader config.");
		return false;
	}
	rom_details rd = bc.roms[bs.current_rom];
	if (ets_strstr(rd.type, ROM_TYPE) != 0 || rd.major != VERSION_MAJOR || rd.minor != VERSION_MINOR || rd.build != VERSION_BUILD) {
		INFO("Writing rom details to bootloader status.");
		ets_memset(&bc.roms[bs.current_rom], 0x00, sizeof(rom_details));
		ets_sprintf(bc.roms[bs.current_rom].type, ROM_TYPE);
		bc.roms[bs.current_rom].major = VERSION_MAJOR;
		bc.roms[bs.current_rom].minor = VERSION_MINOR;
		bc.roms[bs.current_rom].build = VERSION_BUILD;
		boot_set_config(&bc);
	}
	return true;
}

void ICACHE_FLASH_ATTR boot_reboot() {
	DEBUG("boot_reboot");
	system_restart();
	while (true);
}

void ICACHE_FLASH_ATTR boot_reboot_to_slot(uint8 temp_slot) {
	DEBUG("boot_reboot_to_slot");
	if (temp_slot != 0)
		boot_set_temp_rom(temp_slot);
	boot_reboot();
}

void ICACHE_FLASH_ATTR boot_reboot_to_factory() {
	DEBUG("boot_reboot_to_factory");
	bootloader_config romconf;
	if (boot_get_config(&romconf))
		boot_reboot_to_slot(boot_find_latest(romconf, "FACTORY"));
}

bool ICACHE_FLASH_ATTR boot_get_config(bootloader_config *conf) {
	DEBUG("boot_get_config");
	spi_flash_read(SECTOR_CONFIG_BOOT * SECTOR_SIZE, (uint32*)conf, sizeof(bootloader_config));
	if (conf->magic != BOOT_CONFIG_MAGIC)
		return false;
	if (conf->chksum != calc_chksum((uint8*)conf, (uint8*)&conf->chksum))
		return false;
	return true;
}

bool ICACHE_FLASH_ATTR boot_set_config(bootloader_config *conf) {
	DEBUG("boot_set_config");
	uint8 *buffer = (uint8*)os_malloc(SECTOR_SIZE);
	if (!buffer)
		return false;
	conf->chksum = calc_chksum((uint8*)conf, (uint8*)&conf->chksum);
	spi_flash_read(SECTOR_CONFIG_BOOT * SECTOR_SIZE, (uint32*)((void*)buffer), SECTOR_SIZE);
	ets_memcpy(buffer, conf, sizeof(bootloader_config));
	spi_flash_erase_sector(SECTOR_CONFIG_BOOT);
	spi_flash_write(SECTOR_CONFIG_BOOT * SECTOR_SIZE, (uint32*)((void*)buffer), SECTOR_SIZE);
	os_free(buffer);
	return true;
}

bool ICACHE_FLASH_ATTR boot_get_status(bootloader_status *rtc) {
	DEBUG("boot_get_status");
	if (system_rtc_mem_read(RTCADDR_BOOT, rtc, sizeof(bootloader_status)))
		return (rtc->chksum == calc_chksum((uint8*)rtc, (uint8*)&rtc->chksum));
	return false;
}

bool ICACHE_FLASH_ATTR boot_set_status(bootloader_status *rtc) {
	DEBUG("boot_set_status");
	rtc->chksum = calc_chksum((uint8*)rtc, (uint8*)&rtc->chksum);
	return system_rtc_mem_write(RTCADDR_BOOT, rtc, sizeof(bootloader_status));
}

bool ICACHE_FLASH_ATTR boot_set_temp_rom(uint8 rom) {
	DEBUG("boot_set_temp_rom");
	bootloader_status rtc;
	// invalid data in rtc?
	if (!boot_get_status(&rtc)) {
		rtc.magic = BOOT_RTC_MAGIC;
		rtc.current_rom = 0;
	}
	rtc.temp_rom = rom;
	return boot_set_status(&rtc);
}

int ICACHE_FLASH_ATTR boot_sys_info_rom(bootloader_config bc, char *entry, char *buff, int maxlen) {
	DEBUG("boot_sys_info_rom");
	char *buffend = buff;
	uint8 slot = 0;
	if (entry[0] != 'B')
		slot = boot_find_latest(bc, entry);
	if (entry[0] == 'B' || slot > 0)
		buffend += ets_sprintf(buffend, "\"%s\":{\"slot\":%d,\"major\":%d,\"minor\":%d,\"build\":%d},", entry, slot, bc.roms[slot].major, bc.roms[slot].minor, bc.roms[slot].build);
	return buffend - buff;
}

int ICACHE_FLASH_ATTR boot_sys_info(bool hw, bool sw, char *buff, int maxlen) {
	DEBUG("boot_sys_info");
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{");
	if (hw)
		buffend += ets_sprintf(buffend, "\"HARDWARE\":{\"type\":\"%s\",\"rev\":%d},", HwConfig.Type, HwConfig.Rev);
	if (sw) {
		bootloader_config bc;
		if (boot_get_config(&bc)) {
			buffend += boot_sys_info_rom(bc, "BOOTLOADER", buffend, maxlen - (buffend - buff));
			buffend += boot_sys_info_rom(bc, "FACTORY", buffend, maxlen - (buffend - buff));
			buffend += boot_sys_info_rom(bc, "FIRMWARE", buffend, maxlen - (buffend - buff));
		}
	}
	char *last = buffend - 1;
	if (last[0] == ',')
		buffend--;
	buffend += ets_sprintf(buffend, "}");
	return buffend - buff;
}




/*
bool ICACHE_FLASH_ATTR boot_get_current_rom(uint8 *rom) {
	DEBUG("boot_get_current_rom");
	bootloader_status rtc;
	if (boot_get_status(&rtc)) {
		*rom = rtc.current_rom;
		return true;
	}
	return false;
}
*/
