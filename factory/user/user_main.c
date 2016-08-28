#include <user_config.h>
#include <esp8266.h>
#include <debug.h>
#include <boot.h>
#include <user_interface.h>
#include <sys_config.h>
#include <hw_config.h>
#include <wifi.h>
#include <mqttc.h>
#include <fota.h>
#include <cjson.h>
#include <user_httpd.h>
#include <cache_read_enable.c>

void ICACHE_FLASH_ATTR user_fota_cb(FotaResult result, uint8 rom_slot) {
	DEBUG("user_fota_cb");
	if (result == FOTA_SUCCESS) {
		if (rom_slot == 0) {
			INFO("Bootloader updated, rebooting.");
			boot_reboot();
		} else {
			INFO("ROM updated, rebooting to slot %d.", rom_slot);
			boot_reboot_to_slot(rom_slot);
		}
	} else if (result == FOTA_FAILURE)
		WARN("ROM update failed!");
	else
		INFO("ROM is up-to-date.");
}

void ICACHE_FLASH_ATTR user_mqtt_receive_cb(char *topic, char *data) {
	DEBUG("user_mqtt_receive_cb");
	char *subtopicBuf = topic;
	if (ets_strstr(subtopicBuf, "/hoco/") == subtopicBuf) {
		subtopicBuf += 6;
		if (ets_strstr(subtopicBuf, SysConfig.DeviceId) == subtopicBuf) {
			subtopicBuf += ets_strlen(SysConfig.DeviceId);
			if (subtopicBuf[0] == '/') {
				subtopicBuf += 1;
				if (subtopicBuf[0] == '$') {
					subtopicBuf += 1;
					if (ets_strstr(subtopicBuf, "reset") == subtopicBuf) {
						if (ets_strstr(data, "factory") == data)
							boot_reboot_to_factory();
						else
							boot_reboot();
					} else if (ets_strstr(subtopicBuf, "fota") == subtopicBuf) {
						char type[12];
						JsonGetStr(data, "type", type, sizeof(type));
						if (type[0] != 0) {
							if (ets_strstr(type, "none") != type) {
								uint8 major = (uint8)JsonGetUInt(data, "major");
								uint8 minor = (uint8)JsonGetUInt(data, "minor");
								uint16 build = (uint16)JsonGetUInt(data, "build");
								fota_start(type, major, minor, build);
							}
						}
					} else if (ets_strstr(subtopicBuf, "config/$set") == subtopicBuf) {
						ets_memset(HwConfig.Conf, 0, sizeof(HwConfig.Conf));
						ets_strcpy(HwConfig.Conf, data);
					}
				}
			}
		}
	}
}

void ICACHE_FLASH_ATTR user_mqtt_state_cb(MqttState state) {
	DEBUG("user_mqtt_state_cb");
	if (state == MQTT_DISCONNECTED) {
		mqtt_connect();
	} else if (state == MQTT_CONNECTED) {
		mqtt_announce();
		mqtt_fota_check(false);
		mqtt_publish("$online", "true", true);
	}
}

void ICACHE_FLASH_ATTR user_wifi_state_cb(uint8 mode, WifiStState state) {
	DEBUG("user_wifi_state_cb");
	INFO("WiFi mode: %d / state: %d", mode, state);
	if (state == WIFI_ST_DISCONNECTED)
		wifi_reinit(STATIONAP_MODE, false);
	else if (state == WIFI_ST_CONNECTED)
		mqtt_connect();
}

void ICACHE_FLASH_ATTR user_rf_pre_init() {
}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set() {
	return SECTOR_CONFIG_SDK_CALI;
}

void ICACHE_FLASH_ATTR user_init_done_cb() {
	DEBUG("user_init_done_cb");
	sys_config_load(false);
	hw_config_load(false);
	fota_init(user_fota_cb);
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s/$online", SysConfig.DeviceId);
	mqtt_init(user_mqtt_state_cb, topic, (char *)"false", user_mqtt_receive_cb);
	httpd_init(true);
	wifi_init(STATION_MODE, false, user_wifi_state_cb);
}

void ICACHE_FLASH_ATTR user_init(void) {
	boot_init();
	system_init_done_cb(user_init_done_cb);
}
