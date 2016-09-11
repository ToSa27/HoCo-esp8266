#include <HoCoI2C.h>

#include <CppJson.h>
#include <debug.h>

extern "C" {
#include <i2c.h>
}

HoCoI2CClass::HoCoI2CClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
	_State = I2C_UNKNOWN;
}

HoCoI2CClass::~HoCoI2CClass() {
}

void ICACHE_FLASH_ATTR HoCoI2CClass::SetConfig(char *Config) {
	DEBUG("HoCoI2CClass::SetConfig");
	_SdaPin = CppJson::jsonGetInt(Config, "sdapin");
	_SclPin = CppJson::jsonGetInt(Config, "sclpin");
	i2c_init(_SdaPin, _SclPin);
}

void ICACHE_FLASH_ATTR HoCoI2CClass::Start() {
	DEBUG("HoCoI2CClass::Start");
}

void ICACHE_FLASH_ATTR HoCoI2CClass::Stop() {
	DEBUG("HoCoI2CClass::Stop");
}

void ICACHE_FLASH_ATTR HoCoI2CClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoI2CClass::HandlePropertyMessage");
	if (ets_strstr(Topic, "$write")) {
		uint8 addr = CppJson::jsonGetInt(Data, "addr");
		uint8 reg = CppJson::jsonGetInt(Data, "reg");
		uint8 data = CppJson::jsonGetInt(Data, "data");
		i2c_writeByte(addr, reg, data);
	}
}

I2CState ICACHE_FLASH_ATTR HoCoI2CClass::GetState() {
	return _State;
}
/*
bool ICACHE_FLASH_ATTR HoCoI2CClass::block() {
	if (_State == I2C_IDLE) {
		_State = I2C_BUSY;
		return true;
	}
	return false;
}

void ICACHE_FLASH_ATTR HoCoI2CClass::unblock() {
	_State = I2C_IDLE;
}

bool ICACHE_FLASH_ATTR HoCoI2CClass::writeBytes(uint8 address, uint8 *values, uint8 length, bool blocked)
{
	if (!blocked)
		if (!block())
			return false;
	i2c_master_start();
	i2c_master_writeByte(address);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		return false;
	}
	for (uint8 i = 0; i < length; i++) {
		i2c_master_writeByte(values[i]);
		if (!i2c_master_checkAck())
		{
			i2c_master_stop();
			return false;
		}
	}
	i2c_master_stop();
	if (!blocked)
		unblock();
	return true;
}

bool ICACHE_FLASH_ATTR HoCoI2CClass::readBytes(uint8 address, uint8 *values, uint8 length, bool blocked)
{
	if (!blocked)
		if (!block())
			return false;
	if (values[0] > 0)
		if (!writeBytes(address, values, 1, true))
			return false;
	uint8 timeout = 100;
	do {
		i2c_master_start();
		i2c_master_writeByte(address + 1);
		if(!i2c_master_checkAck()){
			i2c_master_stop();
			i2c_master_wait(1000);
			continue;
		}
		break;
	} while (--timeout > 0);
	if (timeout == 0)
		return false;
	uint8 readed = 0;
	while ((readed < length) && (--timeout > 0)) {
		uint8 byte = i2c_master_readByte();
		values[readed++] = byte;
		i2c_master_setAck((readed == length));	// send the ACK or NAK as applicable
//		i2c_master_setDC(1, 0); // release SDA
	}
	i2c_master_stop();
	if (!blocked)
		unblock();
	return true;
}

bool HoCoI2CClass::readUint8(uint8 address, uint8 regaddr, uint8 *value)
{
	uint8 data[1];
	data[0] = regaddr;
	if(readBytes(address, data, 1)){
		*value = data[0];
		return true;
	}
	value = 0;
	return false;
}
*/

bool ICACHE_FLASH_ATTR HoCoI2CClass::CheckAddress(uint8 device_addr) // return true if something there
{
	return i2c_check(device_addr);
}

uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_writeByte(uint8 device_addr, uint8 data)
{
	uint8 d[1] = { data };
	return i2c_writeTo(device_addr, d, 1, true);
}

uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_writeByte(uint8 device_addr, uint8 register_addr, uint8 data)
{
	uint8 d[1] = { data };
	return i2c_writeToReg(device_addr, register_addr, d, 1, true);
}

uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_writeTwoBytes(uint8 device_addr, uint8 register_addr, uint8 data1, uint8 data2)
{
	uint8 d[2] = { data1, data2 };
	return i2c_writeToReg(device_addr, register_addr, d, 2, true);
}

uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_writeData(uint8 device_addr, uint8 register_addr, uint8* data, uint16_t length)
{
	return i2c_writeToReg(device_addr, register_addr, data, length, true);
}

uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_readData(uint8 device_addr, uint8* data, uint16_t length)
{
	return i2c_readFrom(device_addr, data, length, true);
}

uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_readData(uint8 device_addr, uint8 register_addr, uint8* data, uint16_t length)
{
	return i2c_readFromReg(device_addr, register_addr, data, length, true);
}

/*
uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_readDataWithParam(uint8 device_addr, uint8 register_addr, uint8 parm, uint8* data, uint16_t length)
{
	uint16_t cnt;

	i2c_master_start();						//Start

	i2c_master_writeByte(device_addr << 1);
	if(!i2c_master_checkAck())
	{
		i2c_master_stop();
		return 1;
	}

	i2c_master_writeByte(register_addr);	//Register address
	if(!i2c_master_checkAck())
	{
		i2c_master_stop();
		return 2;
	}

	i2c_master_writeByte(parm);	           //parameter
	if(!i2c_master_checkAck())
	{
		i2c_master_stop();
		return 2;
	}

	i2c_master_start();						//Start - now get stuff back

	i2c_master_writeByte((device_addr << 1) | 0x01);
	if(!i2c_master_checkAck())
	{
		i2c_master_stop();
		return 3;
	}

	if(length)
	{
		if(length > 1)							//Data
		{
			for(cnt = 0; cnt < length - 1; cnt++)
			{
				data[cnt] = i2c_master_readByte();
				i2c_master_send_ack();
			}
		}

		data[length - 1] = i2c_master_readByte();
		i2c_master_send_nack();
	}

	i2c_master_stop();						//Stop

	return 0;
}


uint8 ICACHE_FLASH_ATTR HoCoI2CClass::i2c_general(uint8 device_addr, uint8* data, uint16_t slength, uint16_t rlength)
{
	uint16_t cnt;
	uint8* tdata;

	if (slength)
	{
		i2c_master_start();						//Start
		i2c_master_writeByte(device_addr << 1);
		if(!i2c_master_checkAck())
		{
			i2c_master_stop();
			return 1;
		}

		tdata=data;
		cnt=slength;
		while (cnt--)
		{
			i2c_master_writeByte(*tdata++);	//Register address
			if(!i2c_master_checkAck())
			{
				i2c_master_stop();
				return 2;
			}
		}
	}

	if (rlength)
	{
		i2c_master_start();						//Start - now get stuff back
		i2c_master_writeByte((device_addr << 1) | 0x01);
		if(!i2c_master_checkAck())
		{
			i2c_master_stop();
			return 4;
		}

			if(rlength > 1)							//Data
			{
				for(cnt = 0; cnt < rlength - 1; cnt++)
				{
					data[cnt] = i2c_master_readByte();
					i2c_master_send_ack();
				}
			}

			data[rlength - 1] = i2c_master_readByte();
			i2c_master_send_nack();
	}
	if (slength || rlength) i2c_master_stop();	//Stop
	return 0;
}
*/
