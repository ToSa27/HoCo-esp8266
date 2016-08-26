#include <user_httpd.h>
#include <user_httpd_requests.h>
#include <debug.h>
#include <platform.h>
#include <httpd.h>
#include <httpdespfs.h>
#include <espfs.h>
#include <webpages-espfs.h>
#include <cgiwebsocket.h>
#include <captdns.h>
//#include <sys_config.h>
//#include <hw_config.h>
//#include <boot.h>
//#include <user_interface.h>
//#include <wifi.h>
//#include <cjson.h>
//#include <user_wifi.h>
//#include <user_ota.h>
//#include <user_mqtt.h>

#define HTTPD_RESPONSE_BUFLEN		1000

/*
int ICACHE_FLASH_ATTR accessPointResponse(char *buff, int maxlen) {
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"wifi\",\"cmd\":\"scan\"");
	buffend += ets_sprintf(buffend, ",\"busy\":\"%d\"", wifiScanInProgress ? 1 : 0);
	if (!wifiScanInProgress) {
		buffend += ets_sprintf(buffend, ",\"ap\":[");
		int i;
		for (i = 0; i < wifiStationCount && buffend - buff < maxlen - 64; i++) {
			if (i > 0)
				buffend += ets_sprintf(buffend, ",");
			buffend += ets_sprintf(buffend, "{");
			buffend += ets_sprintf(buffend, "\"ssid\":\"%s\"", wifiStation[i].name);
			buffend += ets_sprintf(buffend, ",\"mac\":\"%s\"", wifiStation[i].mac);
			buffend += ets_sprintf(buffend, ",\"rssi\":\"%d\"", wifiStation[i].rssi);
			buffend += ets_sprintf(buffend, ",\"channel\":\"%d\"", wifiStation[i].channel);
			buffend += ets_sprintf(buffend, ",\"sec\":\"%s\"", wifiEncTypes[wifiStation[i].encryption]);
			buffend += ets_sprintf(buffend, "}");
		}
		buffend += ets_sprintf(buffend, "]");
	}
	buffend += ets_sprintf(buffend, "}");
	return buffend - buff;
}

int ICACHE_FLASH_ATTR wifiRead(char *buff, int maxlen) {
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
	buffend += ets_sprintf(buffend, ",\"pw\":\"%s\"", wst.password);
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
			wifiStationConfigChanged = true;
		}
		char pw[64];
		JsonGetStr(val, "pw", pw, sizeof(pw));
		if (ets_strcmp(pw, cst.password) != 0) {
			ets_memset(cst.password, 0, sizeof(cst.password));
			ets_strcpy(cst.password, pw);
			wifiStationConfigChanged = true;
		}
		if (wifiStationConfigChanged) {
			if (!(wifi_get_opmode() & STATION_MODE))
				wifiInit(wifi_get_opmode() + STATION_MODE, true, wifi_cb);
			cst.bssid_set = 0;
			wifi_station_set_config(&cst);
			INFO("Station config: %s / %s", cst.ssid, cst.password);
		}
	}
}

int ICACHE_FLASH_ATTR otaRead(char *buff, int maxlen) {
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"ota\",\"cmd\":\"read\"");
	buffend += ets_sprintf(buffend, ",\"host\":\"%s\"", SysConfig.OtaHost);
	buffend += ets_sprintf(buffend, ",\"port\":%d", SysConfig.OtaPort);
	buffend += ets_sprintf(buffend, ",\"path\":\"%s\"", SysConfig.OtaPath);
	buffend += ets_sprintf(buffend, ",\"sec\":%d", SysConfig.OtaSecure);
	buffend += ets_sprintf(buffend, ",\"user\":\"%s\"", SysConfig.OtaUser);
	buffend += ets_sprintf(buffend, ",\"pw\":\"%s\"", SysConfig.OtaPass);
	buffend += ets_sprintf(buffend, ",\"auto\":%d", SysConfig.OtaAuto);
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

void otaWrite(char *reqdata) {
	JsonGetStr(reqdata, "host", SysConfig.OtaHost, sizeof(SysConfig.OtaHost));
	SysConfig.OtaPort = JsonGetUInt(reqdata, "port");
	JsonGetStr(reqdata, "path", SysConfig.OtaPath, sizeof(SysConfig.OtaPath));
	SysConfig.OtaSecure = (uint8)JsonGetUInt(reqdata, "sec");
	JsonGetStr(reqdata, "user", SysConfig.OtaUser, sizeof(SysConfig.OtaUser));
	JsonGetStr(reqdata, "pw", SysConfig.OtaPass, sizeof(SysConfig.OtaPass));
	SysConfig.OtaAuto = (uint8)JsonGetUInt(reqdata, "auto");
	sys_config_save();
}

int ICACHE_FLASH_ATTR mqttRead(char *buff, int maxlen) {
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "{\"data\":\"mqtt\",\"cmd\":\"read\"");
	buffend += ets_sprintf(buffend, ",\"host\":\"%s\"", SysConfig.MqttHost);
	buffend += ets_sprintf(buffend, ",\"port\":%d", SysConfig.MqttPort);
	buffend += ets_sprintf(buffend, ",\"ka\":%d", SysConfig.MqttKeepAlive);
	buffend += ets_sprintf(buffend, ",\"sec\":%d", SysConfig.MqttSecure);
	buffend += ets_sprintf(buffend, ",\"user\":\"%s\"", SysConfig.MqttUser);
	buffend += ets_sprintf(buffend, ",\"pw\":\"%s\"", SysConfig.MqttPass);
	buffend += ets_sprintf(buffend, "}");
	return buffend - buff;
}

void mqttWrite(char *reqdata) {
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
	if (ets_strcmp(JsonGet(reqdata, "pw"), SysConfig.MqttPass) != 0) {
		JsonGetStr(reqdata, "pw", SysConfig.MqttPass, sizeof(SysConfig.MqttPass));
		mqttConfigChanged = true;
	}
	if (mqttConfigChanged) {
		sys_config_save();
		mqttConnect();
	}
}

int ICACHE_FLASH_ATTR hardwareRead(char *buff, int maxlen) {
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
	INFO("WifiScanCb");
	int retsize = 1000;
	char buff[1000];
	char *buffend = buff;
	buffend += accessPointResponse(buffend, retsize);
	cgiWebsockBroadcast("/ws", buff, buffend - buff, WEBSOCK_FLAG_NONE);
}

int ICACHE_FLASH_ATTR fullRead(char *buff, int maxlen) {
	char *buffend = buff;
	buffend += ets_sprintf(buffend, "[");
	buffend += wifiRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, ",");
	buffend += otaRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, ",");
	buffend += mqttRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, ",");
	buffend += hardwareRead(buffend, maxlen - (buffend - buff));
	buffend += ets_sprintf(buffend, "]");
	return buffend - buff;
}

int ICACHE_FLASH_ATTR handleRequest(char *reqdata, int reqlen, char *resdata, int reslen) {
	INFO("req: %s", reqdata);
	int retsize = reslen;
	char *buff = resdata;
	char *buffend = buff;
	if (reqlen > 0) {
		reqdata[reqlen] = 0;
		char *cmd = JsonGet(reqdata, "cmd") + 1;
		char *dat = JsonGet(reqdata, "data") + 1;
		switch (dat[0]) {
			case 'f': // full
				if (cmd[0] == 'r') // read
					buffend += fullRead(buffend, retsize);
				break;
			case 'w': // wifi
				if (cmd[0] == 'w') // write
					wifiWrite(reqdata);
				if (cmd[0] == 'r' || cmd[0] == 'w') // read
					buffend += wifiRead(buffend, retsize);
				else if (cmd[0] == 's') { // scan
					char *mode = JsonGet(reqdata, "mode") + 1;
					if (ets_strstr(mode, "push") == 0)
						buffend += ets_sprintf(buffend, "{\"data\":\"scan\",\"cmd\":\"triggered\"}");
					else if (ets_strstr(mode, "pull") == 0)
						buffend += accessPointResponse(buffend, retsize);
					wifiStartScan(wifiScanCb);
				}
				break;
			case 'm': // mqtt
				if (cmd[0] == 'w') // write
					mqttWrite(reqdata);
				if (cmd[0] == 'r' || cmd[0] == 'w') // read
					buffend += mqttRead(buffend, retsize);
				break;
			case 'o': // ota
				if (cmd[0] == 'w') // write
					otaWrite(reqdata);
				if (cmd[0] == 'r' || cmd[0] == 'w') // read
					buffend += otaRead(buffend, retsize);
				else if (cmd[0] == 'c') { // check
					// ToDo
				} else if (cmd[0] == 'f') { // flash
					buffend += ets_sprintf(buffend, "{\"data\":\"ota\",\"cmd\":\"flash\",\"status\":\"");
					buffend += ets_sprintf(buffend, otaAutoStartJson(reqdata) ? "started" : "failed");
					buffend += ets_sprintf(buffend, "\"}");
				}
				break;
			case 'h': // hardware
				if (cmd[0] == 'w') // write
					hardwareWrite(reqdata);
				if (cmd[0] == 'r' || cmd[0] == 'w') // read
					buffend += hardwareRead(buffend, retsize);
				break;
			case 's': case 'S': // system
				if (cmd[0] == 'r') { // restart
					buffend += ets_sprintf(buffend, "{\"data\":\"system\",\"cmd\":\"restart\"}");
					// ToDo: schedule reboot instead
					system_restart();
				}
				break;
		}
	}
	if (buff == buffend)
		buffend += ets_sprintf(buffend, "{\"data\":\"error\",\"cmd\":\"error\"}");
	buffend[0] = 0;
	INFO("res: %s", buff);
	return buffend - buff;
}
*/

