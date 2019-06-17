#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "src/utils.h"

unsigned int flag;

void fct (int x) {
    if (x == SIGALRM) {
        flag = 0;
    }
}

static struct option const long_opt[] =
{
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"utilization", required_argument, NULL, 'u'},
    {"duration", required_argument, NULL, 'd'},
    {"start", required_argument, NULL, 't'},
    {NULL, 0, NULL, 0}
};

static const char *cpuoccupy_usage = "CPU intensive orphan process.\n\n"
    "-u, --utilization (=100%)  The utilization (%) of one core.\n"
    "-d, --duration (=-1.0)     The total duration (in seconds), -1 for infinite.\n"
    "-t, --start (=0.0)         The time to wait (in seconds) before starting the anomaly.\n"
    "-v, --verbose              Prints execution information.\n"
    "-h, --help                 Prints this message.\n";
static const char short_opt[] = "hvd:u:t:";

int cpuoccupy(int argc, char* argv[]){
    time_t rawtime;
    struct tm *timeinfo;
    bool verbose = false;

    unsigned int codebegin, codeend;
    long utilsec, utilusec, intervalsec, intervalusec;
    double dursec = -1;
    int counter = 0;

    struct itimerval itv;
    int percutil = 100;
    double intervalsize, utiltime, sleeptime;
    double res = 1.0;
    int c;
    double start_time = 0;


    while ((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
        switch(c) {
            case -1:
            case 0:
                break;

            case 'u':
                percutil = (int) strtol(optarg, NULL, 10);
                if (percutil < 0 || percutil > 100){
                    printf("bad percent util\n");
                    exit(-1);
                }
                break;

            case 't':
                start_time = strtod(optarg, NULL);
                if (start_time < 0) {
                    printf("Start time cannot be negative.\n");
                    _exit(0);
                }
                break;

            case 'd':
                dursec = strtod(optarg, NULL);
                break;

            case 'v':
                verbose = true;
                break;

            case 'h':
            default:
                printf("Usage: %s [OPTIONS]\n%s", argv[0], cpuoccupy_usage);
                return 0;
        }
    }

    hpas_sleep(start_time);

    //check args
    //get times
    intervalsize = 0.25;
    intervalsec = (long) intervalsize;
    intervalusec = (long) ((intervalsize - (double) intervalsec) * 1000000);
    utiltime = ((double)percutil/100.0) *(intervalsize);
    utilsec = (long) utiltime;
    utilusec = (long) ((utiltime - (double) utilsec) * 1000000);

    sleeptime = intervalsize - utiltime;
    if (sleeptime < 0){ //this shouldnt happen
        printf("bad sleeptime\n");
        exit(-1);
    }

    //start the timing of the code
    codebegin = time(NULL);

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("%sStarting cpuoccupy with utilization:%d%%\n\n",
            asctime(timeinfo), percutil);
    fflush(stdout);

    //set times for ending calc
    itv.it_interval.tv_sec = intervalsec;
    itv.it_interval.tv_usec = intervalusec;
    itv.it_value.tv_sec = utilsec;
    itv.it_value.tv_usec = utilusec;

    //set up the signal for the calc
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &fct;
    sigaction(SIGALRM, &sa, NULL);
    setitimer(ITIMER_REAL, &itv, NULL);

    while(1){
        // do the sleep
        hpas_sleep(sleeptime);

        flag = 1;
        while(flag){     //do the calc
            double vala, valb;

            //calculation
            srand(time(NULL));
            vala=1+(int) (0.9*rand()/(RAND_MAX+2.0));
            valb=1+(int) (0.9*rand()/(RAND_MAX+1.0));
            res += vala/valb;
        }

        //test if we want to exit the code entirely
        codeend = time(NULL);
        if (dursec > 0 && codeend - codebegin > dursec){
            break;
        }
        if (verbose && ++counter % 100 == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf("Exiting cpuoccupy, duration=%us\n", codeend - codebegin);
    return 0;
}
