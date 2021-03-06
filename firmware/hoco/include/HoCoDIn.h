#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>

class HoCoDInClass : public HoCoDeviceClass {
private:
	bool _Inverted;
	uint8_t _Pin;
	uint8_t Get();
	uint16_t _Sample;
	uint16_t _Debounce;
	uint8_t _DebounceState;
	uint32_t _DebounceLastMillis;
	char _Trigger; // n = NONE / r = RISING / f = FALLING / a = ALL
	ETSTimer LoopTimer;
	static void TimerLoop(void *data);
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoDInClass(char *Name, char *Config);
	~HoCoDInClass();
	void Start();
	void Stop();
	void SendStatus();
	void _TimerLoop();
};
