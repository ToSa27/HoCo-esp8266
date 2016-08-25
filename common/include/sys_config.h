#ifndef __HOCO_SYS_CONFIG_H__
#define __HOCO_SYS_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

#define DEVICE_ID_PREFIX				"HANG_"

#define SYS_CONFIG_SECTOR				0x301
#define SYS_CONFIG_MAGIC				0x2334ae68

#define SYS_CONFIG_DEVICEID_MAXLEN		16
#define SYS_CONFIG_DEVICENAME_MAXLEN	64
#define SYS_CONFIG_OTAHOST_MAXLEN		64
#define SYS_CONFIG_OTAPATH_MAXLEN		64
#define SYS_CONFIG_OTAUSER_MAXLEN		32
#define SYS_CONFIG_OTAPASS_MAXLEN		32
#define SYS_CONFIG_MQTTHOST_MAXLEN		64
#define SYS_CONFIG_MQTTUSER_MAXLEN		32
#define SYS_CONFIG_MQTTPASS_MAXLEN		32

struct sys_config {
	uint32 magic;
	uint8 major;
	uint8 minor;
	uint8 res_ver_3;
	uint8 res_ver_4;
	char DeviceId[SYS_CONFIG_DEVICEID_MAXLEN];
	char DeviceName[SYS_CONFIG_DEVICENAME_MAXLEN];
	char FotaHost[SYS_CONFIG_OTAHOST_MAXLEN];
	char FotaPath[SYS_CONFIG_OTAPATH_MAXLEN];
	char FotaUser[SYS_CONFIG_OTAUSER_MAXLEN];
	char FotaPass[SYS_CONFIG_OTAPASS_MAXLEN];
	uint16 FotaPort;
	uint8 FotaSecure;
	uint8 FotaAuto;
	char MqttHost[SYS_CONFIG_MQTTHOST_MAXLEN];
	char MqttUser[SYS_CONFIG_MQTTUSER_MAXLEN];
	char MqttPass[SYS_CONFIG_MQTTPASS_MAXLEN];
	uint16 MqttPort;
	uint16 MqttKeepAlive;
	uint8 MqttSecure;
	uint8 res_mqtt_2;
	uint8 res_mqtt_3;
	uint8 res_mqtt_4;
};

extern struct sys_config SysConfig;

void sys_config_load(bool reset);
void sys_config_save();

#ifdef __cplusplus
}
#endif

#endif
