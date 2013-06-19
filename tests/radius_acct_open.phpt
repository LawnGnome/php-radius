--TEST--
radius_acct_open()
--INI--
display_errors=1
error_reporting=-1
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
$res = radius_acct_open();
var_dump($res);
?>
--EXPECTF--
resource(%d) of type (rad_handle)
