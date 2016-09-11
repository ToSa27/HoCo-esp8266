#ifndef __HOCO_I2C_H__
#define __HOCO_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

void ICACHE_FLASH_ATTR i2c_init(uint8 sda, uint8 scl);
bool ICACHE_FLASH_ATTR i2c_check(uint8 address);
uint8 ICACHE_FLASH_ATTR i2c_writeTo(uint8 address, uint8 *buf, uint16 len, bool sendStop);
uint8 ICACHE_FLASH_ATTR i2c_writeToReg(uint8 address, uint8 reg, uint8 *buf, uint16 len, bool sendStop);
uint8 ICACHE_FLASH_ATTR i2c_readFrom(uint8 address, uint8 *buf, uint16 len, bool sendStop);
uint8 ICACHE_FLASH_ATTR i2c_readFromReg(uint8 address, uint8 reg, uint8 *buf, uint16 len, bool sendStop);

#ifdef __cplusplus
}
#endif

#endif
