--TEST--
radius_get_attr()
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
));

$response = new \RADIUS\FakeServer\RadiusResponse;
$response->code = RADIUS_ACCESS_REJECT;
$response->attributes = array(
    \RADIUS\FakeServer\Attribute\expect(RADIUS_REPLY_MESSAGE, 'Go away'),
);

$server->addTransaction($request, $response);
$server->handle();

radius_create_request($res, RADIUS_ACCESS_REQUEST);
radius_put_string($res, RADIUS_USER_NAME, 'foo');
radius_put_string($res, RADIUS_USER_PASSWORD, 'bar');

var_dump(radius_get_attr($res));

radius_send_request($res);

var_dump(radius_get_attr($res));

var_dump($server->wait());
?>
--EXPECTF--
bool(false)
array(2) {
  ["attr"]=>
  int(18)
  ["data"]=>
  string(7) "Go away"
}
int(0)
