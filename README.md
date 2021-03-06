
# Marley Accel

A small Linux user space mouse accel driver that supports the quake live accel
settings. It allows for acceleration that is frame rate independent and consistent
across applications and games. It additionally allows you to save and use multiple
configurations and provides a minimal GUI that plots your current settings to assist
coniguration.

## Requirements

There are only a few simple requirements for the driver:

* A modern C compiler
* pkg-config
* libusb-1.0
* uinput (kernel module, requires use of su)

## Usage

This is still a work in progress. The
config file should adhere to the form of the provided example,
``configs/ex.cfg``. When you want to quickly test the driver with a new config,
you can compile and run it like this

~~~~
make run CONFIG_FILE_PATH=configs/ex.cfg
~~~~

Similarly, to run the GUI, you should pass the path to the config file that you want to modify.

~~~~
python mod.py configs/ex.cfg
~~~~

Configuration files allow for single-line comments and whitespace. The variables can
be specified in any order. Everything needs to be spelled correctly, in lowercase,
and it needs to have the equal sign.


## Tests

There is a small suite of unit tests for the acceleration functions and map.
it implements a very minimal unit testing framework, rather than adding
a new dependency. To run the tests, simply run

~~~~
export ASAN_SYMBOLIZER=/usr/bin/llvm-symbolizer  # if available, for address sanitizer
make test
~~~~
