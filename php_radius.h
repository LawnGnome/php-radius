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
  | Author:                                                              |
  +----------------------------------------------------------------------+

  $Id$ 
*/

#include "radlib.h"
#include "radlib_private.h"

#ifndef PHP_RADIUS_H
#define PHP_RADIUS_H

#define phpext_radius_ptr &radius_module_entry

#ifdef PHP_WIN32
#define PHP_RADIUS_API __declspec(dllexport)
#else
#define PHP_RADIUS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

extern zend_module_entry radius_module_entry;

typedef struct {
	int id;
 	short request_created;
 	char errmsg[ERRSIZE];
	struct rad_handle *radh;
} radius_descriptor;

PHP_MINIT_FUNCTION(radius);
PHP_MSHUTDOWN_FUNCTION(radius);
PHP_RINIT_FUNCTION(radius);
PHP_RSHUTDOWN_FUNCTION(radius);
PHP_MINFO_FUNCTION(radius);

PHP_FUNCTION(radius_auth_open);
PHP_FUNCTION(radius_acct_open);
PHP_FUNCTION(radius_close);
PHP_FUNCTION(radius_strerror);
PHP_FUNCTION(radius_config);
PHP_FUNCTION(radius_add_server);
PHP_FUNCTION(radius_create_request);
PHP_FUNCTION(radius_put_string);
PHP_FUNCTION(radius_put_int);
PHP_FUNCTION(radius_put_attr);
PHP_FUNCTION(radius_put_addr);
PHP_FUNCTION(radius_put_vendor_string);
PHP_FUNCTION(radius_put_vendor_int);
PHP_FUNCTION(radius_put_vendor_attr);
PHP_FUNCTION(radius_put_vendor_addr);
PHP_FUNCTION(radius_send_request);
PHP_FUNCTION(radius_get_attr);
PHP_FUNCTION(radius_get_vendor_attr);
PHP_FUNCTION(radius_cvt_addr);
PHP_FUNCTION(radius_cvt_int);
PHP_FUNCTION(radius_cvt_string);
PHP_FUNCTION(radius_request_authenticator);
PHP_FUNCTION(radius_server_secret);

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(radius)
	int   global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(radius)
*/

/* In every utility function you add that needs to use variables 
   in php_radius_globals, call TSRM_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMG_CC
   after the last function argument and declare your utility function
   with TSRMG_DC after the last declared argument.  Always refer to
   the globals in your function as RADIUS_G(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define RADIUS_G(v) TSRMG(radius_globals_id, zend_radius_globals *, v)
#else
#define RADIUS_G(v) (radius_globals.v)
#endif

#endif	/* PHP_RADIUS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */