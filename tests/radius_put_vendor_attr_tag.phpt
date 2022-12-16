--TEST--
radius_put_vendor_attr() with tag
--INI--
display_errors=1
error_reporting=22527
--SKIPIF--
<?php
include dirname(__FILE__).'/server/fake_server.php';

if (\RADIUS\FakeServer\FakeServer::skip()) {
    die('SKIP: pcntl, radius and sockets extensions required');
}
?>
--FILE--
<?php
include dirname(__FILE__).'/server/fake_server.php';

$server = new \RADIUS\FakeServer\FakeServer;
$res = $server->getAuthResource();

$request = \RADIUS\FakeServer\Request::expect(RADIUS_ACCESS_REQUEST, array(
    \RADIUS\FakeServer\VendorSpecificAttribute\expect(RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VENDOR, 'vendor', 10),
    \RADIUS\FakeServer\VendorSpecificAttribute\expect(RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_PRIMARY_DNS_SERVER, 'vendor', 10, true),
));

$response = new \RADIUS\FakeServer\RadiusResponse;
$response->code = RADIUS_ACCESS_REJECT;
$response->attributes = array(
    \RADIUS\FakeServer\Attribute\expect(RADIUS_REPLY_MESSAGE, 'Go away'),
);

$server->addTransaction($request, $response);
$server->handle();

var_dump(radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VENDOR, 'vendor', RADIUS_OPTION_TAGGED, -1));
var_dump(radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VENDOR, 'vendor', RADIUS_OPTION_TAGGED, 256));
var_dump(radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VENDOR, 'vendor', RADIUS_OPTION_TAGGED, 10));
var_dump(radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_PRIMARY_DNS_SERVER, 'foo', RADIUS_OPTION_SALT|RADIUS_OPTION_TAGGED, 10));

radius_create_request($res, RADIUS_ACCESS_REQUEST);
var_dump(radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_RAS_VENDOR, 'vendor', RADIUS_OPTION_TAGGED, 10));
var_dump(radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_PRIMARY_DNS_SERVER, 'foo', RADIUS_OPTION_SALT|RADIUS_OPTION_TAGGED, 10));
radius_put_string($res, RADIUS_USER_PASSWORD, 'bar');
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
