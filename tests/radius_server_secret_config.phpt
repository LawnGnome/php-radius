--TEST--
radius_server_secret(): load from configuration file
--INI--
display_errors=1
error_reporting=-1
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
$file = dirname(__FILE__).'/radius_server_secret.conf';
$fp = fopen($file, 'w');
fwrite($fp, 'acct localhost "a shared secret"'."\n");
fclose($fp);

$res = radius_acct_open();
radius_config($res, $file);
var_dump(radius_server_secret($res));
radius_close($res);

$file = dirname(__FILE__).'/radius_server_secret.conf';
$fp = fopen($file, 'w');
fwrite($fp, 'acct what.is.hopefully.a.nonexistent.domain "a shared secret"');
fclose($fp);

// This shouldn't create a server, but crashed before 8f1de6f.
$res = radius_acct_open();
radius_config($res, $file);
var_dump(radius_server_secret($res));
radius_close($res);
?>
--CLEAN--
<?php
$file = dirname(__FILE__).'/radius_server_secret.conf';
@unlink($file);
?>
--EXPECTF--
string(15) "a shared secret"
bool(false)
