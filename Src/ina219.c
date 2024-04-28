#include "ina219.h"

static SWIIC_Config* ina219_swiic;

void INA219_Init(SWIIC_Config *swiic) {
  ina219_swiic = swiic;
  uint8_t config[] = {0x36, 0xEF};
  SWIIC_WriteBytes8(ina219_swiic, INA219_ADDR, INA219_REG_CONF, config, 2);
}

int16_t INA219_ReadShuntVoltage(void) {
  uint8_t data[2];
  SWIIC_ReadBytes8(ina219_swiic, INA219_ADDR, INA219_REG_SHUNT_VOLTAGE, data, 2);
  // printf("%.2x %.2x\n", data[0], data[1]);
  return (int16_t)((data[0] << 8) | data[1]);
}

int16_t INA219_ReadBusVoltage(void) {
  uint8_t data[2];
  SWIIC_ReadBytes8(ina219_swiic, INA219_ADDR, INA219_REG_BUS_VOLTAGE, data, 2);
  // printf("%.2x %.2x\n", data[0], data[1]);
  return (int16_t)((data[0] << 5) | (data[1] >> 3));
}