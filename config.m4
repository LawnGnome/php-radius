dnl $Id$
dnl config.m4 for extension radius

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(radius, for radius support,
[  --with-radius             Include radius support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(radius, whether to enable radius support,
dnl Make sure that the comment is aligned:
dnl [  --enable-radius           Enable radius support])

if test "$PHP_RADIUS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-radius -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/radius.h"  # you most likely want to change this
  dnl if test -r $PHP_RADIUS/; then # path given as parameter
  dnl   RADIUS_DIR=$PHP_RADIUS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for radius files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       RADIUS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$RADIUS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the radius distribution])
  dnl fi

  dnl # --with-radius -> add include path
  dnl PHP_ADD_INCLUDE($RADIUS_DIR/include)
  


  dnl # --with-radius -> chech for lib and symbol presence
  dnl LIBNAME=radius # you may want to change this
  dnl LIBSYMBOL=rad_auth_open # you most likely want to change this


  PHP_ADD_LIBRARY(radlib, 1, RADIUS_LIBADD)

  PHP_EXTENSION(radius, $ext_shared)
fi
