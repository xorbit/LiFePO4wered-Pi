# LiFePO4wered-Pi
Access library, command line tool and daemon for the LiFePO4wered/Pi module

## Installation

Starting from a fresh Raspbian Jessie Lite image (tested with 2016-02-09 version),
first install build-essential and git:

```
sudo apt-get -y install build-essential git
```

In a directory where you keep source code, clone the LiFePO4wered-Pi repository
from GitHub:

```
git clone https://github.com/xorbit/LiFePO4wered-Pi.git
```

Get in to the source code directory:

```
cd LiFePO4wered-Pi/
```

Build the code:

```
./build.py
```

And install it:

```
sudo ./INSTALL.sh
```

That's it!  You may need to restart for some configuration changes to take effect.

## CLI

The `lifepo4wered-cli` tool provides convenient access to the LiFePO4wered/Pi's
I2C registers.  Run it without parameters to get help information:

```
lifepo4wered-cli
```

To get the current battery voltage in millivolts, try:

```
lifepo4wered-cli get vbat
```

To set the wake up time to an hour, run:

```
lifepo4wered-cli set wake_time 60
```

To set the auto-boot flag to make the Pi run whenever there is power to do so, run:

```
lifepo4wered-cli set auto_boot 1
```

To make this change permanent by saving it to flash, run:

```
lifepo4wered-cli set cfg_write 0x46
```

The `0x46` value is a magic key to allow config flash writes.

Adjusting some of the register values can cause problems such as not being able
to turn on the system using the touch button.  To prevent permanently bricking
the LiFePO4wered/Pi, always test your changes thoroughly before writing them to
flash.  If you made a change that makes your LiFePO4wered/Pi not work correctly,
and it is not written to flash, you can undo it by unplugging the LiFePO4wered/Pi
and removing the LiFePO4 cell from the battery holder for a couple of seconds.

Check out the product brief for the [LiFePO4wered/Pi](http://lifepo4wered.com/files/LiFePO4wered-Pi-Product-Brief.pdf) and [LiFePO4wered/Pi3](http://lifepo4wered.com/files/LiFePO4wered-Pi3-Product-Brief.pdf) for a complete list of registers and options available in each product.
