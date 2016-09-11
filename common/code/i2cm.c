/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 * Improvements August 2016 by Peter Scargill and ESPRUINO code have quadrupled the speed
 * while adding in clock stretching while retaining overal compatibility
 *
 * FileName: i2c_master.c
 *
 * Description: i2c master API
 *
 * Modification history:
 *     2014/3/12, v1.0 create this file.
*******************************************************************************/
#include <esp8266.h>

#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <pin.h>

#include <i2cm.h>

LOCAL uint8 sdagpio = 4;
LOCAL uint8 sclgpio = 5;

LOCAL uint8 m_nLastSDA;
LOCAL uint8 m_nLastSCL;

LOCAL uint8_t i2cStarted=0; // ensure that the port initialisation only happens once

#define I2C_DELAY_8 1 // up to now, in reality - reducing them to 1,1,1 still works reliably and makes big speed difference
#define I2C_DELAY_5 1
#define I2C_DELAY_3 1

LOCAL uint8 ICACHE_FLASH_ATTR
i2c_master_setDC(uint8 sda, uint8 scl) {
	uint32 set = ((sda&1)<<sdagpio) | ((scl&1)<<sclgpio);
	uint32 both = (1<<sdagpio) | (1<<sclgpio);
	gpio_output_set(set, set^both, both, 0);
}


/******************************************************************************
 * FunctionName : i2c_master_getDC
 * Description  : Internal used function -
 *                    get i2c SDA bit value
 * Parameters   : NONE
 * Returns      : uint8 - SDA bit value
*******************************************************************************/
LOCAL uint8 ICACHE_FLASH_ATTR
i2c_master_getDC(void)
{
   uint8 sda_out;
   sda_out = GPIO_INPUT_GET(GPIO_ID_PIN(sdagpio));
    return sda_out;

}


/******************************************************************************
 * FunctionName : i2c_master_init
 * Description  : initilize I2C bus to enable i2c operations
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_init(void)
{
	i2c_master_setDC(1, 1);
}

/******************************************************************************
 * FunctionName : i2c_master_gpio_init
 * Description  : config SDA and SCL gpio to open-drain output mode,
 *                mux and gpio num defined in i2c_master.h
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_gpio_init(uint8 sda, uint8 scl)
{
	if (i2cStarted) return;
	i2cStarted=1;

	sdagpio = sda;
	sclgpio = scl;

    ETS_GPIO_INTR_DISABLE() ;
//    ETS_INTR_LOCK();

    pinMode(sdagpio, OUTPUT_OPEN_DRAIN);
    pinMode(sclgpio, OUTPUT_OPEN_DRAIN);
//    PIN_FUNC_SELECT(I2C_MASTER_SDA_MUX, I2C_MASTER_SDA_FUNC);
//    PIN_FUNC_SELECT(I2C_MASTER_SCL_MUX, I2C_MASTER_SCL_FUNC);

//    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
//    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << I2C_MASTER_SDA_GPIO));
//    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
//    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << I2C_MASTER_SCL_GPIO));

//    I2C_MASTER_SDA_HIGH_SCL_HIGH();
//    i2c_master_sda_high_scl_high();
    gpio_output_set(1<<sdagpio | 1<<sclgpio, 0, 1<<sdagpio | 1<<sclgpio, 0);

    ETS_GPIO_INTR_ENABLE() ;
//    ETS_INTR_UNLOCK();

    i2c_master_init();
}

/******************************************************************************
 * FunctionName : i2c_master_start
 * Description  : set i2c to send state
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_start(void)
{
	i2c_master_setDC(1, m_nLastSCL);
    i2c_master_wait(I2C_DELAY_5);
    i2c_master_setDC(1, 1);
    i2c_master_wait(I2C_DELAY_5);	// sda 1, scl 1
    i2c_master_setDC(0, 1);
    i2c_master_wait(I2C_DELAY_5);	// sda 0, scl 1
}

/******************************************************************************
 * FunctionName : i2c_master_stop
 * Description  : set i2c to stop sending state
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_stop(void)
{
	i2c_master_setDC(0, 1);
    while (!GPIO_INPUT_GET(GPIO_ID_PIN(sclgpio))) {}
    i2c_master_wait(I2C_DELAY_5);	// sda 0, scl 1
    i2c_master_setDC(1, 1);
    i2c_master_wait(I2C_DELAY_5);	// sda 1, scl 1
}

/******************************************************************************
 * FunctionName : i2c_master_setAck
 * Description  : set ack to i2c bus as level value
 * Parameters   : uint8 level - 0 or 1
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_setAck(uint8 level)
{

	i2c_master_setDC(level, 0);
    i2c_master_wait(I2C_DELAY_5);	// sda level, scl 0
    i2c_master_setDC(level, 1);
    while (!GPIO_INPUT_GET(GPIO_ID_PIN(sclgpio))) {}
    i2c_master_setDC(level, 0);
    i2c_master_wait(I2C_DELAY_5);	// sda level, scl 0
    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_DELAY_5);
}

/******************************************************************************
 * FunctionName : i2c_master_getAck
 * Description  : confirm if peer send ack
 * Parameters   : NONE
 * Returns      : uint8 - ack value, 0 or 1
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
i2c_master_getAck(void)
{
    uint8 retVal;
    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_DELAY_5);
    i2c_master_setDC(1, 1);
    while (!GPIO_INPUT_GET(GPIO_ID_PIN(sclgpio))) {}

    retVal = i2c_master_getDC();
    i2c_master_wait(I2C_DELAY_5);
    i2c_master_setDC(1, 0);
    i2c_master_wait(I2C_DELAY_5);

    return retVal;
}

/******************************************************************************
* FunctionName : i2c_master_checkAck
* Description  : get dev response
* Parameters   : NONE
* Returns      : true : get ack ; false : get nack
*******************************************************************************/
bool ICACHE_FLASH_ATTR
i2c_master_checkAck(void)
{
    if(i2c_master_getAck()){
        return FALSE;
    }else{
        return TRUE;
    }
}

