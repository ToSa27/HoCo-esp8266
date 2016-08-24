# HoCo common makefile config
# Tobias Hoff

# SDK locations
ESP_BASE				= C:/Espressif
SDK_BASE				= $(ESP_BASE)/ESP8266_SDK
SDK_INCDIR				= $(SDK_BASE)/include $(SDK_BASE)/include/json
SDK_LIBDIR				= $(SDK_BASE)/lib
SDK_LDDIR				= $(SDK_BASE)/ld
SDK_TOOLS				= $(ESP_BASE)/utils/ESP8266
EXTRA_BASE				= $(ESP_BASE)/extra
XTENSA_BINDIR			= $(ESP_BASE)/xtensa-lx106-elf/bin

# esp module type
SPI_SPEED				= 40
SPI_MODE				= qio
SPI_SIZE_BIT			= 32M
SPI_SIZE_KBYTE			= 4096

# flash settings
ESPTOOL					= $(SDK_TOOLS)/esptool-ck.exe
ESPMODE					= wifio
ESPPORT					= COM3
ESPBAUD					= 460800
ESPOPTIONS				= -bf $(SPI_SPEED) -bm $(SPI_MODE) -bz $(SPI_SIZE_BIT) -cc esp8266 -cd $(ESPMODE)

# tools
ESPTOOL2				= tools/esptool2/esptool2.exe
CURL					= tools/curl/curl.exe

# hardware type/rev
HW_TYPE					= Default
HW_REV					= 1

# overall major / minor version
VERSION_MAJOR			= 1
VERSION_MINOR			= 1

OTA_SCHEMA				= http
OTA_HOST				= dns_or_ip.com
OTA_PORT				= 1885
OTA_URL					= $(OTA_SCHEMA)://$(OTA_HOST):$(OTA_PORT)/hoco/ota
OTA_USER				= hoco
OTA_PASS				= admin

BUILD_BASE				= build
FIRMWARE_BASE			= firmware
