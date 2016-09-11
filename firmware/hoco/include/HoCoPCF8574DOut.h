#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoPCF8574.h>

class HoCoPCF8574DOutClass : public HoCoDeviceClass {
private:
	HoCoPCF8574Class *_PCF8574Device;
	uint8 _Channel;
	bool _Inverted;
	uint8_t Get();
	void Set(uint8_t Value);
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoPCF8574DOutClass(char *Name, char *Config);
	~HoCoPCF8574DOutClass();
	void Start();
	void Stop();
	void SendStatus();
};
