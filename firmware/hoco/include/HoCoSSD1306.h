#pragma once

#include <esp8266.h>
#include <HoCoDevice.h>
#include <HoCoI2C.h>

class HoCoSSD1306Class : public HoCoDeviceClass {
private:
	HoCoI2CClass *_I2CDevice;
	uint8_t _Address;
	void ssd1306_command(uint8 c);
	void ssd1306_command2(uint8 c1, uint8 c2);
protected:
	void SetConfig(char *Config);
	void HandlePropertyMessage(char *Topic, char *Data);
public:
	HoCoSSD1306Class(char *Name, char *Config);
	~HoCoSSD1306Class();
	void Start();
	void Stop();
	void InitDisplay();
	void Display();
	void ClearDisplay();
//	void SetXY(uint8 x, uint8 y);
//	void SendCharXY(unsigned char data, uint8 X, uint8 Y);
//	void SendStrXY(const char *string, uint8 X, uint8 Y);
};
