<?php

/**
 * A basic set of classes that allow a RADIUS server to be mocked.
 *
 * @author Adam Harvey <aharvey@php.net>
 */

namespace RADIUS\FakeServer;

require __DIR__.'/attribute.php';
require __DIR__.'/vsa.php';

/** A RADIUS request. */
class Request {
    /**
     * The request attributes.
     *
     * @var Attribute[] $attributes
     */
    var $attributes = array();

    /**
     * The authenticator field from the packet.
     *
     * @var string $authenticator
     */
    var $authenticator;

    /**
     * The request code.
     *
     * @var integer $code
     */
    var $code;

    /**
     * The request identifier.
     *
     * @var integer $id
     */
    var $id;

    /**
     * The raw request data.
     *
     * @var string $raw
     */
    var $raw;

    /**
     * Compares an actual request to its expected values.
     *
     * If the comparison fails, the error message is printed.
     *
     * @param Request $expected
     * @param Request $actual
     * @return boolean
     */
    static function compare($expected, $actual) {
        if ($expected->code != $actual->code) {
            printf("Expected code %d does not match actual code %d\n", $expected->code, $actual->code);
            return false;
        }

        foreach ($expected->attributes as $attribute) {
            $found = false;

            foreach ($actual->attributes as $actual_attr) {
                if (\RADIUS\FakeServer\Attribute\compare($attribute, $actual_attr)) {
                    $found = true;
                    break;
                }
            }

            if (!$found) {
                printf("Expected attribute %d with value '%s' not found in attribute set\n", $attribute->type, bin2hex($attribute->value));
                return false;
            }
        }

        return true;
    }

    /**
     * Creates a new Request with the given code and attributes.
     *
     * @param integer     $code
     * @param Attribute[] $attributes
     * @return Request
     */
    static function expect($code, $attributes = array()) {
        $request = new Request;

        $request->code = $code;
        $request->attributes = $attributes;

        return $request;
    }

    /**
     * Parses a RADIUS request packet.
     *
     * @param string $raw
     * @return Request
     */
    static function parse($raw) {
        $request = new Request;
        $data = unpack('Ccode/Cid/nsize', $raw);

        $request->raw = $raw;
        $request->code = $data['code'];
        $request->id = $data['id'];
        $request->authenticator = substr($raw, 4, 16);
        $attributes = substr($raw, 20);

        while (strlen($attributes)) {
            $attribute = \RADIUS\FakeServer\Attribute\parse($attributes);
            $attributes = substr($attributes, $attribute->consumed);
            $request->attributes[] = $attribute;
        }

        return $request;
    }
}

/** A basic RADIUS response containing arbitrary data. */
class Response {
    /**
     * The data to send.
     *
     * @var string $data
     */
    var $data;

    /**
     * Serialises the data (which in this case means do nothing).
     *
     * @param Request $request The request being responded to.
     * @param string  $secret  The shared secret.
     * @return string The RADIUS response packet.
     */
    function serialise($request, $secret) {
        return $this->data;
    }
}

/** A higher level RADIUS response packet that handles encoding and packing. */
class RadiusResponse extends Response {
    /**
     * The attributes to include.
     *
     * @var Attribute[] $attributes
     */
    var $attributes;

    /**
     * The response code.
     *
     * @var integer $code
     */
    var $code;

    /**
     * Serialises the data into a RADIUS response packet.
     *
     * @param Request $request The request being responded to.
     * @param string  $secret  The shared secret.
     * @return string The RADIUS response packet.
     */
    function serialise($request, $secret) {
        $attributes = '';
        if (is_array($this->attributes)) {
            foreach ($this->attributes as $attribute) {
                $attributes .= $attribute->serialise();
            }
        }

        $header = pack('CCn', $this->code, $request->id, 20 + strlen($attributes));
        $auth = md5($header.$request->authenticator.$attributes.$secret);
        $auth = pack('H*', $auth);
        return $header.$auth.$attributes;
    }
}

/** The fake RADIUS server. */
class FakeServer {
    /**
     * The address to listen at.
     *
     * @var string $address
     */
    var $address = '127.0.0.1';

    /**
     * The port to listen on.
     *
     * @var integer $port
     */
    var $port = 63563;

    /**
     * The RADIUS shared secret.
     *
     * @var string $secret
     */
    var $secret = 'This is a shared secret';

    /**
     * The expected transactions that will occur once the server starts
     * handing requests.
     *
     * @access private
     * @var array[] $transactions
     */
    var $transactions = array();

    /**
     * Adds an expected transaction.
     *
     * @param Request  $request  The request to expect.
     * @param Response $response The response to issue in response to the
     *                           request. If omitted or NULL, no response will
     *                           be sent.
     * @return void
     */
    function addTransaction($expectedRequest, $response = null) {
        $this->transactions[] = array(
            'request'  => clone $expectedRequest,
            'response' => clone $response,
        );
    }

    /**
     * Convenience method to create an accounting client resource for this
     * server.
     *
     * @return resource
     */
    function getAcctResource() {
        $res = radius_acct_open();
        radius_add_server($res, $this->address, $this->port, $this->secret, 5, 1);

        return $res;
    }

    /**
     * Convenience method to create an authentication client resource for this
     * server.
     *
     * @return resource
     */
    function getAuthResource() {
        $res = radius_auth_open();
        radius_add_server($res, $this->address, $this->port, $this->secret, 5, 1);

        return $res;
    }

    /**
     * Forks the server and handles requests.
     *
     * @return integer The child PID.
     */
    function handle() {
        // Hook up the signal handler for the child's signal that it's ready.
        pcntl_signal(SIGUSR1, array($this, 'signal'));

        $pid = pcntl_fork();

        if ($pid == 0) {
            $success = true;
            $socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
            socket_bind($socket, $this->address, $this->port);

            // Tell the parent we're good to go.
            posix_kill(posix_getppid(), SIGUSR1);

            while (count($this->transactions)) {
                $buffer = null;
                $from = null;
                $port = null;

                socket_recvfrom($socket, $buffer, 4096, 0, $from, $port);
                $request = Request::parse($buffer);
                $transaction = array_shift($this->transactions);

                if (Request::compare($transaction['request'], $request)) {
                    if (!is_null($transaction['response'])) {
                        $response = $transaction['response']->serialise($request, $this->secret);
                        socket_sendto($socket, $response, strlen($response), 0, $from, $port);
                    }
                } else {
                    echo "ERROR: Request did not match\n";
                    $success = false;
                    break;
                }
            }

            socket_close($socket);
            exit(!$success);
        } else {
            // Wait for SIGUSR1 from the child process to indicate it's
            // started up. If it takes more than 10 seconds, there are bigger
            // problems.
            if (sleep(10) || version_compare(phpversion(), '5.0.0', '<')) {
                // OK, the child is ready.
                return $pid;
            } else {
                echo "ERROR: Fake RADIUS server never signalled it was ready\n";
                posix_kill($pid, SIGINT);

                return null;
            }
        }
    }

    /**
     * Tests if a test involving the fake server should be skipped due to PHP
     * lacking support for a required feature.
     *
     * @return boolean
     */
    function skip() {
        return !(function_exists('socket_create') && function_exists('pcntl_fork') && function_exists('radius_acct_open'));
    }

    /**
     * The signal handler which does nothing, successfully.
     *
     * @param integer $signo
     * @return void
     */
    function signal($signo) {
    }

    /**
     * Waits for the child process to finish running and returns the exit code.
     *
     * @return integer
     */
    function wait() {
        pcntl_waitpid(-1, $status);
        return $status;
    }
}
