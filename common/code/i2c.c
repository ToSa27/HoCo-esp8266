#include <i2c.h>
#include <Arduino.h>
#include <pin.h>

static uint8 i2c_sda = 4;
static uint8 i2c_scl = 5;

#define SDA_LOW()   (GPES = (1 << i2c_sda)) //Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_HIGH()  (GPEC = (1 << i2c_sda)) //Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_READ()  ((GPI & (1 << i2c_sda)) != 0)
#define SCL_LOW()   (GPES = (1 << i2c_scl))
#define SCL_HIGH()  (GPEC = (1 << i2c_scl))
#define SCL_READ()  ((GPI & (1 << i2c_scl)) != 0)

#define I2C_DCOUNT				19
#define I2C_CLOCKSTRETCHLIMIT	230 * 3

static void i2c_delay(uint8 v) {
	unsigned int i;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	unsigned int reg;
	for (i = 0; i < v; i++)
		reg = GPI;
#pragma GCC diagnostic pop
}

void ICACHE_FLASH_ATTR i2c_init(uint8 sda, uint8 scl) {
	i2c_sda = sda;
	i2c_scl = scl;
	pinMode(i2c_sda, INPUT_PULLUP);
	pinMode(i2c_scl, INPUT_PULLUP);
}

static bool ICACHE_FLASH_ATTR i2c_write_start() {
	SCL_HIGH();
	SDA_HIGH();
	if (SDA_READ() == 0)
		return false;
	i2c_delay(I2C_DCOUNT);
	SDA_LOW();
	i2c_delay(I2C_DCOUNT);
	return true;
}

static bool ICACHE_FLASH_ATTR i2c_write_stop(){
	uint32_t i = 0;
	SCL_LOW();
	SDA_LOW();
	i2c_delay(I2C_DCOUNT);
	SCL_HIGH();
	while (SCL_READ() == 0 && (i++) < I2C_CLOCKSTRETCHLIMIT); // Clock stretching
	i2c_delay(I2C_DCOUNT);
	SDA_HIGH();
	i2c_delay(I2C_DCOUNT);
	return true;
}

static bool ICACHE_FLASH_ATTR i2c_write_bit(bool bit) {
	uint32_t i = 0;
	SCL_LOW();
	if (bit)
		SDA_HIGH();
	else
		SDA_LOW();
	i2c_delay(I2C_DCOUNT+1);
	SCL_HIGH();
	while (SCL_READ() == 0 && (i++) < I2C_CLOCKSTRETCHLIMIT);// Clock stretching
	i2c_delay(I2C_DCOUNT);
	return true;
}

static bool ICACHE_FLASH_ATTR i2c_read_bit() {
	uint32_t i = 0;
	SCL_LOW();
	SDA_HIGH();
	i2c_delay(I2C_DCOUNT+2);
	SCL_HIGH();
	while (SCL_READ() == 0 && (i++) < I2C_CLOCKSTRETCHLIMIT);// Clock stretching
	bool bit = SDA_READ();
	i2c_delay(I2C_DCOUNT);
	return bit;
}

static bool ICACHE_FLASH_ATTR i2c_write_byte(uint8 byte) {
	uint8 bit;
	for (bit = 0; bit < 8; bit++) {
		i2c_write_bit(byte & 0x80);
		byte <<= 1;
	}
	return !i2c_read_bit();//NACK/ACK
}

static uint8 ICACHE_FLASH_ATTR i2c_read_byte(bool nack) {
	uint8 byte = 0;
	uint8 bit;
	for (bit = 0; bit < 8; bit++)
		byte = (byte << 1) | i2c_read_bit();
	i2c_write_bit(nack);
	return byte;
}

bool ICACHE_FLASH_ATTR i2c_check(uint8 address) {
	if (!i2c_write_start())
		return false;
	bool ret = i2c_write_byte(((address << 1) | 0) & 0xFF);
	i2c_write_stop();
	return ret;
}

