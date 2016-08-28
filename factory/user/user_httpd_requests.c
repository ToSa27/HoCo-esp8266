#include <user_httpd_requests.h>
#include <debug.h>
#include <cjson.h>
#include <platform.h>
#include <httpd.h>
#include <httpdespfs.h>
#include <espfs.h>
#include <webpages-espfs.h>
#include <cgiwebsocket.h>
#include <boot.h>
#include <user_interface.h>
#include <sys_config.h>
#include <hw_config.h>
#include <wifi.h>
#include <fota.h>
#include <mqttc.h>

int ICACHE_FLASH_ATTR accessPointResponse(char *buff, int maxlen) {
	DEBUG("accessPointResponse");
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"wifi\",\"cmd\":\"scan\"");
	buffend += ets_sprintf(buffend, ",\"busy\":\"%d\"", wifi_scanning ? 1 : 0);
	if (!wifi_scanning) {
		buffend += ets_sprintf(buffend, ",\"ap\":[");
		int i;
		for (i = 0; i < wifi_station_count && buffend - buff < maxlen - 64; i++) {
			if (i > 0)
				buffend += ets_sprintf(buffend, ",");
			buffend += ets_sprintf(buffend, "{");
			buffend += ets_sprintf(buffend, "\"ssid\":\"%s\"", wifi_stations[i].name);
			buffend += ets_sprintf(buffend, ",\"mac\":\"%s\"", wifi_stations[i].mac);
			buffend += ets_sprintf(buffend, ",\"rssi\":\"%d\"", wifi_stations[i].rssi);
			buffend += ets_sprintf(buffend, ",\"channel\":\"%d\"", wifi_stations[i].channel);
			buffend += ets_sprintf(buffend, ",\"sec\":\"%s\"", wifi_enc_types[wifi_stations[i].encryption]);
			buffend += ets_sprintf(buffend, "}");
		}
		buffend += ets_sprintf(buffend, "]");
	}
	buffend += ets_sprintf(buffend, "}");
	return buffend - buff;
}

int ICACHE_FLASH_ATTR wifiRead(char *buff, int maxlen) {
	DEBUG("wifiRead");
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"wifi\",\"cmd\":\"read\"");
	buffend += ets_sprintf(buffend, ",\"mode\":");
	switch (wifi_get_opmode()) {
		case 0:
			buffend += ets_sprintf(buffend, "\"none\"");
			break;
		case STATION_MODE:
			buffend += ets_sprintf(buffend, "\"st\"");
			break;
		case SOFTAP_MODE:
			buffend += ets_sprintf(buffend, "\"ap\"");
			break;
		case STATIONAP_MODE:
			buffend += ets_sprintf(buffend, "\"both\"");
			break;
		default:
			buffend += ets_sprintf(buffend, "\"\"");
	}
	buffend += ets_sprintf(buffend, ",\"phy\":");
	switch (wifi_get_phy_mode()) {
		case PHY_MODE_11B:
			buffend += ets_sprintf(buffend, "\"b\"");
			break;
		case PHY_MODE_11G:
			buffend += ets_sprintf(buffend, "\"g\"");
			break;
		case PHY_MODE_11N:
			buffend += ets_sprintf(buffend, "\"n\"");
			break;
		default:
			buffend += ets_sprintf(buffend, "\"\"");
	}
	uint8_t mac[6];
	struct softap_config ap;
	wifi_softap_get_config(&ap);
	wifi_get_macaddr(SOFTAP_IF, mac);
	buffend += ets_sprintf(buffend, ",\"ap\":{");
	buffend += ets_sprintf(buffend, "\"mac\":\""MACSTR"\"", MAC2STR(mac));
	buffend += ets_sprintf(buffend, ",\"ssid\":\"%s\"", ap.ssid);
	buffend += ets_sprintf(buffend, ",\"pw\":\"%s\"", ap.password);
	buffend += ets_sprintf(buffend, ",\"ch\":\"%d\"", ap.channel);
	struct ip_info ipi;
	wifi_get_ip_info(SOFTAP_IF, &ipi);
	buffend += ets_sprintf(buffend, ",\"ip\":\""IPSTR"\"", IP2STR(&ipi.ip));
	struct station_config wst;
	wifi_station_get_config(&wst);
	wifi_get_macaddr(STATION_IF, mac);
	buffend += ets_sprintf(buffend, "},\"st\":{");
	buffend += ets_sprintf(buffend, "\"mac\":\""MACSTR"\"", MAC2STR(mac));
	buffend += ets_sprintf(buffend, ",\"ssid\":\"%s\"", wst.ssid);
