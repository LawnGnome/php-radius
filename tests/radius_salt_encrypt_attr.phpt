--TEST--
radius_salt_encrypt_attr()
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
if (!extension_loaded('radius')) echo 'SKIP: radius extension required';
if (version_compare(PHP_VERSION, '5.0.0', '<')) echo 'SKIP: PHP 5 required';
?>
--FILE--
<?php
function salt_encrypt($value, $res, $salt) {
    if ($value == '') {
        return '';
    }

    $block = md5(radius_server_secret($res).radius_request_authenticator($res).substr($salt, 0, 2), true);
    $output = '';

    for ($i = 0; $i < 16; $i++) {
        if (strlen($value)) {
            $output .= chr(ord($value[0]) ^ ord($block[$i]));
            $value = substr($value, 1);
        } else {
            $output .= chr(0 ^ ord($block[$i]));
        }
    }

    while (strlen($value)) {
        $block = md5(radius_server_secret($res).substr($output, -16), true);

        for ($i = 0; $i < 16; $i++) {
            if (strlen($value)) {
                $output .= chr(ord($value[0]) ^ ord($block[$i]));
                $value = substr($value, 1);
            } else {
                $output .= chr(0 ^ ord($block[$i]));
            }
        }
    }

    if (strlen($output) % 16 != 0) {
        $output = str_pad($output, 16 * ceil(strlen($output) / 16), "\0");
    }

    return substr($salt, 0, 2).chr(strlen($output)).$output;
}

$res = radius_acct_open();

var_dump(radius_salt_encrypt_attr($res, ''));
var_dump(radius_salt_encrypt_attr($res, 'foo'));

radius_add_server($res, 'localhost', 1234, 'a shared secret', 5, 3);

var_dump(radius_salt_encrypt_attr($res, ''));

$input = 'foo';
$salted = radius_salt_encrypt_attr($res, $input);
var_dump(salt_encrypt($input, $res, $salted) == $salted);

$input = implode('', range('a', 'z'));
$salted = radius_salt_encrypt_attr($res, $input);
var_dump(salt_encrypt($input, $res, $salted) == $salted);
?>
--EXPECTF--
string(0) ""

Warning: Cannot obtain the RADIUS server secret in %s on line %f
bool(false)
string(0) ""
bool(true)
bool(true)
