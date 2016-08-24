#include <user_config.h>
#include <esp8266.h>
#include <debug.h>
#include <user_interface.h>
#include <cache_read_enable.c>

void ICACHE_FLASH_ATTR user_rf_pre_init() {
}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set() {
	return SECTOR_CONFIG_SDK_CALI;
}

void ICACHE_FLASH_ATTR user_init_done_cb() {
	DEBUG("user_init_done_cb");
}

void ICACHE_FLASH_ATTR user_init(void) {
	system_set_os_print(0);
	ets_printf("\r\nHoCo %s v%d.%d-%d\r\n", ROM_TYPE, VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
	wifi_station_set_auto_connect(false);
	wifi_softap_dhcps_stop();
	system_init_done_cb(user_init_done_cb);
}