//	buffend += ets_sprintf(buffend, ",\"pw\":\"%s\"", wst.password);
	wifi_get_ip_info(STATION_IF, &ipi);
	buffend += ets_sprintf(buffend, ",\"ip\":\""IPSTR"\"", IP2STR(&ipi.ip));
	buffend += ets_sprintf(buffend, ",\"rssi\":\"%d\"", wifi_station_get_rssi());
	buffend += ets_sprintf(buffend, ",\"state\":");
	switch(wifi_station_get_connect_status()) {
		case STATION_IDLE:
			buffend += ets_sprintf(buffend, "\"IDLE\"");
			break;
		case STATION_CONNECTING:
			buffend += ets_sprintf(buffend, "\"CONNECTING\"");
			break;
		case STATION_WRONG_PASSWORD:
			buffend += ets_sprintf(buffend, "\"WRONG_PASSWORD\"");
			break;
		case STATION_NO_AP_FOUND:
			buffend += ets_sprintf(buffend, "\"NO_AP_FOUND\"");
			break;
		case STATION_CONNECT_FAIL:
			buffend += ets_sprintf(buffend, "\"CONNECT_FAIL\"");
			break;
		case STATION_GOT_IP:
			buffend += ets_sprintf(buffend, "\"GOT_IP\"");
			break;
		default:
			buffend += ets_sprintf(buffend, "\"\"");
	}
	buffend += ets_sprintf(buffend, "}}");
	return buffend - buff;
}

/*
void wifiWrite(char *reqdata) {
	DEBUG("wifiWrite");
	char *val;
	int ival;
	val = JsonGet(reqdata, "st");
	if (val != NULL) {
		val++;
		struct station_config cst;
		wifi_station_get_config(&cst);
		char ssid[32];
		JsonGetStr(val, "ssid", ssid, sizeof(ssid));
		if (ets_strcmp(ssid, cst.ssid) != 0) {
			ets_memset(cst.ssid, 0, sizeof(cst.ssid));
			ets_strcpy(cst.ssid, ssid);
			wifi_station_config_changed = true;
		}
		char pw[64];
		JsonGetStr(val, "pw", pw, sizeof(pw));
		if (ets_strcmp(pw, cst.password) != 0) {
			ets_memset(cst.password, 0, sizeof(cst.password));
			ets_strcpy(cst.password, pw);
			wifi_station_config_changed = true;
		}
		if (wifi_station_config_changed) {
			cst.bssid_set = 0;
			wifi_station_set_config(&cst);
			INFO("Station config: %s / %s", cst.ssid, cst.password);
		}
	}
	val = JsonGet(reqdata, "phy");
	if (val != NULL) {
		val++;
		ival = PHY_MODE_11N;
		if (val[0] == 'b')
			ival = PHY_MODE_11B;
		else if (val[0] == 'g')
			ival = PHY_MODE_11G;
		if (ival != wifi_get_phy_mode())
			wifi_set_phy_mode(ival);
	}
	val = JsonGet(reqdata, "mode");
	if (val != NULL) {
		val++;
		ival = STATIONAP_MODE;
		if (ets_strstr(val, "st") == val)
			ival = STATION_MODE;
		else if (ets_strstr(val, "ap") == val)
			ival = SOFTAP_MODE;
		if (ival != wifi_get_opmode())
			wifi_set_opmode(ival);
	}
//	if (wifi_station_config_changed) {
//		if (!(wifi_get_opmode() & STATION_MODE))
//			wifi_reinit(wifi_get_opmode() + STATION_MODE, true);
//	}
}
*/

