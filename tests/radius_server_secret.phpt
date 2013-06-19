--TEST--
radius_server_secret(): load from radius_add_server()
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
radius_add_server($res, 'localhost', 1234, 'a shared secret', 5, 3);
var_dump(radius_server_secret($res));
radius_close($res);

// This shouldn't create a server, but crashed before 8f1de6f.
$res = radius_acct_open();
radius_add_server($res, 'what.is.hopefully.a.nonexistent.domain', 1234, 'a shared secret', 5, 3);
var_dump(radius_server_secret($res));
var_dump(radius_strerror($res));
radius_close($res);

$res = radius_acct_open();
var_dump(radius_server_secret($res));
radius_close($res);
?>
--EXPECTF--
string(15) "a shared secret"
bool(false)
string(27) "No RADIUS servers specified"
bool(false)
