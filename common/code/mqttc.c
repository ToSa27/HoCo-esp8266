#include <mqttc.h>
#include <mqtt.h>
#include <esp8266.h>
#include <debug.h>
#include <osapi.h>
#include <mem.h>
#include <boot.h>
#include <sys_config.h>
#include <hw_config.h>
#include <user_config.h>

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

void ICACHE_FLASH_ATTR mqtt_announce() {
	DEBUG("mqtt_announce");
	mqtt_subscribe("$reset");
	mqtt_subscribe("$fota");
	mqtt_publish("$name", SysConfig.DeviceName, true);
	mqtt_publish("$hwtype", HwConfig.Type, true);
	char data[32];
	ets_memset(data, 0, sizeof(data));
	ets_sprintf(data, "%d", HwConfig.Rev);
	mqtt_publish("$hwrev", data, true);
	mqtt_publish("$fwtype", ROM_TYPE, true);
	ets_memset(data, 0, sizeof(data));
	ets_sprintf(data, "%d.%d-%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
	mqtt_publish("$fwver", data, true);
	ets_memset(data, 0, sizeof(data));
	struct ip_info ipi;
	wifi_get_ip_info(STATION_IF, &ipi);
	ets_sprintf(data, IPSTR, IP2STR(&ipi.ip));
	mqtt_publish("$ip", data, true);
	ets_memset(data, 0, sizeof(data));
	ets_sprintf(data, "%d", wifi_station_get_rssi());
	mqtt_publish("$signal", data, true);
}

void ICACHE_FLASH_ATTR mqtt_fota_check(bool force) {
	DEBUG("mqtt_fota_check");
	if (!SysConfig.FotaAuto && !force)
		return;
	int maxlen = 500;
	char buff[500];
	char *buffend = buff;
	buffend += boot_sys_info(true, true, buffend, maxlen - (buffend - buff));
	buffend[0] = 0;
	mqtt_publish("$fota/check", buff, false);
}

// topic: "/hoco/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR mqtt_subscribe_broadcast(char *subtopic) {
	DEBUG("mqtt_subscribe_broadcast");
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s", subtopic);
	MQTT_Subscribe(&mqtt_client, topic, 0);
}

// topic: "/hoco/{DeviceId}/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR mqtt_subscribe(char *subtopic) {
	DEBUG("mqtt_subscribe");
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s/%s", SysConfig.DeviceId, subtopic);
	MQTT_Subscribe(&mqtt_client, topic, 0);
}

// topic: "/hoco/{DeviceId}/{subtopic}"
// qos: 0
void ICACHE_FLASH_ATTR mqtt_publish(char *subtopic, char *data, bool retain) {
	DEBUG("mqtt_publish");
	char topic[35];
	ets_memset(topic, 0, sizeof(topic));
	ets_sprintf(topic, "/hoco/%s/%s", SysConfig.DeviceId, subtopic);
	MQTT_Publish(&mqtt_client, topic, data, os_strlen(data), 0, retain ? 1 : 0);
}
