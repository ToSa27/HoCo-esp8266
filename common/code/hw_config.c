#include <hw_config.h>
#include <user_interface.h>
#include <debug.h>

struct hw_config HwConfig __attribute__((aligned(4)));

void ICACHE_FLASH_ATTR hw_config_load(bool reset) {
	INFO("Loading hardware config.");
	ets_memset(&HwConfig, 0, sizeof(HwConfig));
	system_param_load(HW_CONFIG_SECTOR, 0, (uint32*)&HwConfig, sizeof(HwConfig));
	if(HwConfig.magic != HW_CONFIG_MAGIC || reset) {
		WARN("Initializing hardware config.");
		ets_memset(&HwConfig, 0, sizeof(HwConfig));
		ets_sprintf(HwConfig.Type, "Default");
		HwConfig.Rev = 0;
		hw_config_save();
	}
	INFO("Hardware: %s rev. %d", HwConfig.Type, HwConfig.Rev);
}

void ICACHE_FLASH_ATTR hw_config_save() {
	INFO("Saving hardware config.");
	HwConfig.magic = HW_CONFIG_MAGIC;
	system_param_save_with_protect(HW_CONFIG_SECTOR, (uint32*)&HwConfig, sizeof(HwConfig));
}
