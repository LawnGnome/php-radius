--TEST--
radius_get_vendor_attr()
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
include dirname(__FILE__).'/server/fake_server.php';

var_dump(radius_get_vendor_attr(''));

// Test with an incorrect length.
$data = 'foo';
var_dump(radius_get_vendor_attr(pack('NCC', RADIUS_VENDOR_MICROSOFT, 1, strlen($data) + 3).$data));

$data = 'foo';
var_dump(radius_get_vendor_attr(pack('NCC', RADIUS_VENDOR_MICROSOFT, 1, strlen($data) + 2).$data));
?>
--EXPECT--
bool(false)
bool(false)
array(3) {
  ["attr"]=>
  int(1)
  ["vendor"]=>
  int(311)
  ["data"]=>
  string(3) "foo"
}
