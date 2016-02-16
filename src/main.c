/*
 * This file is part of baseflight
 * Licensed under GPL V3 or modified DCL - see https://github.com/multiwii/baseflight/blob/master/README.md
 */

#define I2C_DEVICE (I2CDEV_2)

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "stm32f10x_conf.h"

#include "board/revision.h"
#include "board/printf.h"

#include "board/drv_adc.h"
#include "board/drv_i2c.h"
#include "board/drv_serial.h"
#include "board/drv_spi.h"
#include "board/drv_gpio.h"
#include "board/drv_system.h"
#include "board/drv_pwm.h"

#include "config.h"
#include "axes.h"
#include "mw.h"

static serialPort_t * telemport;

// gcc/GNU version
static void _putc(void *p, char c)
{
    (void)p; // avoid unused-argument warning
    serialWrite(telemport, c);
    while (!isSerialTransmitBufferEmpty(telemport));
}

int main(void)
{
    // from system_stm32f10x.c
    extern void SetSysClock(bool overclock);

    extern void activateConfig(void);

    uint8_t i;

    // Configure clock, this figures out HSE for hardware autodetect
    SetSysClock(CONFIG_EMF_AVOIDANCE);

    // determine hardware revision based on clock frequency
    int hw_revision = 0;
    if (hse_value == 8000000)
        hw_revision = NAZE32;
    else if (hse_value == 12000000)
        hw_revision = NAZE32_REV5;

    systemInit(hw_revision);

    // sleep for 100ms
    delay(100);

    activateConfig();

    if (spiInit() == SPI_DEVICE_MPU && hw_revision == NAZE32_REV5)
        hw_revision = NAZE32_SP;

    if (hw_revision != NAZE32_SP)
        i2cInit(I2C_DEVICE);

    adcInit(hw_revision);

    initSensors(hw_revision);

    LED1_ON;
    LED0_OFF;
    for (i = 0; i < 10; i++) {
        LED1_TOGGLE;
        LED0_TOGGLE;
        delay(50);
    }
    LED0_OFF;
    LED1_OFF;

    imuInit(); 
    mixerInit(); 

    telemport = serialInit(CONFIG_SERIAL_BAUDRATE);

    init_printf(NULL, _putc);

    pwmInit(CONFIG_FAILSAFE_DETECT_THRESHOLD, CONFIG_PWM_FILTER, CONFIG_USE_CPPM, CONFIG_MOTOR_PWM_RATE,
            CONFIG_FAST_PWM, CONFIG_PWM_IDLE_PULSE);

    // configure PWM/CPPM read function and max number of channels
    // these, if enabled
    for (i = 0; i < RC_CHANS; i++)
        rcData[i] = 1502;

    calibratingG = CONFIG_CALIBRATING_GYRO_CYCLES;

    // set up initial conditions
    setup();
    
    // loopy
    while (1) 
        loop();
}
