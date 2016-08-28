#include <fota.h>
#include <debug.h>
#include <os_type.h>
#include <osapi.h>
#include <mem.h>
#include <ip_addr.h>
#include <espconn.h>
#include <user_interface.h>
#include <sys_config.h>
#include <hw_config.h>
#include <boot.h>

typedef struct {
	uint32 start_addr;
	uint32 start_sector;
	int32 last_sector_erased;
	uint8 extra_count;
	uint8 extra_bytes[4];
} FlashStatus;

typedef struct {
	uint8 rom_slot;
	uint32 total_len;
	uint32 content_len;
	struct espconn *conn;
	ip_addr_t ip;
	FlashStatus write_status;
	char* filename;
} FotaStatus;

static fota_callback callback;
static FotaStatus *upgrade;
static os_timer_t fota_timer;

bool ICACHE_FLASH_ATTR fota_write_flash(FlashStatus *status, uint8 *data, uint16 len) {
	DEBUG("fota_write_flash");
	bool ret = false;
	uint8 *buffer;
	int32 lastsect;
	if (data == NULL || len == 0)
		return true;
	buffer = (uint8 *)os_malloc(len + status->extra_count);
	if (!buffer) {
		ERROR("No ram!");
		return false;
	}
	ets_memcpy(buffer, status->extra_bytes, status->extra_count);
	ets_memcpy(buffer + status->extra_count, data, len);
	len += status->extra_count;
	status->extra_count = len % 4;
	len -= status->extra_count;
	ets_memcpy(status->extra_bytes, buffer + len, status->extra_count);
	lastsect = ((status->start_addr + len) - 1) / SECTOR_SIZE;
	while (lastsect > status->last_sector_erased) {
		status->last_sector_erased++;
		spi_flash_erase_sector(status->last_sector_erased);
	}
	DEBUG("write addr: 0x%08x, len: 0x%04x\r\n", status->start_addr, len);
	if (spi_flash_write(status->start_addr, (uint32 *)((void*)buffer), len) == SPI_FLASH_RESULT_OK) {
		ret = true;
		status->start_addr += len;
	}
	os_free(buffer);
	return ret;
}

FlashStatus ICACHE_FLASH_ATTR fota_write_init(uint32 start_addr) {
	DEBUG("fota_write_init");
	FlashStatus status = {0};
	status.start_addr = start_addr;
	status.start_sector = start_addr / SECTOR_SIZE;
	status.last_sector_erased = status.start_sector - 1;
	DEBUG("init addr: 0x%08x\r\n", start_addr);
	return status;
}

void ICACHE_FLASH_ATTR fota_deinit() {
	DEBUG("fota_deinit");
	FotaResult result;
	os_timer_disarm(&fota_timer);
	struct espconn *conn = upgrade->conn;
	uint8 rom_slot = upgrade->rom_slot;
	os_free(upgrade->filename);
	os_free(upgrade);
	upgrade = 0;
	if (conn) {
		if (SysConfig.FotaSecure)
			espconn_secure_disconnect(conn);
		else
			espconn_disconnect(conn);
	}
	if (system_upgrade_flag_check() == FOTA_FLAG_FINISH)
		result = FOTA_SUCCESS;
	else {
		system_upgrade_flag_set(FOTA_FLAG_IDLE);
		result = FOTA_FAILURE;
	}
	if (callback) {
		callback(result, rom_slot);
	}
}

