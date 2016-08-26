#ifndef __HOCO_BOOT_H__
#define __HOCO_BOOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>
#include <bootloader.h>

// plain sdk defaults to iram
//#ifndef IRAM_ATTR
//#define IRAM_ATTR
//#endif

void ICACHE_FLASH_ATTR boot_init();
bool ICACHE_FLASH_ATTR boot_validate();
void ICACHE_FLASH_ATTR boot_reboot();
void ICACHE_FLASH_ATTR boot_reboot_to_slot(uint8 temp_slot);
void ICACHE_FLASH_ATTR boot_reboot_to_factory();
bool ICACHE_FLASH_ATTR boot_get_config(bootloader_config *conf);
bool ICACHE_FLASH_ATTR boot_set_config(bootloader_config *conf);
bool ICACHE_FLASH_ATTR boot_get_status(bootloader_status *rtc);
bool ICACHE_FLASH_ATTR boot_set_status(bootloader_status *rtc);
bool ICACHE_FLASH_ATTR boot_set_temp_rom(uint8 rom);

/*
bool ICACHE_FLASH_ATTR boot_get_current_rom(uint8 *rom);
int ICACHE_FLASH_ATTR boot_sys_info(bool hw, bool sw, char *buff, int maxlen);
bool ICACHE_FLASH_ATTR boot_ota_pending(char *type, uint8 major, uint8 minor, uint16 build);
bool ICACHE_FLASH_ATTR boot_ota_pending_json(char *json);
*/

#ifdef __cplusplus
}
#endif

#endif
