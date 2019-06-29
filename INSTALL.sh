#!/bin/sh

# Binary names
CLI_NAME=lifepo4wered-cli
DAEMON_NAME=lifepo4wered-daemon
SO_NAME=liblifepo4wered.so

# Install prefix
PREFIX=${PREFIX-/usr/local}

install -d $PREFIX/lib
install -d $PREFIX/bin
install -d $PREFIX/sbin

# Install the shared object
install -p build/SO/$SO_NAME $PREFIX/lib
# Install the CLI
install -s -p build/CLI/$CLI_NAME $PREFIX/bin
# Install the daemon
install -s -p build/DAEMON/$DAEMON_NAME $PREFIX/sbin

# Install the init script
if test -d /etc/init.d ; then
    sed "s:DAEMON_DIRECTORY:$PREFIX/sbin:" <initscript >/etc/init.d/$DAEMON_NAME

    # Enable the service to start on boot
    update-rc.d $DAEMON_NAME defaults
    # Restart the service
    service $DAEMON_NAME restart
fi

# Install the systemd service
if test -d /etc/systemd/system ; then
    sed "s:DAEMON_DIRECTORY:$PREFIX/sbin:" <systemdscript >/etc/systemd/system/$DAEMON_NAME.service
    systemd daemon-reload
    systemd enable $DAEMON_NAME.service
    systemd restart $DAEMON_NAME.service
fi

# Check whether I2C is enabled in the device tree
if ! grep -q ^dtparam=i2c_arm=on /boot/config.txt
then
  echo "" >> /boot/config.txt
  echo "# I2C device tree settings" >> /boot/config.txt
  echo "dtparam=i2c1=on" >> /boot/config.txt
  echo "dtparam=i2c_arm=on" >> /boot/config.txt
  echo "Enabling I2C in device tree configuration"
  echo "You need to reboot for this to take effect!"
fi

# Check whether I2C device module is loaded
if ! grep -q i2c-dev /etc/modules
then
  echo "" >> /etc/modules
  echo "# Load I2C device module" >> /etc/modules
  echo "i2c-dev" >> /etc/modules
  echo "Enabling I2C device module"
  echo "You need to reboot for this to take effect!"
fi

# Check whether the UART is enabled in the device tree
# We need the UART to detect when the Pi shut down
if ! grep -q ^enable_uart=1 /boot/config.txt
then
  echo "" >> /boot/config.txt
  echo "# UART device tree settings" >> /boot/config.txt
  echo "enable_uart=1" >> /boot/config.txt
  echo "Enabling UART in device tree configuration"
  echo "You need to reboot for this to take effect!"
fi
