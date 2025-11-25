/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "buffer.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "LCD1602.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include "keypad.h"
#include "stringer.h"
#include "clock.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
int keypressed = 0;
uint16_t colpin;
extern char buffer[100];
extern char key;
char time[100];
char date[100];

// Alarm variables
uint8_t alarm_enabled = 0;
uint8_t alarm_hour = 0;
uint8_t alarm_minute = 0;
uint8_t alarm_second = 0;
uint8_t alarm_triggered = 0;

// Timer variables
uint8_t timer_enabled = 0;
uint32_t timer_seconds = 0;
uint32_t timer_remaining = 0;
uint8_t timer_start_hour = 0;
uint8_t timer_start_minute = 0;
uint8_t timer_start_second = 0;

// Buzzer pin
#define BUZZER_PIN GPIO_PIN_5
#define BUZZER_PORT GPIOB
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

// Helper functions for keypad input
void collect_keypad_input(char* buffer, uint8_t length, const char* prompt, const char* format);
void display_formatted_input(char* buffer, uint8_t current_idx, uint8_t total_length, const char* format);
void handle_time_setting(void);
void handle_date_setting(void);
void handle_alarm_setting(void);
void handle_timer_setting(void);
void check_alarm(void);
void update_timer(void);
void display_timer(void);
void buzzer_on(void);
void buzzer_off(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * Clock Settings Interface:
  * - Press '#' to set time (format: HHMMSS)
  * - Press '*' to set date (format: DDMMYYWW where WW is day of week 1-7)
  * - Display shows real-time input with formatting
  * - Validates input ranges before setting RTC
  */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  
 
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  
  HAL_TIM_Base_Start(&htim2);
  lcd_init();
  lcd_clear();
  lcd_put_cur(0, 0);
  lcd_send_string("hello");
  HAL_Delay(3000);
  lcd_clear();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_SET);
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    get_time_date(time, date);
    
    // Display date on row 0 left (first 8 chars)
    lcd_put_cur(0, 0);
    char date_short[9];
    snprintf(date_short, 9, "%.8s", date);
    lcd_send_string(date_short);
    
    // Display time on row 1 left (first 8 chars)
    lcd_put_cur(1, 0);
    lcd_send_string(time);
    
    // Display alarm if set (row 0 right)
    if(alarm_enabled) {
      lcd_put_cur(0, 9);
      char alarm_str[9];
      snprintf(alarm_str, 9, "A:%02d:%02d", alarm_hour, alarm_minute);
      lcd_send_string(alarm_str);
    } else {
      lcd_put_cur(0, 8);
      lcd_send_string("        ");
    }
    
    // Display timer if set (row 1 right)
    if(timer_enabled && timer_remaining > 0) {
      display_timer();
    } else if(!timer_enabled) {
      lcd_put_cur(1, 8);
      lcd_send_string("        ");
    }

    if(keypressed == 1){
      scanKeypad(colpin);
      keypressed = 0;
    } 

    // Time setting mode
    if(key == '#'){
      handle_time_setting();
    }
    
    // Date setting mode
    if(key == '*'){
      handle_date_setting();
    }
    
    // Alarm setting mode
    if(key == 'A'){
      handle_alarm_setting();
    }
    
    // Timer setting mode
    if(key == 'B'){
      handle_timer_setting();
    }
    
    // Check alarm
    if(alarm_enabled){
      check_alarm();
    }
    
    // Update timer
    if(timer_enabled){
      update_timer();
      display_timer();
    }
    
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7
                           PA8 PA9 PA10 PA11
                           PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB14 PB15 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);


  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief Collect keypad input with dynamic display
  */
void collect_keypad_input(char* buffer, uint8_t length, const char* prompt, const char* format)
{
  uint8_t input_idx = 0;
  key = '\0';
  
  lcd_clear();
  lcd_put_cur(0, 0);
  lcd_send_string((char*)prompt);
  HAL_Delay(1500);
  
  lcd_clear();
  lcd_put_cur(0, 0);
  lcd_send_string((char*)format);
  lcd_put_cur(1, 0);
  
  while(input_idx < length) {
    if(keypressed == 1) {
      scanKeypad(colpin);
      keypressed = 0;
      
      if(key >= '0' && key <= '9') {
        buffer[input_idx] = key;
        display_formatted_input(buffer, input_idx, length, format);
        input_idx++;
        key = '\0';
      }
    }
    HAL_Delay(50);
  }
}

