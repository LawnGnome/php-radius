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
  | Author: Michael Bretterklieber <michael@bretterklieber.com>         |
  +----------------------------------------------------------------------+

  $Id$
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_network.h"
#include "ext/standard/info.h"
#include "php_radius.h"
#include "radlib.h"
#include "radlib_private.h"

#ifndef PHP_WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

void _radius_close(zend_rsrc_list_entry *rsrc TSRMLS_DC);

/* If you declare any globals in php_radius.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(radius)
*/

/* True global resources - no need for thread safety here */
static int le_radius;

/* {{{ radius_functions[]
 *
 * Every user visible function must have an entry in radius_functions[].
 */
function_entry radius_functions[] = {
    PHP_FE(radius_auth_open,    NULL)
    PHP_FE(radius_acct_open,    NULL)
    PHP_FE(radius_close,        NULL)
    PHP_FE(radius_strerror,     NULL)
    PHP_FE(radius_config,       NULL)
    PHP_FE(radius_add_server,	NULL)
    PHP_FE(radius_create_request,	NULL)
    PHP_FE(radius_put_string,	NULL)
    PHP_FE(radius_put_int,	NULL)
    PHP_FE(radius_put_attr,	NULL)
    PHP_FE(radius_put_addr,	NULL)
    PHP_FE(radius_put_vendor_string,	NULL)
    PHP_FE(radius_put_vendor_int,	NULL)
    PHP_FE(radius_put_vendor_attr,	NULL)
    PHP_FE(radius_put_vendor_addr,	NULL)
    PHP_FE(radius_send_request,	NULL)
    PHP_FE(radius_get_attr,	NULL)
    PHP_FE(radius_get_vendor_attr,	NULL)
    PHP_FE(radius_cvt_addr,	NULL)
    PHP_FE(radius_cvt_int,	NULL)
    PHP_FE(radius_cvt_string,	NULL)
    PHP_FE(radius_request_authenticator,	NULL)
    PHP_FE(radius_server_secret,	NULL)
    PHP_FE(radius_demangle,	NULL)    
    PHP_FE(radius_demangle_mppe_key,	NULL)    
    {NULL, NULL, NULL}	/* Must be the last line in radius_functions[] */
};
/* }}} */

/* {{{ radius_module_entry
 */
zend_module_entry radius_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "radius",
    radius_functions,
    PHP_MINIT(radius),
    PHP_MSHUTDOWN(radius),
    PHP_RINIT(radius),      /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(radius),  /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(radius),
#if ZEND_MODULE_API_NO >= 20010901
    "1.1", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_RADIUS
