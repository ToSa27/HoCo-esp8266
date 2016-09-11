#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoPCF8574.h>

class HoCoPCF8574DInClass : public HoCoDeviceClass {
private:
	HoCoPCF8574Class *_PCF8574Device;
	bool _Inverted;
	uint8_t _Channel;
	uint8_t Get();
	uint16_t _Sample;
	uint8_t _DebounceState;
	char _Trigger; // n = NONE / r = RISING / f = FALLING / a = ALL
	ETSTimer LoopTimer;
	static void TimerLoop(void *data);
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoPCF8574DInClass(char *Name, char *Config);
	~HoCoPCF8574DInClass();
	void Start();
	void Stop();
	void SendStatus();
	void _TimerLoop();
};
