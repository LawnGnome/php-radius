<?
if(!extension_loaded('radius')) {
	if (preg_match('/windows/i', getenv('OS'))) {
		dl('php_radius.dll');
	} else {
		dl('radius.so');
	}
}

$module = 'radius';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:<br>\n";
foreach($functions as $func) {
	echo $func . "<br>\n";
}

//$auth_type = 'chap';
//$auth_type = 'pap';
$auth_type = 'mschapv1';

$res = radius_auth_open();
echo "$res<br>\n";

//if (!radius_config($res, '/etc/radius.conf')) {
/*if (!radius_config($res, 'D:/php-devel/pear/PECL/radius/radius.conf')) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}*/


if (!radius_add_server($res, 'carlo.jawa.at', 1812, 'testing123', 3, 3)) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}

/*if (!radius_add_server($res, 'localhost', 1812, 'testing123', 3, 3)) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
} */

if (!radius_add_server($res, '192.168.201.12', 1812, 'testing123', 3, 3)) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}

if (!radius_create_request($res, RADIUS_ACCESS_REQUEST)) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}

if (!radius_put_string($res, RADIUS_USER_NAME, "sepp")) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}

if ($auth_type == 'chap') {
	echo "CHAP<br>\n";

	/* generate Challenge */
	mt_srand(time());
	$chall = mt_rand();

	// FYI: CHAP = md5(ident + plaintextpass + challenge)
	$chapval = md5(pack('Ca*',1 , 'sepp' . $chall));
	// Radius wants the CHAP Ident in the first byte of the CHAP-Password
	$pass = pack('CH*', 1, $chapval);

	if (!radius_put_attr($res, RADIUS_CHAP_PASSWORD, $pass)) {
		echo 'RadiusError: RADIUS_CHAP_PASSWORD:' . radius_strerror($res). "<br>\n";
		exit;
	}

	if (!radius_put_attr($res, RADIUS_CHAP_CHALLENGE, $chall)) {
		echo 'RadiusError: RADIUS_CHAP_CHALLENGE:' . radius_strerror($res). "<br>\n";
		exit;
	}

}  else if ($auth_type == 'mschapv1') {
	echo "MS-CHAPv1<br>\n";
    include_once('mschap.php');
    
    $chall = GenerateChallenge();
    printf ("Challenge:%s\n", bin2hex($chall));

	if (!radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_CHAP_CHALLENGE, $chall)) {
		echo 'RadiusError: RADIUS_MICROSOFT_MS_CHAP_CHALLENGE:' . radius_strerror($res). "<br>\n";
		exit;
	}
    
    $ntresp = ChallengeResponse(NtPasswordHash('sepp'), $chall);
    $lmresp = str_repeat ("\0", 24);

    printf ("NT Response:%s\n", bin2hex($ntresp));
    $resp = pack('CCa48',1 , 1, $lmresp . $ntresp);
    printf ("Response:%d %s\n", strlen($resp), bin2hex($resp));    
    
/*      mschapres.ident = chapid;
      mschapres.flags = 0x01;
      memcpy(mschapres.lm_response, mschapv->lmHash, 24);
      memcpy(mschapres.nt_response, mschapv->ntHash, 24);
  */    
	if (!radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_CHAP_RESPONSE, $resp)) {
		echo 'RadiusError: RADIUS_MICROSOFT_MS_CHAP_RESPONSE:' . radius_strerror($res). "<br>\n";
		exit;
	}
    
        
} else {
	echo "PAP<br>\n";

	if (!radius_put_string($res, RADIUS_USER_PASSWORD, "sepp")) {
		echo 'RadiusError:' . radius_strerror($res). "<br>\n";
		exit;
	}
}

if (!radius_put_int($res, RADIUS_SERVICE_TYPE, RADIUS_FRAMED)) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}

if (!radius_put_int($res, RADIUS_FRAMED_PROTOCOL, RADIUS_PPP)) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}