ZEND_GET_MODULE(radius)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("radius.global_value",      "42", PHP_INI_ALL, OnUpdateInt, global_value, zend_radius_globals, radius_globals)
    STD_PHP_INI_ENTRY("radius.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_radius_globals, radius_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_radius_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_radius_init_globals(zend_radius_globals *radius_globals)
{
	radius_globals->global_value = 0;
	radius_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(radius)
{
    /* If you have INI entries, uncomment these lines
    ZEND_INIT_MODULE_GLOBALS(radius, php_radius_init_globals, NULL);
    REGISTER_INI_ENTRIES();
    */
    le_radius = zend_register_list_destructors_ex(_radius_close, NULL, "rad_handle", module_number);
	#include "radius_init_const.h"
    REGISTER_LONG_CONSTANT("RADIUS_MPPE_KEY_LEN", MPPE_KEY_LEN, CONST_PERSISTENT);    
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(radius)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(radius)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(radius)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(radius)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "radius support", "enabled");
	php_info_print_table_row(2, "Revision", "$Revision$");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* {{{ proto ressource radius_auth_open(string arg) */
PHP_FUNCTION(radius_auth_open)
{
    radius_descriptor *raddesc;

    raddesc = emalloc(sizeof(radius_descriptor));
    raddesc->radh = rad_auth_open();
    raddesc->request_created = 0;

    if (raddesc->radh != NULL) {
        ZEND_REGISTER_RESOURCE(return_value, raddesc, le_radius);
        raddesc->id = Z_LVAL_P(return_value);
    } else {
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto ressource radius_acct_open(string arg) */
PHP_FUNCTION(radius_acct_open)
{
    radius_descriptor *raddesc;

    raddesc = emalloc(sizeof(radius_descriptor));
    raddesc->radh = rad_acct_open();
    raddesc->request_created = 0;

    if (raddesc->radh != NULL) {
        ZEND_REGISTER_RESOURCE(return_value, raddesc, le_radius);
        raddesc->id = Z_LVAL_P(return_value);
    } else {
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool radius_close(radh) */
PHP_FUNCTION(radius_close)
{
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    zend_list_delete(raddesc->id);
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto string radius_strerror(radh) */
PHP_FUNCTION(radius_strerror)
{
    char *msg;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    msg = (char *)rad_strerror(raddesc->radh);
    RETURN_STRINGL(msg, strlen(msg), 1);
}
/* }}} */

/* {{{ proto bool radius_config(desc, configfile) */
PHP_FUNCTION(radius_config)
{
    char *filename;
    int filename_len;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &filename, &filename_len) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_config(raddesc->radh, filename) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_add_server(desc, hostname, port, secret, timeout, maxtries) */
PHP_FUNCTION(radius_add_server)
{
    char *hostname, *secret;
    int hostname_len, secret_len, port, timeout, maxtries;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rslsll", &z_radh,
                              &hostname, &hostname_len,
                              &port,
                              &secret, &secret_len,
                              &timeout, &maxtries) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_add_server(raddesc->radh, hostname, port, secret, timeout, maxtries) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_create_request(desc, code) */
PHP_FUNCTION(radius_create_request)
{
    int code;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &z_radh, &code) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_create_request(raddesc->radh, code) == -1) {
        RETURN_FALSE;
    } else {
        raddesc->request_created = 1;
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_string(desc, type, str) */
PHP_FUNCTION(radius_put_string)
{
    char *str;
    int str_len, type;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls", &z_radh, &type, &str, &str_len)
            == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_put_string(raddesc->radh, type, str) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_int(desc, type, int) */
PHP_FUNCTION(radius_put_int)
{
    int type;
    unsigned int val;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll", &z_radh, &type, &val)
            == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_put_int(raddesc->radh, type, val) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_attr(desc, type, data) */
PHP_FUNCTION(radius_put_attr)
{
    int type, len;
    char *data;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls", &z_radh, &type, &data, &len)
            == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_put_attr(raddesc->radh, type, data, len) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_addr(desc, type, addr) */
PHP_FUNCTION(radius_put_addr)
{
    int type, addrlen;
    char	*addr;
    radius_descriptor *raddesc;
    zval *z_radh;
    struct in_addr intern_addr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls", &z_radh, &type, &addr, &addrlen)
            == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (inet_aton(addr, &intern_addr) == 0) {
        strcpy(raddesc->errmsg, "Error converting Address");
        RETURN_FALSE;
    }

    if (rad_put_addr(raddesc->radh, type, intern_addr) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_vendor_string(desc, vendor, type, str) */
PHP_FUNCTION(radius_put_vendor_string)
{
    char *str;
    int str_len, type, vendor;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls", &z_radh, &vendor, &type, &str, &str_len)
            == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_put_vendor_string(raddesc->radh, vendor, type, str) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_vendor_int(desc, vendor, type, int) */
PHP_FUNCTION(radius_put_vendor_int)
{
    int type, vendor;
    unsigned int val;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlll", &z_radh, &vendor, &type, &val)
            == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_put_vendor_int(raddesc->radh, vendor, type, val) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_vendor_attr(desc, vendor, type, data) */
PHP_FUNCTION(radius_put_vendor_attr)
{
    int type, len, vendor;
    char *data;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls", &z_radh, &vendor, &type,
                              &data, &len) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (rad_put_vendor_attr(raddesc->radh, vendor, type, data, len) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_put_vendor_addr(desc, vendor, type, addr) */
PHP_FUNCTION(radius_put_vendor_addr)
{
    int type, addrlen, vendor;
    char	*addr;
    radius_descriptor *raddesc;
    zval *z_radh;
    struct in_addr intern_addr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls", &z_radh, &vendor,
                              &type, &addr, &addrlen) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    if (inet_aton(addr, &intern_addr) == 0) {
        strcpy(raddesc->errmsg, "Error converting Address");
        RETURN_FALSE;
    }

    if (rad_put_vendor_addr(raddesc->radh, vendor, type, intern_addr) == -1) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }

}
/* }}} */

/* {{{ proto bool radius_send_request(desc) */
PHP_FUNCTION(radius_send_request)
{
    radius_descriptor *raddesc;
    zval *z_radh;
    int res;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh)
            == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    res = rad_send_request(raddesc->radh);
    if (res == -1) {
        RETURN_FALSE;
    } else {
        RETURN_LONG(res);
    }

}
/* }}} */

/* {{{ proto string radius_get_attr(desc) */
PHP_FUNCTION(radius_get_attr)
{
    radius_descriptor *raddesc;
    int res;
    const void *data;
    size_t len;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    res = rad_get_attr(raddesc->radh, &data, &len);
    if (res == -1) {
        RETURN_FALSE;
    } else {

        if (res > 0) {

            if(array_init(return_value) != SUCCESS) {
                zend_error(E_WARNING, "Could not initialize array");
                RETURN_FALSE;
            }
            add_assoc_long(return_value, "attr", res);
            add_assoc_stringl(return_value, "data", (char *) data, len, 1);
            return;
        }

        RETURN_LONG(res);
    }

}
/* }}} */

/* {{{ proto string radius_get_vendor_attr(data) */
PHP_FUNCTION(radius_get_vendor_attr)
{
    int res, vendor;
    const void *data;
    size_t len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &len) == FAILURE) {
        return;
    }

    res = rad_get_vendor_attr(&vendor, &data, &len);
    if (res == -1) {
        RETURN_FALSE;
    } else {

        if(array_init(return_value) != SUCCESS) {
            zend_error(E_WARNING, "Could not initialize array");
            RETURN_FALSE;
        }
        add_assoc_long(return_value, "attr", res);
        add_assoc_long(return_value, "vendor", vendor);
        add_assoc_stringl(return_value, "data", (char *) data, len, 1);
        return;
    }

}
/* }}} */

/* {{{ proto string radius_cvt_addr(data) */
PHP_FUNCTION(radius_cvt_addr)
{
    const void *data;
    char *addr_dot;
    int len;
    struct in_addr addr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &len) == FAILURE) {
        return;
    }

    addr = rad_cvt_addr(data);
    addr_dot = inet_ntoa(addr);
    RETURN_STRINGL(addr_dot, strlen(addr_dot), 1);
}
/* }}} */

/* {{{ proto int radius_cvt_int(data) */
PHP_FUNCTION(radius_cvt_int)
{
    const void *data;
    int len, val;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &len)
            == FAILURE) {
        return;
    }

    val = rad_cvt_int(data);
    RETURN_LONG(val);
}
/* }}} */

/* {{{ proto string radius_cvt_string(data) */
PHP_FUNCTION(radius_cvt_string)
{
    const void *data;
    char *val;
    int len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &len)
            == FAILURE) {
        return;
    }

    val = rad_cvt_string(data, len);

    if (val == NULL) RETURN_FALSE;
    RETVAL_STRINGL(val, strlen(val), 1);
    free(val);
    return;
}
/* }}} */

/* {{{ proto string radius_request_authenticator(radh) */
PHP_FUNCTION(radius_request_authenticator)
{
    radius_descriptor *raddesc;
    ssize_t res;
    char buf[LEN_AUTH];
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    res = rad_request_authenticator(raddesc->radh, buf, sizeof buf);

    if (res == -1) {
        RETURN_FALSE;
    } else {
        RETURN_STRINGL(buf, res, 1);
    }

}
/* }}} */

/* {{{ proto string radius_server_secret(radh) */
PHP_FUNCTION(radius_server_secret)
{
    char *secret;
    radius_descriptor *raddesc;
    zval *z_radh;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    secret = (char *)rad_server_secret(raddesc->radh);
    RETURN_STRINGL(secret, strlen(secret), 1);
}
/* }}} */

/* {{{ proto string radius_demangle(radh, mangled) */
PHP_FUNCTION(radius_demangle)
{
    radius_descriptor *raddesc;
    zval *z_radh;
    const void *mangled;
    unsigned char *buf;
    size_t len;
    int res;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &mangled, &len) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    buf = emalloc(len);
    res = rad_demangle(raddesc->radh, mangled, len, buf);

    if (res == -1) {
        efree(buf);
        RETURN_FALSE;
    } else {
        RETVAL_STRINGL(buf, len, 1); 
        efree(buf);
        return;
    }

}
/* }}} */

/* {{{ proto string radius_demangle_mppe_key(radh, mangled) */
PHP_FUNCTION(radius_demangle_mppe_key)
{
    radius_descriptor *raddesc;
    zval *z_radh;
    const void *mangled;
    unsigned char *buf;
    size_t len, dlen;
    int res;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &mangled, &len) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

    buf = emalloc(len);
    res = rad_demangle_mppe_key(raddesc->radh, mangled, len, buf, &dlen);

    if (res == -1) {
        efree(buf);
        RETURN_FALSE;
    } else {
        RETVAL_STRINGL(buf, dlen, 1); 
        efree(buf);
        return;
    }

}
/* }}} */

/* {{{ _radius_close() */
void _radius_close(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    radius_descriptor *raddesc = (radius_descriptor *)rsrc->ptr;
    rad_close(raddesc->radh);
    efree(raddesc);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
