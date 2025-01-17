<?php

/** @generate-function-entries */

/**
 * @return resource|false
 */
function radius_acct_open ( ) {}

/**
 * @param resource $radius_handle
 */
function radius_add_server ( $radius_handle , string $hostname , int $port , string $secret , int $timeout , int $max_tries ) : bool {}

/**
 * @return resource|false
 */
function radius_auth_open ( ) {}

/**
 * @param resource $radius_handle
 */
function radius_close ( $radius_handle ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_config ( $radius_handle , string $file ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_create_request ( $radius_handle , int $type ) : bool {}

function radius_cvt_addr ( string $data ) : string {}

function radius_cvt_int ( string $data ) : int {}

function radius_cvt_string ( string $data ) : string {}

/**
 * @param resource $radius_handle
 */
function radius_demangle_mppe_key ( $radius_handle , string $mangled ) : string|false {}

/**
 * @param resource $radius_handle
 */
function radius_demangle ( $radius_handle , string $mangled ) : string|false {}

/**
 * @param resource $radius_handle
 */
function radius_get_attr ( $radius_handle ) : array|int {}

function radius_get_tagged_attr_data ( string $data ) : string|false {}

function radius_get_tagged_attr_tag ( string $data ) : int|false {}

function radius_get_vendor_attr ( string $data ) : array|false {}

/**
 * @param resource $radius_handle
 */
function radius_put_addr ( $radius_handle , int $type , string $address , ?int $options = 0 , int $tag = 0 ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_put_attr ( $radius_handle , int $type , string $value , ?int $options = 0 , int $tag = 0 ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_put_int ( $radius_handle , int $type , int $value , ?int $options = 0 , int $tag = 0 ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_put_string ( $radius_handle , int $type , string $value , ?int $options = 0 , int $tag = 0 ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_put_vendor_addr ( $radius_handle , int $vendor , int $type , string $address ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_put_vendor_attr ( $radius_handle , int $vendor , int $type , string $value , ?int $options = 0 , int $tag = 0 ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_put_vendor_int ( $radius_handle , int $vendor , int $type , int $value , ?int $options = 0 , int $tag = 0 ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_put_vendor_string ( $radius_handle , int $vendor , int $type , string $value , ?int $options = 0 , int $tag = 0 ) : bool {}

/**
 * @param resource $radius_handle
 */
function radius_request_authenticator ( $radius_handle ) : string|false {}

/**
 * @param resource $radius_handle
 */
function radius_salt_encrypt_attr ( $radius_handle , string $data ) : string|false {}

/**
 * @param resource $radius_handle
 */
function radius_send_request ( $radius_handle ) : int|false {}

/**
 * @param resource $radius_handle
 */
function radius_server_secret ( $radius_handle ) : string {}

/**
 * @param resource $radius_handle
 */
function radius_strerror ( $radius_handle ) : string {}

