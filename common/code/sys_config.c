#include <sys_config.h>
#include <user_interface.h>
#include <debug.h>

struct sys_config SysConfig __attribute__((aligned(4)));

void ICACHE_FLASH_ATTR sys_config_load(bool reset) {
	INFO("Loading system config.");
	ets_memset(&SysConfig, 0, sizeof(SysConfig));
	system_param_load(SYS_CONFIG_SECTOR, 0, (uint32*)&SysConfig, sizeof(SysConfig));
	// ToDo : check major and minor version
	//if(SysConfig.magic != SYS_CONFIG_MAGIC || SysConfig.major != VERSION_MAJOR || SysConfig.minor != VERSION_MINOR || reset) {
	if(SysConfig.magic != SYS_CONFIG_MAGIC || reset) {
		WARN("Initializing system config.");
		ets_memset(&SysConfig, 0, sizeof(SysConfig));
		uint8_t mac[6];
		if(!wifi_get_macaddr(0, mac))
			wifi_get_macaddr(1, mac);
		ets_sprintf(SysConfig.DeviceId, "%s%02X%02X%02X", DEVICE_ID_PREFIX, mac[3], mac[4], mac[5]);
		ets_sprintf(SysConfig.DeviceName, "Default");
		ets_sprintf(SysConfig.FotaUser, "hoco");
		ets_sprintf(SysConfig.FotaPath, "/hoco/fota");
		SysConfig.FotaPort = 1885;
		SysConfig.FotaAuto = true;
		ets_sprintf(SysConfig.MqttUser, "hoco");
		SysConfig.MqttPort = 1883;
		SysConfig.MqttKeepAlive = 120;
		sys_config_save();
	}
	INFO("DeviceId: %s", SysConfig.DeviceId);
	if (SysConfig.DeviceName[0] != 0) {
		INFO("DeviceName: %s", SysConfig.DeviceName);
	}
}

void ICACHE_FLASH_ATTR sys_config_save() {
	INFO("Saving system config.");
	SysConfig.magic = SYS_CONFIG_MAGIC;
	SysConfig.major = VERSION_MAJOR;
	SysConfig.minor = VERSION_MINOR;
	system_param_save_with_protect(SYS_CONFIG_SECTOR, (uint32*)&SysConfig, sizeof(SysConfig));
}
