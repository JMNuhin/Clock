#include "main.h"
#include <stdio.h>
extern RTC_HandleTypeDef hrtc;
void set_time (uint8_t hr, uint8_t min, uint8_t sec)
{
	RTC_TimeTypeDef sTime = {0};
	sTime.Hours = hr;
	sTime.Minutes = min;
	sTime.Seconds = sec;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}
void set_date (uint8_t year, uint8_t month, uint8_t date, uint8_t day)  // monday = 1
{
	RTC_DateTypeDef sDate = {0};
	sDate.WeekDay = day;
	sDate.Month = month;
	sDate.Date = date;
	sDate.Year = year;
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x2345);  // backup register
}
void get_time_date(char *time, char *date)
{
  RTC_DateTypeDef gDate;
  RTC_TimeTypeDef gTime;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

  /* Display time Format: hh:mm:ss */
  sprintf((char*)time,"%02d:%02d:%02d",gTime.Hours, gTime.Minutes, gTime.Seconds);

  /* Display date Format: dd-mm-yyyy */
  sprintf((char*)date,"%02d-%02d-%2d",gDate.Date, gDate.Month, 2000 + gDate.Year);
}
void set_alarm (uint8_t hr, uint8_t min, uint8_t sec, uint8_t date)
{
	RTC_AlarmTypeDef sAlarm = {0};
	sAlarm.AlarmTime.Hours = hr;
	sAlarm.AlarmTime.Minutes = min;
	sAlarm.AlarmTime.Seconds = sec;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}