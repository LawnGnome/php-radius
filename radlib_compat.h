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
#define timeradd(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;       \
                if ((vvp)->tv_usec >= 1000000) {                        \
                        (vvp)->tv_sec++;                                \
                        (vvp)->tv_usec -= 1000000;                      \
                }                                                       \
        } while (0)
#endif

#ifndef timersub
#define timersub(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;       \
                if ((vvp)->tv_usec < 0) {                               \
                        (vvp)->tv_sec--;                                \
                        (vvp)->tv_usec += 1000000;                      \
                }                                                       \
        } while (0)
#endif

#endif