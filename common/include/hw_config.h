#ifndef __HOCO_HW_CONFIG_H__
#define __HOCO_HW_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

#define HW_CONFIG_SECTOR				0x304
#define HW_CONFIG_MAGIC					0x2334ae68

#define HW_CONFIG_TYPE_MAXLEN			32
#define HW_CONFIG_CONF_MAXLEN			1024

struct hw_config {
	uint32 magic;
	char Type[HW_CONFIG_TYPE_MAXLEN];
	uint16 Rev;
	uint8 res_3;
	uint8 res_4;
	char Conf[HW_CONFIG_CONF_MAXLEN];
};

extern struct hw_config HwConfig;

void hw_config_load(bool reset);
void hw_config_save();

#ifdef __cplusplus
}
#endif

#endif
