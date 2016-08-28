#include <pin.h>
#include <eagle_soc.h>
#include <user_interface.h>

#define MODIFY_PERI_REG(reg, mask, val) WRITE_PERI_REG(reg, (READ_PERI_REG(reg) & (~mask)) | (uint32_t) val)

static const uint32_t g_pin_muxes[PINCOUNT] = { PERIPHS_IO_MUX_GPIO0_U,
		                                        PERIPHS_IO_MUX_U0TXD_U,
												PERIPHS_IO_MUX_GPIO2_U,
												PERIPHS_IO_MUX_U0RXD_U,
												PERIPHS_IO_MUX_GPIO4_U,
												PERIPHS_IO_MUX_GPIO5_U,
												0, 0, 0, 0, 0, 0,
												PERIPHS_IO_MUX_MTDI_U,
												PERIPHS_IO_MUX_MTCK_U,
												PERIPHS_IO_MUX_MTMS_U,
												PERIPHS_IO_MUX_MTDO_U };

static const uint32_t g_pin_funcs[PINCOUNT] = { FUNC_GPIO0,
												FUNC_GPIO1,
												FUNC_GPIO2,
												FUNC_GPIO3,
												FUNC_GPIO4,
												FUNC_GPIO5,
												0, 0, 0, 0, 0, 0,
												FUNC_GPIO12,
												FUNC_GPIO13,
												FUNC_GPIO14,
												FUNC_GPIO15 };

void ICACHE_FLASH_ATTR pinMode(uint8_t pin, uint8_t mode) {
    if(pin == 16) {
        uint32_t val = (mode == OUTPUT) ? 1 : 0;
        MODIFY_PERI_REG(PAD_XPD_DCDC_CONF, 0x43, 1);
        MODIFY_PERI_REG(RTC_GPIO_CONF, 1, 0);
        MODIFY_PERI_REG(RTC_GPIO_ENABLE, 1, val);
        return;
    }
    uint32_t mux = g_pin_muxes[pin];
    if(mode == INPUT) {
        gpio_output_set(0, 0, 0, 1 << pin);
        PIN_PULLUP_DIS(mux);
    } else if(mode == INPUT_PULLUP) {
        gpio_output_set(0, 0, 0, 1 << pin);
//        PIN_PULLDWN_DIS(mux);
        PIN_PULLUP_EN(mux);
    } else if(mode == INPUT_PULLDOWN) {
        gpio_output_set(0, 0, 0, 1 << pin);
        PIN_PULLUP_DIS(mux);
//        PIN_PULLDWN_EN(mux);
    } else if(mode == OUTPUT) {
        gpio_output_set(0, 0, 1 << pin, 0);
    } else if(mode == OUTPUT_OPEN_DRAIN) {
        GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(pin)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(pin))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE));
        GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << pin));
    }
    PIN_FUNC_SELECT(mux, g_pin_funcs[pin]);
}

void ICACHE_FLASH_ATTR pullup(uint16_t pin)
{
	if (pin >= 16) return;
//	PIN_PULLDWN_DIS(g_pin_muxes[pin]);
	PIN_PULLUP_EN(g_pin_muxes[pin]);
}

void ICACHE_FLASH_ATTR pulldown(uint16_t pin)
{
	if (pin >= 16) return;
	PIN_PULLUP_DIS(g_pin_muxes[pin]);
//	PIN_PULLDWN_EN(g_pin_muxes[pin]);
}

void ICACHE_FLASH_ATTR noPullup(uint16_t pin)
{
	if (pin >= 16) return;
//	PIN_PULLDWN_DIS(g_pin_muxes[pin]);
	PIN_PULLUP_DIS(g_pin_muxes[pin]);
}

void ICACHE_FLASH_ATTR digitalWrite(uint8_t pin, uint8_t val) {
    if(pin == 16) {
        MODIFY_PERI_REG(RTC_GPIO_OUT, 1, (val & 1));
        return;
    }
    uint32_t mask = 1 << pin;
    if(val)
        GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, mask);
    else
        GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, mask);
}

uint8_t ICACHE_FLASH_ATTR digitalRead(uint8_t pin) {
    if (pin == 16)
        return (uint8_t)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
    else
        return (uint8_t)((gpio_input_get() >> pin) & 1);
}

void ICACHE_FLASH_ATTR analogWrite(uint8_t pin, int duty)
{
	// ToDo
}

uint16_t ICACHE_FLASH_ATTR analogRead()
{
	// ToDo : ensure that no transmission happening while reading ADC
	return (uint16_t)system_adc_read();
}

bool _gpioInterruptsInitialized = false;
InterruptCallback _gpioInterruptsList[ESP_MAX_INTERRUPTS] = {0};
void* _gpioInterruptsData[ESP_MAX_INTERRUPTS] = {0};

static void ICACHE_IRAM_ATTR interruptHandler(uint32 intr_mask, void *arg)
{
	bool processed;
	uint32 gpio_status;
	do
	{
		gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
		processed = false;
		uint8_t i;
		for (i = 0; i < ESP_MAX_INTERRUPTS; i++, gpio_status<<1)
		{
			if ((gpio_status & BIT(i)) && _gpioInterruptsList[i] != NULL)
			{
				//clear interrupt status
				GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(i));
				_gpioInterruptsList[i](_gpioInterruptsData[i]);
				processed = true;
			}
		}
	} while (processed);
}

void ICACHE_FLASH_ATTR attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE mode, void *data)
{
	if (pin >= 16) return; // WTF o_O
	_gpioInterruptsList[pin] = callback;
	_gpioInterruptsData[pin] = data;

	ETS_GPIO_INTR_DISABLE();

	if (!_gpioInterruptsInitialized)
	{
		ETS_GPIO_INTR_ATTACH((void*)interruptHandler, NULL); // Register interrupt handler
		_gpioInterruptsInitialized = true;
	}

	if (mode == GPIO_PIN_INTR_HILEVEL)
		pinMode(pin, INPUT_PULLDOWN);
	else
		pinMode(pin, INPUT);

	gpio_pin_intr_state_set(GPIO_ID_PIN(pin), mode); // Enable GPIO pin interrupt

	ETS_GPIO_INTR_ENABLE();
}

void ICACHE_FLASH_ATTR detachInterrupt(uint8_t pin)
{
	attachInterrupt(pin, NULL, GPIO_PIN_INTR_DISABLE, NULL);
}

void ICACHE_IRAM_ATTR noInterrupts()
{
	ETS_INTR_LOCK();
}

void ICACHE_IRAM_ATTR interrupts()
{
	ETS_INTR_UNLOCK();
}
