--TEST--
radius_cvt_string()
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
var_dump(radius_cvt_string('127.0.0.1'));
?>
--EXPECT--
string(9) "127.0.0.1"
