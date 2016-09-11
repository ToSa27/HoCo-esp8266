#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>

typedef enum {
	I2C_UNKNOWN,
	I2C_IDLE,
	I2C_BUSY,
} I2CState;

class HoCoI2CClass : public HoCoDeviceClass {
private:
	uint8_t _SdaPin;
	uint8_t _SclPin;
	I2CState _State;
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoI2CClass(char *Name, char *Config);
	~HoCoI2CClass();
	void Start();
	void Stop();
	I2CState GetState();
//    bool block();
//   	void unblock();
//   	bool writeBytes(uint8 address, uint8 *values, uint8 length, bool blocked = false);
//   	bool readBytes(uint8 address, uint8 *values, uint8 length, bool blocked = false);
//   	bool readUint8(uint8 address, uint8 regaddr, uint8 *value);
	bool CheckAddress(uint8 device_addr);
	uint8 i2c_writeByte(uint8 device_addr, uint8 data);
	uint8 i2c_writeByte(uint8 device_addr, uint8 register_addr, uint8 data);
	uint8 i2c_writeTwoBytes(uint8 device_addr, uint8 register_addr, uint8 data1, uint8 data2);
	uint8 i2c_writeData(uint8 device_addr, uint8 register_addr, uint8* data, uint16_t length);
//	uint8 i2c_writeSameData(uint8 device_addr, uint8 register_addr, uint8* data, uint16_t length);
	uint8 i2c_readData(uint8 device_addr, uint8* data, uint16_t length);
	uint8 i2c_readData(uint8 device_addr, uint8 register_addr, uint8* data, uint16_t length);
//	uint8 i2c_readDataWithParam(uint8 device_addr, uint8 register_addr, uint8 parm, uint8* data, uint16_t length);
//	uint8 i2c_readDataWithData(uint8 device_addr, uint8 register_addr, uint8 param, uint8* data, uint16_t length);
//   	uint8 i2c_general(uint8 device_addr, uint8* data, uint16_t slength, uint16_t rlength);
};