void wifiWrite(char *reqdata) {
	INFO("wifiWrite");
	char *val;
	int ival;
	val = JsonGet(reqdata, "mode");
	if (val != NULL) {
		val++;
		ival = STATIONAP_MODE;
		if (val[0] == 's')
			ival = STATION_MODE;
		else if (val[0] == 'a')
			ival = SOFTAP_MODE;
		if (ival != wifi_get_opmode())
			wifi_set_opmode(ival);
	}
	val = JsonGet(reqdata, "phy");
	if (val != NULL) {
		val++;
		ival = PHY_MODE_11N;
		if (val[0] == 'b')
			ival = PHY_MODE_11B;
		else if (val[0] == 'g')
			ival = PHY_MODE_11G;
		if (ival != wifi_get_phy_mode())
			wifi_set_phy_mode(ival);
	}
	val = JsonGet(reqdata, "st");
	if (val != NULL) {
		val++;
		struct station_config cst;
		wifi_station_get_config(&cst);
		char ssid[32];
		JsonGetStr(val, "ssid", ssid, sizeof(ssid));
		if (ets_strcmp(ssid, cst.ssid) != 0) {
			ets_memset(cst.ssid, 0, sizeof(cst.ssid));
			ets_strcpy(cst.ssid, ssid);
			wifi_station_config_changed = true;
		}
		char pw[64];
		JsonGetStr(val, "pw", pw, sizeof(pw));
		if (ets_strlen(pw) > 0) {
			if (ets_strcmp(pw, cst.password) != 0) {
				ets_memset(cst.password, 0, sizeof(cst.password));
				ets_strcpy(cst.password, pw);
				wifi_station_config_changed = true;
			}
		}
		if (wifi_station_config_changed) {
			if (!(wifi_get_opmode() & STATION_MODE))
				wifi_reinit(wifi_get_opmode() + STATION_MODE, true);
				//wifiInit(wifi_get_opmode() + STATION_MODE, true, wifi_cb);
			cst.bssid_set = 0;
			wifi_station_set_config(&cst);
			INFO("Station config: %s / %s", cst.ssid, cst.password);
		}
	}
}

int ICACHE_FLASH_ATTR fotaRead(char *buff, int maxlen) {
	DEBUG("fotaRead");
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"fota\",\"cmd\":\"read\"");
	buffend += ets_sprintf(buffend, ",\"host\":\"%s\"", SysConfig.FotaHost);
	buffend += ets_sprintf(buffend, ",\"port\":%d", SysConfig.FotaPort);
	buffend += ets_sprintf(buffend, ",\"path\":\"%s\"", SysConfig.FotaPath);
	buffend += ets_sprintf(buffend, ",\"sec\":%d", SysConfig.FotaSecure);
	buffend += ets_sprintf(buffend, ",\"user\":\"%s\"", SysConfig.FotaUser);
//	buffend += ets_sprintf(buffend, ",\"pw\":\"%s\"", SysConfig.FotaPass);
	buffend += ets_sprintf(buffend, ",\"auto\":%d", SysConfig.FotaAuto);
	bootloader_status romstat;
	if (boot_get_status(&romstat)) {
		buffend += ets_sprintf(buffend, ",\"firmware\":{\"current\":%d", romstat.current_rom);
		buffend += ets_sprintf(buffend, ",\"components\":");
		buffend += boot_sys_info(false, true, buffend, maxlen - (buffend - buff));
		buffend += ets_sprintf(buffend, "}");
	}
	buffend += ets_sprintf(buffend, "}");
	return buffend - buff;
}

void fotaWrite(char *reqdata) {
	DEBUG("fotaWrite");
	JsonGetStr(reqdata, "host", SysConfig.FotaHost, sizeof(SysConfig.FotaHost));
	SysConfig.FotaPort = JsonGetUInt(reqdata, "port");
	JsonGetStr(reqdata, "path", SysConfig.FotaPath, sizeof(SysConfig.FotaPath));
	SysConfig.FotaSecure = (uint8)JsonGetUInt(reqdata, "sec");
	JsonGetStr(reqdata, "user", SysConfig.FotaUser, sizeof(SysConfig.FotaUser));
	char pw[64];
	JsonGetStr(reqdata, "pw", pw, sizeof(pw));
	if (ets_strlen(pw) > 0)
		JsonGetStr(reqdata, "pw", SysConfig.FotaPass, sizeof(SysConfig.FotaPass));
	SysConfig.FotaAuto = (uint8)JsonGetUInt(reqdata, "auto");
	sys_config_save();
}

int ICACHE_FLASH_ATTR mqttRead(char *buff, int maxlen) {
	DEBUG("mqttRead");
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"mqtt\",\"cmd\":\"read\"");
	buffend += ets_sprintf(buffend, ",\"host\":\"%s\"", SysConfig.MqttHost);
	buffend += ets_sprintf(buffend, ",\"port\":%d", SysConfig.MqttPort);
	buffend += ets_sprintf(buffend, ",\"ka\":%d", SysConfig.MqttKeepAlive);
	buffend += ets_sprintf(buffend, ",\"sec\":%d", SysConfig.MqttSecure);
	buffend += ets_sprintf(buffend, ",\"user\":\"%s\"", SysConfig.MqttUser);
