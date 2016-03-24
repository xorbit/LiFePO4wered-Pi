#!/bin/sh

# Binary names
CLI_NAME=lifepo4wered-cli
DAEMON_NAME=lifepo4wered-daemon
SO_NAME=liblifepo4wered.so

# Install prefix
PREFIX=${PREFIX-/usr/local}

# Install the shared object
install -p build/SO/$SO_NAME $PREFIX/lib
# Install the CLI
install -s -p build/CLI/$CLI_NAME $PREFIX/bin
# Install the daemon
install -s -p build/DAEMON/$DAEMON_NAME $PREFIX/sbin
# Install the init script
install -p -T initscript /etc/init.d/$DAEMON_NAME

# Set the daemon directory in the init script
sed -i "s:DAEMON_DIRECTORY:$PREFIX/sbin:" /etc/init.d/$DAEMON_NAME

# Enable the service to start on boot
update-rc.d $DAEMON_NAME defaults
# Restart the service
service $DAEMON_NAME restart

# Check whether the low voltage warning is not disabled yet
if ! grep -q avoid_warnings= /boot/config.txt
then
  echo "" >> /boot/config.txt
  echo "# Turn off low voltage warning" >> /boot/config.txt
  echo "avoid_warnings=2" >> /boot/config.txt
  echo "Turned off low voltage warning"
fi

# Check whether I2C is enabled in the device tree
if ! grep -q i2c_arm=on /boot/config.txt
then
  echo "" >> /boot/config.txt
  echo "# Device tree settings" >> /boot/config.txt
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
