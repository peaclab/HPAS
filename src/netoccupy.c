#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"
#include "src/utils.h"
#include <shmem.h>

char *s_buf_original;
char *r_buf_original;

static struct option const long_opt[] =
{
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v'},
    {"size", required_argument, NULL, 's'},
    {"duration", required_argument, NULL, 'd'},
    {"start", required_argument, NULL, 't'},
    {NULL, 0, NULL, 0}
};

const char *netoccupy_usage = "Network contention.\n\n"
    "-s, --size (=100M)      The size (in bytes) of messages.\n"
    "-d, --duration (=-1.0)  The total duration (in miliseconds), -1 for infinite.\n"
    "-t, --start (=0.0)      The time to wait (in miliseconds) before starting the anomaly.\n"
    "-v, --verbose           Prints execution information.\n"
    "-h, --help              Prints this message.\n";
static const char *short_opt = "hvs:d:t:";

int netoccupy(int argc, char *argv[])
{
    long int myid, numprocs, i;
    ssize_t size = 100000000;
    double duration = -1;
    double start_time = 0;
    int c;
    bool verbose = false;
    char *s_buf, *r_buf;
    char hostname[64];
    int *hostnum;
    int *all_hosts;
    int first_host = -1;
    int second_host = -1;
    int loop = 100;
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
                    return -1;
                }
                break;

            case 's':
                size = parse_size(optarg);
                if (size == -1) {
                    printf("Wrong size input\n");
                    return -1;
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
                printf("Usage: %s [OPTIONS]\n%s", argv[0], netoccupy_usage);
                return 0;
        }
    }
    hpas_sleep(start_time);
    static long pSync[_SHMEM_COLLECT_SYNC_SIZE];
    for (i=0; i<_SHMEM_COLLECT_SYNC_SIZE; i++) {
        pSync[i] = _SHMEM_SYNC_VALUE;
    }
    s_buf_original = (char *)malloc(size);
    r_buf_original = (char *)malloc(size);

    shmem_init();
    myid = shmem_my_pe();
    numprocs = shmem_n_pes();

    s_buf = (char *)shmalloc(size);
    r_buf = (char *)shmalloc(size);
    all_hosts = (int *)shmalloc(sizeof(int) * numprocs);
    hostnum = (int *)shmalloc(sizeof(int));

    if(!myid && verbose) {
        fprintf(stdout, "Starting network anomaly.\n");
        fflush(stdout);
    }

    gethostname(hostname, sizeof(hostname));
    sscanf(hostname, "nid%d", hostnum);

    shmem_barrier_all();
    shmem_fcollect32(all_hosts, hostnum, 1, 0, 0, numprocs, pSync);
    shmem_barrier_all();
    if (myid == 0 && verbose) {
        for (i=0; i<numprocs; i++) {
            printf("Rank %ld at %d\n", i, all_hosts[i]);
        }
    }

    for (i=0; i<numprocs; i++) {
        if (first_host == -1) {
            first_host = all_hosts[i];
        } else if (second_host == -1 && all_hosts[i] != first_host) {
            second_host = all_hosts[i];
        } else if (all_hosts[i] != first_host && all_hosts[i] != second_host) {
            fprintf(stderr, "Please run with only 2 nodes. More nodes require multiple mpi jobs.\n");
            return -1;
        }
    }
    if (second_host < first_host) {
        int tmp = second_host;
        second_host = first_host;
        first_host = tmp;
    }
    int counter = 0;
    int pair_no = 0;
    if(*hostnum == first_host) {
        for (i=0; i<numprocs; i++) {
            if (i == myid) {
                pair_no = counter;
                break;
            }
            if (all_hosts[i] == first_host) {
                counter++;
            }
        }
        counter = 0;
        for (i=0; i<numprocs; i++) {
            if (all_hosts[i] == second_host) {
                if (counter == pair_no) {
                    pair_no = i;
                    break;
                }
                counter++;
            }
        }
        if (verbose) {
            printf("Rank %ld sending to %d\n", myid, pair_no);
            fflush(stdout);
        }
    }

    /* touch the data */
    for(i = 0; i < size; i++) {
        s_buf[i] = 'a';
        r_buf[i] = 'b';
    }
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (!myid) {
        printf("%sStarting netoccupy with npes:%ld size:%ld duration:%f\n\n",
               asctime(timeinfo), numprocs, size, duration);
        fflush(stdout);
    }

    shmem_barrier_all();
    set_duration(duration);
    if(*hostnum == first_host)
    {
        while (timer_flag) {
            for(i = 0; i < loop; i++) {

                shmem_putmem(r_buf, s_buf, size, pair_no);
            }
            shmem_quiet();
            if (!myid && verbose) {
                printf("Finished %d puts\n", loop);
                fflush(stdout);
            }
        }
    }
    shmem_barrier_all();

    shmem_free(s_buf);
    shmem_free(r_buf);
    shmem_free(all_hosts);
    free(s_buf_original);
    free(r_buf_original);

    shmem_barrier_all();
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (!myid) {
        printf("%sCompleted netoccupy.\n\n", asctime(timeinfo));
        fflush(stdout);
    }
    shmem_finalize();

    return 0;
}
