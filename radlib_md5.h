#include "php.h"
#include "ext/standard/md5.h"

#define MD5Init PHP_MD5Init
#define MD5Update PHP_MD5Update
#define MD5Final PHP_MD5Final
#define MD5_CTX PHP_MD5_CTX