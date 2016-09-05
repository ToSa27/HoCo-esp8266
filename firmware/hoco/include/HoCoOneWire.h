#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <OneWire.h>

#define ONEWIRE_MAX_DEVICES 4

typedef enum {
	ONEWIRE_UNKNOWN,
	ONEWIRE_SEARCHING,
	ONEWIRE_IDLE,
	ONEWIRE_BUSY,
} ONEWIREState;

class HoCoOneWireClass : public HoCoDeviceClass {
private:
	uint8_t _Pin;
	OneWire *_OneWire;
	ONEWIREState _State;
	uint8 _Devices;
	uint8 _Address[8][ONEWIRE_MAX_DEVICES];
	void Search();
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoOneWireClass(char *Name, char *Config);
	~HoCoOneWireClass();
	void Start();
	void Stop();
//	void SendStatus();
	ONEWIREState GetState();
	bool Find(uint8 type[], size_t types, uint8 pos, uint8 address[]);
    uint8_t reset(void);
    void select(const uint8_t rom[8]);
    void skip(void);
    uint8_t read(void);
    void write(uint8_t v, uint8_t power = 0);
    bool block();
   	void unblock();
};
