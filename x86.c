/*
 * x86.h
 *
 * Created: 28-10-2016 15:27:07
 *  Author: nxa03718
 */ 
#include <sys/time.h>
#include "x86.h"

static struct timeval timeStart;

void millis_init(void)
{
     gettimeofday(&timeStart, 0);
}

unsigned long millis(void)
{
    struct timeval timeNow;
    gettimeofday(&timeNow, 0);

    timeNow.tv_sec  -= timeStart.tv_sec;
    timeNow.tv_usec -= timeStart.tv_usec;
    
    return (unsigned long)((timeNow.tv_sec * 1000) + (timeNow.tv_usec / 1000));
}

void sleep(unsigned int seconds)
{
    struct timeval starttime, now;
    gettimeofday(&starttime, 0);
    do
    {
        gettimeofday(&now,0);
    } while((now.tv_sec - starttime.tv_sec) < seconds);
}