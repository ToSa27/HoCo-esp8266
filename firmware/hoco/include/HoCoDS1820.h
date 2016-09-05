#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoOneWire.h>

#define DS1820_INVALID_TEMP	-999

typedef enum {
	DS1820_UNKNOWN,
	DS1820_IDLE,
	DS1820_READING,
} DS1820State;

class HoCoDS1820Class : public HoCoDeviceClass {
private:
	HoCoOneWireClass *_OneWireDevice;
	uint8_t _Pos;
	uint16_t _Interval;
	DS1820State _State;
	double _LastTemp;
	unsigned long _LastTime;
	uint8_t _Address[8];
	uint8_t _Type;
	ETSTimer LoopTimer;
	static void TimerLoop(void *data);
	bool TriggerRead();
	double Read();
protected:
//	void SetStatus(char *Status);
	void SetConfig(char *Config);
//	void SendConfig();
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoDS1820Class(char *Name, char *Config);
	~HoCoDS1820Class();
	void Start();
	void Stop();
	void SendStatus();
//	char *DeviceType();
//	void _InterruptCallback();
	void _TimerLoop();
};
