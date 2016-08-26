#ifndef __HOCO_WIFI_H__
#define __HOCO_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

#define WIFI_MAXSTATIONS 20

struct WifiStation
{
	char name[32];
	uint8 mac[6];
	int8_t rssi;
	uint8_t channel;
	uint8_t encryption;
} wifi_stations[WIFI_MAXSTATIONS] __attribute__((aligned(4)));

extern int wifi_station_count;
extern const char *wifi_enc_types[6];

/*
extern bool wifiStationConfigChanged;
extern bool wifiApConfigChanged;
extern int wifiStationCount;
extern bool wifiStationConnected;
*/
extern bool wifi_scanning;

typedef void (*wifi_callback)(uint8 mode, bool connected);
typedef void (*wifi_scan_callback)();

void ICACHE_FLASH_ATTR wifi_init(uint8 mode, bool force, wifi_callback cb);
void ICACHE_FLASH_ATTR wifi_reinit(uint8 mode, bool force);
void ICACHE_FLASH_ATTR wifi_scan(wifi_scan_callback cb, bool fresh);
//void ICACHE_FLASH_ATTR wifiScanDoneCb(void *arg, STATUS status);

#ifdef __cplusplus
}
#endif

#endif
