#include <cjson.h>

char * ICACHE_FLASH_ATTR JsonGet(char *data, char *name) {
	char fname[20];
	ets_sprintf(fname, "\"%s\"", name);
	char *val = ets_strstr(data, fname);
	if (val != NULL) {
		val = ets_strstr(val, ":") + 1;
		if (val != NULL)
			while (val[0] == ' ')
				val++;
	}
	return val;
}

uint32 ICACHE_FLASH_ATTR JsonGetUInt(char *data, char *name) {
	uint32 ret = 0;
	char *val = JsonGet(data, name);
	if (val != NULL) {
		while (val[0] == '\"' || val[0] == ' ')
			val++;
		while (val[0] >= '0' && val[0] <= '9') {
			ret = (ret * 10) + (val[0] - '0');
			val++;
		}
	}
	return ret;
}

void ICACHE_FLASH_ATTR JsonGetStr(char *data, char *name, char *dest, int len) {
	ets_memset(dest, 0, len);
	char *val = JsonGet(data, name) + 1;
	if (val != NULL) {
		int vallen = ets_strstr(val, "\"") - val;
		if (vallen > len)
			vallen = len;
		ets_memcpy(dest, val, vallen);
	}
}

void ICACHE_FLASH_ATTR JsonGetIP(char *data, char *name, uint8 *dest) {
	uint8 oct;
	for (oct = 0; oct < 4; oct++)
		dest[oct] = 0;
	char *ips = JsonGet(data, "ip") + 1;
	if (ips != NULL) {
		char *ipsend = ets_strstr(ips, "\"");
		oct = 0;
		while (ips < ipsend && oct < 4) {
			if (ips[0] == '.')
				oct++;
			else if (ips[0] >= '0' && ips[0] <= '9')
				dest[oct] = (dest[oct] * 10) + (ips[0] - '0');
			ips++;
		}
	}
}
