# LiFePO4wered-Pi
Access library, command line tool and daemon for the LiFePO<sub>4</sub>wered/Pi+ and legacy LiFePO<sub>4</sub>wered/Pi and LiFePO<sub>4</sub>wered/Pi3 modules

## Installation

Starting from a fresh Raspbian image, first install the `build-essential`,
`git` and the `systemd support library` packages:

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
sudo make user-install
```

That's it!  You may need to restart for some configuration changes (such as enabling the I<sup>2</sup>C device) to take effect.

## Legacy build scripts

The build and install commands were changed from what they originally were.
To prevent confusion for people not reading this but following installation
instructions in older manuals, `build.py` and `INSTALL.sh` are provided that
just call the `make` commands above for legacy compatibility.

## Daemon

The installation command installs a background program
(lifepo4wered-daemon), along with scripts to start it. You can also start
the daemon manually, it will by default run in the background, but you can force it to run in the foreground by adding the `-f` argument.

The daemon supports startup via `systemd`, including its notification
and keepalive features. See `man systemd.service` for details.

If you do not want to include `systemd` support in the daemon, you can build
the code with:

```
make all USE_SYSTEMD=0
```

## CLI

The `lifepo4wered-cli` tool provides convenient access to the LiFePO<sub>4</sub>wered
device's I<sup>2</sup>C registers.  Run it without parameters to get help information:

```
lifepo4wered-cli
```

To get a full dump of all register values, try:

```
lifepo4wered-cli get
```

To get the current battery voltage in millivolts, try:

```
lifepo4wered-cli get vbat
```

To set the wake up time to an hour, run:

```
lifepo4wered-cli set wake_time 60
```

To set the auto-boot flag to make the Pi run whenever there is power to do so, but still be able to turn the Pi off with the button or from software, run:

```
lifepo4wered-cli set auto_boot 2
```

To make this change permanent by saving it to flash, run:

```
lifepo4wered-cli set cfg_write 0x46
```

The `0x46` value is a magic key to allow config flash writes.

Adjusting some of the register values can cause problems such as not being able
to turn on the system using the touch button.  To prevent permanently bricking
the LiFePO<sub>4</sub>wered device, always test your changes thoroughly before writing them
to flash.  If you made a change that makes your LiFePO<sub>4</sub>wered device not work
correctly, and it is not written to flash, you can undo it by unplugging the
LiFePO<sub>4</sub>wered device and removing the LiFePO<sub>4</sub> cell from the battery holder for
a couple of minutes.  The LiFePO<sub>4</sub>wered device should revert to its previous
last saved state when you put the battery back.

Check out the product brief for the
[LiFePO<sub>4</sub>wered/Pi+](https://lifepo4wered.com/files/LiFePO4wered-Pi+-Product-Brief.pdf) or legacy [LiFePO<sub>4</sub>wered/Pi](http://lifepo4wered.com/files/LiFePO4wered-Pi-Product-Brief.pdf) or [LiFePO<sub>4</sub>wered/Pi3](http://lifepo4wered.com/files/LiFePO4wered-Pi3-Product-Brief.pdf) devices for a complete list of registers and valid values and options available in each product.  Alternatively, running `lifepo4wered-cli get` returns a dump with all valid registers for the connected device.

## Permissions

The user running the `lifepo4wered-cli` tool needs to have sufficient
privileges to access the I<sup>2</sup>C bus.  On Raspbian, the `pi` user by default can
access the bus because it is in the `i2c` group.  If you run as a different
user, you either need to add this user to the `i2c` group or run the tool with
`sudo`.  On other distributions, a different group name may be used.  You can
check the owner and group of the I<sup>2</sup>C device with:

```
ls -l /dev/i2c-1
```

The command line tool returns the following negative values to indicate error
conditions:

| Return value | Condition |
| -- | -- |
| -1 | Could not access the LiFePO<sub>4</sub>wered device to perform the specified operation.  Usually this condition is caused by insufficient privileges when trying to access the I<sup>2</sup>C bus.  Trying to run the command as root or with `sudo` to fix the problem.  When writing settings, this value is also returned if a register is not writable. |
| -2 | The I<sup>2</sup>C bus could be accessed and the operation is valid, but communication with the LiFePO<sub>4</sub>wered device failed.  After trying several times (20 by default), the I<sup>2</sup>C bus transaction could not be completed successfully.  This happens if the LiFePO<sub>4</sub>wered device is not physically present or if something (possibly another HAT) is preventing the I<sup>2</sup>C bus from operating correctly. |

## Balena

The included `Dockerfile` can be used to compile the daemon as a [Balena](https://www.balena.io/) compatible service.  Typically this would be used in a multicontainer setup where the LiFePO<sub>4</sub>wered service would be separate from your application container(s).  The `Dockerfile` uses the `USE_BALENA=1` `make` parameter to alter the shutdown command to send a shutdown request to the Balena supervisor container and runs the daemon code in foreground mode using the `-f` flag.

The included `Dockerfile` uses a 64-bit Raspberry Pi 4 base image, if you are using a different Pi or 32-bit Balena base OS, you need to alter the base image in both ("build" and "run") `FROM` lines.

A `docker-compose.yml` file such as the one below should be created to include the LiFePO<sub>4</sub>wered service in your application, make it privileged (to access the I<sup>2</sup>C bus and set system time), and give it access to the supervisor API:

```yaml
version: '2'
services:
  your-own-application-service:
    build: ./balena-node-hello-world
    ports:
      - "80:80"
  lifepo4wered-pi:
    build: ./LiFePO4wered-Pi
    privileged: true
    labels:
      io.balena.features.supervisor-api: '1'
```

If your application needs access to the LiFePO<sub>4</sub>wered device, the easiest approach may be to include the `lifepo4wered-cli` or `liblifepo4wered.so` with language bindings in your application's container, building it using the multistage approach demonstrated in our `Dockerfile`.  Make sure you then also give your own application access to the I<sup>2</sup>C bus.

