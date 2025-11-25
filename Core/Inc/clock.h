#ifndef __CLOCK_H__
#define __CLOCK_H__

void set_time (uint8_t hr, uint8_t min, uint8_t sec);
void set_date (uint8_t year, uint8_t month, uint8_t date, uint8_t day);
void get_time_date(char *time, char *date);
void set_alarm (uint8_t hr, uint8_t min, uint8_t sec, uint8_t date);

#endif