/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2003 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Michael Bretterklieber <mbretter@bretterklieber.com>         |
  +----------------------------------------------------------------------+

  $Id$ 
*/

#ifndef _RADLIB_COMPAT_H_
#define _RADLIB_COMPAT_H_

#include "php.h"
#include "ext/standard/php_rand.h"
#include "ext/standard/php_standard.h"

#ifdef PHP_WIN32
typedef unsigned int u_int32_t;
typedef long ssize_t;
int inet_aton(const char *cp, struct in_addr *inp);
char *strsep(char **stringp,	const char *delim);
#define MSG_WAITALL 0
#include "php_network.h"
#endif

#ifndef __printflike
#define	__printflike(fmtarg, firstvararg)
#endif

#ifndef timeradd
#define timeradd(tvp, uvp, vvp)                                     \
    do {                                                            \
        (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;              \
        (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;           \
        if ((vvp)->tv_usec >= 1000000) {                            \
            (vvp)->tv_sec++;                                        \
            (vvp)->tv_usec -= 1000000;                              \
        }                                                           \
    } while (0)
#endif

#ifndef timersub
#define timersub(tvp, uvp, vvp)                                     \
    do {                                                            \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;              \
        (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;           \
        if ((vvp)->tv_usec < 0) {                                   \
            (vvp)->tv_sec--;                                        \
            (vvp)->tv_usec += 1000000;                              \
        }                                                           \
    } while (0)
#endif

#endif
