#ifndef __HOCO_MQTTC_H__
#define __HOCO_MQTTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

typedef enum {
	MQTT_UNKNOWN,
	MQTT_INITIALIZED,
	MQTT_DISCONNECTED,
	MQTT_CONNECTING,
	MQTT_CONNECTED,
} MqttState;

typedef void (*mqtt_state_callback)(MqttState state);
typedef void (*mqtt_receive_callback)(char*, char*);

/*
extern bool mqttConfigChanged;
extern MqttState mqtt_state;
*/

void ICACHE_FLASH_ATTR mqtt_init(mqtt_state_callback scb, char *lwt_topic, char *lwt_data, mqtt_receive_callback rcb);
void ICACHE_FLASH_ATTR mqtt_connect();
void ICACHE_FLASH_ATTR mqtt_disconnect();
void ICACHE_FLASH_ATTR mqtt_announce();
void ICACHE_FLASH_ATTR mqtt_fota_check(bool force);
void ICACHE_FLASH_ATTR mqtt_subscribe_broadcast(char *subtopic);
void ICACHE_FLASH_ATTR mqtt_subscribe(char *subtopic);
void ICACHE_FLASH_ATTR mqtt_publish(char *subtopic, char *data, bool retain);

#ifdef __cplusplus
}
#endif

#endif
