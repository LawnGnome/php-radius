<?php

/**
 * Basic, non-vendored RADIUS attributes.
 *
 * @author Adam Harvey <aharvey@php.net>
 */

namespace RADIUS\FakeServer\Attribute;

/** An attribute in a RADIUS request or response. */
class Attribute {
    /**
     * The number of bytes consumed when parsing.
     *
     * @var integer $consumed
     */
    var $consumed;

    /**
     * Whether the attribute is salted.
     *
     * @var boolean $salted
     */
    var $salted = false;

    /**
     * An optional attribute tag.
     *
     * @var integer $tag
     */
    var $tag;

    /**
     * The attribute type.
     *
     * @var integer $type
     */
    var $type;

    /**
     * The attribute value.
     *
     * @var string $value
     */
    var $value;

    /**
     * Serialises the attribute into the RADIUS wire structure.
     *
     * @return string
     */
    function serialise() {
        return pack('CC', $this->type, strlen($this->value) + 2).$this->value;
    }
}

/**
 * Compares two attributes for equality.
 *
 * @param Attribute $expected
 * @param Attribute $actual
 * @return boolean
 */
function compare($expected, $actual) {
    if (is_a($expected, 'VendorSpecificAttribute')) {
        if (!is_a($actual, 'VendorSpecificAttribute')) {
            return false;
        }

        if ($expected->salted) {
            $expectedLength = (16 * ceil(strlen($expected->value) / 16) + 3);

            if ($expected->tag) {
                $tag = unpack('Ctag', $actual->value);
                return (($expected->type == $actual->type) && (strlen($actual->value) == $expectedLength + 1) && ($expected->tag == $tag['tag']));
            } else {
                return (($expected->type == $actual->type) && (strlen($actual->value) == $expectedLength));
            }
        } else {
            return ($expected->type == $actual->type) && ($expected->value == $actual->value) && ($expected->vendorId == $actual->vendorId) && ($expected->vendorType == $actual->vendorType);
        }
        $expectedLength = (16 * ceil(strlen($expected->value) / 16) + 3);

        if ($expected->tag) {
            $tag = unpack('Ctag', $actual->value);
            return (($expected->type == $actual->type) && (strlen($actual->value) == $expectedLength + 1) && ($expected->tag == $tag['tag']));
        } else {
            return (($expected->type == $actual->type) && (strlen($actual->value) == $expectedLength));
        }
    } elseif ($expected->salted) {
        $expectedLength = (16 * ceil(strlen($expected->value) / 16) + 3);

        if ($expected->tag) {
            $tag = unpack('Ctag', $actual->value);
            return (($expected->type == $actual->type) && (strlen($actual->value) == $expectedLength + 1) && ($expected->tag == $tag['tag']));
        } else {
            return (($expected->type == $actual->type) && (strlen($actual->value) == $expectedLength));
        }
    } else {
        return ($expected->type == $actual->type) && ($expected->value == $actual->value);
    }
}

/**
 * Creates an attribute with the given type and value.
 *
 * @param integer $type
 * @param string  $value
 * @param integer $tag
 * @return Attribute
 */
function expect($type, $value, $tag = null, $salted = false) {
    $attribute = new Attribute;

    $attribute->salted = $salted;
    $attribute->type = $type;

    if (!is_null($tag)) {
        $attribute->tag = $tag;
        $attribute->value = pack('C', $tag).$value;
    } else {
        $attribute->value = $value;
    }

    return $attribute;
}

/**
 * Parses the given attribute from a RADIUS packet.
 *
 * The error checking here is minimal, to say the least. We don't even
 * check the size field is valid.
 *
 * @param string $raw
 * @return Attribute
 */
function parse($raw) {
    $attribute = new Attribute;
    $data = unpack('Ctype/Csize', $raw);

    if ($data['type'] == RADIUS_VENDOR_SPECIFIC) {
        return \RADIUS\FakeServer\VendorSpecificAttribute\parse($raw);
    }

    $attribute->consumed = $data['size'];
    $attribute->type = $data['type'];
    $attribute->value = substr($raw, 2, $data['size'] - 2);

    return $attribute;
}

// vim: set ts=4 sw=4 et:
