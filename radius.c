/*
Copyright (c) 2003, Michael Bretterklieber <michael@bretterklieber.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:

1. Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the distribution.
3. The names of the authors may not be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This code cannot simply be copied and put under the GNU Public License or 
any other GPL-like (LGPL, GPL2) License.

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

struct salted_value {
	size_t len;
	char *data;
};
static int _salt_value(struct rad_handle *h, const char *in, size_t len, struct salted_value *out TSRMLS_DC);

/* If you declare any globals in php_radius.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(radius)
*/

/* True global resources - no need for thread safety here */
static int le_radius;

/* {{{ radius_functions[]
 *
 * Every user visible function must have an entry in radius_functions[].
 */
zend_function_entry radius_functions[] = {
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
	PHP_FE(radius_get_tagged_attr_data, NULL)
	PHP_FE(radius_get_tagged_attr_tag, NULL)
	PHP_FE(radius_get_vendor_attr,	NULL)
	PHP_FE(radius_cvt_addr,	NULL)
	PHP_FE(radius_cvt_int,	NULL)
	PHP_FE(radius_cvt_string,	NULL)
	PHP_FE(radius_salt_encrypt_attr,	NULL)
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
	NULL,
	NULL,
	PHP_MINFO(radius),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_RADIUS_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_RADIUS
ZEND_GET_MODULE(radius)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(radius)
{
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
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(radius)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "radius support", "enabled");
	php_info_print_table_row(2, "version", PHP_RADIUS_VERSION);
	php_info_print_table_end();
}
/* }}} */

/* {{{ proto ressource radius_auth_open(string arg) */
PHP_FUNCTION(radius_auth_open)
{
	radius_descriptor *raddesc;

	raddesc = emalloc(sizeof(radius_descriptor));
	raddesc->radh = rad_auth_open();

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
	int hostname_len, secret_len;
	long  port, timeout, maxtries;
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
	long code;
	radius_descriptor *raddesc;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &z_radh, &code) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (rad_create_request(raddesc->radh, code) == -1) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto bool radius_put_string(desc, type, str, options, tag) */
