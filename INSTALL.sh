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
