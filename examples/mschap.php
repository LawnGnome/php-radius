<?php
/*
Copyright (c) 2002, Michael Bretterklieber <michael@bretterklieber.com>
All rights reserved.
 
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:
 
1. Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the distribution.
3. Neither the name Michael Bretterklieber nor the names of its contributors 
   may be used to endorse or promote products derived from this software without 
   specific prior written permission.
 
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
 
    $Id$
*/

/*
$pass = 'MyPw';
$challenge = GenerateChallenge();
$challenge = pack('H*', '102DB5DF085D3041');
echo bin2hex($challenge). "\n";
$unipw = str2unicode($pass);
echo bin2hex($unipw) . "\n";
$nthash = NtPasswordHash($pass);
echo bin2hex($nthash) . "\n";
$challresp = ChallengeResponse($nthash, $challenge);
echo bin2hex($challresp) . "\n";
echo "\n";*/

/*
$user = 'User';
$pass = 'clientPass';
 
echo bin2hex($user) . "\n";
//$challenge = GenerateChallenge();
$challenge = pack('H*', '5b5d7c7d7b3f2f3e3c2c602132262628');
echo bin2hex($challenge). "\n";
$peerChallenge = GenerateChallenge();
$peerChallenge = pack('H*', '21402324255E262A28295F2B3A337C7E');
echo bin2hex($peerChallenge). "\n";
$unipw = str2unicode($pass);
echo bin2hex($unipw) . "\n";
$nthash = NtPasswordHash($pass);
echo bin2hex($nthash) . "\n";
$resp = GenerateNtResponse($peerChallenge, $challenge, $user, $pass);
echo bin2hex($resp) . "\n";
echo "\n";
*/

function NtPasswordHash($plain) 
{
    return mhash (MHASH_MD4, str2unicode($plain));
}

function str2unicode($str) 
{

    for ($i=0;$i<strlen($str);$i++) {
        $a = ord($str{$i}) << 8;
        $uni .= sprintf("%X",$a);
    }
    return pack('H*', $uni);
}

function GenerateChallenge() 
{
    mt_srand((double)microtime()*1000000);

    return pack('H*', sprintf("%X%X", mt_rand(), mt_rand()));
}

function ChallengeResponse($challenge, $nthash) 
{
    global $odd_parity;

    while (strlen($nthash) < 21)
        $nthash .= "\0";
    $td = mcrypt_module_open ('des', '', 'ecb', '');
    $iv = mcrypt_create_iv (mcrypt_enc_get_iv_size ($td), MCRYPT_RAND);

    $k = DESAddParity(substr($nthash, 0, 7));
    mcrypt_generic_init ($td, $k, $iv);
    $resp1 = mcrypt_generic ($td, $challenge);
    mcrypt_generic_deinit ($td);

    $k = DESAddParity(substr($nthash, 7, 7));
    mcrypt_generic_init ($td, $k, $iv);
    $resp2 = mcrypt_generic ($td, $challenge);
    mcrypt_generic_deinit ($td);

    $k = DESAddParity(substr($nthash, 14, 7));
    mcrypt_generic_init ($td, $k, $iv);
    $resp3 = mcrypt_generic ($td, $challenge);
    mcrypt_generic_deinit ($td);

    mcrypt_module_close ($td);

    return $resp1 . $resp2 . $resp3;
}

// MS-CHAPv2

function GeneratePeerChallenge() 
{
    mt_srand((double)microtime()*1000000);

    return pack('H*', sprintf("%X%X%X%X", mt_rand(), mt_rand(), mt_rand(), mt_rand()));
}

function NtPasswordHashHash($hash) 
{
    return mhash (MHASH_MD4, $hash);
}

function ChallengeHash($challenge, $peerChallenge, $username) 
{
    return substr(mhash (MHASH_SHA1, $peerChallenge . $challenge . $username), 0, 8);
}

function GenerateNTResponse($challenge, $peerChallenge, $username, $password) 
{
    $challengeHash = ChallengeHash($challenge, $peerChallenge, $username);
    $pwhash = NtPasswordHash($password);
    return ChallengeResponse($challengeHash, $pwhash);
}

// DES helper function
function DESAddParity($key) 
{

    static $odd_parity = array(
        1,  1,  2,  2,  4,  4,  7,  7,  8,  8, 11, 11, 13, 13, 14, 14,
        16, 16, 19, 19, 21, 21, 22, 22, 25, 25, 26, 26, 28, 28, 31, 31,
        32, 32, 35, 35, 37, 37, 38, 38, 41, 41, 42, 42, 44, 44, 47, 47,
        49, 49, 50, 50, 52, 52, 55, 55, 56, 56, 59, 59, 61, 61, 62, 62,
        64, 64, 67, 67, 69, 69, 70, 70, 73, 73, 74, 74, 76, 76, 79, 79,
        81, 81, 82, 82, 84, 84, 87, 87, 88, 88, 91, 91, 93, 93, 94, 94,
        97, 97, 98, 98,100,100,103,103,104,104,107,107,109,109,110,110,
        112,112,115,115,117,117,118,118,121,121,122,122,124,124,127,127,
        128,128,131,131,133,133,134,134,137,137,138,138,140,140,143,143,
        145,145,146,146,148,148,151,151,152,152,155,155,157,157,158,158,
        161,161,162,162,164,164,167,167,168,168,171,171,173,173,174,174,
        176,176,179,179,181,181,182,182,185,185,186,186,188,188,191,191,
        193,193,194,194,196,196,199,199,200,200,203,203,205,205,206,206,
        208,208,211,211,213,213,214,214,217,217,218,218,220,220,223,223,
        224,224,227,227,229,229,230,230,233,233,234,234,236,236,239,239,
        241,241,242,242,244,244,247,247,248,248,251,251,253,253,254,254);

    for ($i = 0; $i < strlen($key); $i++) {
        $bin .= sprintf('%08s', decbin(ord($key{$i})));
    }

    $str1 = explode('-', substr(chunk_split($bin, 7, '-'), 0, -1));
    foreach($str1 as $s) {
        $x .= sprintf('%02s', dechex($odd_parity[bindec($s . '0')]));
    }

    return pack('H*', $x);

}
?>
