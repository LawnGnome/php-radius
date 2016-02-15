--TEST--
radius_cvt_int()
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
var_dump(radius_cvt_int(pack('N', 1234)));
?>
--EXPECT--
int(1234)
