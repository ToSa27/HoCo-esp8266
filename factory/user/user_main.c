#include <user_config.h>
#include <esp8266.h>
#include <debug.h>
#include <boot.h>
#include <user_interface.h>
#include <sys_config.h>
#include <hw_config.h>
#include <wifi.h>
#include <mqttc.h>
#include <cache_read_enable.c>

void ICACHE_FLASH_ATTR user_mqtt_receive_cb(char *topic, char *data) {
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
					} else if (ets_strstr(subtopicBuf, "ota") == subtopicBuf) {
//						otaAutoStartJson(data);
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
	if (state == MQTT_UNKNOWN) {
		// do nothing
	} else if (state == MQTT_INITIALIZED) {
		// do nothing
	} else if (state == MQTT_DISCONNECTED) {
		mqtt_connect();
	} else if (state == MQTT_CONNECTING) {
		// do nothing
	} else if (state == MQTT_CONNECTED) {
//-->
	}
}

void ICACHE_FLASH_ATTR user_wifi_cb(uint8 mode, bool connected) {
	DEBUG("user_wifi_cb");
	INFO("WiFi mode: %d / connected: %d", mode, connected ? 1 : 0);
	if (connected) {
		mqtt_connect();
	} else {
		mqtt_disconnect();
		wifi_reinit(STATIONAP_MODE, false);
	}
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
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s/$online", SysConfig.DeviceId);
	mqtt_init(user_mqtt_state_cb, topic, (char *)"false", user_mqtt_receive_cb);
//	webInit();
//	captdnsInit();
	wifi_init(STATION_MODE, false, user_wifi_cb);
}

void ICACHE_FLASH_ATTR user_init(void) {
	boot_init();
	system_init_done_cb(user_init_done_cb);
}