//	buffend += ets_sprintf(buffend, ",\"pw\":\"%s\"", SysConfig.MqttPass);
	buffend += ets_sprintf(buffend, "}");
	return buffend - buff;
}

void mqttWrite(char *reqdata) {
	DEBUG("mqttWrite");
	bool mqttConfigChanged = false;
	if (ets_strcmp(JsonGet(reqdata, "host"), SysConfig.MqttHost) != 0) {
		JsonGetStr(reqdata, "host", SysConfig.MqttHost, sizeof(SysConfig.MqttHost));
		mqttConfigChanged = true;
	}
	if ((uint16)JsonGetUInt(reqdata, "port") != SysConfig.MqttPort) {
		SysConfig.MqttPort = (uint16)JsonGetUInt(reqdata, "port");
		mqttConfigChanged = true;
	}
	if ((uint16)JsonGetUInt(reqdata, "ka") != SysConfig.MqttKeepAlive) {
		SysConfig.MqttKeepAlive = (uint16)JsonGetUInt(reqdata, "ka");
		mqttConfigChanged = true;
	}
	if ((uint8)JsonGetUInt(reqdata, "sec") != SysConfig.MqttSecure) {
		SysConfig.MqttSecure = (uint8)JsonGetUInt(reqdata, "sec");
		mqttConfigChanged = true;
	}
	if (ets_strcmp(JsonGet(reqdata, "user"), SysConfig.MqttUser) != 0) {
		JsonGetStr(reqdata, "user", SysConfig.MqttUser, sizeof(SysConfig.MqttUser));
		mqttConfigChanged = true;
	}
	char pw[64];
	JsonGetStr(reqdata, "pw", pw, sizeof(pw));
	if (ets_strlen(pw) > 0) {
		if (ets_strcmp(pw, SysConfig.MqttPass) != 0) {
			JsonGetStr(reqdata, "pw", SysConfig.MqttPass, sizeof(SysConfig.MqttPass));
			mqttConfigChanged = true;
		}
	}
	if (mqttConfigChanged) {
		sys_config_save();
		mqtt_connect();
	}
}

int ICACHE_FLASH_ATTR hardwareRead(char *buff, int maxlen) {
	DEBUG("hardwareRead");
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"hardware\",\"cmd\":\"read\"");
	buffend += ets_sprintf(buffend, ",\"type\":\"%s\"", HwConfig.Type);
	buffend += ets_sprintf(buffend, ",\"rev\":%d", HwConfig.Rev);
	buffend += ets_sprintf(buffend, ",\"name\":\"%s\"", SysConfig.DeviceId);
	buffend += ets_sprintf(buffend, ",\"desc\":\"%s\"", SysConfig.DeviceName);
	// ToDo : send proper config data
	buffend += ets_sprintf(buffend, ",\"config\":\"%s\"", "");
	buffend += ets_sprintf(buffend, "}");
	return buffend - buff;
}

void hardwareWrite(char *reqdata) {
	DEBUG("hardwareWrite");
	bool sysConfigChanged = false;
	bool hwConfigChanged = false;
	if (ets_strcmp(JsonGet(reqdata, "type"), HwConfig.Type) != 0) {
		JsonGetStr(reqdata, "type", HwConfig.Type, sizeof(HwConfig.Type));
		hwConfigChanged = true;
	}
	if ((uint16)JsonGetUInt(reqdata, "rev") != HwConfig.Rev) {
		HwConfig.Rev = (uint16)JsonGetUInt(reqdata, "rev");
		hwConfigChanged = true;
	}
	if (ets_strcmp(JsonGet(reqdata, "name"), SysConfig.DeviceId) != 0) {
		JsonGetStr(reqdata, "name", SysConfig.DeviceId, sizeof(SysConfig.DeviceId));
		sysConfigChanged = true;
	}
	if (ets_strcmp(JsonGet(reqdata, "desc"), SysConfig.DeviceName) != 0) {
		JsonGetStr(reqdata, "desc", SysConfig.DeviceName, sizeof(SysConfig.DeviceName));
		sysConfigChanged = true;
	}
	// ToDo : handle config
	if (sysConfigChanged)
		sys_config_save();
	if (hwConfigChanged)
		hw_config_save();
}

