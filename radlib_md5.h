/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2002 The PHP Group                                |
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

#include "php.h"
#include "ext/standard/md5.h"

#define MD5Init PHP_MD5Init
#define MD5Update PHP_MD5Update
#define MD5Final PHP_MD5Final
#define MD5_CTX PHP_MD5_CTX