/******************************************************************************
* FunctionName : i2c_master_send_ack
* Description  : response ack
* Parameters   : NONE
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_send_ack(void)
{
    i2c_master_setAck(0x0);
}
/******************************************************************************
* FunctionName : i2c_master_send_nack
* Description  : response nack
* Parameters   : NONE
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_send_nack(void)
{
    i2c_master_setAck(0x1);
}

uint8_t ICACHE_FLASH_ATTR
i2c_master_check_stretch(void)
{
	uint16_t del;
	del=10000;
	while (!GPIO_INPUT_GET(GPIO_ID_PIN(sclgpio))) { if (--del==0) { I2CERROR=1; i2c_master_setDC(1,1); return -1 ; } i2c_master_wait(1); }
	I2CERROR=0;
	return 0;
}

/******************************************************************************
 * FunctionName : i2c_master_readByte
 * Description  : read Byte from i2c bus
 * Parameters   : NONE
 * Returns      : uint8 - readed value
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
i2c_master_readByte(void)
{
    uint8 retVal = 0;
    uint8 k, i;


    for (i = 0; i < 8; i++) {
        i2c_master_wait(I2C_DELAY_5);
        i2c_master_setDC(1,0);
        i2c_master_wait(I2C_DELAY_5);	// sda 1, scl 0
        i2c_master_setDC(1,1);
        if (i2c_master_check_stretch()) return 0; // with I2CERROR set
        k = i2c_master_getDC();
        //i2c_master_wait(I2C_DELAY_5);
        if (i == 7) {
            i2c_master_wait(I2C_DELAY_3);   ////
        }
        k <<= (7 - i);
        retVal |= k;
    }
    i2c_master_setDC(1,0);
    i2c_master_wait(I2C_DELAY_5);	// sda 1, scl 0

    return retVal;
}

/******************************************************************************
 * FunctionName : i2c_master_writeByte
 * Description  : write wrdata value(one byte) into i2c
 * Parameters   : uint8 wrdata - write value
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
i2c_master_writeByte(uint8 wrdata)
{
    uint8 dat;
    sint8 i;

    for (i = 7; i >= 0; i--) {
        dat = wrdata >> i;
        i2c_master_setDC(dat,0);
      //  i2c_master_wait(I2C_DELAY_5);
        i2c_master_setDC(dat,1);
        if (i2c_master_check_stretch()) return; // with I2CERROR set
       i2c_master_setDC(dat,0);
    }
}
