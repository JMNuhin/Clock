#include "stringer.h"
#include "string.h"
extern char key;
extern char buffer[100];



void strup()
{
    int i = 0;
    char c = buffer[0];
    while(c != '\0')
    {
        i++;
        c = buffer[i];
    }
    buffer[i] = key;
    buffer[i+1] = '\0';

}