# LiFePO4wered-Pi
Access library, command line tool and daemon for the LiFePO4wered/Pi+ and legacy LiFePO4wered/Pi and LiFePO4wered/Pi3 modules

## Installation

Starting from a fresh Raspbian image, first install the `build-essential`,
`git` and and the systemd support library:

```
sudo apt-get -y install build-essential git libsystemd-dev
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
make all
```

And install it:

```
sudo ./INSTALL.sh
```

That's it!  You may need to restart for some configuration changes to take effect.

## Daemon

The installation script installs a background program
("lifepo4wered-daemon"), along with scripts to start it. You can also start
the daemon manually; it will continue run in the background.

The daemon supports startup via systemd, including its notification
and keepalive features. See "man systemd.service" for details.

## CLI

The `lifepo4wered-cli` tool provides convenient access to the LiFePO4wered
device's I2C registers.  Run it without parameters to get help information:

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
the LiFePO4wered device, always test your changes thoroughly before writing them
to flash.  If you made a change that makes your LiFePO4wered device not work
correctly, and it is not written to flash, you can undo it by unplugging the
LiFePO4wered device and removing the LiFePO4 cell from the battery holder for
a couple of seconds.

The user running the `lifepo4wered-cli` tool needs to have sufficient
permissions to access the I2C bus.  On Raspbian, the `pi` user by default can
access the bus because it is in the `i2c` group.  If you run as a different
user, you either need to add this user to the `i2c` group or run the tool with
`sudo`.  On other distributions, a different group name may be used.  You can
check the owner and group of the I2C device with:

```
ls -l /dev/i2c-1
```

The command line tool returns the following negative values to indicate error
conditions:

| Return value | Condition |
| -- | -- |
| -1 | Could not access the LiFePO4wered device to perform the specified operation.  Usually this condition is caused by insufficient privileges when trying to access the I2C bus.  Trying to run the command as root or with `sudo` to fix the problem.  When writing settings, this value is also returned if a register is not writable. |
| -2 | The I2C bus could be accessed and the operation is valid, but communication with the LiFePO4wered device failed.  After trying several times (20 by default), the I2C bus transaction could not be completed successfully.  This happens if the LiFePO4wered device is not physically present or if something (possibly another HAT) is preventing the I2C bus from operating correctly. |

Check out the product brief for the
[LiFePO4wered/Pi+](https://lifepo4wered.com/files/LiFePO4wered-Pi+-Product-Brief.pdf) or legacy [LiFePO4wered/Pi](http://lifepo4wered.com/files/LiFePO4wered-Pi-Product-Brief.pdf)or [LiFePO4wered/Pi3](http://lifepo4wered.com/files/LiFePO4wered-Pi3-Product-Brief.pdf) devices for a complete list of registers and options available in each product.
