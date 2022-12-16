--TEST--
radius_put_addr()
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
    \RADIUS\FakeServer\Attribute\expect(RADIUS_USER_NAME, 'foo'),
    \RADIUS\FakeServer\Attribute\expect(RADIUS_NAS_IP_ADDRESS, pack('N', ip2long('127.0.0.1'))),
    \RADIUS\FakeServer\Attribute\expect(RADIUS_LOGIN_IP_HOST, pack('N', ip2long('0.0.0.0')), null, true),
));

$response = new \RADIUS\FakeServer\RadiusResponse;
$response->code = RADIUS_ACCESS_REJECT;
$response->attributes = array(
    \RADIUS\FakeServer\Attribute\expect(RADIUS_REPLY_MESSAGE, 'Go away'),
);

$server->addTransaction($request, $response);
$server->handle();

var_dump(radius_put_addr($res, RADIUS_NAS_IP_ADDRESS, '127.0.0.1'));
var_dump(radius_put_addr($res, RADIUS_LOGIN_IP_HOST, '0.0.0.0', RADIUS_OPTION_SALT));

radius_create_request($res, RADIUS_ACCESS_REQUEST);
radius_put_string($res, RADIUS_USER_NAME, 'foo');
radius_put_string($res, RADIUS_USER_PASSWORD, 'bar');
var_dump(radius_put_addr($res, RADIUS_NAS_IP_ADDRESS, '127.0.0.1'));
var_dump(radius_put_addr($res, RADIUS_LOGIN_IP_HOST, '0.0.0.0', RADIUS_OPTION_SALT));
radius_send_request($res);

var_dump($server->wait());
?>
--EXPECTF--
bool(false)
bool(false)
bool(true)
bool(true)
int(0)
