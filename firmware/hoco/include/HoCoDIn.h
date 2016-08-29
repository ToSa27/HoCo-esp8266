#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>

class HoCoDInClass : public HoCoDeviceClass {
private:
	bool _Inverted;
	uint8_t _Pin;
//	uint8_t _LastState;
	uint8_t Get();
	uint16_t _Debounce;
	uint8_t _DebounceState;
	uint32_t _DebounceLastMillis;
	uint8_t _Trigger; // 0 = NONE / 1 = RISING / 2 = FALLING / 3 = BOTH / 4 = POLLING
//	unsigned long _LastInterrupt;
//	static void InterruptCallback(void *data);
	ETSTimer LoopTimer;
	static void TimerLoop(void *data);
protected:
//	void SetStatus(char *Status);
	void SetConfig(char *Config);
//	void SendConfig();
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoDInClass(char *Name, char *Config, subscribe_callback subscribe, publish_callback publish);
	~HoCoDInClass();
	void Start();
	void Stop();
	void SendStatus();
//	char *DeviceType();
//	void _InterruptCallback();
	void _TimerLoop();
};
