#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>

class HoCoCounterClass : public HoCoDeviceClass {
private:
	uint8_t _Pin;
	uint16_t _Interval;
	uint32_t _Count;
	char _Trigger; // r = RISING / f = FALLING / a = ANY
	ETSTimer LoopTimer;
	static void TimerLoop(void *data);
	static void InterruptCallback(void *data);
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoCounterClass(char *Name, char *Config);
	~HoCoCounterClass();
	void Start();
	void Stop();
	void SendStatus();
	void _TimerLoop();
	void _InterruptCallback();
};
