#!/bin/sh

DAEMON_NAME=lifepo4wered-daemon

make install

# Install the init script
if test -d /etc/init.d ; then
    # Enable the service to start on boot
    update-rc.d $DAEMON_NAME defaults
    # Restart the service
    service $DAEMON_NAME restart
fi

# Install the systemd service
if test -d /etc/systemd/system ; then
    systemctl daemon-reload
    systemctl enable $DAEMON_NAME.service
    systemctl restart $DAEMON_NAME.service
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

# Load the I2C module
echo "Loading I2C device module"
echo "You may need to reboot for this to take effect!"
modprobe i2c-dev

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
