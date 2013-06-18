--TEST--
radius_auth_open()
--INI--
display_errors=1
error_reporting=-1
--FILE--
<?php
$res = radius_auth_open();
var_dump($res);
?>
--EXPECTF--
resource(%d) of type (rad_handle)