uint8 ICACHE_FLASH_ATTR i2c_writeTo(uint8 address, uint8 *buf, uint16 len, bool sendStop) {
	unsigned int i;
	if(!i2c_write_start())
		return 4;//line busy
	if(!i2c_write_byte(((address << 1) | 0) & 0xFF)) {
		if (sendStop)
			i2c_write_stop();
		return 2; //received NACK on transmit of address
	}
	for (i = 0; i < len; i++) {
		if (!i2c_write_byte(buf[i])) {
			if (sendStop)
				i2c_write_stop();
			return 3;//received NACK on transmit of data
		}
	}
	if(sendStop)
		i2c_write_stop();
	i = 0;
	while (SDA_READ() == 0 && (i++) < 10) {
		SCL_LOW();
		i2c_delay(I2C_DCOUNT);
		SCL_HIGH();
		i2c_delay(I2C_DCOUNT);
	}
	return 0;
}

uint8 ICACHE_FLASH_ATTR i2c_writeToReg(uint8 address, uint8 reg, uint8 *buf, uint16 len, bool sendStop) {
	unsigned int i;
	if(!i2c_write_start())
		return 4;//line busy
	if(!i2c_write_byte(((address << 1) | 0) & 0xFF)) {
		if (sendStop)
			i2c_write_stop();
		return 2; //received NACK on transmit of address
	}
	if (!i2c_write_byte(reg)) {
		if (sendStop)
			i2c_write_stop();
		return 3;//received NACK on transmit of data
	}
	for (i = 0; i < len; i++) {
		if (!i2c_write_byte(buf[i])) {
			if (sendStop)
				i2c_write_stop();
			return 3;//received NACK on transmit of data
		}
	}
	if(sendStop)
		i2c_write_stop();
	i = 0;
	while (SDA_READ() == 0 && (i++) < 10) {
		SCL_LOW();
		i2c_delay(I2C_DCOUNT);
		SCL_HIGH();
		i2c_delay(I2C_DCOUNT);
	}
	return 0;
}

uint8 ICACHE_FLASH_ATTR i2c_readFrom(uint8 address, uint8 *buf, uint16 len, bool sendStop) {
	unsigned int i;
	if (!i2c_write_start())
		return 4;//line busy
	if (!i2c_write_byte(((address << 1) | 1) & 0xFF)) {
		if (sendStop)
			i2c_write_stop();
		return 2;//received NACK on transmit of address
	}
	for (i = 0; i < (len - 1); i++)
		buf[i] = i2c_read_byte(false);
	buf[len - 1] = i2c_read_byte(true);
	if (sendStop)
		i2c_write_stop();
	i = 0;
	while (SDA_READ() == 0 && (i++) < 10) {
		SCL_LOW();
		i2c_delay(I2C_DCOUNT);
		SCL_HIGH();
		i2c_delay(I2C_DCOUNT);
	}
	return 0;
}

uint8 ICACHE_FLASH_ATTR i2c_readFromReg(uint8 address, uint8 reg, uint8 *buf, uint16 len, bool sendStop) {
	unsigned int i;
	uint8 ret = i2c_writeToReg(address, reg, buf, 0, false);
	if (reg != 0) {
		i2c_write_stop();
		return ret;
	}
	if (!i2c_write_start())
		return 4;//line busy
	if (!i2c_write_byte(((address << 1) | 1) & 0xFF)) {
		if (sendStop)
			i2c_write_stop();
		return 2;//received NACK on transmit of address
	}
	for (i = 0; i < (len - 1); i++)
		buf[i] = i2c_read_byte(false);
	buf[len - 1] = i2c_read_byte(true);
	if (sendStop)
		i2c_write_stop();
	i = 0;
	while (SDA_READ() == 0 && (i++) < 10) {
		SCL_LOW();
		i2c_delay(I2C_DCOUNT);
		SCL_HIGH();
		i2c_delay(I2C_DCOUNT);
	}
	return 0;
}
