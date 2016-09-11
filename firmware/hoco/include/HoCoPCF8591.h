#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoI2C.h>

typedef enum {
	PCF8591_UNKNOWN,
	PCF8591_IDLE,
	PCF8591_BUSY,
} PCF8591State;

#define PCF8591_INVALID_VAL		-9999
#define PCF8591_REG_DAC			0x40

class HoCoPCF8591Class : public HoCoDeviceClass {
private:
	HoCoI2CClass *_I2CDevice;
	uint8_t _Address;
	bool _DAC;
	PCF8591State _State;
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoPCF8591Class(char *Name, char *Config);
	~HoCoPCF8591Class();
	void Start();
	void Stop();
	bool Read(uint8 channel, uint8_t *val);
	bool Write(uint8_t val);
};
