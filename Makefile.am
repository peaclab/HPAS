AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4 --install
AM_CFLAGS = -DBINDIR='"$(bindir)"'

dist_bin_SCRIPTS = src/iobandwidth
bin_PROGRAMS = hpas

hpas_SOURCES = src/cachecopy.c src/cpuoccupy.c src/iometadata.c src/main.c src/membw.c src/memeater.c src/memleak.c src/utils.c src/utils.h src/anomalies.h
if HAVE_SHMEM
hpas_SOURCES += src/netoccupy.c
endif
