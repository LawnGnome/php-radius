--TEST--
radius_close()
--INI--
display_errors=1
error_reporting=-1
--FILE--
<?php
var_dump(radius_close(radius_acct_open()));
var_dump(radius_close(radius_auth_open()));
var_dump(radius_close(opendir('.')));
?>
--EXPECTF--
bool(true)
bool(true)

Warning: radius_close(): supplied resource is not a valid rad_handle resource in %s on line %d
bool(false)
