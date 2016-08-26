#include <wifi.h>
#include <debug.h>
#include <user_interface.h>
#include <sys_config.h>

wifi_callback callback;
wifi_scan_callback scan_callback;

uint8 wifi_timeout;
ETSTimer wifi_timer;

const char *wifi_enc_types[6] = { "open", "wep", "wpa", "wpa2", "wpa_wpa2", 0 };

/*
bool wifiStationConfigChanged = true;
bool wifiApConfigChanged = true;
*/

bool wifi_scanning = false;
int wifi_station_count = 0;

void ICACHE_FLASH_ATTR wifi_connect_success() {
	INFO("wifi_connect_success");
	if (callback)
		callback(wifi_get_opmode(), true);
}

void ICACHE_FLASH_ATTR wifi_connect_failure() {
	INFO("wifi_connect_failure");
	wifi_station_disconnect();
	if (callback)
		callback(wifi_get_opmode(), false);
}

bool ICACHE_FLASH_ATTR wifi_has_station() {
	DEBUG("wifi_has_station");
	struct station_config cst;
	wifi_station_get_config(&cst);
	return cst.ssid[0] != 0;
}

void ICACHE_FLASH_ATTR wifi_timer_cb(void *arg) {
	INFO("wifi_timer_cb %d...", wifi_timeout);
	struct ip_info ipconfig;
	ets_timer_disarm(&wifi_timer);
	switch(wifi_station_get_connect_status()) {
	case STATION_GOT_IP:
        wifi_get_ip_info(STATION_IF, &ipconfig);
        if( ipconfig.ip.addr != 0) {
    		INFO("...Connected");
    		wifi_connect_success();
        	return;
        }
		break;
	case STATION_WRONG_PASSWORD:
		INFO("...WrongPassword");
		wifi_connect_failure();
		return;
	case STATION_NO_AP_FOUND:
		INFO("...NoApFound");
		wifi_connect_failure();
		return;
	case STATION_CONNECT_FAIL:
		INFO("...ConnectFailed");
		wifi_connect_failure();
		return;
	}
	wifi_timeout--;
	if (wifi_timeout <= 0) {
		INFO("...Timeout");
		wifi_connect_failure();
	} else {
		ets_timer_setfn(&wifi_timer, (os_timer_func_t *)wifi_timer_cb, NULL);
		ets_timer_arm_new(&wifi_timer, 1000, 0, 1);
	}
}

void ICACHE_FLASH_ATTR wifi_station() {
	INFO("wifi_station");
//	if (!wifiStationConfigChanged)
//		return;
	wifi_station_set_hostname(SysConfig.DeviceId);
	wifi_timeout = 15;
	ets_timer_disarm(&wifi_timer);
	ets_timer_setfn(&wifi_timer, (os_timer_func_t *)wifi_timer_cb, NULL);
	ets_timer_arm_new(&wifi_timer, 1000, 0, 1);
	wifi_station_connect();
//	wifiStationConfigChanged = false;
}

void ICACHE_FLASH_ATTR wifi_ap() {
	INFO("wifi_ap");
//	if (!wifiApConfigChanged)
//		return;
	wifi_softap_dhcps_stop();
	struct softap_config conf;
	wifi_softap_get_config(&conf);
	ets_memset(conf.ssid, 0, sizeof(conf.ssid));
	ets_memset(conf.password, 0, sizeof(conf.password));
	ets_sprintf((char*)conf.ssid, "%s", SysConfig.DeviceId);
	conf.channel = 7;
	conf.ssid_len = ets_strlen(SysConfig.DeviceId);
	conf.ssid_hidden = false;
	conf.max_connection = 4;
	conf.beacon_interval = 100;
	conf.authmode = AUTH_OPEN;
	*conf.password = 0;
	struct ip_info info;
	IP4_ADDR(&info.ip, 192, 168, 27, 1);
	IP4_ADDR(&info.gw, 192, 168, 27, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	wifi_set_ip_info(SOFTAP_IF, &info);
	struct dhcps_lease dhcp;
	IP4_ADDR(&dhcp.start_ip, 192, 168, 27, 100);
	IP4_ADDR(&dhcp.end_ip, 192, 168, 27, 200);
	wifi_softap_set_dhcps_lease(&dhcp);
	wifi_set_broadcast_if(STATIONAP_MODE);
	wifi_softap_set_config(&conf);
	wifi_softap_dhcps_start();
//	wifiApConfigChanged = false;
	INFO("AccessPoint: %s", conf.ssid);
}

void ICACHE_FLASH_ATTR wifi_init(uint8 mode, bool force, wifi_callback cb) {
	DEBUG("wifi_init");
	INFO("WifiInit - mode: %d->%d", wifi_get_opmode(), mode);
	if (cb)
		callback = cb;
	if ((mode & STATION_MODE) && !wifi_has_station() && !force) {
		INFO("No WiFi Station config -> AP");
		mode = SOFTAP_MODE;
	}
	if (wifi_get_opmode() != mode)
		wifi_set_opmode(mode);
	if (mode & SOFTAP_MODE)
		wifi_ap();
	if (mode & STATION_MODE) {
		wifi_station();
	}
}

void ICACHE_FLASH_ATTR wifi_reinit(uint8 mode, bool force) {
	wifi_init(mode, force, NULL);
}

int ICACHE_FLASH_ATTR wifi_station_known(uint8* mac) {
	for (int i = 0; i < wifi_station_count; i++) {
		bool same = true;
		for (int j = 0; j < 6; j++)
			if (wifi_stations[i].mac[j] != mac[j]) {
				same = false;
				break;
			}
		if (same)
			return i;
	}
	return -1;
}

void ICACHE_FLASH_ATTR wifi_scan_done_cb(void *arg, STATUS status) {
	INFO("wifi_scan_done_cb");
	if (status == OK) {
		scaninfo *c = arg;
		struct bss_info *inf;
		if (c->pbss) {
			STAILQ_FOREACH(inf, c->pbss, next) {
				INFO("%s", inf->ssid);
				int i = wifi_station_known(inf->bssid);
				if (i < 0 && wifi_station_count < WIFI_MAXSTATIONS) {
					ets_memcpy(wifi_stations[wifi_station_count].name, inf->ssid, 32);
					ets_memcpy(wifi_stations[wifi_station_count].mac, inf->bssid, 6);
					wifi_stations[wifi_station_count].rssi = inf->rssi;
					wifi_stations[wifi_station_count].channel = inf->channel;
					wifi_stations[wifi_station_count].encryption = inf->authmode;
					wifi_station_count++;
					inf = (struct bss_info *)&inf->next;
				} else {
					wifi_stations[i].rssi = inf->rssi;
				}
			}
			if (scan_callback)
				scan_callback();
		}
	}
	wifi_scanning = false;
}

void ICACHE_FLASH_ATTR wifi_scan(wifi_scan_callback cb, bool fresh) {
	INFO("wifi_scan");
	if (wifi_scanning)
		return;
	wifi_scanning = true;
	if (cb)
		scan_callback = cb;
	if (fresh)
		wifi_station_count = 0;
	struct scan_config sc;
	sc.ssid = 0;
	sc.bssid = 0;
	sc.channel = 0;
	sc.show_hidden = 1;
	wifi_station_scan(&sc, wifi_scan_done_cb);
}
