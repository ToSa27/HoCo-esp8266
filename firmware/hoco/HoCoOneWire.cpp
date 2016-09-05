#include <HoCoOneWire.h>

#include <CppJson.h>
#include <debug.h>
#include <pin.h>

HoCoOneWireClass::HoCoOneWireClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
	_State = ONEWIRE_UNKNOWN;
}

HoCoOneWireClass::~HoCoOneWireClass() {
	delete(_OneWire);
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::SetConfig(char *Config) {
	DEBUG("HangOneWireClass::SetConfig");
	_Pin = CppJson::jsonGetInt(Config, "pin");
	DEBUG("pin %d", _Pin);
	_OneWire = new OneWire(_Pin);
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::Start() {
	DEBUG("HoCoOneWireClass::Start");
	Search();
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::Stop() {
	DEBUG("HoCoOneWireClass::Stop");
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoOneWireClass::HandlePropertyMessage");
}

ONEWIREState ICACHE_FLASH_ATTR HoCoOneWireClass::GetState() {
	return _State;
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::Search() {
	if (_State == ONEWIRE_SEARCHING)
		return;
	_State = ONEWIRE_SEARCHING;
	_Devices = 0;
	_OneWire->reset_search();
	uint8 NewAddr[8];
	while (_OneWire->search(_Address[_Devices], true))
		_Devices++;
	_State = ONEWIRE_IDLE;
}

bool ICACHE_FLASH_ATTR HoCoOneWireClass::Find(uint8 type[], size_t types, uint8 pos, uint8 address[]) {
	uint8 lp = 0;
	for (uint8 i = 0; i < _Devices; i++) {
		for (uint8 j = 0; j < types; j++)
			if (_Address[i][0] == type[j]) {
				lp++;
				if (lp == pos) {
					ets_memcpy(address, _Address[i], 8);
					return true;
				}
			}
	}
	return false;
}

bool ICACHE_FLASH_ATTR HoCoOneWireClass::block() {
	if (_State == ONEWIRE_IDLE) {
		_State = ONEWIRE_BUSY;
		return true;
	}
	return false;
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::unblock() {
	_State = ONEWIRE_IDLE;
}

uint8_t ICACHE_FLASH_ATTR HoCoOneWireClass::reset() {
	return _OneWire->reset();
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::select(const uint8_t rom[8]) {
	_OneWire->select(rom);
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::skip() {
	_OneWire->skip();
}

uint8_t ICACHE_FLASH_ATTR HoCoOneWireClass::read() {
	return _OneWire->read();
}

void ICACHE_FLASH_ATTR HoCoOneWireClass::write(uint8_t v, uint8_t power) {
	_OneWire->write(v, power);
}
