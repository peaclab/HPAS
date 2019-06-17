#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <xmmintrin.h>
#include <getopt.h>
#include <stdbool.h>
#include "src/utils.h"

static struct option const long_opt[] =
{
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"size", required_argument, NULL, 's'},
    {"start", required_argument, NULL, 't'},
    {"duration", required_argument, NULL, 'd'},
    {NULL, 0, NULL, 0}
};

const char *membw_usage = "Memory Bandwidth Contention.\n\n"
    "-s, --size (=4K)       One dimension (in bytes) of 2D array.\n"
    "-d, --duration (=-1.0) The total duration (in seconds), -1 for infinite.\n"
    "-t, --start (=0.0)     The time to wait (in seconds) before starting the anomaly.\n"
    "-v, --verbose          Prints execution information.\n"
    "-h, --help             Prints this message.\n";
static const char short_opt[] = "hvd:s:t:";

void temporalCopy(const double *orig, double *swap, size_t size)
{
    size_t m, i;
    for(m = 0; m < size; m++) {
        for (i = 0; i < size; i++) {
             _mm_stream_pi(
                 (__m64 *) (&swap[size * i + m]),
                 *(__m64 *) (&orig[size * m + i])); // MOVNTQ
             _mm_empty(); // EMMS
        }
    }
}

int membw(int argc, char *argv[])
{

    clock_t start_loop, end_loop;
    double loop_time;
    double *array;
    double *swap_array;
    bool verbose = false;
    ssize_t size = 4096;
    double start_time = 0;
    double duration = -1;

    int m, i, c;
    srand(time(NULL));
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

            case 'd':
                duration = strtod(optarg, NULL);
                break;

            case 's':
                size = parse_size(optarg);
                if (size == -1) {
                    printf("Wrong size input\n");
                    return -1;
                }
                break;

            case 'v':
                verbose = true;
                break;

            case 'h':
            default:
                printf("Usage: %s [OPTIONS]\n%s", argv[0], membw_usage);
                return 0;
        }
    }
    hpas_sleep(start_time);
    size = size / sizeof(double);
    array = (double *) malloc(size * size * sizeof(double));
    swap_array = (double *) malloc(size * size * sizeof(double));
    if (array == NULL || swap_array == NULL) {
        printf("Failed to allocate memory\n");
        return -1;
    }

    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("%sStarting membw with doublecount:%ld duration:%f\n\n",
            asctime(timeinfo), size * size, duration);
    fflush(stdout);

    start_loop = clock();
    for(m = 0; m < size; m++) {
        for (i = 0; i < size; i++) {
            swap_array[m * size + i] = (double) rand();
        }
    }
    end_loop = clock();

    if (verbose) {
        loop_time = ((double)(end_loop - start_loop))/CLOCKS_PER_SEC;
        printf("Started memory bandwidth anomaly, array is filled: %fs\n", loop_time);
    }

    int counter = 0;
    start_loop = clock();
    set_duration(duration);
    while (timer_flag) {
        temporalCopy(array, swap_array, size);
        if (verbose && ++counter % 100 == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    end_loop = clock();

    if (verbose) {
        loop_time = ((double)(end_loop - start_loop))/CLOCKS_PER_SEC;
        printf("\n Array generation, first row is filled: %fs\n", loop_time);
    }
    free(array);
    free(swap_array);
    return 0;
}



