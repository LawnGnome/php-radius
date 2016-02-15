--TEST--
radius_cvt_addr()
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
var_dump(radius_cvt_addr(pack('N', ip2long('127.0.0.1'))));
?>
--EXPECT--
string(9) "127.0.0.1"
