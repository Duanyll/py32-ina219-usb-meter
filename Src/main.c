/**
 * Demo: Write Option Bytes
 *
 * Board: PY32F003W1XS (SOP16)
 *
 * This demo shows how to config reset pin as gpio output
 */

#include "main.h"
#include "py32f0xx_bsp_clock.h"
#include "py32f0xx_bsp_printf.h"

#include "swiic.h"
#include "ssd1306.h"
#include "ina219.h"

static void APP_PrintInt(int num);
static void APP_PrintString(char *str);
static void APP_SPrintInt(char *str, int num);
static void APP_EnsureOptionBytes(void);
static void APP_GPIOConfig(void);
static void APP_FlashSetOptionBytes(void);
static void APP_SSD1306Demo(void);

SWIIC_Config swiic_config;

// >>> CHANGE THIS VALUE TO MATCH YOUR HARDWARE
// 2mR shunt resistor -> 5000 / 10000
// 10mR shunt resistor -> 1000 / 10000
// PCB layout may affect the calibration value,
// it's better to measure the current and adjust the value.
#define CURRENT_CALIBRATION (5000 / 10000)

int main(void) {
  BSP_RCC_HSI_24MConfig();
  LL_mDelay(1000);
  APP_EnsureOptionBytes();
  /* Don't config GPIO before changing the option bytes */
  APP_GPIOConfig();
  BSP_USART_Config(115200);

  swiic_config.SDA_Port = GPIOA;
  swiic_config.SDA_Pin = LL_GPIO_PIN_4;
  swiic_config.SCL_Port = GPIOA;
  swiic_config.SCL_Pin = LL_GPIO_PIN_1;
  swiic_config.delay = 10;
  SWIIC_Init(&swiic_config);

  SSD1306_Init();
  INA219_Init(&swiic_config);  

  while (1) {
    int shuntVoltage = INA219_ReadShuntVoltage() * 10; // uV
    int busVoltage = INA219_ReadBusVoltage() * 4; // mV
    int current = shuntVoltage * CURRENT_CALIBRATION; // mA
    int power = current * busVoltage / 1000; // mW
    if (power < 0) {
      power = -power;
    }
    APP_PrintString("Shunt Voltage: ");
    APP_PrintInt(shuntVoltage);
    APP_PrintString(" uV\n");
    APP_PrintString("Bus Voltage: ");
    APP_PrintInt(busVoltage);
    APP_PrintString(" mV\n");
    APP_PrintString("Current: ");
    APP_PrintInt(current);
    APP_PrintString(" mA\n");
    APP_PrintString("Power: ");
    APP_PrintInt(power);
    APP_PrintString(" mW\n\n");

    SSD1306_Fill(0);

    char buf[32] = {0};
    if (current < 0) {
      sprintf(buf, "-%d.%dA", -current / 1000, -current % 1000);
    } else {
      sprintf(buf, "%d.%dA", current / 1000, current % 1000);
    }
    SSD1306_GotoXY(0, 5);
    SSD1306_Puts(buf, &Font_6x10, 1);
    
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d.%dV", busVoltage / 1000, busVoltage % 1000);
    SSD1306_GotoXY(0, 20);
    SSD1306_Puts(buf, &Font_6x10, 1);

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d.%dW", power / 1000, power % 1000);
    SSD1306_GotoXY(50, 8);
    SSD1306_Puts(buf, &Font_11x18, 1);

    SSD1306_UpdateScreen();
    LL_mDelay(100);
  }
}

static void APP_PrintInt(int num) {
  // Print the number to str
  if (num < 0) {
    num = -num;
    putchar('-');
  }
  if (num / 10 != 0) {
    APP_PrintInt(num / 10);
  }
  putchar(num % 10 + '0');
}

static void APP_PrintString(char *str) {
  while (*str) {
    putchar(*str++);
  }
}

static void APP_SPrintInt(char *str, int num) {
  // Print the number to str
  if (num < 0) {
    num = -num;
    *str++ = '-';
  }
  if (num / 10 != 0) {
    APP_SPrintInt(str, num / 10);
  }
  *str++ = num % 10 + '0';
}

static void APP_EnsureOptionBytes(void) {
  if (READ_BIT(FLASH->OPTR, FLASH_OPTR_NRST_MODE) == OB_RESET_MODE_RESET) {
    /* This will reset the MCU */
    APP_FlashSetOptionBytes();
  }
}

static void APP_GPIOConfig(void) {
  // PA0
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  // PF0 -> GPIO output, PF2 -> analog
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOF);
  LL_GPIO_SetPinMode(GPIOF, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);
  // PY32F003W1XS: PF0 and PF2 share the same pin, it's recommended to set PF2
  // as analog
  LL_GPIO_SetPinMode(GPIOF, LL_GPIO_PIN_2, LL_GPIO_MODE_ANALOG);
}

static void APP_FlashSetOptionBytes(void) {
  FLASH_OBProgramInitTypeDef OBInitCfg;

  LL_FLASH_Unlock();
  LL_FLASH_OB_Unlock();

  OBInitCfg.OptionType = OPTIONBYTE_USER;
  OBInitCfg.USERType = OB_USER_BOR_EN | OB_USER_BOR_LEV | OB_USER_IWDG_SW |
                       OB_USER_NRST_MODE | OB_USER_nBOOT1;
  /*
   * The default value: OB_BOR_DISABLE | OB_BOR_LEVEL_3p1_3p2 | OB_IWDG_SW |
   * OB_WWDG_SW | OB_RESET_MODE_RESET | OB_BOOT1_SYSTEM;
   */
  OBInitCfg.USERConfig = OB_BOR_DISABLE | OB_BOR_LEVEL_3p1_3p2 | OB_IWDG_SW |
                         OB_RESET_MODE_GPIO | OB_BOOT1_SYSTEM;
  LL_FLASH_OBProgram(&OBInitCfg);

  LL_FLASH_Lock();
  LL_FLASH_OB_Lock();
  /* Reload option bytes */
  LL_FLASH_OB_Launch();
}

void APP_SSD1306Demo(void) {
  SSD1306_Init();

  SSD1306_DrawLine(0, 0, 127, 0, 1);
  SSD1306_DrawLine(0, 0, 0, 31, 1);
  SSD1306_DrawLine(127, 0, 127, 31, 1);
  SSD1306_DrawLine(0, 31, 127, 31, 1);
  SSD1306_GotoXY(5, 5);
  SSD1306_Puts("OLED:128x64", &Font_6x10, 1);
  SSD1306_GotoXY(5, 20);
  SSD1306_Puts("Font size: 6x10", &Font_6x10, 1);
  SSD1306_UpdateScreen(); // display

  SSD1306_Fill(0);
  SSD1306_GotoXY(5, 5);
  SSD1306_Puts("OLED:128x64", &Font_6x10, 1);
  SSD1306_GotoXY(5, 20);
  SSD1306_Puts("SSD1306 Demo", &Font_6x10, 1);
  SSD1306_UpdateScreen();
  LL_mDelay(1000);
}

void APP_I2C_Transmit(uint8_t devAddress, uint8_t memAddress, uint8_t *pData,
                      uint16_t len) {
  SWIIC_WriteBytes8(&swiic_config, devAddress, memAddress, pData, len);
}

void APP_ErrorHandler(void) {
  while (1)
    ;
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
  while (1)
    ;
}
#endif /* USE_FULL_ASSERT */
