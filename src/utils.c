#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "src/utils.h"

unsigned int timer_flag = 1;

void timer_callback(int x) {
    if (x == SIGALRM) {
        timer_flag = 0;
    }
}

void set_duration(double duration) {
    timer_flag = 1;
    if (duration < 0) {
        return;
    }
    struct itimerval itv;
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_callback;
    long utilsec, utilusec;
    utilsec = (long) duration;
    utilusec = (long) ((duration - (double) utilsec) * 1000000);
    //set times for ending calc
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = utilsec;
    itv.it_value.tv_usec = utilusec;

    //set up the signal for the calc
    sigaction(SIGALRM, &sa, NULL);
    setitimer(ITIMER_REAL, &itv, NULL);
}

void hpas_sleep(double sleeptime) {
    long sleepsec, sleepusec;
    struct timeval sleeptv;
    sleepsec = (long) sleeptime;
    sleepusec = (long) ((sleeptime - (double) sleepsec) * 1000000);
    sleeptv.tv_sec = sleepsec;
    sleeptv.tv_usec = sleepusec;

    select(0,NULL,NULL,NULL,&sleeptv);   //do the sleep
}

ssize_t parse_size(char *input)
{
    char *c;
    unsigned multiple;
    long result = strtol(input, &c, 10);
    switch(*c) {
        case '\0':
            multiple = 0;
            break;
        case 'K':
        case 'k':
            multiple = 1;
            break;
        case 'M':
        case 'm':
            multiple = 2;
            break;
        case 'G':
        case 'g':
            multiple = 3;
            break;
        case 'T':
        case 't':
            multiple = 4;
            break;
        case 'P':
        case 'p':
            multiple = 5;
            break;
        case 'E':
        case 'e':
            multiple = 6;
            break;
        default:
            return -1;
    }
    result *= ((unsigned long) 1) << (multiple * 10);
    return result;
}
