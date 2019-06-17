#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <stdbool.h>
#include <src/utils.h>

static struct option const long_opt[] =
{
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"size", required_argument, NULL, 's'},
    {"period", required_argument, NULL, 'p'},
    {"duration", required_argument, NULL, 'd'},
    {"start", required_argument, NULL, 't'},
    {NULL, 0, NULL, 0}
};

const char *memleak_usage = "Memory leak.\n\n"
    "-s, --size (=20M)       The size (in bytes) of the array to be allocated.\n"
    "-p, --period (=0.2)     The time to wait (in seconds) between array allocations.\n"
    "-d, --duration (=-1.0)  The total duration (in seconds), -1 for infinite.\n"
    "-t, --start (=0.0)      The time to wait (in seconds) before starting the anomaly.\n"
    "-v, --verbose           Prints execution information.\n"
    "-h, --help              Prints this message.\n";
static const char *short_opt = "hvd:s:p:t:";

int memleak(int argc, char *argv[])
{
    int c;

    bool verbose = false;
    long int size = 20 * 1024 * 1024;
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
                printf("Usage: %s [OPTIONS]\n%s", argv[0], memleak_usage);
                return 0;
        }
    }
    hpas_sleep(start_time);
    time_t rawtime;
    struct tm *timeinfo;
    FILE *fpipe;
    char *command = "cat /proc/meminfo | grep -i active";
    char line[256];
    char *keep;
    long int j;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("%sStarting leak with size:%ld period:%f duration:%f\n\n",
            asctime(timeinfo), size, period, duration);
    fflush(stdout);

    /* this is a loop calling the malloc function which
     * allocates the memory but without saving the address of the
     * allocated place
     */
    set_duration(duration);
    while (timer_flag) {
        hpas_sleep(period);

        char *temp = malloc(size * sizeof(char));
        if (!temp){
            break;
            /* malloc will return NULL sooner or later, due to lack of memory */
        }

        for (j = 0; j < size; j++){
            temp[j] = 'a';
        }

        if (verbose) {
            if (!(fpipe = (FILE*) popen(command, "r"))){
                perror("Problems with pipe");
                exit(1);
            }
            while (fgets( line, sizeof line, fpipe)){
                printf("%s", line);
            }
            printf("\n");
            pclose(fpipe);
        }

        keep = temp;
    }
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("%sFinished leak.\n", asctime(timeinfo));
    return keep + 0 - keep; /* suppress unused variable warning */
    /* free the allocated memory by operating system itself after program exits */
}
