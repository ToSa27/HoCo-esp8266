#include <user_cpp.h>

extern "C" {

#include <user_config.h>
#include <esp8266.h>
#include <debug.h>
#include <boot.h>
#include <user_interface.h>
#include <sys_config.h>
#include <hw_config.h>
#include <cache_read_enable.c>

void ICACHE_FLASH_ATTR user_rf_pre_init() {
}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set() {
	return SECTOR_CONFIG_SDK_CALI;
}

void ICACHE_FLASH_ATTR user_init_done_cb() {
	DEBUG("user_init_done_cb");
	sys_config_load(false);
	hw_config_load(false);
}

void ICACHE_FLASH_ATTR user_init() {
	do_global_ctors();
	boot_init();
	system_init_done_cb(user_init_done_cb);
}

}
