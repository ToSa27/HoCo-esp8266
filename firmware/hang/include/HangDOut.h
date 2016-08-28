#pragma once

#include <esp8266.h>
#include <HangDevice.h>

class HangDOutClass : public HangDeviceClass {
private:
	bool _Inverted;
	uint8_t _Pin;
	uint8_t Get();
	void Set(uint8_t Value);
protected:
	void SetStatus(char *Status);
	void SetConfig(char *Config);
	void SendConfig();
	void ReceivedSub(char *Topic, char *Data);
public:
	HangDOutClass(char *Name, char *Config, publish_callback publish);
	~HangDOutClass();
	void Start();
	void Stop();
	void SendStatus();
	char *DeviceType();
};
