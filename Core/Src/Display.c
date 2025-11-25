#include "Display.h"


void Display_Alarm()
{

}
void Display_SetTime()
{

}
void Display_Date()
{

}
void Displayer(char key)
{
    switch (key)
    {
        case 'A':
        Display_Alarm();
        break;
        case '*':
        Display_SetTime();
        break;
        case '#':
        Display_Date();
        break;
        default:
            get_time_date(time, date);
    lcd_put_cur(0, 0);
    lcd_send_string(date);
    lcd_put_cur(1, 0);
    lcd_send_string(time);
    
        break;
    }

}