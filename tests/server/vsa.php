<?php

/**
 * Vendored RADIUS attributes.
 *
 * @author Adam Harvey <aharvey@php.net>
 */

namespace RADIUS\FakeServer\VendorSpecificAttribute;

use RADIUS\FakeServer\Attribute\Attribute;

/** A vendor specific attribute. */
class VendorSpecificAttribute extends Attribute {
    /**
     * The vendor ID.
     *
     * @var integer $vendorId
     */
    var $vendorId;

    /**
     * The vendor type.
     *
     * @var integer $vendorType
     */
    var $vendorType;

    /** {@inheritDoc} */
    function serialise() {
        return pack('CCNCC', $this->type, strlen($this->value) + 8, $this->vendorId, $this->vendorType, strlen($this->value) + 2);
    }
}

/**
 * Creates a vendor specific attribute with the given ID, type and value.
 *
 * @param integer $vendorId
 * @param integer $vendorType
 * @param string  $value
 * @param integer $tag
 * @return VendorSpecificAttribute
 */
function expect($vendorId, $vendorType, $value, $tag = null, $salted = false) {
    $attribute = new VendorSpecificAttribute;

    $attribute->salted = $salted;
    $attribute->type = RADIUS_VENDOR_SPECIFIC;
    $attribute->vendorId = $vendorId;
    $attribute->vendorType = $vendorType;

    if (!is_null($tag)) {
        $attribute->tag = $tag;
        $attribute->value = pack('C', $tag).$value;
    } else {
        $attribute->value = $value;
    }

    return $attribute;
}

/** {@inheritDoc} */
function parse($raw) {
    $attribute = new VendorSpecificAttribute;
    $attribute->type = RADIUS_VENDOR_SPECIFIC;
    $data = unpack('Ctype/Csize/NvendorId/CvendorType/CvendorSize', $raw);

    if ($data['type'] != RADIUS_VENDOR_SPECIFIC) {
        trigger_error('VendorSpecificAttribute::parse() called for a non-VS attribute', E_USER_ERROR);
    }

    $attribute->consumed = $data['size'];
    $attribute->vendorId = $data['vendorId'];
    $attribute->vendorType = $data['vendorType'];
    $attribute->value = substr($raw, 8, $data['vendorSize'] - 2);

    return $attribute;
}

// vim: set ts=4 sw=4 et:
