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

include_once('mschap.php');


echo "MS-CHAPv1 TEST\n";
$pass = 'MyPw';
$challenge = pack('H*', '102DB5DF085D3041');
printf ("Test Challenge: %s\n", bin2hex($challenge));
$unipw = str2unicode($pass);
printf ("Unicode PW: %s\nexpected  : 4d00790050007700\n", bin2hex($unipw));
$nthash = NtPasswordHash($pass);
printf ("NT HASH   : %s\nexpected  : fc156af7edcd6c0edde3337d427f4eac\n", bin2hex($nthash));
$challresp = ChallengeResponse($challenge, $nthash);
printf ("ChallResp : %s\nexpected  : 4e9d3c8f9cfd385d5bf4d3246791956ca4c351ab409a3d61\n", bin2hex($challresp));
echo "\n";

echo "MS-CHAPv2 TEST\n";
$user = 'User';
$pass = 'clientPass';
printf ("Username  : %s\nexpected  : 55736572\n", bin2hex($user));
$challenge = pack('H*', 'd02e4386bce91226');
printf ("Challenge     : %s\n", bin2hex($challenge));
$authchallenge = pack('H*', '5b5d7c7d7b3f2f3e3c2c602132262628');
printf ("Auth Challenge: %s\n", bin2hex($authchallenge));
$peerChallenge = pack('H*', '21402324255E262A28295F2B3A337C7E');
printf ("Peer Challenge: %s\n", bin2hex($peerChallenge));
$nthash = NtPasswordHash($pass);
printf ("NT HASH      : %s\nexpected     : 44ebba8d5312b8d611474411f56989ae\n", bin2hex($nthash));
$nthashhash = NtPasswordHashHash($nthash);
printf ("NT HASH-HASH : %s\nexpected     : 41c00c584bd2d91c4017a2a12fa59f3f\n", bin2hex($nthashhash));
$resp = GenerateNtResponse($authchallenge, $peerChallenge, $user, $pass);
printf ("ChallResp    : %s\nexpected     : 82309ecd8d708b5ea08faa3981cd83544233114a3d85d6df\n", bin2hex($resp));
echo "\n";

?>
