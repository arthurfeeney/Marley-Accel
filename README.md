
## Marley Accel

A small Linux user space mouse accel driver that supports the quake live accel settings. It allows for acceleration that is frame rate independent and consistent across applications and games. It additionally allows you to save and use multiple configurations and provides a minimal GUI that plots your current settings to assist coniguration. 

#### Requirements

There are only a few simple requirements for the driver:

* A modern C compiler
* pkg-config
* libusb-1.0
* uinput (kernel module)

In order to use the (optional) setup GUI you also need:

* Python3
* matplotlib
* numpy
* tkinter
* argparse

#### Usage

This is still a work in progress. It is very simple to run, but there are some annoying requirements at the moment. The main one being that you should have a config file that adheres strictly to the form of the provided example, example_config/ex.cfg. Then, when you want to use the driver, you can compile and run it like this

~~~~
make run CONFIG_FILE_PATH=configs/ex.cfg
~~~~

Similarly, to run the GUI, you should pass the path to the config file that you want to modify.

~~~~
python mod.py configs/ex.cfg
~~~~
