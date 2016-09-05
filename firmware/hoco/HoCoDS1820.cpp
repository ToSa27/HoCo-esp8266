#include <HoCoDS1820.h>

#include <HoCoBase.h>
#include <CppJson.h>
#include <debug.h>
#include <tick.h>
#include <helper.h>

HoCoDS1820Class::HoCoDS1820Class(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
}

HoCoDS1820Class::~HoCoDS1820Class() {
}

void ICACHE_FLASH_ATTR HoCoDS1820Class::SendStatus() {
	DEBUG("HoCoDS1820Class::SendStatus");
	if (_LastTemp != DS1820_INVALID_TEMP) {
		char temp[8];
		dtostrf(_LastTemp, 2, temp);
		Publish((char*)"temperature", (char*)temp, true);
	}
}

void ICACHE_FLASH_ATTR HoCoDS1820Class::SetConfig(char *Config) {
	DEBUG("HoCoDS1820Class::SetConfig %s", Config);
	char *OneWireName = CppJson::jsonGetString(Config, "OneWire");
	_OneWireDevice = (HoCoOneWireClass*)HoCo::DeviceByName(OneWireName);
	_Pos = CppJson::jsonGetInt(Config, "pos");
	DEBUG("position %d", _Pos);
	_Interval = CppJson::jsonGetInt(Config, "interval");
	DEBUG("interval %d", _Interval);
}

void ICACHE_FLASH_ATTR HoCoDS1820Class::Start() {
	DEBUG("HoCoDS1820Class::Start");
	_State = DS1820_UNKNOWN;
	_LastTemp = DS1820_INVALID_TEMP;
	ets_timer_disarm(&LoopTimer);
	ets_timer_setfn(&LoopTimer, (ETSTimerFunc *)TimerLoop, (void*)this);
	ets_timer_arm_new(&LoopTimer, 1000, 1, 0);
}

void ICACHE_FLASH_ATTR HoCoDS1820Class::Stop() {
	DEBUG("HoCoDS1820Class::Stop");
	ets_timer_disarm(&LoopTimer);
}

void ICACHE_FLASH_ATTR HoCoDS1820Class::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoDS1820Class::HandlePropertyMessage");
}

void ICACHE_FLASH_ATTR HoCoDS1820Class::TimerLoop(void *data) {
	HoCoDS1820Class* h = (HoCoDS1820Class*)data;
	h->_TimerLoop();
}

bool ICACHE_FLASH_ATTR HoCoDS1820Class::TriggerRead() {
	DEBUG("HoCoDS1820Class::TriggerRead");
	// ToDo : only 9 bit resolution required instead of 12 (.5 deg)
	if (_OneWireDevice->block()) {
		_OneWireDevice->reset();
		_OneWireDevice->select(_Address);
	//	_OneWireDevice->skip();
		_OneWireDevice->write(0x44, 1);
		_OneWireDevice->unblock();
		return true;
	}
	return false;
}

double ICACHE_FLASH_ATTR HoCoDS1820Class::Read() {
//	debugf("HangDS1820Class::Read");
	uint8_t data[12];
	if (_OneWireDevice->block()) {
		_OneWireDevice->reset();
		_OneWireDevice->select(_Address);
		//	_OneWireDevice->skip();
		_OneWireDevice->write(0xBE, 1);
		for (uint8_t i = 0; i < 9; i++)
			data[i] = _OneWireDevice->read();
		_OneWireDevice->unblock();
		int16_t raw = (data[1] << 8) | data[0];
		if (_Type) {
			raw = raw << 3; // 9 bit resolution default
			if (data[7] == 0x10)
				raw = (raw & 0xFFF0) + 12 - data[6];
		} else {
			uint8_t cfg = (data[4] & 0x60);
			if (cfg == 0x00)
				raw = raw & ~7;  // 9 bit resolution, 93.75 ms
			else if (cfg == 0x20)
				raw = raw & ~3; // 10 bit res, 187.5 ms
			else if (cfg == 0x40)
				raw = raw & ~1; // 11 bit res, 375 ms
			//// default is 12 bit resolution, 750 ms conversion time
		}
		double d = (double)raw / 16.0;
		DEBUG("DS1820::Read*100 : %d (%d)", (int)(d * 100.0), (int)(_LastTemp * 100.0));
		return d;
	}
	return DS1820_INVALID_TEMP;
}

void ICACHE_FLASH_ATTR HoCoDS1820Class::_TimerLoop() {
	if (!_OneWireDevice->GetState() == ONEWIRE_IDLE)
		return;
	DS1820State newState = _State;
	if (_State == DS1820_UNKNOWN) {
		uint8 types[3] = { 0x10, 0x28, 0x22 };
		if (_OneWireDevice->Find(types, 3, _Pos, _Address)) {
			DEBUG("DS1820 Address: %d %d %d %d %d %d %d %d", _Address[0], _Address[1], _Address[2], _Address[3], _Address[4], _Address[5], _Address[6], _Address[7]);
			_Type = (_Address[0] == 0x10) ? 1 : 0;
			DEBUG("DS1820 Type: %d", _Type);
			newState = DS1820_IDLE;
		}
	} else if (_State == DS1820_IDLE) {
		if (_Interval > 0) {
			unsigned long m = mymillis();
			if (m - _LastTime > _Interval) {
				if (TriggerRead()) {
					_LastTime = m;
					newState = DS1820_READING;
				}
			}
		}
	} else if (_State == DS1820_READING) {
		if (_Interval > 0) {
			unsigned long m = mymillis();
			if (m - _LastTime > 750) {
				double newTemp = Read();
				if (newTemp != DS1820_INVALID_TEMP) {
					int diff = (int)(newTemp * 100.0) - (int)(_LastTemp * 100.0);
					if (diff > 25 || diff < -25) {
						newTemp = (double)(((int)((newTemp + 0.25) * 2.0)) / 2.0);
						if (newTemp != _LastTemp) {
							_LastTemp = newTemp;
							SendStatus();
						}
					}
					newState = DS1820_IDLE;
				}
			}
		}
	}
	_State = newState;
}
