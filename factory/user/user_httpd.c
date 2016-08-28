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

#define HTTPD_RESPONSE_BUFLEN		1000

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