/**
  * @brief Display formatted input on LCD
  */
void display_formatted_input(char* buffer, uint8_t current_idx, uint8_t total_length, const char* format)
{
  char display[17];
  uint8_t buf_idx = 0;
  uint8_t disp_idx = 0;
  
  // Build display string with formatting characters
  for(uint8_t i = 0; format[i] != '\0' && disp_idx < 16; i++) {
    if(format[i] >= '0' && format[i] <= '9') {
      // This is a placeholder position
      if(buf_idx <= current_idx) {
        display[disp_idx++] = buffer[buf_idx];
      } else {
        display[disp_idx++] = '_';
      }
      buf_idx++;
    } else {
      // This is a formatting character
      display[disp_idx++] = format[i];
    }
  }
  display[disp_idx] = '\0';
  
  lcd_put_cur(1, 0);
  lcd_send_string(display);
}

/**
  * @brief Handle time setting via keypad
  */
void handle_time_setting(void)
{
  char time_input[7] = {0};
  uint8_t hours, minutes, seconds;
  
  collect_keypad_input(time_input, 6, "Set Time:", "00:00:00");
  
  // Parse time
  hours = (time_input[0] - '0') * 10 + (time_input[1] - '0');
  minutes = (time_input[2] - '0') * 10 + (time_input[3] - '0');
  seconds = (time_input[4] - '0') * 10 + (time_input[5] - '0');
  
  // Validate and set
  if(hours < 24 && minutes < 60 && seconds < 60) {
    set_time(hours, minutes, seconds);
    lcd_clear();
    lcd_send_string("Time Set!");
    HAL_Delay(1500);
  } else {
    lcd_clear();
    lcd_send_string("Invalid Time!");
    HAL_Delay(1500);
  }
  
  lcd_clear();
  key = '\0';
}

/**
  * @brief Handle date setting via keypad
  */
void handle_date_setting(void)
{
  char date_input[9] = {0};
  uint8_t day, month, year;
  
  collect_keypad_input(date_input, 8, "Set Date:", "00-00-00");
  
  // Parse date (DD-MM-YY format)
  day = (date_input[0] - '0') * 10 + (date_input[1] - '0');
  month = (date_input[2] - '0') * 10 + (date_input[3] - '0');
  year = (date_input[4] - '0') * 10 + (date_input[5] - '0');
  
  // Get day of week (simplified - you can enhance this)
  uint8_t weekday = (date_input[6] - '0') * 10 + (date_input[7] - '0');
  if(weekday < 1) weekday = 1;
  if(weekday > 7) weekday = 7;
  
  // Validate and set
  if(day >= 1 && day <= 31 && month >= 1 && month <= 12 && year <= 99) {
    set_date(year, month, day, weekday);
    lcd_clear();
    lcd_send_string("Date Set!");
    HAL_Delay(1500);
  } else {
    lcd_clear();
    lcd_send_string("Invalid Date!");
    HAL_Delay(1500);
  }
  
  lcd_clear();
  key = '\0';
}

/**
  * @brief Handle alarm setting via keypad
  */
void handle_alarm_setting(void)
{
  char alarm_input[7] = {0};
  uint8_t hours, minutes, seconds;
  
  collect_keypad_input(alarm_input, 6, "Set Alarm:", "00:00:00");
  
  // Parse alarm time
  hours = (alarm_input[0] - '0') * 10 + (alarm_input[1] - '0');
  minutes = (alarm_input[2] - '0') * 10 + (alarm_input[3] - '0');
  seconds = (alarm_input[4] - '0') * 10 + (alarm_input[5] - '0');
  
  // Validate and set
  if(hours < 24 && minutes < 60 && seconds < 60) {
    alarm_hour = hours;
    alarm_minute = minutes;
    alarm_second = seconds;
    alarm_enabled = 1;
    alarm_triggered = 0;
    
    lcd_clear();
    lcd_send_string("Alarm Set!");
    HAL_Delay(1500);
  } else {
    lcd_clear();
    lcd_send_string("Invalid Alarm!");
    HAL_Delay(1500);
  }
  
  lcd_clear();
  key = '\0';
}

