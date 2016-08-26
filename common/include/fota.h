#ifndef __HOCO_OTA_H__
#define __HOCO_OTA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

#define FOTA_FLAG_IDLE			0x00
#define FOTA_FLAG_START			0x01
#define FOTA_FLAG_FINISH		0x02

#define FOTA_NETWORK_TIMEOUT 	10000
#define FOTA_SSL_SIZE			5120

typedef enum {
	FOTA_SUCCESS,
	FOTA_FAILURE,
	FOTA_NOUPDATE,
} FotaResult;

typedef void (*fota_callback)(FotaResult result, uint8 rom_slot);

void ICACHE_FLASH_ATTR fota_init(fota_callback cb);
bool ICACHE_FLASH_ATTR fota_start(char *type, uint8 major, uint8 minor, uint16 build);

#ifdef __cplusplus
}
#endif

#endif
