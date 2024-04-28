#include "swiic.h"

#define SWIIC_USE_OPEN_DRAIN

// Initializes the IIC bus
void SWIIC_Init(SWIIC_Config *config) {
#ifdef SWIIC_USE_OPEN_DRAIN
  LL_GPIO_InitTypeDef GPIO_InitStruct = {
      .Mode = LL_GPIO_MODE_OUTPUT,
      .OutputType = LL_GPIO_OUTPUT_OPENDRAIN,
      .Pull = LL_GPIO_PULL_UP,
      .Speed = LL_GPIO_SPEED_FREQ_HIGH,
  };
  GPIO_InitStruct.Pin = config->SDA_Pin;
  LL_GPIO_Init(config->SDA_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = config->SCL_Pin;
  LL_GPIO_Init(config->SCL_Port, &GPIO_InitStruct);
#else
  LL_GPIO_InitTypeDef GPIO_InitStruct = {
      .Mode = LL_GPIO_MODE_OUTPUT,
      .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
      .Pull = LL_GPIO_NOPULL,
      .Speed = LL_GPIO_SPEED_FREQ_HIGH,
  };
  GPIO_InitStruct.Pin = config->SDA_Pin;
  LL_GPIO_Init(config->SDA_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = config->SCL_Pin;
  LL_GPIO_Init(config->SCL_Port, &GPIO_InitStruct);
#endif
}

#ifdef SWIIC_USE_OPEN_DRAIN
#define SDA_INPUT()
#define SDA_OUTPUT()
#else
#define SDA_INPUT()                                                            \
  do {                                                                         \
    LL_GPIO_InitTypeDef GPIO_InitStruct = {                                    \
        .Mode = LL_GPIO_MODE_INPUT,                                            \
        .Pull = LL_GPIO_PULL_UP,                                               \
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,                                      \
    };                                                                         \
    GPIO_InitStruct.Pin = config->SDA_Pin;                                     \
    LL_GPIO_Init(config->SDA_Port, &GPIO_InitStruct);                          \
  } while (0)
#define SDA_OUTPUT()                                                           \
  do {                                                                         \
    LL_GPIO_InitTypeDef GPIO_InitStruct = {                                    \
        .Mode = LL_GPIO_MODE_OUTPUT,                                           \
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,                                 \
        .Pull = LL_GPIO_NOPULL,                                                \
        .Speed = LL_GPIO_SPEED_FREQ_HIGH,                                      \
    };                                                                         \
    GPIO_InitStruct.Pin = config->SDA_Pin;                                     \
    LL_GPIO_Init(config->SDA_Port, &GPIO_InitStruct);                          \
  } while (0)
#endif

#define DELAY()                                                                \
  do {                                                                         \
    for (int i = 0; i < config->delay; i++) {                                  \
      __NOP();                                                                 \
    }                                                                          \
  } while (0)
#define HIGH 1
#define LOW 0
#define WRITE_SDA(x)                                                           \
  config->SDA_Port->BSRR = x ? config->SDA_Pin : (config->SDA_Pin << 16)
#define WRITE_SCL(x)                                                           \
  config->SCL_Port->BSRR = x ? config->SCL_Pin : (config->SCL_Pin << 16)
#define READ_SDA() ((config->SDA_Port->IDR & config->SDA_Pin) ? 1 : 0)

void SWIIC_Start(SWIIC_Config *config) {
  WRITE_SDA(HIGH);
  WRITE_SCL(HIGH);
  DELAY();
  WRITE_SDA(LOW);
  DELAY();
  WRITE_SCL(LOW);
  DELAY();
}

void SWIIC_Stop(SWIIC_Config *config) {
  WRITE_SDA(LOW);
  WRITE_SCL(HIGH);
  DELAY();
  WRITE_SDA(HIGH);
  DELAY();
}

uint8_t SWIIC_WaitAck(SWIIC_Config *config) {
  WRITE_SDA(HIGH);
  DELAY();
  SDA_INPUT();
  WRITE_SCL(HIGH);
  DELAY();
  uint8_t ack = READ_SDA();
  int tries = 10;
  while (ack && tries--) {
    DELAY();
    ack = READ_SDA();
  }
  WRITE_SCL(LOW);
  SDA_OUTPUT();
  DELAY();
  return !ack;
}

void SWIIC_WriteAck(SWIIC_Config *config) {
  WRITE_SDA(LOW);
  DELAY();
  WRITE_SCL(HIGH);
  DELAY();
  WRITE_SCL(LOW);
  DELAY();
  WRITE_SDA(HIGH);
  DELAY();
}

void SWIIC_WriteNotAck(SWIIC_Config *config) {
  WRITE_SDA(HIGH);
  DELAY();
  WRITE_SCL(HIGH);
  DELAY();
  WRITE_SCL(LOW);
  DELAY();
}

void SWIIC_WriteByte(SWIIC_Config *config, uint8_t data) {
  WRITE_SCL(LOW);
  for (int i = 7; i >= 0; i--) {
    WRITE_SDA((data >> i) & 1);
    DELAY();
    WRITE_SCL(HIGH);
    DELAY();
    WRITE_SCL(LOW);
  }
  DELAY();
}

uint8_t SWIIC_ReadByte(SWIIC_Config *config) {
  uint8_t data = 0;
  WRITE_SDA(HIGH);
  SDA_INPUT();
  for (int i = 7; i >= 0; i--) {
    WRITE_SCL(HIGH);
    DELAY();
    data |= READ_SDA() << i;
    WRITE_SCL(LOW);
    DELAY();
  }
  SDA_OUTPUT();
  return data;
}

#define CHECK_ACK()                                                            \
  if (!SWIIC_WaitAck(config))                                                  \
    return SWIIC_ERROR;                                                        \
  DELAY();

// Read bytes from the IIC bus. Register address is 8 bits.
SWIIC_State SWIIC_ReadBytes8(SWIIC_Config *config, uint8_t addr, uint8_t reg,
                             uint8_t *data, uint16_t count) {
  SWIIC_Start(config);
  SWIIC_WriteByte(config, addr << 1);
  CHECK_ACK();
  SWIIC_WriteByte(config, reg);
  CHECK_ACK();
  SWIIC_Start(config);
  SWIIC_WriteByte(config, (addr << 1) | 1);
  CHECK_ACK();
  for (int i = 0; i < count; i++) {
    data[i] = SWIIC_ReadByte(config);
    if (i < count - 1) {
      SWIIC_WriteAck(config);
    } else {
      SWIIC_WriteNotAck(config);
    }
  }
  SWIIC_Stop(config);
  return SWIIC_OK;
}
// Read bytes from the IIC bus. Register address is 16 bits.
SWIIC_State SWIIC_ReadBytes16(SWIIC_Config *config, uint8_t addr, uint16_t reg,
                              uint8_t *data, uint16_t count) {
  SWIIC_Start(config);
  SWIIC_WriteByte(config, addr << 1);
  CHECK_ACK();
  SWIIC_WriteByte(config, reg >> 8);
  CHECK_ACK();
  SWIIC_WriteByte(config, reg);
  CHECK_ACK();
  SWIIC_Start(config);
  SWIIC_WriteByte(config, (addr << 1) | 1);
  CHECK_ACK();
  for (int i = 0; i < count; i++) {
    data[i] = SWIIC_ReadByte(config);
    if (i < count - 1) {
      SWIIC_WriteAck(config);
    } else {
      SWIIC_WriteNotAck(config);
    }
  }
  SWIIC_Stop(config);
  return SWIIC_OK;
}
// Write bytes to the IIC bus. Register address is 8 bits.
SWIIC_State SWIIC_WriteBytes8(SWIIC_Config *config, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t count) {
  SWIIC_Start(config);
  SWIIC_WriteByte(config, addr << 1);
  CHECK_ACK();
  SWIIC_WriteByte(config, reg);
  CHECK_ACK();
  for (int i = 0; i < count; i++) {
    SWIIC_WriteByte(config, data[i]);
    CHECK_ACK();
  }
  SWIIC_Stop(config);
  return SWIIC_OK;
}
// Write bytes to the IIC bus. Register address is 16 bits.
SWIIC_State SWIIC_WriteBytes16(SWIIC_Config *config, uint8_t addr, uint16_t reg,
                               uint8_t *data, uint16_t count) {
  SWIIC_Start(config);
  SWIIC_WriteByte(config, addr << 1);
  CHECK_ACK();
  SWIIC_WriteByte(config, reg >> 8);
  CHECK_ACK();
  SWIIC_WriteByte(config, reg);
  CHECK_ACK();
  for (int i = 0; i < count; i++) {
    SWIIC_WriteByte(config, data[i]);
    CHECK_ACK();
  }
  SWIIC_Stop(config);
  return SWIIC_OK;
}
// Check if a device is present on the IIC bus.
SWIIC_State SWIIC_CheckDevice(SWIIC_Config *config, uint8_t addr) {
  SWIIC_Start(config);
  SWIIC_WriteByte(config, addr << 1);
  if (!SWIIC_WaitAck(config)) {
    SWIIC_Stop(config);
    return SWIIC_ERROR;
  }
  SWIIC_Stop(config);
  return SWIIC_OK;
}