
#include "keypad.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
int row=0;
int col=0;
char key;
uint32_t previousMillis = 0;
uint32_t currentMillis = 0;
GPIO_InitTypeDef GPIO_InitStructKey = {0};

void scanKeypad(uint16_t colpin)
{
  currentMillis = HAL_GetTick();
  if (currentMillis - previousMillis > 30) {
    GPIO_InitStructKey.Pin = COL_1|COL_2|COL_3|COL_4;
    GPIO_InitStructKey.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructKey.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(KEY_PORT, &GPIO_InitStructKey);
   
    switch(colpin)
  {
    case GPIO_PIN_0:
      col = 3;
      break;
    case GPIO_PIN_1:
      col = 2;
      break;
    case GPIO_PIN_2:
      col = 1;
      break;
    case GPIO_PIN_3:
      col = 0;
      break;
  }
    
      HAL_GPIO_WritePin(KEY_PORT, ROW_1|ROW_2|ROW_3|ROW_4, GPIO_PIN_RESET);
      for(row=0; row<4; row++)
      {
        HAL_GPIO_WritePin(KEY_PORT, ROW_4, (row==3)?GPIO_PIN_SET:GPIO_PIN_RESET);
        HAL_GPIO_WritePin(KEY_PORT, ROW_3, (row==2)?GPIO_PIN_SET:GPIO_PIN_RESET);
        HAL_GPIO_WritePin(KEY_PORT, ROW_2, (row==1)?GPIO_PIN_SET:GPIO_PIN_RESET);
        HAL_GPIO_WritePin(KEY_PORT, ROW_1, (row==0)?GPIO_PIN_SET:GPIO_PIN_RESET);
        HAL_Delay(10);

        if(HAL_GPIO_ReadPin(KEY_PORT, colpin) == GPIO_PIN_SET)
        {
          
          char keys[4][4] = {
              {'1', '2', '3', 'A'},
              {'4', '5', '6', 'B'},
              {'7', '8', '9', 'C'},
              {'*', '0', '#', 'D'}
          };
          key = keys[row][col];
          break;
        }
        
      
      }
      HAL_Delay(60);
      GPIO_InitStructKey.Pin = COL_1|COL_2|COL_3|COL_4;
      GPIO_InitStructKey.Mode = GPIO_MODE_IT_RISING;
      GPIO_InitStructKey.Pull = GPIO_PULLDOWN;
      HAL_GPIO_Init(KEY_PORT, &GPIO_InitStructKey);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_SET);
    
    previousMillis = currentMillis;

  }
    


    
}