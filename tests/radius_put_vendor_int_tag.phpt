--TEST--
radius_put_vendor_int() with tag
--INI--
display_errors=1
error_reporting=30719
--SKIPIF--
<?php
include dirname(__FILE__).'/server/fake_server.php';

if (FakeServer::skip()) {
    die('SKIP: pcntl, radius and sockets extensions required');
}
?>
--FILE--
<?php
include dirname(__FILE__).'/server/fake_server.php';

$server = new FakeServer;
$res = $server->getAuthResource();

$request = Request::expect(RADIUS_ACCESS_REQUEST, array(
    Attribute::expect(RADIUS_USER_NAME, 'foo'),
    VendorSpecificAttribute::expect(RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, pack('N', 1234), 10),
    VendorSpecificAttribute::expect(RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, pack('N', 1234), 10, true),
));

$response = new RadiusResponse;
$response->code = RADIUS_ACCESS_REJECT;
$response->attributes = array(
    Attribute::expect(RADIUS_REPLY_MESSAGE, 'Go away'),
);

$server->addTransaction($request, $response);
$server->handle();

var_dump(radius_put_vendor_int($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, 1234, RADIUS_OPTION_TAGGED, -1));
var_dump(radius_put_vendor_int($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, 1234, RADIUS_OPTION_TAGGED, 256));
var_dump(radius_put_vendor_int($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, 1234, RADIUS_OPTION_TAGGED, 10));
var_dump(radius_put_vendor_int($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, 1234, RADIUS_OPTION_SALT|RADIUS_OPTION_TAGGED, 10));

radius_create_request($res, RADIUS_ACCESS_REQUEST);
radius_put_string($res, RADIUS_USER_NAME, 'foo');
radius_put_string($res, RADIUS_USER_PASSWORD, 'bar');
var_dump(radius_put_vendor_int($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, 1234, RADIUS_OPTION_TAGGED, 10));
var_dump(radius_put_vendor_int($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VERSION, 1234, RADIUS_OPTION_SALT|RADIUS_OPTION_TAGGED, 10));
radius_send_request($res);

var_dump($server->wait());
?>
--EXPECTF--
Notice: Tag must be between 0 and 255 in %s on line %d
bool(false)

Notice: Tag must be between 0 and 255 in %s on line %d
bool(false)
bool(false)
bool(false)
bool(true)
bool(true)
int(0)
