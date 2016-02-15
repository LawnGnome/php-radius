# RADIUS

This is the source code for the PHP RADIUS extension.

[![Build Status](https://travis-ci.org/LawnGnome/php-radius.svg?branch=master)](https://travis-ci.org/LawnGnome/php-radius)

## Installation

### Via PECL

You can install the extension through PECL:

```sh
pecl install radius
```

For more information on using PECL, please see
[the PHP manual](http://php.net/manual/en/install.pecl.php).

### From source

As this extension is self contained, you can build it very easily, provided you
have a PHP build environment:

```sh
phpize
./configure
make
sudo make install
echo extension=radius.so | sudo tee -a $(php --ini | head -1 | cut -d: -f 2- | cut -c 2-)/php.ini
```

You can also run the test suite with `make test`.

## Usage

[The PHP manual includes a section on this extension.](http://php.net/radius)

## Contributing

Pull requests and bug reports are most welcome through GitHub.