void ICACHE_FLASH_ATTR httpd_ws_recv_cb(Websock *ws, char *data, int len, int flags) {
	INFO("httpd_ws_recv_cb");
	char resdata[HTTPD_RESPONSE_BUFLEN];
	int reslen = httpd_handle_request(data, len, resdata, sizeof(resdata));
	cgiWebsocketSend(ws, resdata, reslen, WEBSOCK_FLAG_NONE);
}

void ICACHE_FLASH_ATTR httpd_ws_connect_cb(Websock *ws) {
	INFO("httpd_ws_connect_cb");
	ws->recvCb = httpd_ws_recv_cb;
}

int ICACHE_FLASH_ATTR cgiAjax(HttpdConnData *connData) {
	if (connData->conn == NULL)
		return HTTPD_CGI_DONE;
	char resdata[HTTPD_RESPONSE_BUFLEN];
	int reslen = httpd_handle_request(connData->post->buff, ets_strlen(connData->post->buff), resdata, sizeof(resdata));
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Cache-Control", "max-age=3600, must-revalidate");
	httpdHeader(connData, "Content-Type", "application/json");
	httpdEndHeaders(connData);
	httpdSend(connData, resdata, reslen);
	return HTTPD_CGI_DONE;
}

HttpdBuiltInUrl httpd_urls[]={
	{"*", cgiRedirectApClientToHostname, "hoco.nonet"},
	{"/", cgiRedirect, "/hoco.html"},
	{"/ws.cgi", cgiWebsocket, httpd_ws_connect_cb},
	{"/aj.cgi", cgiAjax, NULL},
	{"*", cgiEspFsHook, NULL},
	{NULL, NULL, NULL}
};

void ICACHE_FLASH_ATTR httpd_init(bool capt) {
	espFsInit((void*)(webpages_espfs_start));
	httpdInit(httpd_urls, 80);
	if (capt)
		captdnsInit();
}
