#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <config.h>
#include "src/anomalies.h"

#ifndef BINDIR
#  define BINDIR "."
#endif

static const char *main_usage = "Please input a valid anomaly name:\n"
    "memleak, memeater, membw, cpuoccupy, netoccupy, cachecopy,\n"
    "iometadata, iobandwidth.\n";

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("%s", main_usage);
        _exit(0);
    }
    if (strcmp(argv[1], "memleak") == 0) {
        return memleak(argc - 1, &argv[1]);
    }
    if (strcmp(argv[1], "memeater") == 0) {
        return memeater(argc - 1, &argv[1]);
    }
    if (strcmp(argv[1], "membw") == 0) {
#ifndef HAVE_XMM_H
        printf("Please compile with xmmintrin.h to get memory bandwidth anomaly.\n");
#else
        return membw(argc - 1, &argv[1]);
#endif
    }
    if (strcmp(argv[1], "cpuoccupy") == 0) {
        return cpuoccupy(argc - 1, &argv[1]);
    }
    if (strcmp(argv[1], "netoccupy") == 0) {
#ifndef HAVE_SHMEM_H
        printf("Please compile with SHMEM to get network contention anomaly.\n");
        return 0;
#else
        return netoccupy(argc - 1, &argv[1]);
#endif
    }
    if (strcmp(argv[1], "cachecopy") == 0) {
        return cachecopy(argc - 1, &argv[1]);
    }
    if (strcmp(argv[1], "iometadata") == 0) {
        return iometadata(argc - 1, &argv[1]);
    }
    if (strcmp(argv[1], "iobandwidth") == 0) {
        execvp(BINDIR"/iobandwidth", &argv[1]);
        if (errno != 0) {
            printf("exec failed: %s\n"
                   "This may be because `make install` is not executed.\n"
                   "Check if %s exists.\n",
                   strerror(errno), BINDIR"/iobandwidth");
            _exit(0);
        }
        return 0;
    }
    printf("%s", main_usage);
    _exit(0);
}
