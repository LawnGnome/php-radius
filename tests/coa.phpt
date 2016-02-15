--TEST--
RADIUS: RFC 3576 CoA message support
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
$response->code = RADIUS_COA_REQUEST;
$response->attributes = array(
    \RADIUS\FakeServer\Attribute\expect(RADIUS_FILTER_ID, 'filter'),
);

$server->addTransaction($request, $response);

$request = \RADIUS\FakeServer\Request::expect(RADIUS_COA_NAK, array(
    \RADIUS\FakeServer\Attribute\expect(RADIUS_ERROR_CAUSE, pack('N', RADIUS_ERROR_CAUSE_MISSING_ATTRIBUTE)),
));

$server->addTransaction($request, $response);

$request = \RADIUS\FakeServer\Request::expect(RADIUS_COA_ACK, array(
));

$response = new \RADIUS\FakeServer\RadiusResponse;
$response->code = RADIUS_COA_ACK;

$server->addTransaction($request, $response);

$server->handle();

radius_create_request($res, RADIUS_ACCESS_REQUEST);
radius_put_string($res, RADIUS_USER_NAME, 'foo');
radius_put_string($res, RADIUS_USER_PASSWORD, 'bar');

var_dump(radius_send_request($res) == RADIUS_COA_REQUEST);
var_dump(radius_get_attr($res));

radius_create_request($res, RADIUS_COA_NAK);
radius_put_int($res, RADIUS_ERROR_CAUSE, RADIUS_ERROR_CAUSE_MISSING_ATTRIBUTE);

var_dump(radius_send_request($res) == RADIUS_COA_REQUEST);
var_dump(radius_get_attr($res));

radius_create_request($res, RADIUS_COA_ACK);
radius_send_request($res);

var_dump($server->wait());
?>
--EXPECTF--
bool(true)
array(2) {
  ["attr"]=>
  int(11)
  ["data"]=>
  string(6) "filter"
}
bool(true)
array(2) {
  ["attr"]=>
  int(11)
  ["data"]=>
  string(6) "filter"
}
int(0)
