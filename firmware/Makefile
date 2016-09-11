# HoCo firmware makefile
# Tobias Hoff

include ../config.mk

ESPTOOL2				:= $(addprefix ../,$(ESPTOOL2))
ifdef CURL
CURL					:= $(addprefix ../,$(CURL))
else
CURL					= curl
endif

# toolchain binaries
CC						= $(XTENSA_BINDIR)/xtensa-lx106-elf-gcc
CPP						= $(XTENSA_BINDIR)/xtensa-lx106-elf-g++
AR						= $(XTENSA_BINDIR)/xtensa-lx106-elf-ar
LD						= $(XTENSA_BINDIR)/xtensa-lx106-elf-gcc
OBJCOPY					= $(XTENSA_BINDIR)/xtensa-lx106-elf-objcopy

TARGET					= firmware
MODULES					= user
LIBS					= c gcc hal phy pp net80211 wpa lwip crypto ssl m
COMMONS					= boot sys_config hw_config wifi mqttc cjson tick pin helper i2c
COMMONMODULES			= TimeLib CppJson OneWire
#HOCODEVICES				= DIn DOut OneWire DS1820 Counter I2C PCF8591 PCF8591AIn PCF8591AOut PCF8574 PCF8574DIn PCF8574DOut SSD1306
HOCODEVICES				= DIn DOut I2C PCF8591 PCF8591AIn PCF8591AOut PCF8574 PCF8574DIn PCF8574DOut SSD1306

# build and firmware locations
BUILD_BASE				= build
BUILD_DIRS				:= $(addprefix $(BUILD_BASE)/,$(MODULES))
BUILD_DIRS				+= $(BUILD_BASE)/hoco
BUILD_DIRS_COMMON		:= $(addprefix $(BUILD_BASE)/,$(COMMONMODULES))
FIRMWARE_BASE			= firmware

VERSION_BUILD			:= $(strip $(shell cat buildnumber.txt))
TARGET_LIB				:= $(addprefix $(BUILD_BASE)/lib,$(TARGET).a)
TARGET_ELF				:= $(addprefix $(BUILD_BASE)/,$(TARGET).elf)
TARGET_BIN				:= $(addprefix $(FIRMWARE_BASE)/,$(TARGET).bin)

