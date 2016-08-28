#ifndef __HOCO_WIFI_H__
#define __HOCO_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

typedef enum {
	WIFI_ST_UNKNOWN,
	WIFI_ST_DISCONNECTED,
	WIFI_ST_CONNECTING,
	WIFI_ST_CONNECTED,
} WifiStState;

//typedef enum {
//	WIFI_AP_UNKNOWN,
//	WIFI_AP_DISABLED,
//	WIFI_ST_ENABLED,
//} WifiApState;

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
extern bool wifi_scanning;
extern const char *wifi_enc_types[6];

extern bool wifi_station_config_changed;
extern bool wifi_ap_config_changed;

typedef void (*wifi_st_state_callback)(uint8 mode, WifiStState state);
typedef void (*wifi_scan_callback)();

void ICACHE_FLASH_ATTR wifi_init(uint8 mode, bool force, wifi_st_state_callback cb);
void ICACHE_FLASH_ATTR wifi_reinit(uint8 mode, bool force);
void ICACHE_FLASH_ATTR wifi_scan(wifi_scan_callback cb, bool fresh);

#ifdef __cplusplus
}
#endif

#endif
