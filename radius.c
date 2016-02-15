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

#include "pecl-compat/compat.h"

void _radius_close(zend_resource *res TSRMLS_DC);

static int _init_options(struct rad_attr_options *out, int options, int tag);

#define RADIUS_FETCH_RESOURCE(radh, zv) \
	radh = (struct rad_handle *)compat_zend_fetch_resource(zv, "rad_handle", le_radius TSRMLS_CC); \
	if (!radh) { \
		RETURN_FALSE; \
	}

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

/* {{{ proto resource radius_auth_open(string arg) */
PHP_FUNCTION(radius_auth_open)
{
	struct rad_handle *radh = rad_auth_open();

	if (radh != NULL) {
		compat_zend_register_resource(return_value, radh, le_radius TSRMLS_CC);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource radius_acct_open(string arg) */
PHP_FUNCTION(radius_acct_open)
{
	struct rad_handle *radh = rad_acct_open();

	if (radh != NULL) {
		compat_zend_register_resource(return_value, radh, le_radius TSRMLS_CC);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool radius_close(radh) */
PHP_FUNCTION(radius_close)
{
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
		return;
	}

	/* Fetch the resource to verify it. */
	RADIUS_FETCH_RESOURCE(radh, z_radh);
	compat_zend_delete_resource(z_radh TSRMLS_CC);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string radius_strerror(radh) */
PHP_FUNCTION(radius_strerror)
{
	char *msg;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);
	msg = (char *)rad_strerror(radh);
	RETURN_STRINGL(msg, strlen(msg));
}
/* }}} */

/* {{{ proto bool radius_config(desc, configfile) */
PHP_FUNCTION(radius_config)
{
	char *filename;
	COMPAT_ARG_SIZE_T filename_len;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &filename, &filename_len) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (rad_config(radh, filename) == -1) {
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
	COMPAT_ARG_SIZE_T hostname_len, secret_len;
	long  port, timeout, maxtries;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rslsll", &z_radh,
		&hostname, &hostname_len,
		&port,
		&secret, &secret_len,
		&timeout, &maxtries) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (rad_add_server(radh, hostname, port, secret, timeout, maxtries) == -1) {
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
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &z_radh, &code) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (rad_create_request(radh, code) == -1) {
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
	COMPAT_ARG_SIZE_T str_len;
	long type, options = 0, tag = 0;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls|ll", &z_radh, &type, &str, &str_len, &options, &tag)
		== FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_string(radh, type, str, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_int(desc, type, int, options, tag) */
PHP_FUNCTION(radius_put_int)
{
	long type, val, options = 0, tag = 0;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll|ll", &z_radh, &type, &val, &options, &tag)
		== FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_int(radh, type, val, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_attr(desc, type, data, options, tag) */
PHP_FUNCTION(radius_put_attr)
{
	long type, options = 0, tag = 0;
	COMPAT_ARG_SIZE_T len;
	char *data;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls|ll", &z_radh, &type, &data, &len, &options, &tag)
		== FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_attr(radh, type, data, len, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;

}
/* }}} */

/* {{{ proto bool radius_put_addr(desc, type, addr, options, tag) */
PHP_FUNCTION(radius_put_addr)
{
	COMPAT_ARG_SIZE_T addrlen;
	long type, options = 0, tag = 0;
	char	*addr;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;
	struct in_addr intern_addr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rls|ll", &z_radh, &type, &addr, &addrlen, &options, &tag)
		== FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (inet_aton(addr, &intern_addr) == 0) {
		zend_error(E_ERROR, "Error converting Address");
		RETURN_FALSE;
	}

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_addr(radh, type, intern_addr, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_string(desc, vendor, type, str, options, tag) */
PHP_FUNCTION(radius_put_vendor_string)
{
	char *str;
	COMPAT_ARG_SIZE_T str_len;
	long type, vendor, options = 0, tag = 0;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls|ll", &z_radh, &vendor, &type, &str, &str_len, &options, &tag)
		== FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_vendor_string(radh, vendor, type, str, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_int(desc, vendor, type, int, options, tag) */
PHP_FUNCTION(radius_put_vendor_int)
{
	long type, vendor, val, options = 0, tag = 0;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlll|ll", &z_radh, &vendor, &type, &val, &options, &tag)
		== FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_vendor_int(radh, vendor, type, val, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_attr(desc, vendor, type, data, options, tag) */
PHP_FUNCTION(radius_put_vendor_attr)
{
	long type, vendor, options = 0, tag = 0;
	COMPAT_ARG_SIZE_T len;
	char *data;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls|ll", &z_radh, &vendor, &type,
		&data, &len, &options, &tag) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_vendor_attr(radh, vendor, type, data, len, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_put_vendor_addr(desc, vendor, type, addr) */
PHP_FUNCTION(radius_put_vendor_addr)
{
	long type, vendor, options = 0, tag = 0;
	COMPAT_ARG_SIZE_T addrlen;
	char	*addr;
	struct rad_attr_options attr_options;
	struct rad_handle *radh;
	zval *z_radh;
	struct in_addr intern_addr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls|ll", &z_radh, &vendor,
		&type, &addr, &addrlen, &options, &tag) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (inet_aton(addr, &intern_addr) == 0) {
		zend_error(E_ERROR, "Error converting Address");
		RETURN_FALSE;
	}

	if (_init_options(&attr_options, options, tag) == -1) {
		RETURN_FALSE;
	} else if (rad_put_vendor_addr(radh, vendor, type, intern_addr, &attr_options) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool radius_send_request(desc) */
PHP_FUNCTION(radius_send_request)
{
	struct rad_handle *radh;
	zval *z_radh;
	int res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh)
		== FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	res = rad_send_request(radh);
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
	struct rad_handle *radh;
	int res;
	const void *data;
	size_t len;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	res = rad_get_attr(radh, &data, &len);
	if (res == -1) {
		RETURN_FALSE;
	} else {
		if (res > 0) {

			array_init(return_value);
			add_assoc_long(return_value, "attr", res);
			add_assoc_stringl(return_value, "data", (char *) data, len);
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
	COMPAT_ARG_SIZE_T len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &attr, &len) == FAILURE) {
		return;
	}

	if (len < 1) {
		zend_error(E_NOTICE, "Empty attributes cannot have tags");
		RETURN_FALSE;
	} else if (len == 1) {
		RETURN_EMPTY_STRING();
	}

	RETURN_STRINGL(attr + 1, len - 1);
}
/* }}} */

/* {{{ proto string radius_get_tagged_attr_tag(string attr) */
PHP_FUNCTION(radius_get_tagged_attr_tag)
{
	const char *attr;
	COMPAT_ARG_SIZE_T len;

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
	COMPAT_ARG_SIZE_T len;
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
		add_assoc_stringl(return_value, "data", (char *) data, data_len);
		return;
	}
}
/* }}} */

/* {{{ proto string radius_cvt_addr(data) */
PHP_FUNCTION(radius_cvt_addr)
{
	const void *data;
	char *addr_dot;
	COMPAT_ARG_SIZE_T len;
	struct in_addr addr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &len) == FAILURE) {
		return;
	}

	addr = rad_cvt_addr(data);
	addr_dot = inet_ntoa(addr);
	RETURN_STRINGL(addr_dot, strlen(addr_dot));
}
/* }}} */

/* {{{ proto int radius_cvt_int(data) */
PHP_FUNCTION(radius_cvt_int)
{
	const void *data;
	COMPAT_ARG_SIZE_T len;
	int val;

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
	COMPAT_ARG_SIZE_T len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &len)
		== FAILURE) {
		return;
	}

	val = rad_cvt_string(data, len);
	if (val == NULL) RETURN_FALSE;
	RETVAL_STRINGL(val, strlen(val));
	free(val);
	return;
}
/* }}} */

/* {{{ proto string radius_salt_encrypt_attr(resource radh, string data) */
PHP_FUNCTION(radius_salt_encrypt_attr)
{
	char *data;
	COMPAT_ARG_SIZE_T len;
	struct rad_handle *radh;
	struct rad_salted_value salted;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &data, &len) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	if (rad_salt_value(radh, data, len, &salted) == -1) {
		zend_error(E_WARNING, "%s", rad_strerror(radh));
		RETURN_FALSE;
	} else if (salted.len == 0) {
		RETURN_EMPTY_STRING();
	}

	RETVAL_STRINGL(salted.data, salted.len);
	efree(salted.data);
}
/* }}} */

/* {{{ proto string radius_request_authenticator(radh) */
PHP_FUNCTION(radius_request_authenticator)
{
	struct rad_handle *radh;
	ssize_t res;
	char buf[LEN_AUTH];
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	res = rad_request_authenticator(radh, buf, sizeof buf);
	if (res == -1) {
		RETURN_FALSE;
	} else {
		RETURN_STRINGL(buf, res);
	}
}
/* }}} */

/* {{{ proto string radius_server_secret(radh) */
PHP_FUNCTION(radius_server_secret)
{
	char *secret;
	struct rad_handle *radh;
	zval *z_radh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_radh) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);
	secret = (char *)rad_server_secret(radh);

	if (secret) {
		RETURN_STRINGL(secret, strlen(secret));
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto string radius_demangle(radh, mangled) */
PHP_FUNCTION(radius_demangle)
{
	struct rad_handle *radh;
	zval *z_radh;
	const void *mangled;
	unsigned char *buf;
	COMPAT_ARG_SIZE_T len;
	int res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &mangled, &len) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	buf = emalloc(len);
	res = rad_demangle(radh, mangled, len, buf);

	if (res == -1) {
		efree(buf);
		RETURN_FALSE;
	} else {
		RETVAL_STRINGL((char *) buf, len);
		efree(buf);
		return;
	}
}
/* }}} */

/* {{{ proto string radius_demangle_mppe_key(radh, mangled) */
PHP_FUNCTION(radius_demangle_mppe_key)
{
	struct rad_handle *radh;
	zval *z_radh;
	const void *mangled;
	unsigned char *buf;
	size_t dlen;
	COMPAT_ARG_SIZE_T len;
	int res;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &z_radh, &mangled, &len) == FAILURE) {
		return;
	}

	RADIUS_FETCH_RESOURCE(radh, z_radh);

	buf = emalloc(len);
	res = rad_demangle_mppe_key(radh, mangled, len, buf, &dlen);
	if (res == -1) {
		efree(buf);
		RETURN_FALSE;
	} else {
		RETVAL_STRINGL((char *) buf, dlen);
		efree(buf);
		return;
	}
}
/* }}} */

/* {{{ _init_options() */
int _init_options(struct rad_attr_options *out, int options, int tag) {
	memset(out, 0, sizeof(struct rad_attr_options));

	if (options & RADIUS_OPTION_SALT) {
		out->options |= RAD_OPTION_SALT;
	}

	if (options & RADIUS_OPTION_TAGGED) {
		if (tag < 0 || tag > 255) {
			zend_error(E_NOTICE, "Tag must be between 0 and 255");
			return -1;
		}

		out->options |= RAD_OPTION_TAG;
		out->tag = tag;
	}

	return 0;
}
/* }}} */

/* {{{ _radius_close() */
void _radius_close(zend_resource *res TSRMLS_DC)
{
	struct rad_handle *radh = (struct rad_handle *)res->ptr;
	rad_close(radh);
	res->ptr = NULL;
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
