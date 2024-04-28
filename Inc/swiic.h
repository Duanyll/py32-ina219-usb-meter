#pragma once

#include "main.h"

typedef struct SWIIC_Config {
  GPIO_TypeDef *SDA_Port;
  uint16_t SDA_Pin;
  GPIO_TypeDef *SCL_Port;
  uint16_t SCL_Pin;

  uint32_t delay;
} SWIIC_Config;

typedef uint8_t SWIIC_State;
#define SWIIC_OK 0
#define SWIIC_ERROR 1

// Initializes the IIC bus
void SWIIC_Init(SWIIC_Config *config);

// Read bytes from the IIC bus. Register address is 8 bits.
SWIIC_State SWIIC_ReadBytes8(SWIIC_Config *config, uint8_t addr, uint8_t reg,
                             uint8_t *data, uint16_t count);
// Read bytes from the IIC bus. Register address is 16 bits.
SWIIC_State SWIIC_ReadBytes16(SWIIC_Config *config, uint8_t addr, uint16_t reg,
                              uint8_t *data, uint16_t count);
// Write bytes to the IIC bus. Register address is 8 bits.
SWIIC_State SWIIC_WriteBytes8(SWIIC_Config *config, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t count);
// Write bytes to the IIC bus. Register address is 16 bits.
SWIIC_State SWIIC_WriteBytes16(SWIIC_Config *config, uint8_t addr, uint16_t reg,
                               uint8_t *data, uint16_t count);
// Check if a device is present on the IIC bus.
SWIIC_State SWIIC_CheckDevice(SWIIC_Config *config, uint8_t addr);