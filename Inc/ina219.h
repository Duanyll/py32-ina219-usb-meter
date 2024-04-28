#pragma once

#include "main.h"
#include "swiic.h"

#define INA219_ADDR 0x40
#define INA219_REG_CONF 0x00
#define INA219_REG_SHUNT_VOLTAGE 0x01
#define INA219_REG_BUS_VOLTAGE 0x02
#define INA219_REG_POWER 0x03
#define INA219_REG_CURRENT 0x04
#define INA219_REG_CALIBRATION 0x05

void INA219_Init(SWIIC_Config *swiic);
int16_t INA219_ReadShuntVoltage(void); // 16-bit signed integer in 10uV
int16_t INA219_ReadBusVoltage(void); // 16-bit unsigned integer in 4mV