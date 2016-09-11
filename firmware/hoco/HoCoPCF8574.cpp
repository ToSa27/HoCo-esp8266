#include <HoCoPCF8574.h>

#include <HoCoBase.h>
#include <CppJson.h>
#include <debug.h>
#include <tick.h>
#include <helper.h>

HoCoPCF8574Class::HoCoPCF8574Class(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
}

HoCoPCF8574Class::~HoCoPCF8574Class() {
}

void ICACHE_FLASH_ATTR HoCoPCF8574Class::SetConfig(char *Config) {
	DEBUG("HoCoPCF8574Class::SetConfig %s", Config);
	char *I2CName = CppJson::jsonGetString(Config, "I2C");
	_I2CDevice = (HoCoI2CClass*)HoCo::DeviceByName(I2CName);
	delete I2CName;
	_Address = 0x20 | CppJson::jsonGetInt(Config, "addr");
	_State = PCF8574_UNKNOWN;
}

void ICACHE_FLASH_ATTR HoCoPCF8574Class::Start() {
	DEBUG("HoCoPCF8574Class::Start");
	if (_I2CDevice->CheckAddress(_Address)) {
		_State = PCF8574_IDLE;
	} else {
		_State = PCF8574_UNKNOWN;
		WARN("No I2C device found at address %d", _Address);
	}
}

void ICACHE_FLASH_ATTR HoCoPCF8574Class::Stop() {
	DEBUG("HoCoPCF8574Class::Stop");
	_State = PCF8574_UNKNOWN;
}

void ICACHE_FLASH_ATTR HoCoPCF8574Class::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoPCF8574Class::HandlePropertyMessage");
}

bool ICACHE_FLASH_ATTR HoCoPCF8574Class::ReadAll(uint8 *val, bool blocked) {
	DEBUG("HoCoPCF8574Class::ReadAll");
	if (!blocked) {
		if (_State != PCF8574_IDLE)
			return false;
		_State = PCF8574_BUSY;
	}
	bool s = false;
	uint8 data;
	if (_I2CDevice->i2c_readData(_Address, &data, 1) == 0) {
		*val = data;
		s = true;
	}
	if (!blocked)
		_State = PCF8574_IDLE;
	return s;
}

bool ICACHE_FLASH_ATTR HoCoPCF8574Class::WriteAll(uint8_t val, bool blocked) {
	DEBUG("HoCoPCF8574Class::WriteAll");
	if (!blocked) {
		if (_State != PCF8574_IDLE)
			return false;
		_State = PCF8574_BUSY;
	}
	bool s = false;
	if (_I2CDevice->i2c_writeByte(_Address, val) == 0)
		s = true;
	if (!blocked)
		_State = PCF8574_IDLE;
	return s;
}

bool ICACHE_FLASH_ATTR HoCoPCF8574Class::Read(uint8 channel, bool *val) {
	DEBUG("HoCoPCF8574Class::Read");
	if (_State != PCF8574_IDLE)
		return false;
	_State = PCF8574_BUSY;
	bool s = false;
	uint8 all;
	if (ReadAll(&all, true)) {
		*val = (all & (1 << channel)) > 0;
		s = true;
	}
	_State = PCF8574_IDLE;
	return s;
}

bool ICACHE_FLASH_ATTR HoCoPCF8574Class::Write(uint8 channel, bool val) {
	DEBUG("HoCoPCF8574Class::Write");
	if (_State != PCF8574_IDLE)
		return false;
	bool s = false;
	uint8 all;
	if (ReadAll(&all, true)) {
		if (val)
			all |= (1 << channel);
		else
			all &= !(1 << channel);
		if (WriteAll(all, true))
			s = true;
	}
	_State = PCF8574_IDLE;
	return s;
}
