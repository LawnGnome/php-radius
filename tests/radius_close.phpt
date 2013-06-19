--TEST--
radius_close()
--INI--
display_errors=1
error_reporting=-1
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
var_dump(radius_close(radius_acct_open()));
var_dump(radius_close(radius_auth_open()));

// The boolean cast is because PHP 4 will return NULL rather than false here.
var_dump((boolean) radius_close(opendir('.')));
?>
--EXPECTF--
bool(true)
bool(true)

Warning: radius_close(): supplied resource is not a valid rad_handle resource in %s on line %d
bool(false)
