#include <HoCoPCF8591.h>

#include <HoCoBase.h>
#include <CppJson.h>
#include <debug.h>
#include <tick.h>
#include <helper.h>

HoCoPCF8591Class::HoCoPCF8591Class(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
}

HoCoPCF8591Class::~HoCoPCF8591Class() {
}

void ICACHE_FLASH_ATTR HoCoPCF8591Class::SetConfig(char *Config) {
	DEBUG("HoCoPCF8591Class::SetConfig %s", Config);
	char *I2CName = CppJson::jsonGetString(Config, "I2C");
	_I2CDevice = (HoCoI2CClass*)HoCo::DeviceByName(I2CName);
	delete I2CName;
	_Address = 0x48 | CppJson::jsonGetInt(Config, "addr");
	_State = PCF8591_UNKNOWN;
}

void ICACHE_FLASH_ATTR HoCoPCF8591Class::Start() {
	DEBUG("HoCoPCF8591Class::Start");
	if (_I2CDevice->CheckAddress(_Address)) {
		_State = PCF8591_IDLE;
	} else {
		_State = PCF8591_UNKNOWN;
		WARN("No I2C device found at address %d", _Address);
	}
	_DAC = false;
}

void ICACHE_FLASH_ATTR HoCoPCF8591Class::Stop() {
	DEBUG("HoCoPCF8591Class::Stop");
	_State = PCF8591_UNKNOWN;
}

void ICACHE_FLASH_ATTR HoCoPCF8591Class::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoPCF8591Class::HandlePropertyMessage");
}

bool ICACHE_FLASH_ATTR HoCoPCF8591Class::Read(uint8 channel, uint8 *val) {
	DEBUG("HoCoPCF8591Class::Read");
	if (_State != PCF8591_IDLE)
		return false;
	_State = PCF8591_BUSY;
	bool s = false;
	if (channel < 4) {
		uint8 data[2];
		if (_I2CDevice->i2c_readData(_Address, channel | (_DAC ? PCF8591_REG_DAC : 0), val, 2) == 0) {
			*val = data[1];
			s = true;
		}
	}
	_State = PCF8591_IDLE;
	return s;
}

bool ICACHE_FLASH_ATTR HoCoPCF8591Class::Write(uint8 val) {
	DEBUG("HoCoPCF8591Class::Write");
	if (_State != PCF8591_IDLE)
		return false;
	_DAC = true;
	uint8 data[2];
	data[0] = PCF8591_REG_DAC;
	data[1] = val;
	bool s = _I2CDevice->i2c_writeData(_Address, 0, data, 2) == 0;
	_State = PCF8591_IDLE;
	return s;
}
