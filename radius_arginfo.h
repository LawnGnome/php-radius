/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 973da7dbc73579761a1424964eb63ea6d9035bef */

ZEND_BEGIN_ARG_INFO_EX(arginfo_radius_acct_open, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_add_server, 0, 6, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, secret, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, max_tries, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_radius_auth_open arginfo_radius_acct_open

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_close, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_config, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_create_request, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_cvt_addr, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_cvt_int, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_radius_cvt_string arginfo_radius_cvt_addr

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_demangle_mppe_key, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, mangled, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_radius_demangle arginfo_radius_demangle_mppe_key

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_get_attr, 0, 1, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_INFO(0, radius_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_get_tagged_attr_data, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_get_tagged_attr_tag, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_get_vendor_attr, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_put_addr, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 1, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, tag, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_put_attr, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 1, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, tag, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_put_int, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 1, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, tag, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_radius_put_string arginfo_radius_put_attr

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_put_vendor_addr, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, vendor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_put_vendor_attr, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, vendor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 1, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, tag, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_put_vendor_int, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, vendor, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 1, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, tag, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_radius_put_vendor_string arginfo_radius_put_vendor_attr

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_request_authenticator, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, radius_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_salt_encrypt_attr, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, radius_handle)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_radius_send_request, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, radius_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_radius_server_secret, 0, 1, IS_STRING, 0)
	ZEND_ARG_INFO(0, radius_handle)
ZEND_END_ARG_INFO()

#define arginfo_radius_strerror arginfo_radius_server_secret


ZEND_FUNCTION(radius_acct_open);
ZEND_FUNCTION(radius_add_server);
ZEND_FUNCTION(radius_auth_open);
ZEND_FUNCTION(radius_close);
ZEND_FUNCTION(radius_config);
ZEND_FUNCTION(radius_create_request);
ZEND_FUNCTION(radius_cvt_addr);
ZEND_FUNCTION(radius_cvt_int);
ZEND_FUNCTION(radius_cvt_string);
ZEND_FUNCTION(radius_demangle_mppe_key);
ZEND_FUNCTION(radius_demangle);
ZEND_FUNCTION(radius_get_attr);
ZEND_FUNCTION(radius_get_tagged_attr_data);
ZEND_FUNCTION(radius_get_tagged_attr_tag);
ZEND_FUNCTION(radius_get_vendor_attr);
ZEND_FUNCTION(radius_put_addr);
ZEND_FUNCTION(radius_put_attr);
ZEND_FUNCTION(radius_put_int);
ZEND_FUNCTION(radius_put_string);
ZEND_FUNCTION(radius_put_vendor_addr);
ZEND_FUNCTION(radius_put_vendor_attr);
ZEND_FUNCTION(radius_put_vendor_int);
ZEND_FUNCTION(radius_put_vendor_string);
ZEND_FUNCTION(radius_request_authenticator);
ZEND_FUNCTION(radius_salt_encrypt_attr);
ZEND_FUNCTION(radius_send_request);
ZEND_FUNCTION(radius_server_secret);
ZEND_FUNCTION(radius_strerror);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(radius_acct_open, arginfo_radius_acct_open)
	ZEND_FE(radius_add_server, arginfo_radius_add_server)
	ZEND_FE(radius_auth_open, arginfo_radius_auth_open)
	ZEND_FE(radius_close, arginfo_radius_close)
	ZEND_FE(radius_config, arginfo_radius_config)
	ZEND_FE(radius_create_request, arginfo_radius_create_request)
	ZEND_FE(radius_cvt_addr, arginfo_radius_cvt_addr)
	ZEND_FE(radius_cvt_int, arginfo_radius_cvt_int)
	ZEND_FE(radius_cvt_string, arginfo_radius_cvt_string)
	ZEND_FE(radius_demangle_mppe_key, arginfo_radius_demangle_mppe_key)
	ZEND_FE(radius_demangle, arginfo_radius_demangle)
	ZEND_FE(radius_get_attr, arginfo_radius_get_attr)
	ZEND_FE(radius_get_tagged_attr_data, arginfo_radius_get_tagged_attr_data)
	ZEND_FE(radius_get_tagged_attr_tag, arginfo_radius_get_tagged_attr_tag)
	ZEND_FE(radius_get_vendor_attr, arginfo_radius_get_vendor_attr)
	ZEND_FE(radius_put_addr, arginfo_radius_put_addr)
	ZEND_FE(radius_put_attr, arginfo_radius_put_attr)
	ZEND_FE(radius_put_int, arginfo_radius_put_int)
	ZEND_FE(radius_put_string, arginfo_radius_put_string)
	ZEND_FE(radius_put_vendor_addr, arginfo_radius_put_vendor_addr)
	ZEND_FE(radius_put_vendor_attr, arginfo_radius_put_vendor_attr)
	ZEND_FE(radius_put_vendor_int, arginfo_radius_put_vendor_int)
	ZEND_FE(radius_put_vendor_string, arginfo_radius_put_vendor_string)
	ZEND_FE(radius_request_authenticator, arginfo_radius_request_authenticator)
	ZEND_FE(radius_salt_encrypt_attr, arginfo_radius_salt_encrypt_attr)
	ZEND_FE(radius_send_request, arginfo_radius_send_request)
	ZEND_FE(radius_server_secret, arginfo_radius_server_secret)
	ZEND_FE(radius_strerror, arginfo_radius_strerror)
	ZEND_FE_END
};
