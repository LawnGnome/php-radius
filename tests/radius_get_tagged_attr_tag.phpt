--TEST--
radius_get_tagged_attr_tag()
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
?>
--FILE--
<?php
var_dump(radius_get_tagged_attr_tag(''));
var_dump(radius_get_tagged_attr_tag(pack('C', 20)));
var_dump(radius_get_tagged_attr_tag(pack('C', 20).'foo'));
?>
--EXPECTF--
Notice: Empty attributes cannot have tags in %s on line %d
bool(false)
int(20)
int(20)