static void ICACHE_FLASH_ATTR fota_recvcb(void *arg, char *pusrdata, unsigned short length) {
	DEBUG("fota_recvcb");
	char *ptrData, *ptrLen, *ptr;
	os_timer_disarm(&fota_timer);
	if (upgrade->content_len == 0) {
		if ((ptrLen = (char*)os_strstr(pusrdata, "Content-Length: "))
			&& (ptrData = (char*)os_strstr(ptrLen, "\r\n\r\n"))
			&& (os_strncmp(pusrdata + 9, "200", 3) == 0)) {
			ptrData += 4;
			length -= (ptrData - pusrdata);
			upgrade->total_len += length;
			fota_write_flash(&upgrade->write_status, (uint8*)ptrData, length);
			ptrLen += 16;
			ptr = (char *)os_strstr(ptrLen, "\r\n");
			*ptr = '\0'; // destructive
			upgrade->content_len = atoi(ptrLen);
		} else {
			fota_deinit();
			return;
		}
	} else {
		upgrade->total_len += length;
		fota_write_flash(&upgrade->write_status, (uint8*)pusrdata, length);
	}
	if (upgrade->total_len == upgrade->content_len) {
		system_upgrade_flag_set(FOTA_FLAG_FINISH);
		fota_deinit();
	} else if (upgrade->conn->state != ESPCONN_READ) {
		// fail, but how do we get here? premature end of stream?
		fota_deinit();
	} else {
		os_timer_setfn(&fota_timer, (os_timer_func_t *)fota_deinit, 0);
		os_timer_arm(&fota_timer, FOTA_NETWORK_TIMEOUT, 0);
	}
}

static void ICACHE_FLASH_ATTR fota_disconcb(void *arg) {
	DEBUG("fota_disconcb");
	struct espconn *conn = (struct espconn*)arg;
	os_timer_disarm(&fota_timer);
	if (conn) {
		if (conn->proto.tcp) {
			os_free(conn->proto.tcp);
		}
		os_free(conn);
	}
	if (upgrade && (upgrade->conn == conn)) {
		upgrade->conn = 0;
		fota_deinit();
	}
}

static void ICACHE_FLASH_ATTR fota_recon_cb(void *arg, sint8 errType) {
	DEBUG("fota_recon_cb");
	ERROR("Connection error: %d\r\n", errType);
	fota_disconcb(upgrade->conn);
}

static void ICACHE_FLASH_ATTR fota_connect_cb(void *arg) {
	DEBUG("fota_connect_cb");
	uint8 *request;
	os_timer_disarm(&fota_timer);
	espconn_regist_disconcb(upgrade->conn, fota_disconcb);
	espconn_regist_recvcb(upgrade->conn, fota_recvcb);
	request = (uint8 *)os_malloc(512);
	if (!request) {
		ERROR("No ram!");
		fota_deinit();
		return;
	}
	char basicAuth[50];
	ets_memset(basicAuth, 0, sizeof(basicAuth));
	if (SysConfig.FotaUser[0] != 0) {
		char userPass[50];
		ets_memset(userPass, 0, sizeof(userPass));
		ets_sprintf(userPass, "%s:%s", SysConfig.FotaUser, SysConfig.FotaPass);
		char encUserPass[50];
		ets_memset(encUserPass, 0, sizeof(encUserPass));
		base64_encode(ets_strlen(userPass), userPass, sizeof(encUserPass), encUserPass);
		ets_sprintf(basicAuth, "\r\nAuthorization: Basic %s", encUserPass);
	}
	DEBUG("filename: %s", upgrade->filename);
	os_sprintf((char*)request,
		"GET %s/%s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nCache-Control: no-cache\r\nUser-Agent: HoCo-config/1.0\r\nAccept: */*%s\r\n\r\n",
		SysConfig.FotaPath,
		upgrade->filename,
		SysConfig.FotaHost,
		basicAuth);
	DEBUG("request: %s", request);
	os_timer_setfn(&fota_timer, (os_timer_func_t *)fota_deinit, 0);
	os_timer_arm(&fota_timer, FOTA_NETWORK_TIMEOUT, 0);
	if (SysConfig.FotaSecure) {
		DEBUG("secure send");
		espconn_secure_send(upgrade->conn, request, os_strlen((char*)request));
	} else {
		DEBUG("send");
		espconn_sent(upgrade->conn, request, os_strlen((char*)request));
	}
	os_free(request);
}

static void ICACHE_FLASH_ATTR fota_timeout_cb() {
	DEBUG("fota_timeout_cb");
	WARN("Connection timeout.\r\n");
	fota_disconcb(upgrade->conn);
}

