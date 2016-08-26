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

// topic: "/hoco/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR user_mqtt_subscribe_broadcast(char *subtopic) {
	DEBUG("user_mqtt_subscribe_broadcast");
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s", subtopic);
	mqtt_subscribe(topic);
}

// topic: "/hoco/{DeviceId}/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR user_mqtt_subscribe(char *subtopic) {
	DEBUG("user_mqtt_subscribe");
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s/%s", SysConfig.DeviceId, subtopic);
	mqtt_subscribe(topic);
}

// topic: "/hoco/{DeviceId}/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR user_mqtt_publish(char *subtopic, char *data, bool retain) {
	DEBUG("user_mqtt_publish");
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s/%s", SysConfig.DeviceId, subtopic);
	mqtt_publish(topic, data, retain);
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
						char type[20];
						JsonGetStr(data, "type", type, sizeof(type));
						if (type[0] != 0) {
							uint8 major = (uint8)JsonGetUInt(data, "major");
							uint8 minor = (uint8)JsonGetUInt(data, "minor");
							uint16 build = (uint16)JsonGetUInt(data, "build");
							fota_start(type, major, minor, build);
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

void ICACHE_FLASH_ATTR user_mqtt_fota_check(bool force) {
	DEBUG("user_mqtt_fota_check");
	if (!SysConfig.FotaAuto && !force)
		return;
	int maxlen = 500;
	char buff[500];
	char *buffend = buff;
	buffend += boot_sys_info(true, true, buffend, maxlen - (buffend - buff));
	buffend[0] = 0;
	user_mqtt_publish("$fota/check", buff, false);
}

void ICACHE_FLASH_ATTR user_mqtt_state_cb(MqttState state) {
	DEBUG("user_mqtt_state_cb");
	if (state == MQTT_DISCONNECTED) {
		mqtt_connect();
	} else if (state == MQTT_CONNECTED) {
		user_mqtt_subscribe("$reset");
		user_mqtt_subscribe("$fota");
		user_mqtt_publish("$name", SysConfig.DeviceName, true);
		user_mqtt_publish("$hwtype", HwConfig.Type, true);
		char data[32];
		ets_memset(data, 0, sizeof(data));
		ets_sprintf(data, "%d", HwConfig.Rev);
		user_mqtt_publish("$hwrev", data, true);
		user_mqtt_publish("$fwtype", ROM_TYPE, true);
		ets_memset(data, 0, sizeof(data));
		ets_sprintf(data, "%d.%d-%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
		user_mqtt_publish("$fwver", data, true);
		ets_memset(data, 0, sizeof(data));
		struct ip_info ipi;
		wifi_get_ip_info(STATION_IF, &ipi);
		ets_sprintf(data, IPSTR, IP2STR(&ipi.ip));
		user_mqtt_publish("$ip", data, true);
		ets_memset(data, 0, sizeof(data));
		ets_sprintf(data, "%d", wifi_station_get_rssi());
		user_mqtt_publish("$signal", data, true);
		user_mqtt_publish("$online", "true", true);
		user_mqtt_fota_check(false);
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
	fota_init(user_fota_cb);
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s/$online", SysConfig.DeviceId);
	mqtt_init(user_mqtt_state_cb, topic, (char *)"false", user_mqtt_receive_cb);
	httpd_init(true);
	wifi_init(STATION_MODE, false, user_wifi_cb);
}

void ICACHE_FLASH_ATTR user_init(void) {
	boot_init();
	system_init_done_cb(user_init_done_cb);
}
