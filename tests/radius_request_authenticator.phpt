--TEST--
radius_request_authenticator()
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
$res = radius_acct_open();
var_dump(radius_request_authenticator($res));
?>
--EXPECTF--
string(16) "%s"
