<?php
/*
Copyright (c) 2003-2007, Michael Bretterklieber <michael@bretterklieber.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:

1. Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the distribution.
3. The names of the authors may not be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This code cannot simply be copied and put under the GNU Public License or 
any other GPL-like (LGPL, GPL2) License.

    $Id$
*/

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
foreach($functions as $func) echo $func . "<br>\n";

$username = 'sepp';
$password = 'sepp';
$radserver = 'localhost';
$radport = 1812;
$sharedsecret = 'testing123';
$auth_type = 'pap';
//$auth_type = 'chap';
//$auth_type = 'mschapv1';
//$auth_type = 'mschapv2';

$res = radius_auth_open();
echo "$res<br>\n";

//if (!radius_config($res, '/etc/radius.conf')) {
/*if (!radius_config($res, 'D:/php-devel/pear/PECL/radius/radius.conf')) {
 echo 'RadiusError:' . radius_strerror($res). "\n<br>";
 exit;
}*/


if (!radius_add_server($res, $radserver, $radport, $sharedsecret, 3, 3)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

if (!radius_add_server($res, $radserver, $radport, 'testing123', 3, 3)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

if (!radius_create_request($res, RADIUS_ACCESS_REQUEST)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

if (!radius_put_string($res, RADIUS_NAS_IDENTIFIER, isset($HTTP_HOST) ? $HTTP_HOST : 'localhost'))  {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}
 
if (!radius_put_int($res, RADIUS_SERVICE_TYPE, RADIUS_FRAMED)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}
  
if (!radius_put_int($res, RADIUS_FRAMED_PROTOCOL, RADIUS_PPP)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

if (!radius_put_string($res, RADIUS_CALLING_STATION_ID, isset($REMOTE_HOST) ? $REMOTE_HOST : '127.0.0.1') == -1) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

if (!radius_put_string($res, RADIUS_USER_NAME, $username)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

if ($auth_type == 'chap') {
    echo "CHAP<br>\n";

    /* generate Challenge */
    mt_srand(time());
    $chall = mt_rand();

    // FYI: CHAP = md5(ident + plaintextpass + challenge)
    $chapval = pack('H*', md5(pack('Ca*',1 , $password . $chall)));
//    $chapval = md5(pack('Ca*',1 , $password . $chall));
    // Radius wants the CHAP Ident in the first byte of the CHAP-Password
    $pass = pack('C', 1) . $chapval;

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

    $challenge = GenerateChallenge();
    printf ("Challenge:%s\n", bin2hex($challenge));

    if (!radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_CHAP_CHALLENGE, $challenge)) {
        echo 'RadiusError: RADIUS_MICROSOFT_MS_CHAP_CHALLENGE:' . radius_strerror($res). "<br>\n";
        exit;
    }

    $ntresp = ChallengeResponse($challenge, NtPasswordHash($password));
    $lmresp = str_repeat ("\0", 24);

    printf ("NT Response:%s\n", bin2hex($ntresp));
    // Response: chapid, flags (1 = use NT Response), LM Response, NT Response
    $resp = pack('CCa48',1 , 1, $lmresp . $ntresp);
    printf ("Response:%d %s\n", strlen($resp), bin2hex($resp));

    if (!radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_CHAP_RESPONSE, $resp)) {
        echo 'RadiusError: RADIUS_MICROSOFT_MS_CHAP_RESPONSE:' . radius_strerror($res). "<br>\n";
        exit;
    }

} else if ($auth_type == 'mschapv2') {
    echo "MS-CHAPv2<br>\n";
    include_once('mschap.php');

    $authChallenge = GenerateChallenge(16);
    printf ("Auth Challenge:%s\n", bin2hex($authChallenge));

    if (!radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_CHAP_CHALLENGE, $authChallenge)) {
        echo 'RadiusError: RADIUS_MICROSOFT_MS_CHAP_CHALLENGE:' . radius_strerror($res). "<br>\n";
        exit;
    }

    // we have no client, therefore we generate the Peer-Challenge
    $peerChallenge = GeneratePeerChallenge();
    printf ("Peer Challenge:%s\n", bin2hex($peerChallenge));

    $ntresp = GenerateNTResponse($authChallenge, $peerChallenge, $username, $password);
    $reserved = str_repeat ("\0", 8);

    printf ("NT Response:%s\n", bin2hex($ntresp));
    // Response: chapid, flags (1 = use NT Response), Peer challenge, reserved, Response
    $resp = pack('CCa16a8a24',1 , 1, $peerChallenge, $reserved, $ntresp);
    printf ("Response:%d %s\n", strlen($resp), bin2hex($resp));

    if (!radius_put_vendor_attr($res, RADIUS_VENDOR_MICROSOFT, RADIUS_MICROSOFT_MS_CHAP2_RESPONSE, $resp)) {
        echo 'RadiusError: RADIUS_MICROSOFT_MS_CHAP2_RESPONSE:' . radius_strerror($res). "<br>\n";
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
        echo "Session timeout: $time<br>\n";
        ini_set('max_execution_time', $time);
        break;

    case RADIUS_IDLE_TIMEOUT:
        $idletime = radius_cvt_int($data);
        echo "Idle timeout: $idletime<br>\n";
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

                case RADIUS_MICROSOFT_MS_CHAP2_SUCCESS:
                    $mschap2resp = radius_cvt_string($datav);
                    printf ("MS CHAPv2 success: %s<br>\n", $mschap2resp);                    
                    break;

                case RADIUS_MICROSOFT_MS_CHAP_ERROR:
                    $errormsg = radius_cvt_string(substr($datav,1));
                    echo "MS CHAP Error: $errormsg<br>\n";
                    break;

                case RADIUS_MICROSOFT_MS_CHAP_DOMAIN:
                    $domain = radius_cvt_string($datav);
                    echo "MS CHAP Domain: $domain<br>\n";
                    break;

                case RADIUS_MICROSOFT_MS_MPPE_ENCRYPTION_POLICY:
                    $policy = radius_cvt_int($datav);
                    echo "MS MPPE Policy: $policy<br>\n";
                    break;

                case RADIUS_MICROSOFT_MS_MPPE_ENCRYPTION_TYPES:
                    $type = radius_cvt_int($datav);
                    echo "MS MPPE Type: $type<br>\n";
                    break;

                case RADIUS_MICROSOFT_MS_CHAP_MPPE_KEYS:
                    $demangled = radius_demangle($res, $datav);
                    $lmkey = substr($demangled, 0, 8);
                    $ntkey = substr($demangled, 8, RADIUS_MPPE_KEY_LEN);
                    printf ("MS MPPE Keys: LM-Key: %s NT-Key: %s<br>\n", bin2hex($lmkey), bin2hex($ntkey));
                    break;

                case RADIUS_MICROSOFT_MS_MPPE_SEND_KEY:
                    $demangled = radius_demangle_mppe_key($res, $datav);
                    printf ("MS MPPE Send Key: %s<br>\n", bin2hex($demangled));
                    break;

                case RADIUS_MICROSOFT_MS_MPPE_RECV_KEY:
                    $demangled = radius_demangle_mppe_key($res, $datav);
                    printf ("MS MPPE Send Key: %s<br>\n", bin2hex($demangled));
                    break;

                case RADIUS_MICROSOFT_MS_PRIMARY_DNS_SERVER:
                    $server = radius_cvt_string($datav);
                    printf ("MS Primary DNS Server: %s<br>\n", $server);
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
