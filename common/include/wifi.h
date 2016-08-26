#ifndef __HOCO_WIFI_H__
#define __HOCO_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

/*
#define WIFI_MAXSTATIONS 20

struct wifiStation_t
{
	char name[32];
	char mac[18];
	int8_t rssi;
	uint8_t channel;
	uint8_t encryption;
} wifiStation[WIFI_MAXSTATIONS] __attribute__((aligned(4)));

extern const char *wifiEncTypes[6];

extern bool wifiStationConfigChanged;
extern bool wifiApConfigChanged;
extern int wifiStationCount;
extern bool wifiStationConnected;
extern bool wifiScanInProgress;
*/

typedef void (*wifi_callback)(uint8 mode, bool connected);
//typedef void (*wifiScanCallback)();

void ICACHE_FLASH_ATTR wifi_init(uint8 mode, bool force, wifi_callback cb);
void ICACHE_FLASH_ATTR wifi_reinit(uint8 mode, bool force);
//void ICACHE_FLASH_ATTR wifiStartScan(wifiScanCallback cb);
//void ICACHE_FLASH_ATTR wifiScanDoneCb(void *arg, STATUS status);

#ifdef __cplusplus
}
#endif

#endif
