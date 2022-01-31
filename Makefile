PREFIX ?= /usr/local
CC ?= gcc
LD ?= ld
CFLAGS ?= -std=c99 -Wall -O2
USE_SYSTEMD ?= 1
USE_BALENA ?= 0
OPTCFLAGS-10 = -DSYSTEMD
OPTCFLAGS-01 =
OPTCFLAGS-10 = -DSYSTEMD
OPTCFLAGS-01 = -DBALENA
OPTCFLAGS = $(OPTCFLAGS-$(USE_SYSTEMD)$(USE_BALENA))
OPTLDFLAGS-1 = -lsystemd
OPTLDFLAGS-0 =
OPTLDFLAGS = $(OPTLDFLAGS-$(USE_SYSTEMD))

all: build/lifepo4wered-cli build/lifepo4wered-daemon build/liblifepo4wered.so

build/%.o: %.c
	@test -d build/ || mkdir -p build/
	$(CC) -c $(OPTCFLAGS) $(CFLAGS) $< -o $@
build/liblifepo4wered.so: build/lifepo4wered-access.o build/lifepo4wered-data.o
	$(LD) -o $@ $^ -shared
build/lifepo4wered-cli: build/lifepo4wered-access.o build/lifepo4wered-data.o build/lifepo4wered-cli.o
	$(CC) -o $@ $^
build/lifepo4wered-daemon: build/lifepo4wered-access.o build/lifepo4wered-data.o build/lifepo4wered-daemon.o
	$(CC) -o $@ $^ $(OPTLDFLAGS) 
help:
	@echo "Make goals:"
	@echo "  all     - build programs"
	@echo "  install - install programs to $$DESTDIR$$PREFIX"
	@echo "  clean   - delete generated files"

install-init-0: # sysvinit
	install -D -p initscript $(DESTDIR)/etc/init.d/lifepo4wered-daemon
	sed -i "s:DAEMON_DIRECTORY:$(PREFIX)/sbin:" $(DESTDIR)/etc/init.d/lifepo4wered-daemon
install-init-1: install-init-0 # systemd and sysvinit
	install -D -p systemdscript $(DESTDIR)$(PREFIX)/lib/systemd/system/lifepo4wered-daemon.service
	sed -i "s:DAEMON_DIRECTORY:$(PREFIX)/sbin:" $(DESTDIR)$(PREFIX)/lib/systemd/system/lifepo4wered-daemon.service
build/modules-load.conf:
	echo "i2c-dev" > build/modules-load.conf
install-files: all build/modules-load.conf
	install -D -p build/liblifepo4wered.so $(DESTDIR)$(PREFIX)/lib/liblifepo4wered.so
	install -D -p build/lifepo4wered-cli $(DESTDIR)$(PREFIX)/bin/lifepo4wered-cli
	install -D -p build/lifepo4wered-daemon $(DESTDIR)$(PREFIX)/sbin/lifepo4wered-daemon
	install -D -p build/modules-load.conf $(DESTDIR)/lib/modules-load.d/lifepo4wered.conf

install: install-files install-init-$(USE_SYSTEMD)

enable-bus:
        ifneq (, $(shell command -v raspi-config 2> /dev/null))
	  raspi-config nonint do_i2c 0
	  raspi-config nonint do_serial 0
        endif
enable-init-0: # sysvinit
	update-rc.d lifepo4wered-daemon defaults
	service lifepo4wered-daemon restart
enable-init-1: # systemd
	systemctl daemon-reload
	systemctl enable lifepo4wered-daemon.service
	systemctl restart lifepo4wered-daemon.service
user-install: install enable-bus enable-init-$(USE_SYSTEMD)

clean:
	rm -rf build