static void ICACHE_FLASH_ATTR fota_dns_resolved(const char *name, ip_addr_t *ip, void *arg) {
	DEBUG("fota_dns_resolved");
	if (ip == 0) {
		ERROR("DNS lookup failed for: %s", SysConfig.FotaHost);
		fota_disconcb(upgrade->conn);
		return;
	}
	upgrade->conn->type = ESPCONN_TCP;
	upgrade->conn->state = ESPCONN_NONE;
	upgrade->conn->proto.tcp->local_port = espconn_port();
	upgrade->conn->proto.tcp->remote_port = SysConfig.FotaPort;
	*(ip_addr_t*)upgrade->conn->proto.tcp->remote_ip = *ip;
//	os_memcpy(ip, upgrade->conn->proto.tcp->remote_ip, 4);
	espconn_regist_connectcb(upgrade->conn, fota_connect_cb);
	espconn_regist_reconcb(upgrade->conn, fota_recon_cb);

	if (SysConfig.FotaSecure) {
		DEBUG("secure connect");
		espconn_secure_set_size(ESPCONN_CLIENT, FOTA_SSL_SIZE);
		espconn_secure_connect(upgrade->conn);
	} else {
		DEBUG("connect");
		espconn_connect(upgrade->conn);
	}
	os_timer_disarm(&fota_timer);
	os_timer_setfn(&fota_timer, (os_timer_func_t *)fota_timeout_cb, 0);
	os_timer_arm(&fota_timer, FOTA_NETWORK_TIMEOUT, 0);
}

bool ICACHE_FLASH_ATTR fota_start(char *type, uint8 major, uint8 minor, uint16 build) {
	DEBUG("fota_start");
//	if (!fota_pending(type, major, minor, build))
//		return false;
	char filename[60];
	ets_sprintf(filename, "download?hw=%s&rev=%d&type=%s&major=%d&minor=%d&build=%d", HwConfig.Type, HwConfig.Rev, type, major, minor, build);
	DEBUG("filename: %s", filename);
	err_t result;
	if (system_upgrade_flag_check() == FOTA_FLAG_START)
		return false;
	// create upgrade status structure
	upgrade = (FotaStatus*)os_zalloc(sizeof(FotaStatus));
	if (!upgrade) {
		ERROR("No ram!");
		return false;
	}
	upgrade->filename = (char*)os_zalloc(os_strlen(filename) + 1);
	if (!upgrade->filename) {
		ERROR("No ram!");
		os_free(upgrade);
		return false;
	}
	ets_memcpy(upgrade->filename, filename, os_strlen(filename) + 1);
	uint8 slot = 0;
	if (ets_strcmp(type, "BOOTLOADER") != 0) {
		bootloader_config bootconf;
		if (!boot_get_config(&bootconf))
			return false;
		uint8 latestFactory = boot_find_latest(bootconf, "FACTORY");
		uint8 latestFirmware = boot_find_latest(bootconf, "FIRMWARE");
		slot = 1;
		while (slot == latestFactory || slot == latestFirmware)
			slot++;
	}
	DEBUG("slot: %d", slot);
	upgrade->rom_slot = slot;
	upgrade->write_status = fota_write_init(slotaddr(upgrade->rom_slot));
	upgrade->conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
	if (!upgrade->conn) {
		ERROR("No ram!");
		os_free(upgrade->filename);
		os_free(upgrade);
		return false;
	}
	upgrade->conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	if (!upgrade->conn->proto.tcp) {
		ERROR("No ram!");
		os_free(upgrade->filename);
		os_free(upgrade->conn);
		os_free(upgrade);
		return false;
	}
	system_upgrade_flag_set(FOTA_FLAG_START);
	result = espconn_gethostbyname(upgrade->conn, SysConfig.FotaHost, &upgrade->ip, fota_dns_resolved);
	if (result == ESPCONN_OK) {
		fota_dns_resolved(0, &upgrade->ip, upgrade->conn);
	} else if (result == ESPCONN_INPROGRESS) {
		// lookup taking place, will call upgrade_resolved on completion
	} else {
		ERROR("DNS error!");
		os_free(upgrade->filename);
		os_free(upgrade->conn->proto.tcp);
		os_free(upgrade->conn);
		os_free(upgrade);
		return false;
	}
	return true;
}

void ICACHE_FLASH_ATTR fota_init(fota_callback cb) {
	DEBUG("fota_init");
	callback = cb;
}
