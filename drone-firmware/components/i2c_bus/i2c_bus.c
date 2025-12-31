#include "i2c_bus.h"
#include "board.h"
#include <string.h>

static I2cDrv s_i2c_bus;
static const I2cDef I2cConfig= {
    .i2cPort = I2C_MASTER_NUM,
    .i2cClockSpeed = I2C_MASTER_FREQ_HZ,
    .gpioSCLPin = I2C_MASTER_SCL_IO,
    .gpioSDAPin = I2C_MASTER_SDA_IO,
    .gpioPullup = GPIO_PULLUP_ENABLE
};
static bool s_initialized = false;

void i2c_bus_init(void)
{
    if (s_initialized) {
        return;
    }

    memset(&s_i2c_bus, 0, sizeof(s_i2c_bus));
    s_i2c_bus.def = &I2cConfig;

    i2cdrvInit(&s_i2c_bus);
    s_initialized = true;
}

I2cDrv *i2c_bus_get(void)
{
    return s_initialized ? &s_i2c_bus : NULL;
}
