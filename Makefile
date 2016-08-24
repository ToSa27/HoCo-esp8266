# HoCo root makefile
# Tobias Hoff

include config.mk

.PHONY: all clean rebuild tools tools_clean bootloader bootloader_clean factory factory_clean firmware firmware_clean flash wipe

all: tools bootloader factory firmware

clean: tools_clean bootloader_clean factory_clean firmware_clean

rebuild: clean all

tools:
	$(MAKE) -C tools rebuild

tools_clean:
	$(MAKE) -C tools clean

bootloader:
	$(MAKE) -C bootloader rebuild

bootloader_clean:
	$(MAKE) -C bootloader clean

factory:
	$(MAKE) -C factory rebuild

factory_clean:
	$(MAKE) -C factory clean

firmware:
	$(MAKE) -C firmware rebuild

firmware_clean:
	$(MAKE) -C firmware clean

common/bin/blank3s.bin:
	cat $(SDK_BASE)/bin/blank.bin $(SDK_BASE)/bin/blank.bin $(SDK_BASE)/bin/blank.bin > common/bin/blank3s.bin

common/bin/blank4s.bin:
	cat $(SDK_BASE)/bin/blank.bin $(SDK_BASE)/bin/blank.bin $(SDK_BASE)/bin/blank.bin $(SDK_BASE)/bin/blank.bin > common/bin/blank4s.bin

flash: bootloader common/bin/blank3s.bin common/bin/blank4s.bin
	$(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) $(ESPOPTIONS) \
		-ca $(ADDR_BOOTLOADER) -cf bootloader/$(FIRMWARE_BASE)/bootloader.bin \
		-ca $(ADDR_CONFIG_BOOT) -cf $(SDK_BASE)/bin/blank.bin \
		-ca $(ADDR_CONFIG_SYS) -cf common/bin/blank3s.bin \
		-ca $(ADDR_CONFIG_HW) -cf common/bin/blank3s.bin \
		-ca $(ADDR_CONFIG_SDK_CALI) -cf common/bin/blank4s.bin \
		-ca $(ADDR_CONFIG_SDK) -cf $(SDK_BASE)/bin/esp_init_data_default.bin

#flash: bootloader factory common/bin/blank3s.bin common/bin/blank4s.bin
#	$(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) $(ESPOPTIONS) \
#		-ca $(ADDR_BOOTLOADER) -cf bootloader/$(FIRMWARE_BASE)/bootloader.bin \
#		-ca $(ADDR_SLOT_1) -cf factory/$(FIRMWARE_BASE)/factory.bin \
#		-ca $(ADDR_CONFIG_BOOT) -cf $(SDK_BASE)/bin/blank.bin \
#		-ca $(ADDR_CONFIG_SYS) -cf common/bin/blank3s.bin \
#		-ca $(ADDR_CONFIG_HW) -cf common/bin/blank3s.bin \
#		-ca $(ADDR_CONFIG_SDK_CALI) -cf common/bin/blank4s.bin \
#		-ca $(ADDR_CONFIG_SDK) -cf $(SDK_BASE)/bin/esp_init_data_default.bin

wipe:
	$(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) $(ESPOPTIONS) \
		-ca 0x000000 -cf common/bin/blank_1MB.bin \
		-ca 0x100000 -cf common/bin/blank_1MB.bin \
		-ca 0x200000 -cf common/bin/blank_1MB.bin \
		-ca 0x300000 -cf common/bin/blank_1MB.bin