# components
MAIN					= main2
MAIN_LIB				:= $(addprefix $(BUILD_BASE)/lib,$(MAIN).a)
MQTT					= mqtt
MQTT_DIR				= ../libs/esp_mqtt
MQTT_LIB_SRC			= $(addprefix $(MQTT_DIR)/lib,$(MQTT).a)
MQTT_LIB				= $(addprefix $(BUILD_BASE)/lib,$(MQTT).a)
HOCO					= hoco
HOCO_LIB				= $(addprefix $(BUILD_BASE)/lib,$(HOCO).a)
HOCO_SRC				= Base Device Scheduler
HOCO_SRC				+= $(HOCODEVICES)
HOCO_SRC				:= $(addprefix $(HOCO)/HoCo,$(HOCO_SRC))
HOCO_SRC				:= $(addsuffix .cpp,$(HOCO_SRC))
HOCO_OBJ				= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(HOCO_SRC))
LIBS					+= $(MAIN)
LIBS					+= $(MQTT)
LIBS					+= $(TARGET)
LIBS					+= $(HOCO)
SRC						:= $(foreach sdir,$(MODULES),$(wildcard $(sdir)/*.c))
OBJ						= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
SRCPP					:= $(foreach sdir,$(MODULES),$(wildcard $(sdir)/*.cpp))
OBJ						+= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(SRCPP))
SRCC					:= $(foreach sdir,$(addprefix ../common/modules/,$(COMMONMODULES)),$(wildcard $(sdir)/*.c))
OBJ						+= $(patsubst %.c,$(BUILD_BASE)/%.o,$(subst ../common/modules/,,$(SRCC)))
SRCCPP					:= $(foreach sdir,$(addprefix ../common/modules/,$(COMMONMODULES)),$(wildcard $(sdir)/*.cpp))
OBJ						+= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(subst ../common/modules/,,$(SRCCPP)))
OBJ						+= $(addsuffix .o,$(addprefix $(BUILD_BASE)/common/,$(COMMONS)))

# build settings
E2_OPTS 				= -quiet -bin -boot2 -iromchksum -$(SPI_SIZE_KBYTE) -$(SPI_MODE) -$(SPI_SPEED)
E2_SECT					= .text .data .rodata

# compiler / linker settings
DEFINES					= __ets__
DEFINES					+= ICACHE_FLASH
DEFINES					+= FIRMWARE
DEFINES					+= VERSION_MAJOR=$(VERSION_MAJOR)
DEFINES					+= VERSION_MINOR=$(VERSION_MINOR)
DEFINES					+= VERSION_BUILD=$(VERSION_BUILD)
DEFINES					+= HOCO_VERBOSITY=5
DEFINES					+= SECTOR_CONFIG_BOOT=$(SECTOR_CONFIG_BOOT)
DEFINES					+= SECTOR_CONFIG_SDK=$(SECTOR_CONFIG_SDK)
DEFINES					+= SECTOR_CONFIG_SDK_CALI=$(SECTOR_CONFIG_SDK_CALI)
DEFINES					+= SECTOR_CONFIG_SYS=$(SECTOR_CONFIG_SYS)
DEFINES					+= SECTOR_CONFIG_HW=$(SECTOR_CONFIG_HW)
DEFINES					+= RTCADDR_BOOT=$(RTCADDR_BOOT)
DEFINES					+= ARDUINO=100
DEFINES					+= ARDUINO_ARCH_ESP8266
DEFINES					+= ONEWIRE_CRC=0
DEFINES					+= $(addprefix HoCoDevice_,$(HOCODEVICES))

INCDIRS					= ../common/include
INCDIRS					+= $(addsuffix /include,$(MODULES))
INCDIRS					+= $(addprefix ../common/modules/,$(COMMONMODULES))
INCDIRS					+= $(addsuffix /include,$(HOCO))
INCDIRS					+= $(SDK_BASE)/include
INCDIRS					+= ../bootloader/include
INCDIRS					+= $(MQTT_DIR)/mqtt/include

LIBDIRS					= $(BUILD_BASE)
LIBDIRS					+= $(SDK_BASE)/lib

LDDIRS					= $(SDK_BASE)/../extra/ld
LDDIRS					+= ../common/ld

LDSCRIPTS				= eagle.app.v6.cpp.ld
LDSCRIPTS				+= memory.ld

DEFINES					:= $(addprefix -D,$(DEFINES))
INCDIRS					:= $(addprefix -I,$(INCDIRS))
LDDIRS					:= $(addprefix -L,$(LDDIRS))
LDSCRIPTS				:= $(addprefix -T,$(LDSCRIPTS))
LIBDIRS					:= $(addprefix -L,$(LIBDIRS))
LIBS					:= $(addprefix -l,$(LIBS))

COMMONFLAGS 			= -Os -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -mno-serialize-volatile -finline-functions -fdata-sections -ffunction-sections
CFLAGS					= $(COMMONFLAGS) -std=gnu99
CPPFLAGS				= $(COMMONFLAGS) -std=c++98 -fno-rtti -fno-exceptions
LDFLAGS					= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

vpath %.c $(MODULES) $(HOCO)
vpath %.cpp $(MODULES) $(HOCO)
define compile-modules
$1/%.o: %.c
	$(CC) $(INCDIRS) $(CFLAGS) $(DEFINES) -c $$< -o $$@
$1/%.o: %.cpp
	$(CPP) $(INCDIRS) $(CPPFLAGS) $(DEFINES) -c $$< -o $$@
endef

vpath %.c $(addprefix ../common/modules/,$(COMMONMODULES))
vpath %.cpp $(addprefix ../common/modules/,$(COMMONMODULES))
define compile-common-modules
$1/%.o: %.c
	$(CC) $(INCDIRS) $(CFLAGS) $(DEFINES) -c $$< -o $$@
$1/%.o: %.cpp
	$(CPP) $(INCDIRS) $(CPPFLAGS) $(DEFINES) -c $$< -o $$@
endef

.PHONY: all clean rebuild

all: $(BUILD_BASE) $(BUILD_DIRS) $(BUILD_DIRS_COMMON) $(FIRMWARE_BASE) $(TARGET_BIN) buildnum upload

buildnum:
	buildnumber.bat

upload:
	$(CURL) --insecure -u $(OTA_USER):$(OTA_PASS) -F "hw=$(HW_TYPE)" -F "rev=$(HW_REV)" -F "type=FIRMWARE" -F "major=$(VERSION_MAJOR)" -F "minor=$(VERSION_MINOR)" -F "build=$(VERSION_BUILD)" -F "upload=@$(TARGET_BIN)" $(OTA_URL)/upload

clean:
	rm -rf $(BUILD_BASE)
	rm -rf $(FIRMWARE_BASE)

rebuild: clean all

$(BUILD_BASE):
	mkdir -p $@
	mkdir -p $@/common

$(BUILD_DIRS):
	mkdir -p $@

$(BUILD_DIRS_COMMON):
	mkdir -p $@

$(FIRMWARE_BASE):
	mkdir -p $@

$(BUILD_BASE)/common/%.o: ../common/code/%.c
	$(CC) $(INCDIRS) $(CFLAGS) $(DEFINES) -c $< -o $@

$(MAIN_LIB): $(SDK_BASE)/lib/libmain.a
	@$(OBJCOPY) -W Cache_Read_Enable_New $^ $@

$(MQTT_LIB_SRC):
	$(MAKE) -C $(MQTT_DIR) lib \
		SDK_BASE=$(SDK_BASE) \
		XTENSA_TOOLS_ROOT=$(XTENSA_BINDIR)/ \
		VERBOSE=1

$(MQTT_LIB): $(MQTT_LIB_SRC)
	mv $(MQTT_LIB_SRC) $(MQTT_LIB)

$(HOCO_LIB): $(HOCO_OBJ)
	$(AR) cru $@ $(HOCO_OBJ)

$(TARGET_LIB): $(OBJ)
	$(AR) cru $@ $(OBJ)

$(TARGET_ELF): $(MAIN_LIB) $(MQTT_LIB) $(HOCO_LIB) $(TARGET_LIB)
	$(LD) $(LDDIRS) $(LIBDIRS) $(LDSCRIPTS) $(LDFLAGS) -Wl,--start-group $(LIBS) -Wl,--end-group -o $@

$(TARGET_BIN): $(TARGET_ELF)
	$(ESPTOOL2) $(E2_OPTS) $< $@ $(E2_SECT)

$(foreach bdir,$(BUILD_DIRS),$(eval $(call compile-modules,$(bdir))))
$(foreach bdir,$(BUILD_DIRS_COMMON),$(eval $(call compile-common-modules,$(bdir))))
