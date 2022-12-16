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

#include "radlib.h"
#include "radlib_private.h"

#ifndef PHP_RADIUS_H
#define PHP_RADIUS_H

#define phpext_radius_ptr &radius_module_entry

#define PHP_RADIUS_VERSION "1.4.0b1"

#ifdef PHP_WIN32
#define PHP_RADIUS_API __declspec(dllexport)
#else
#define PHP_RADIUS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

extern zend_module_entry radius_module_entry;

PHP_MINIT_FUNCTION(radius);
PHP_MSHUTDOWN_FUNCTION(radius);
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
PHP_FUNCTION(radius_get_tagged_attr_data);
PHP_FUNCTION(radius_get_tagged_attr_tag);
PHP_FUNCTION(radius_get_vendor_attr);
PHP_FUNCTION(radius_cvt_addr);
PHP_FUNCTION(radius_cvt_int);
PHP_FUNCTION(radius_cvt_string);
PHP_FUNCTION(radius_salt_encrypt_attr);
PHP_FUNCTION(radius_request_authenticator);
PHP_FUNCTION(radius_server_secret);
PHP_FUNCTION(radius_demangle);
PHP_FUNCTION(radius_demangle_mppe_key);

#ifdef ZTS
#define RADIUS_G(v) TSRMG(radius_globals_id, zend_radius_globals *, v)
#else
#define RADIUS_G(v) (radius_globals.v)
#endif

#define RADIUS_OPTION_NONE	RAD_OPTION_NONE
#define RADIUS_OPTION_TAGGED	RAD_OPTION_TAG
#define RADIUS_OPTION_SALT	RAD_OPTION_SALT

#endif	/* PHP_RADIUS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */

/* vim: set ts=8 sw=8 noet: */
