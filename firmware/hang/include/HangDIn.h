#pragma once

#include <HangDevice.h>
#include <esp8266.h>

class HangDInClass : public HangDeviceClass {
private:
	uint8_t _Pin;
	uint8_t _LastState;
	uint16_t _Debounce;
	uint8_t _Trigger; // 0 = NONE / 1 = RISING / 2 = FALLING / 3 = BOTH / 4 = POLLING
	unsigned long _LastInterrupt;
	static void InterruptCallback(void *data);
	ETSTimer LoopTimer;
	static void TimerLoop(void *data);
protected:
	void SetStatus(char *Status);
	void SetConfig(char *Config);
	void SendConfig();
	void ReceivedSub(char *Topic, char *Data);
public:
	HangDInClass(char *Name, char *Config, publish_callback publish);
	~HangDInClass();
	void Start();
	void Stop();
	void SendStatus();
	char *DeviceType();
	void _InterruptCallback();
	void _TimerLoop();
};
