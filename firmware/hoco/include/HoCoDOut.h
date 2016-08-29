#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>

class HoCoDOutClass : public HoCoDeviceClass {
private:
	bool _Inverted;
	uint8_t _Pin;
	uint8_t Get();
	void Set(uint8_t Value);
protected:
//	void SetStatus(char *Status);
	void SetConfig(char *Config);
//	void SendConfig();
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoDOutClass(char *Name, char *Config, subscribe_callback subscribe, publish_callback publish);
	~HoCoDOutClass();
	void Start();
	void Stop();
	void SendStatus();
//	char *DeviceType();
};
