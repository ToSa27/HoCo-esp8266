#include <mqttc.h>

#include <mqtt.h>

#include <esp8266.h>
#include <debug.h>
#include <osapi.h>
#include <mem.h>
#include <sys_config.h>
#include <hw_config.h>

/*
#include <user_config.h>
#include <boot.h>
#include <user_ota.h>
#include <cjson.h>

#ifdef FIRMWARE
#include <TimeLib.h>
#endif
*/
//bool mqttConfigChanged = true;

MQTT_Client mqtt_client;
MqttState mqtt_state = MQTT_UNKNOWN;

mqtt_state_callback state_callback;
mqtt_receive_callback receive_callback;

void ICACHE_FLASH_ATTR mqtt_set_state(MqttState state) {
	DEBUG("mqtt_set_state");
	if (mqtt_state != state) {
		mqtt_state = state;
		if (state_callback)
			state_callback(mqtt_state);
	}
}

void ICACHE_FLASH_ATTR mqtt_connected(uint32_t *args) {
	DEBUG("mqtt_Connected");
//	MQTT_Client* client = (MQTT_Client*)args;
	mqtt_set_state(MQTT_CONNECTED);
/*
	mqttSubscribeBroadcast("$time");
	mqttSubscribe("$reset");
	mqttSubscribe("$ota");
//	mqttSubscribe("$name");
//	mqttPublish("$online", "true", true);
	mqttPublish("$name", SysConfig.DeviceName, true);
	mqttPublish("$hwtype", HwConfig.Type, true);
	char data[32];
	os_memset(data, 0, sizeof(data));
	ets_sprintf(data, "%d", HwConfig.Rev);
	mqttPublish("$hwrev", data, true);
	mqttPublish("$fwtype", ROM_TYPE, true);
	os_memset(data, 0, sizeof(data));
	ets_sprintf(data, "%d.%d-%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
	mqttPublish("$fwver", data, true);
	os_memset(data, 0, sizeof(data));
	struct ip_info ipi;
	wifi_get_ip_info(STATION_IF, &ipi);
	ets_sprintf(data, IPSTR, IP2STR(&ipi.ip));
	mqttPublish("$ip", data, true);
	os_memset(data, 0, sizeof(data));
	ets_sprintf(data, "%d", wifi_station_get_rssi());
	mqttPublish("$signal", data, true);


	// ToDo : announce nodes
//	os_memset(data, 0, sizeof(data));
//	ets_sprintf(data, "%s:%s", , );
//	mqttPublish("nodes", data, true);


	mqttOtaCheck(false);
	if (establishedCb)
		establishedCb();
*/
}

void ICACHE_FLASH_ATTR mqtt_disconnected(uint32_t *args) {
	DEBUG("mqtt_disconnected");
//	MQTT_Client* client = (MQTT_Client*)args;
	mqtt_set_state(MQTT_DISCONNECTED);
}

void ICACHE_FLASH_ATTR mqtt_published(uint32_t *args) {
	DEBUG("mqtt_published");
//	MQTT_Client* client = (MQTT_Client*)args;
}

void ICACHE_FLASH_ATTR mqtt_data(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len) {
	DEBUG("mqtt_data");
	//	MQTT_Client* client = (MQTT_Client*)args;
	char *topicBuf = (char*)os_zalloc(topic_len+1);
	char *dataBuf = (char*)os_zalloc(data_len+1);
	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;
	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;
	INFO("MQTT: Received topic: %s, data: %s \r\n", topicBuf, dataBuf);
	if (receive_callback)
		receive_callback(topicBuf, dataBuf);
/*
	char *subtopicBuf = topicBuf;
	if (ets_strstr(subtopicBuf, "/hang/") == subtopicBuf) {
		subtopicBuf += 6;
		if (subtopicBuf[0] == '$') {
			subtopicBuf += 1;
#ifdef FIRMWARE
			if (ets_strstr(subtopicBuf, "time") == subtopicBuf) {
				subtopicBuf += 4;
				char *subdataBuf = dataBuf;
				uint32_t t = atoi(subdataBuf);
				subdataBuf = ets_strstr(subdataBuf, "/") + 1;
				int32_t o = atoi(subdataBuf);
				setTimeAndOffsetC(t, o);
			}
#endif
			// ToDo : handle broadcast topic
		} else if (ets_strstr(subtopicBuf, SysConfig.DeviceId) == subtopicBuf) {
			subtopicBuf += ets_strlen(SysConfig.DeviceId);
			if (subtopicBuf[0] == '/') {
				subtopicBuf += 1;
				if (subtopicBuf[0] == '$') {
					subtopicBuf += 1;
					if (ets_strstr(subtopicBuf, "reset") == subtopicBuf) {
						if (ets_strstr(dataBuf, "factory") == dataBuf)
							boot_reboot_to_factory();
						else
							boot_reboot();
					} else if (ets_strstr(subtopicBuf, "ota") == subtopicBuf) {
#ifdef FACTORY
						otaAutoStartJson(dataBuf);
#endif
#ifdef FIRMWARE
						char type[10];
						JsonGetStr(dataBuf, "type", type, sizeof(type));
						if (type[0] != 0) {
							uint8 major = (uint8)JsonGetUInt(dataBuf, "major");
							uint8 minor = (uint8)JsonGetUInt(dataBuf, "minor");
							uint16 build = (uint16)JsonGetUInt(dataBuf, "build");
							if (boot_ota_pending(type, major, minor, build))
								boot_reboot_to_factory();
						}
#endif
					} else if (ets_strstr(subtopicBuf, "config/$set") == subtopicBuf) {
						ets_memset(HwConfig.Conf, 0, sizeof(HwConfig.Conf));
						ets_strcpy(HwConfig.Conf, dataBuf);
					}
				} else {
					if (receivedCb)
						receivedCb(subtopicBuf, dataBuf);
				}
			}
		}
	}
*/
	os_free(topicBuf);
	os_free(dataBuf);
}

void ICACHE_FLASH_ATTR mqtt_init(mqtt_state_callback scb, char *lwt_topic, char *lwt_data, mqtt_receive_callback rcb) {
	DEBUG("mqtt_init");
	if (scb)
		state_callback = scb;
	if (rcb)
		receive_callback = rcb;
//	if (mqtt_state != MQTT_UNKNOWN && !mqttConfigChanged)
//		return;
	if (SysConfig.MqttHost[0] == 0 || SysConfig.MqttPort == 0) {
		mqtt_set_state(MQTT_UNKNOWN);
		return;
	}
	MQTT_InitConnection(&mqtt_client, (uint8_t*)SysConfig.MqttHost, SysConfig.MqttPort, SysConfig.MqttSecure);
	MQTT_InitClient(&mqtt_client, (uint8_t*)SysConfig.DeviceId, (uint8_t*)SysConfig.MqttUser, (uint8_t*)SysConfig.MqttPass, SysConfig.MqttKeepAlive, 1);
	MQTT_InitLWT(&mqtt_client, (uint8_t*)lwt_topic, (uint8_t*)lwt_data, 0, 0);
	MQTT_OnConnected(&mqtt_client, mqtt_connected);
	MQTT_OnDisconnected(&mqtt_client, mqtt_disconnected);
	MQTT_OnPublished(&mqtt_client, mqtt_published);
	MQTT_OnData(&mqtt_client, mqtt_data);
	mqtt_set_state(MQTT_INITIALIZED);
}

void ICACHE_FLASH_ATTR mqtt_connect() {
	DEBUG("mqtt_connect");
	if (mqtt_state == MQTT_UNKNOWN)
		return;
	MQTT_Connect(&mqtt_client);
	mqtt_set_state(MQTT_CONNECTING);
}

void ICACHE_FLASH_ATTR mqtt_disconnect() {
	DEBUG("mqtt_disconnect");
	if (mqtt_state == MQTT_CONNECTING || mqtt_state == MQTT_CONNECTED)
		MQTT_Disconnect(&mqtt_client);
	mqtt_set_state(MQTT_DISCONNECTED);
}

/*
void ICACHE_FLASH_ATTR mqttOtaCheck(bool force) {
	DEBUG("mqttOtaCheck");
	if (!SysConfig.OtaAuto && !force)
		return;
	int maxlen = 500;
	char buff[500];
	char *buffend = buff;
	buffend += boot_sys_info(true, true, buffend, maxlen - (buffend - buff));
	buffend[0] = 0;
	mqttPublish("$ota/check", buff, false);
}

// topic: "/hang/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR mqttSubscribeBroadcast(char *subtopic) {
	char topic[35];
	os_memset(topic, 0, sizeof(topic));
	os_sprintf(topic, "/hang/%s", subtopic);
	MQTT_Subscribe(&mqttClient, topic, 0);
}

// topic: "/hang/{DeviceId}/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR mqttSubscribe(char *subtopic) {
	char topic[35];
	os_memset(topic, 0, sizeof(topic));
	os_sprintf(topic, "/hang/%s/%s", SysConfig.DeviceId, subtopic);
	MQTT_Subscribe(&mqttClient, topic, 0);
}

// topic: "/hang/{DeviceId}/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR mqttPublish(char *subtopic, char *data, bool retain) {
	char topic[35];
	os_memset(topic, 0, sizeof(topic));
	os_sprintf(topic, "/hang/%s/%s", SysConfig.DeviceId, subtopic);
	MQTT_Publish(&mqttClient, topic, data, os_strlen(data), 0, retain ? 1 : 0);
}
*/
