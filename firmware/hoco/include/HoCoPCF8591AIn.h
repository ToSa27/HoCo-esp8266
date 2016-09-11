#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoPCF8591.h>

class HoCoPCF8591AInClass : public HoCoDeviceClass {
private:
	HoCoPCF8591Class *_PCF8591Device;
	uint8_t _Channel;
	double _Min;
	double _Max;
	uint16_t _Interval;
	ETSTimer LoopTimer;
	double _LastValue;
	static void TimerLoop(void *data);
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoPCF8591AInClass(char *Name, char *Config);
	~HoCoPCF8591AInClass();
	void Start();
	void Stop();
	void SendStatus();
	void Read();
	void _TimerLoop();
};
