dnl $Id$
dnl config.m4 for extension radius

PHP_ARG_ENABLE(radius, whether to enable radius support,
dnl Make sure that the comment is aligned:
[  --enable-radius           Enable radius support])

if test "$PHP_RADIUS" != "no"; then

  AC_TRY_COMPILE([
#include <sys/types.h>
  ], [
u_int32_t ulongint;
ulongint = 1;
  ], [
    AC_DEFINE(HAVE_U_INT32_T, 1, [ ])
  ])

 PHP_NEW_EXTENSION(radius, radius.c radlib.c, $ext_shared)
fi
