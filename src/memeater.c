#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <stdbool.h>
#include "src/utils.h"

static struct option const long_opt[] =
{
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"size", required_argument, NULL, 's'},
    {"start", required_argument, NULL, 't'},
    {"period", required_argument, NULL, 'p'},
    {"duration", required_argument, NULL, 'd'},
    {NULL, 0, NULL, 0}
};

const char *memeater_usage = "Memory intensive orphan process.\n\n"
    "-s, --size (=35M)       The size (in bytes) of the array to be allocated.\n"
    "-p, --period (=0.2)     The time to wait (in seconds) between array allocations.\n"
    "-d, --duration (=-1.0)  The total duration (in seconds), -1 for infinite.\n"
    "-t, --start (=0.0)      The time to wait (in seconds) before starting the anomaly.\n"
    "-v, --verbose           Prints execution information.\n"
    "-h, --help              Prints this message.\n";
static const char *short_opt = "hvd:s:p:t:";

int memeater(int argc, char *argv[])
{
    int c;

    bool verbose = false;
    long int size = 35 * 1024 * 1024;
    long int intcount;
    double period = 0.2;
    double duration = -1;
    double start_time = 0;
    while ((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {
        switch(c)
        {
            case -1:
            case 0:
                break;

            case 't':
                start_time = strtod(optarg, NULL);
                if (start_time < 0) {
                    printf("Start time cannot be negative.\n");
                    _exit(0);
                }
                break;

            case 's':
                size = parse_size(optarg);
                if (size == -1) {
                    printf("Wrong size input\n");
                    return -1;
                }
                break;

            case 'p':
                period = strtod(optarg, NULL);
                if (period < 0) {
                    printf("Period cannot be smaller than zero.\n");
                    _exit(0);
                }
                break;

            case 'd':
                duration = strtod(optarg, NULL);
                break;

            case 'v':
                verbose = true;
                break;

            case 'h':
            default:
                printf("Usage: %s [OPTIONS]\n%s", argv[0], memeater_usage);
                return 0;
        }
    }
    hpas_sleep(start_time);
    time_t rawtime;
    struct tm * timeinfo;
    FILE *fpipe;
    char *command="cat /proc/meminfo | grep -i active";
    char line[256];
    int *keep = NULL;
    int *temp;
    int i,j,r,count = 0;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    intcount = size / sizeof(int);
    printf("%sStarting memeater with intcount:%ld period:%f duration:%f\n\n",
            asctime(timeinfo), intcount, period, duration);
    fflush(stdout);

    srand(time(NULL));
    set_duration(duration);
    while (1) {
        count = 0;
        for (i =0; i < 10; i++){
            if (!timer_flag) break;
            hpas_sleep(period);

            temp = (int*) realloc (keep, (intcount + count) * sizeof(int));
            if (!temp){
              break;
              /* malloc will return NULL sooner or later, due to lack of memory */
            }

            keep = temp;
            for (j = 0; j < intcount; j++){
              r = rand();
              keep[count+j] = r;
            }
            count += intcount;

            if (verbose) {
                if (!(fpipe = (FILE*) popen(command, "r"))){
                  perror("Problems with pipe");
                  exit(1);
                }
                while (fgets( line, sizeof line, fpipe)){
                  printf("%s", line);
                }
                printf("alloc: %d\n",count);
                printf("\n");
                pclose(fpipe);
            }
        }
        if (!timer_flag) break;
        hpas_sleep(120.0);
        free(keep);
        keep = NULL;
        hpas_sleep(10.0);
    }
    free(keep);
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("%sFinished memeater.\n", asctime(timeinfo));
    return 0;
}
