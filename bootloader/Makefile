# HoCo bootloader makefile
# Tobias Hoff

include ../config.mk

ESPTOOL2				:= $(addprefix ../,$(ESPTOOL2))
CURL					:= $(addprefix ../,$(CURL))

# toolchain binaries
CC						= $(XTENSA_BINDIR)/xtensa-lx106-elf-gcc
AR						= $(XTENSA_BINDIR)/xtensa-lx106-elf-ar
LD						= $(XTENSA_BINDIR)/xtensa-lx106-elf-gcc
OBJCOPY					= $(XTENSA_BINDIR)/xtensa-lx106-elf-objcopy

TARGET					= bootloader
MODULES					= 
LIBS					= 

# build and firmware locations
BUILD_DIRS				:= $(addprefix $(BUILD_BASE)/,$(MODULES))

VERSION_BUILD			:= $(strip $(shell cat buildnumber.txt))
TARGET_OBJ				:= $(addprefix $(BUILD_BASE)/,$(TARGET).o)
TARGET_OBJ_STAGE		:= $(addprefix $(BUILD_BASE)/,$(TARGET)-stage.o)
TARGET_ELF				:= $(addprefix $(BUILD_BASE)/,$(TARGET).elf)
TARGET_ELF_STAGE		:= $(addprefix $(BUILD_BASE)/,$(TARGET)-stage.elf)
TARGET_BIN				:= $(addprefix $(FIRMWARE_BASE)/,$(TARGET).bin)

# components
SRC						:= $(foreach sdir,$(MODULES),$(wildcard $(sdir)/*.c))
OBJ						= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))

# build settings
E2_OPTS 				= -quiet -bin -boot0 -$(SPI_SIZE_KBYTE) -$(SPI_MODE) -$(SPI_SPEED)
E2_SECT					= .text .rodata

# compiler / linker settings
DEFINES					= __ets__
DEFINES					+= ICACHE_FLASH
DEFINES					+= BOOTLOADER
DEFINES					+= VERSION_MAJOR=$(VERSION_MAJOR)
DEFINES					+= VERSION_MINOR=$(VERSION_MINOR)
DEFINES					+= VERSION_BUILD=$(VERSION_BUILD)
DEFINES					+= SECTOR_CONFIG_BOOT=$(SECTOR_CONFIG_BOOT)
DEFINES					+= SECTOR_CONFIG_SDK=$(SECTOR_CONFIG_SDK)
DEFINES					+= SECTOR_CONFIG_SDK_CALI=$(SECTOR_CONFIG_SDK_CALI)
DEFINES					+= SECTOR_CONFIG_SYS=$(SECTOR_CONFIG_SYS)
DEFINES					+= RTCADDR_BOOT=$(RTCADDR_BOOT)

INCDIRS					= include
INCDIRS					+= $(addsuffix /include,$(MODULES))
INCDIRS					+= ../common/include
INCDIRS					+= $(BUILD_BASE)

LIBDIRS					= $(BUILD_BASE)

LDDIRS					= $(SDK_BASE)/ld
LDDIRS					+= ld

LDSCRIPTS				= eagle.app.v6.ld
LDSCRIPTS_STAGE			= bootloader-stage.ld

DEFINES					:= $(addprefix -D,$(DEFINES))
INCDIRS					:= $(addprefix -I,$(INCDIRS))
LDDIRS					:= $(addprefix -L,$(LDDIRS))
LDSCRIPTS				:= $(addprefix -T,$(LDSCRIPTS))
LDSCRIPTS_STAGE			:= $(addprefix -T,$(LDSCRIPTS_STAGE))
LIBDIRS					:= $(addprefix -L,$(LIBDIRS))
LIBS					:= $(addprefix -l,$(LIBS))
LIBS_STAGE				:= $(addprefix -l,$(LIBS_STAGE))

CFLAGS					= -Os -O3 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals
LDFLAGS					= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

vpath %.c $(MODULES)
define compile-objects
$1/%.o: %.c
	$(CC) $(INCDIRS) $(CFLAGS) $(DEFINES) -c $$< -o $$@
endef

.PHONY: all clean rebuild

all: $(BUILD_BASE) $(BUILD_DIRS) $(FIRMWARE_BASE) $(BUILD_BASE)/bootloader-hex.h $(TARGET_BIN) buildnum upload

buildnum:
	buildnumber.bat

upload:
	$(CURL) --insecure -u $(OTA_USER):$(OTA_PASS) -F "hw=$(HW_TYPE)" -F "rev=$(HW_REV)" -F "type=BOOTLOADER" -F "major=$(VERSION_MAJOR)" -F "minor=$(VERSION_MINOR)" -F "build=$(VERSION_BUILD)" -F "upload=@$(TARGET_BIN)" $(OTA_URL)/upload

clean:
	rm -rf $(BUILD_BASE)
	rm -rf $(FIRMWARE_BASE)

rebuild: clean all

$(BUILD_BASE):
	mkdir -p $@

$(BUILD_DIRS):
	mkdir -p $@

$(FIRMWARE_BASE):
	mkdir -p $@

$(BUILD_BASE)/%.o: %.c
	$(CC) $(INCDIRS) $(CFLAGS) $(DEFINES) -c $< -o $@

$(TARGET_ELF_STAGE): $(TARGET_OBJ_STAGE)
	$(LD) $(LDDIRS) $(LIBDIRS) $(LDSCRIPTS_STAGE) $(LDFLAGS) -Wl,--start-group $(TARGET_OBJ_STAGE) -Wl,--end-group -o $@

$(BUILD_BASE)/bootloader-hex.h: $(TARGET_ELF_STAGE)
	$(ESPTOOL2) -quiet -header $< $@ .text

$(TARGET_ELF): $(TARGET_OBJ)
	$(LD) $(LDDIRS) $(LIBDIRS) $(LDSCRIPTS) $(LDFLAGS) -Wl,--start-group $(TARGET_OBJ) -Wl,--end-group -o $@

$(TARGET_BIN): $(TARGET_ELF)
	$(ESPTOOL2) $(E2_OPTS) $< $@ $(E2_SECT)
	