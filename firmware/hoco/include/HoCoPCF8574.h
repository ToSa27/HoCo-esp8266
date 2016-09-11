#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoI2C.h>

typedef enum {
	PCF8574_UNKNOWN,
	PCF8574_IDLE,
	PCF8574_BUSY,
} PCF8574State;

class HoCoPCF8574Class : public HoCoDeviceClass {
private:
	HoCoI2CClass *_I2CDevice;
	uint8_t _Address;
	PCF8574State _State;
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoPCF8574Class(char *Name, char *Config);
	~HoCoPCF8574Class();
	void Start();
	void Stop();
	bool ReadAll(uint8 *val, bool blocked = false);
	bool WriteAll(uint8_t val, bool blocked = false);
	bool Read(uint8 channel, bool *val);
	bool Write(uint8 channel, bool val);
};
