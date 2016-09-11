#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoPCF8591.h>

class HoCoPCF8591AOutClass : public HoCoDeviceClass {
private:
	HoCoPCF8591Class *_PCF8591Device;
	double _Min;
	double _Max;
	double _LastValue;
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoPCF8591AOutClass(char *Name, char *Config);
	~HoCoPCF8591AOutClass();
	void Start();
	void Stop();
	void SendStatus();
	void Write(double val);
};
