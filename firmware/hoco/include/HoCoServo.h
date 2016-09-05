#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>

class HoCoServoClass : public HoCoDeviceClass {
private:
	uint8_t _Pin;
	void Set(uint8_t Value);
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoServoClass(char *Name, char *Config);
	~HoCoServoClass();
	void Start();
	void Stop();
	void SendStatus();
};
