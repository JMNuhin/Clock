#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"

#define KEY_PORT GPIOA
#define ROW_1 GPIO_PIN_7
#define ROW_2 GPIO_PIN_6
#define ROW_3 GPIO_PIN_5
#define ROW_4 GPIO_PIN_4
#define COL_1 GPIO_PIN_3
#define COL_2 GPIO_PIN_2
#define COL_3 GPIO_PIN_1
#define COL_4 GPIO_PIN_0


void scanKeypad(uint16_t colpin);
#endif /* __KEYPAD_H__ */