$req = radius_send_request($res);
if (!$req) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
}

switch($req) {
	case RADIUS_ACCESS_ACCEPT:
		echo "Radius Request accepted<br>\n";
		break;

	case RADIUS_ACCESS_REJECT:
		echo "Radius Request rejected<br>\n";
		break;

	default:
		echo "Unexpected return value:$req\n<br>";
}

while ($resa = radius_get_attr($res)) {

	if (!is_array($resa)) {
		printf ("Error getting attribute: %s\n",  radius_strerror($res));
		exit;
	}

	$attr = $resa['attr'];
	$data = $resa['data'];
	//printf("Got Attr:%d %d Bytes %s\n", $attr, strlen($data), bin2hex($data));

	switch ($attr) {

		case RADIUS_FRAMED_IP_ADDRESS:
			$ip = radius_cvt_addr($data);
			echo "IP: $ip<br>\n";
			break;

		case RADIUS_FRAMED_IP_NETMASK:
			$mask = radius_cvt_addr($data);
			echo "MASK: $mask<br>\n";
			break;

		case RADIUS_FRAMED_MTU:
			$mtu = radius_cvt_int($data);
			echo "MTU: $mtu<br>\n";
			break;

		case RADIUS_FRAMED_COMPRESSION:
			$comp = radius_cvt_int($data);
			echo "Compression: $comp<br>\n";
			break;

		case RADIUS_SESSION_TIMEOUT:
			$time = radius_cvt_int($data);
			echo "Compression: $time<br>\n";
			break;

		case RADIUS_SERVICE_TYPE:
			$type = radius_cvt_int($data);
			echo "Service Type: $type<br>\n";
			break;

		case RADIUS_CLASS:
			$class = radius_cvt_int($data);
			echo "Class: $class<br>\n";
			break;

		case RADIUS_FRAMED_PROTOCOL:
			$proto = radius_cvt_int($data);
			echo "Protocol: $proto<br>\n";
			break;

		case RADIUS_FRAMED_ROUTING:
			$rout = radius_cvt_int($data);
			echo "Routing: $rout<br>\n";
			break;

		case RADIUS_FILTER_ID:
			$id = radius_cvt_string($data);
			echo "Filter ID: $id<br>\n";
			break;

		case RADIUS_VENDOR_SPECIFIC:
			//printf ("Vendor specific (%d)<br>\n", $attr);

			$resv = radius_get_vendor_attr($data);
			if (is_array($resv)) {
				$vendor = $resv['vendor'];
				$attrv = $resv['attr'];
				$datav = $resv['data'];

				if ($vendor == RADIUS_VENDOR_MICROSOFT) {

					switch ($attrv) {

						case RAD_MICROSOFT_MS_CHAP_ERROR:
							echo "MS CHAP Error<br>\n";
							break;

						case RADIUS_MICROSOFT_MS_MPPE_ENCRYPTION_POLICY:
							$policy = radius_cvt_int($datav);
							echo "MS MPPE Policy: $policy<br>\n";
							break;

						case RADIUS_MICROSOFT_MS_MPPE_ENCRYPTION_TYPES:
							$type = radius_cvt_int($datav);
							echo "MS MPPE Type: $type<br>\n";
							break;

						default:
							printf("Unexpected Microsoft attribute: %d<br>\n", $attrv);
					}

				}

			} else {
				printf ("Error getting Vendor attribute %s<br>\n", radius_strerror($res));
			}
			break;

		default:
			printf("Unexpected attribute: %d<br>\n", $attr);
	}
}

$secret = radius_server_secret($res);
if (!$secret) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
} else {
	echo "Shared Secret:$secret<br>\n";
}

$authent = radius_request_authenticator($res);
if (!$authent) {
	echo 'RadiusError:' . radius_strerror($res). "\n<br>";
	exit;
} else {
	printf ("Request Authenticator:%s Len:%d<br>\n", bin2hex($authent), strlen($authent));
}

radius_close($res);

?>
