AC_INIT([HPCAnomalySuite], [0.9], [ates@bu.edu], [hpas], [www.bu.edu/peaclab])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_cache_size.m4])

## Determine the compiler's vendor.

AC_PROG_GREP
COMPILER_VERSION=`"$CC" --version 2>/dev/null`
IS_CRAY_COMPILER=`echo $COMPILER_VERSION | $GREP 'Cray'`
if test ! -z "$IS_CRAY_COMPILER"
then
    # Otherwise cray compiler cannot find libuuid
    LDFLAGS+=' -dynamic '
fi

have_shmem=no
AC_CHECK_HEADERS([shmem.h], [have_shmem=yes],
                 [AC_MSG_WARN(["shmem.h not found, not compiling netoccupy"])])
AM_CONDITIONAL([HAVE_SHMEM], [test "$have_shmem" = "yes"])
AX_CACHE_SIZE
AM_INIT_AUTOMAKE([1.11 subdir-objects -Wall -Werror])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AM_PROG_CC_C_O
AC_OUTPUT