PHP_FUNCTION(radius_put_string)
{
	char *str;
	int str_len;
	long type, options = 0, tag = 0;
	radius_descriptor *raddesc;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls|ll", &z_radh, &type, &str, &str_len, &options, &tag)
		== FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_string_tag(raddesc->radh, type, str, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_string(raddesc->radh, type, str) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_int(desc, type, int, options, tag) */
PHP_FUNCTION(radius_put_int)
{
	long type, val, options = 0, tag = 0;
	radius_descriptor *raddesc;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll|ll", &z_radh, &type, &val, &options, &tag)
		== FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_int_tag(raddesc->radh, type, val, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_int(raddesc->radh, type, val) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_attr(desc, type, data, options, tag) */
PHP_FUNCTION(radius_put_attr)
{
	long type, options = 0, tag = 0;
	int len;
	char *data;
	radius_descriptor *raddesc;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls|ll", &z_radh, &type, &data, &len, &options, &tag)
		== FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_attr_tag(raddesc->radh, type, data, len, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_attr(raddesc->radh, type, data, len) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;

}
/* }}} */

/* {{{ proto bool radius_put_addr(desc, type, addr, options, tag) */
PHP_FUNCTION(radius_put_addr)
{
	int addrlen;
	long type, options = 0, tag = 0;
	char	*addr;
	radius_descriptor *raddesc;
	zval *z_radh;
	struct in_addr intern_addr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls|ll", &z_radh, &type, &addr, &addrlen, &options, &tag)
		== FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (inet_aton(addr, &intern_addr) == 0) {
		zend_error(E_ERROR, "Error converting Address");
		RETURN_FALSE;
	}

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_addr_tag(raddesc->radh, type, intern_addr, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_addr(raddesc->radh, type, intern_addr) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_string(desc, vendor, type, str, options, tag) */
PHP_FUNCTION(radius_put_vendor_string)
{
	char *str;
	int str_len;
	long type, vendor, options = 0, tag = 0;
	radius_descriptor *raddesc;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls|ll", &z_radh, &vendor, &type, &str, &str_len, &options, &tag)
		== FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_vendor_string_tag(raddesc->radh, vendor, type, str, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_vendor_string(raddesc->radh, vendor, type, str) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_int(desc, vendor, type, int, options, tag) */
PHP_FUNCTION(radius_put_vendor_int)
{
	long type, vendor, val, options = 0, tag = 0;
	radius_descriptor *raddesc;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlll|ll", &z_radh, &vendor, &type, &val, &options, &tag)
		== FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_vendor_int_tag(raddesc->radh, vendor, type, val, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_vendor_int(raddesc->radh, vendor, type, val) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_attr(desc, vendor, type, data, options, tag) */
PHP_FUNCTION(radius_put_vendor_attr)
{
	long type, vendor, options = 0, tag = 0;
	int len;
	char *data;
	radius_descriptor *raddesc;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls|ll", &z_radh, &vendor, &type,
		&data, &len, &options, &tag) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_vendor_attr_tag(raddesc->radh, vendor, type, data, len, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_vendor_attr(raddesc->radh, vendor, type, data, len) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_addr(desc, vendor, type, addr) */
PHP_FUNCTION(radius_put_vendor_addr)
{
	long type, vendor, options = 0, tag = 0;
	int addrlen;
	char	*addr;
	radius_descriptor *raddesc;
	zval *z_radh;
	struct in_addr intern_addr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls|ll", &z_radh, &vendor,
		&type, &addr, &addrlen, &options, &tag) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (inet_aton(addr, &intern_addr) == 0) {
		zend_error(E_ERROR, "Error converting Address");
		RETURN_FALSE;
	}

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			RETURN_FALSE;
		}

		if (rad_put_vendor_addr_tag(raddesc->radh, vendor, type, intern_addr, tag) == -1) {
			RETURN_FALSE;
		}
	} else if (rad_put_vendor_addr(raddesc->radh, vendor, type, intern_addr) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
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

			array_init(return_value);
			add_assoc_long(return_value, "attr", res);
			add_assoc_stringl(return_value, "data", (char *) data, len, 1);
			return;
		}
		RETURN_LONG(res);
	}
}
/* }}} */

/* {{{ proto string radius_get_tagged_attr_data(string attr) */
PHP_FUNCTION(radius_get_tagged_attr_data)
{
	const char *attr;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &attr, &len) == FAILURE) {
		return;
	}

	if (len < 1) {
		zend_error(E_NOTICE, "Empty attributes cannot have tags");
		RETURN_FALSE;
	} else if (len == 1) {
		RETURN_EMPTY_STRING();
	}

	RETURN_STRINGL(attr + 1, len - 1, 1);
}
/* }}} */

/* {{{ proto string radius_get_tagged_attr_tag(string attr) */
PHP_FUNCTION(radius_get_tagged_attr_tag)
{
	const char *attr;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &attr, &len) == FAILURE) {
		return;
	}

	if (len < 1) {
		zend_error(E_NOTICE, "Empty attributes cannot have tags");
		RETURN_FALSE;
	}

	RETURN_LONG((long) *attr);
}
/* }}} */

/* {{{ proto string radius_get_vendor_attr(data) */
PHP_FUNCTION(radius_get_vendor_attr)
{
	const void *data, *raw;
	int len;
	u_int32_t vendor;
	unsigned char type;
	size_t data_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &raw, &len) == FAILURE) {
		return;
	}

	if (rad_get_vendor_attr(&vendor, &type, &data, &data_len, raw, len) == -1) {
		RETURN_FALSE;
	} else {

		array_init(return_value);
		add_assoc_long(return_value, "attr", type);
		add_assoc_long(return_value, "vendor", vendor);
		add_assoc_stringl(return_value, "data", (char *) data, data_len, 1);
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

/* {{{ proto string radius_salt_encrypt_attr(resource radh, string data) */
PHP_FUNCTION(radius_salt_encrypt_attr)
{
	char *data;
	int len;
	radius_descriptor *raddesc;
	struct salted_value salted;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &data, &len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(raddesc, radius_descriptor *, &z_radh, -1, "rad_handle", le_radius);

	if (_salt_value(raddesc->radh, data, len, &salted TSRMLS_CC) == -1) {
		RETURN_FALSE;
	} else if (salted.len == 0) {
		RETURN_EMPTY_STRING();
	}

	RETVAL_STRINGL(salted.data, salted.len, 1);
	efree(salted.data);
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

	if (secret) {
		RETURN_STRINGL(secret, strlen(secret), 1);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto string radius_demangle(radh, mangled) */
PHP_FUNCTION(radius_demangle)
{
	radius_descriptor *raddesc;
	zval *z_radh;
	const void *mangled;
	unsigned char *buf;
	int len, res;

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
		RETVAL_STRINGL((char *) buf, len, 1);
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
	size_t dlen;
	int len, res;

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
		RETVAL_STRINGL((char *) buf, dlen, 1);
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

/* {{{ _salt_value() */
int _salt_value(struct rad_handle *h, const char *in, size_t len, struct salted_value *out TSRMLS_DC)
{
	char authenticator[16];
	size_t i;
	char intermediate[16];
	const char *in_pos;
	PHP_MD5_CTX md5;
	char *out_pos;
	php_uint32 random;
	size_t salted_len;
	const char *secret;

	if (len == 0) {
		out->len = 0;
		out->data = NULL;
		return 0;
	}

	/* Calculate the padded salted value length. */
	salted_len = len;
	if ((salted_len & 0x0f) != 0) {
		salted_len += 0x0f;
		salted_len &= ~0x0f;
	}

	/* 250 because there's a five byte overhead: one byte for type, one for
	 * length, two for the salt, and one for the encrypted value length,
	 * and the maximum RADIUS attribute size is 255 bytes. */
	if (salted_len > 250) {
		zend_error(E_WARNING, "Value is too long to be salt-encrypted");
		return -1;
	}

	/* Actually allocate the buffer. */
	out->len = salted_len + 3;
	out->data = emalloc(out->len);

	if (out->data == NULL) {
		return -1;
	}

	memset(out->data, 0, out->len);

	/* Grab the request authenticator. */
	if (rad_request_authenticator(h, authenticator, sizeof authenticator) != sizeof authenticator) {
		zend_error(E_WARNING, "Cannot obtain the RADIUS request authenticator");
		goto err;
	}

	/* Grab the server secret. */
	secret = rad_server_secret(h);
	if (secret == NULL) {
		zend_error(E_WARNING, "Cannot obtain the RADIUS server secret");
		goto err;
	}

	/* Generate a random number to use as the salt. */
	random = php_rand(TSRMLS_C);

	/* The RFC requires that the high bit of the salt be 1. Otherwise,
	 * let's set up the header. */
	out->data[0] = (unsigned char) random | 0x80;
	out->data[1] = (unsigned char) (random >> 8);
	out->data[2] = (unsigned char) salted_len;

	/* OK, let's get cracking on this. We have to calculate what the RFC
	 * calls b1 first. */
	PHP_MD5Init(&md5);
	PHP_MD5Update(&md5, secret, strlen(secret));
	PHP_MD5Update(&md5, authenticator, sizeof authenticator);
	PHP_MD5Update(&md5, out->data, 2);
	PHP_MD5Final(intermediate, &md5);

	/* XOR the first chunk. */
	in_pos = in - 1;
	out_pos = out->data + 2;
	for (i = 0; i < 16; i++) {
		if (in_pos < (in + len)) {
			*(++out_pos) = *(++in_pos) ^ intermediate[i];
		} else {
			*(++out_pos) = '\0' ^ intermediate[i];
		}
	}

	/* Now walk over the rest of the input. */
	while (in_pos < (in + len)) {
		PHP_MD5Init(&md5);
		PHP_MD5Update(&md5, secret, strlen(secret));
		PHP_MD5Update(&md5, out_pos - 15, 16);
		PHP_MD5Final(intermediate, &md5);

		for (i = 0; i < 16; i++) {
			if (in_pos < (in + len)) {
				*(++out_pos) = *(++in_pos) ^ intermediate[i];
			} else {
				*(++out_pos) = '\0' ^ intermediate[i];
			}
		}
	}

	return 0;

err:
	efree(out->data);
	out->data = NULL;
	out->len = 0;

	return -1;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=8 ts=8 fdm=marker
 * vim<600: noet sw=8 ts=8
 */