/**
  * @brief Handle timer setting via keypad
  */
void handle_timer_setting(void)
{
  char timer_input[7] = {0};
  uint8_t hours, minutes, seconds;
  
  collect_keypad_input(timer_input, 6, "Set Timer:", "00:00:00");
  
  // Parse timer duration
  hours = (timer_input[0] - '0') * 10 + (timer_input[1] - '0');
  minutes = (timer_input[2] - '0') * 10 + (timer_input[3] - '0');
  seconds = (timer_input[4] - '0') * 10 + (timer_input[5] - '0');
  
  // Validate and set
  if(hours < 24 && minutes < 60 && seconds < 60) {
    timer_seconds = hours * 3600 + minutes * 60 + seconds;
    timer_remaining = timer_seconds;
    timer_enabled = (timer_seconds > 0) ? 1 : 0;
    
    // Save current RTC time as timer start time
    RTC_TimeTypeDef gTime;
    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
    timer_start_hour = gTime.Hours;
    timer_start_minute = gTime.Minutes;
    timer_start_second = gTime.Seconds;
    
    lcd_clear();
    lcd_send_string("Timer Set!");
    HAL_Delay(1500);
  } else {
    lcd_clear();
    lcd_send_string("Invalid Timer!");
    HAL_Delay(1500);
  }
  
  lcd_clear();
  key = '\0';
}

/**
  * @brief Check if alarm time matches current time
  */
void check_alarm(void)
{
  RTC_TimeTypeDef gTime;
  HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
  
  if(!alarm_triggered && 
     gTime.Hours == alarm_hour && 
     gTime.Minutes == alarm_minute && 
     gTime.Seconds == alarm_second) {
    alarm_triggered = 1;
    buzzer_on();
    
    // Keep buzzer on for 5 seconds
    HAL_Delay(5000);
    buzzer_off();
    
    // Disable alarm after triggering
    alarm_enabled = 0;
  }
}

/**
  * @brief Update timer countdown
  */
void update_timer(void)
{
  // Get current RTC time
  RTC_TimeTypeDef gTime;
  HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
  
  // Calculate elapsed seconds since timer started
  uint32_t current_total_seconds = gTime.Hours * 3600 + gTime.Minutes * 60 + gTime.Seconds;
  uint32_t start_total_seconds = timer_start_hour * 3600 + timer_start_minute * 60 + timer_start_second;
  uint32_t elapsed_seconds;
  
  // Handle day rollover
  if(current_total_seconds >= start_total_seconds) {
    elapsed_seconds = current_total_seconds - start_total_seconds;
  } else {
    // Day rolled over (past midnight)
    elapsed_seconds = (86400 - start_total_seconds) + current_total_seconds;
  }
  
  // Calculate remaining time
  if(elapsed_seconds >= timer_seconds) {
    // Timer finished
    timer_enabled = 0;
    timer_remaining = 0;
    buzzer_on();
    HAL_Delay(5000);
    buzzer_off();
  } else {
    timer_remaining = timer_seconds - elapsed_seconds;
  }
}

/**
  * @brief Display timer at row 1 right corner
  */
void display_timer(void)
{
  if(timer_enabled && timer_remaining > 0) {
    uint8_t hrs = timer_remaining / 3600;
    uint8_t mins = (timer_remaining % 3600) / 60;
    uint8_t secs = timer_remaining % 60;
    
    char timer_str[9];
    if(hrs > 0) {
      snprintf(timer_str, 9, "%02d:%02d:%02d", hrs, mins, secs);
    } else {
      snprintf(timer_str, 9, "  %02d:%02d", mins, secs);
    }
    
    lcd_put_cur(1, 8);
    lcd_send_string(timer_str);
  }
}

/**
  * @brief Turn buzzer on
  */
void buzzer_on(void)
{
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

/**
  * @brief Turn buzzer off
  */
void buzzer_off(void)
{
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected to EXTI line
  * Empty handlers for PA0..PA3. Add user code if needed.
  */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  keypressed = 1;
  colpin = GPIO_Pin;

}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
