<?php
/*
Copyright (c) 2003, Michael Bretterklieber <michael@bretterklieber.com>
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

$username = 'sepp';
$radserver = 'localhost';
$radport = 1813;
$starttime = time();
$sharedsecret = 'testing123';

if (!isset($REMOTE_ADDR)) $REMOTE_ADDR = '127.0.0.1';

$res = radius_acct_open();
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

if (!radius_create_request($res, RADIUS_ACCOUNTING_REQUEST)) {
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

if (!radius_put_addr($res, RADIUS_FRAMED_IP_ADDRESS, $REMOTE_ADDR)) {
    echo 'RadiusError1:' . radius_strerror($res). "\n<br>";
    exit;
}

// RADIUS_START => start accounting
// RADIUS_STOP => stop accounting
if (!radius_put_int($res, RADIUS_ACCT_STATUS_TYPE, RADIUS_START)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

/* Generate a session ID */
$sessionid = sprintf("%s:%d-%s", $REMOTE_ADDR, getmypid(), get_current_user());
if (!radius_put_string($res, RADIUS_ACCT_SESSION_ID, $sessionid)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

// RADIUS_AUTH_RADIUS => authenticated via Radius
// RADIUS_AUTH_LOCAL => authenicated local
// RADIUS_AUTH_REMOTE => authenticated remote
if (!radius_put_int($res, RADIUS_ACCT_AUTHENTIC, RADIUS_AUTH_LOCAL)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

sleep(3);
// if RADIUS_ACCT_STATUS_TYPE == RADIUS_STOP 
if (!radius_put_int($res, RADIUS_ACCT_TERMINATE_CAUSE, RADIUS_TERM_USER_REQUEST)) {
    echo 'RadiusError2:' . radius_strerror($res). "\n<br>";
    exit;
}

if (!radius_put_int($res, RADIUS_ACCT_SESSION_TIME, time() - $starttime)) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}
// endif

$req = radius_send_request($res);
if (!$req) {
    echo 'RadiusError:' . radius_strerror($res). "\n<br>";
    exit;
}

switch($req) {

case RADIUS_ACCOUNTING_RESPONSE:
    echo "Radius Accounting response<br>\n";    
    break;

default:
    echo "Unexpected return value:$req\n<br>";
    
}


radius_close($res);

?>