void ICACHE_FLASH_ATTR wifiScanCb() {
	DEBUG("WifiScanCb");
	int retsize = 1000;
	char buff[1000];
	char *buffend = buff;
	buffend += accessPointResponse(buffend, retsize);
	cgiWebsockBroadcast("/ws", buff, buffend - buff, WEBSOCK_FLAG_NONE);
}

int ICACHE_FLASH_ATTR fullRead(char *buff, int maxlen) {
	DEBUG("fullRead");
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "[");
	buffend += wifiRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, ",");
	buffend += fotaRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, ",");
	buffend += mqttRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, ",");
	buffend += hardwareRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, "]");
	return buffend - buff;
}

int ICACHE_FLASH_ATTR httpd_handle_request(char *reqdata, int reqlen, char *resdata, int reslen) {
	DEBUG("httpd_handle_request");
	INFO("req: %s", reqdata);
	int retsize = reslen;
	char *buff = resdata;
	char *buffend = buff;
	if (reqlen > 0) {
		reqdata[reqlen] = 0;
		char *cmd = JsonGet(reqdata, "cmd") + 1;
		char *dat = JsonGet(reqdata, "data") + 1;
		if (ets_strstr(dat, "full") == dat) {
			if (ets_strstr(cmd, "read") == cmd)
				buffend += fullRead(buffend, retsize);
		} else if (ets_strstr(dat, "wifi") == dat) {
			if (ets_strstr(cmd, "write") == cmd)
				wifiWrite(reqdata);
			if (ets_strstr(cmd, "read") == cmd || ets_strstr(cmd, "write") == cmd)
				buffend += wifiRead(buffend, retsize);
			else if (ets_strstr(cmd, "scan") == cmd) {
				char *mode = JsonGet(reqdata, "mode") + 1;
				if (ets_strstr(mode, "push") == mode)
					buffend += ets_sprintf(buffend, "{\"data\":\"scan\",\"cmd\":\"triggered\"}");
				else if (ets_strstr(mode, "pull") == mode)
					buffend += accessPointResponse(buffend, retsize);
				wifi_scan(wifiScanCb, true);
			}
		} else if (ets_strstr(dat, "mqtt") == dat) {
			if (ets_strstr(cmd, "write") == cmd)
				mqttWrite(reqdata);
			if (ets_strstr(cmd, "read") == cmd || ets_strstr(cmd, "write") == cmd)
				buffend += mqttRead(buffend, retsize);
		} else if (ets_strstr(dat, "fota") == dat) {
			if (ets_strstr(cmd, "write") == cmd)
				fotaWrite(reqdata);
			if (ets_strstr(cmd, "read") == cmd || ets_strstr(cmd, "write") == cmd) {
				buffend += fotaRead(buffend, retsize);
			} else if (ets_strstr(cmd, "flash") == cmd) {
				bool started = false;
				char type[20];
				JsonGetStr(reqdata, "type", type, sizeof(type));
				if (type[0] != 0) {
					uint8 major = (uint8)JsonGetUInt(reqdata, "major");
					uint8 minor = (uint8)JsonGetUInt(reqdata, "minor");
					uint16 build = (uint16)JsonGetUInt(reqdata, "build");
					started = fota_start(type, major, minor, build);
				}
				buffend += ets_sprintf(buffend, "{\"data\":\"fota\",\"cmd\":\"flash\",\"status\":\"");
				buffend += ets_sprintf(buffend, started ? "started" : "failed");
				buffend += ets_sprintf(buffend, "\"}");
			}
		} else if (ets_strstr(dat, "hardware") == dat) {
			if (ets_strstr(cmd, "write") == cmd)
				hardwareWrite(reqdata);
			if (ets_strstr(cmd, "read") == cmd || ets_strstr(cmd, "write") == cmd)
				buffend += hardwareRead(buffend, retsize);
		} else if (ets_strstr(dat, "system") == dat) {
			if (ets_strstr(cmd, "restart") == cmd) {
				buffend += ets_sprintf(buffend, "{\"data\":\"system\",\"cmd\":\"restart\"}");
				boot_reboot();
			}
		}
	}
	if (buff == buffend)
		buffend += ets_sprintf(buffend, "{\"data\":\"error\",\"cmd\":\"error\"}");
	buffend[0] = 0;
	INFO("res: %s", buff);
	return buffend - buff;
}
