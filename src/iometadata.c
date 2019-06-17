#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "src/utils.h"


static struct option const long_opt[] =
{
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"start", required_argument, NULL, 't'},
    {"duration", required_argument, NULL, 'd'},
    {"location", required_argument, NULL, 'l'},
    {NULL, 0, NULL, 0}
};

const char *iometadata_usage = "Metadata Server Interference Anomaly\n\n"
    "-d, --duration (=-1.0)         Duration of the anomaly (in seconds).\n"
    "-t, --start (=0.0)             The time to wait (in seconds) before starting the anomaly.\n"
    "-l, --location (='./hpas_tmp') Target directory \n"
    "-v, --verbose                  Prints execution information.\n"
    "-h, --help                     Prints this message.\n";
static const char short_opt[] = "hvd:l:t:";

int iometadata(int argc, char *argv[])
{
    int c=-1;
    bool verbose = false;
    double start_time = 0;
    double duration = -1;
    char *location = NULL;
    size_t location_size;

    while ((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {


        switch(c)
        {
            case -1:
            case 0:
                break;

            case 't':
                start_time = strtol(optarg, NULL, 10);
                if (start_time < 0) {
                    printf("Start time cannot be negative.\n");
                    _exit(0);
                }
                break;

            case 'l':
                location_size = strlen(optarg) + 1;
                if (location_size > PATH_MAX) {
                    printf("Location name too long");
                    _exit(0);
                }
                location = (char *) malloc(location_size);
                if (location == NULL) {
                    printf("Malloc failed.\n");
                    _exit(0);
                }
                memcpy(location, optarg, location_size);
                break;

            case 'd':
                duration = strtod(optarg, NULL);
                break;

            case 'v':
                verbose = true;
                break;

            case 'h':
            default:
                printf("Usage: %s [OPTIONS]\n%s", argv[0], iometadata_usage);
                return 0;
        }
    }
    hpas_sleep(start_time);
    if (location == NULL) {
        location_size = 11;
        location = (char *) malloc(location_size);
        strncpy(location, "./hpas_tmp", location_size);
    }
    location[location_size - 1] = '\0';
    char *foldername;
    foldername = (char *) malloc(location_size + 8);
    sprintf(foldername, "%s/XXXXXX", location);

    char *filename = malloc(location_size + 10);
    char *filenames[10];
    int ite;
    for (ite = 0; ite < 10; ite++) {
        filenames[ite] = malloc(location_size + 10);
    }

    foldername = mkdtemp(foldername);
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("%sStarting metadata anomaly with d=%f, l=%s\n",
            asctime(timeinfo), duration, foldername);
    int write_result;
    int k = 0;
    int i = 0;
    int counter = 0;
    int fds[10];
    set_duration(duration);
    while(timer_flag)
    {
        sprintf(filename, "%s/%d", foldername, k % 10);
        remove(filename);
        // O_RDONLY: Read only
        // O_CREAT: Create file if doesn't exist
        // S_IRUSR: Set read rights for the owner true.
        fds[k % 10] = open(filename, O_RDWR | O_CREAT, S_IRUSR);
        /* printf("Opened the file with fd = %d \n",fds[k % 10]); */
        strncpy(filenames[k % 10], filename, location_size + 10);

        write_result = write(fds[k  % 10], "a\n", 2);
        if (write_result != 2) {
            printf("Write failed.\n");
            _exit(0);
        }
        if ((k % 10) == 9) {
            for (i=0; i < 10; i++) {
                close(fds[i]);
                /* printf("Closed the file with fd = %d \n",fds[i]); */
                remove(filenames[i]);
            }
        }
        if (verbose && ++counter % 1000 == 0) {
            printf(".");
            fflush(stdout);
        }
        k = k + 1;
    }
    for (i=0; i < k % 10; i++) {
        close(fds[i]);
        remove(filenames[i]);
    }
    rmdir(foldername);
    for (i=0; i<10; i++) {
        free(filenames[i]);
    }
    free(filename);
    free(foldername);
    free(location);
    return 0;
}


