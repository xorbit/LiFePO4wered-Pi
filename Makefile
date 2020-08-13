PREFIX ?= /usr/local
CC ?= gcc
LD ?= ld
CFLAGS ?= -std=c99 -Wall -O2
USE_SYSTEMD ?= 1
SYSTEMDFLAGS-1 = -DSYSTEMD -lsystemd
SYSTEMDFLAGS-0 =
SYSTEMDFLAGS = $(SYSTEMDFLAGS-$(USE_SYSTEMD))

build/%.o: %.c
	@test -d build/ || mkdir -p build/
	$(CC) -c $(CFLAGS) $< -o $@
build/liblifepo4wered.so: build/lifepo4wered-data.o
	$(LD) -o $@ $^ -shared
build/lifepo4wered-cli: build/lifepo4wered-access.o build/lifepo4wered-data.o build/lifepo4wered-cli.o
	$(CC) -o $@ $(SYSTEMDFLAGS) $^
build/lifepo4wered-daemon: build/lifepo4wered-access.o build/lifepo4wered-data.o build/lifepo4wered-daemon.o
	$(CC) -o $@ $^
help:
	@echo "Make goals:"
	@echo "  all     - build programs"
	@echo "  install - install programs to $$DESTDIR$$PREFIX"
	@echo "  clean   - delete generated files"
all: build/lifepo4wered-cli build/lifepo4wered-daemon build/liblifepo4wered.so

install-init-0: # sysvinit
	install -D -p initscript $(DESTDIR)$(PREFIX)/etc/init.d/lifepo4wered-daemon
	sed -i "s:DAEMON_DIRECTORY:$(DESTDIR)$(PREFIX)/sbin:" $(DESTDIR)$(PREFIX)/etc/init.d/lifepo4wered-daemon
install-init-1: install-init-0 # systemd and sysvinit
	install -D -p systemdscript $(DESTDIR)$(PREFIX)/etc/systemd/system/lifepo4wered-daemon.service
	sed -i "s:DAEMON_DIRECTORY:$(DESTDIR)$(PREFIX)/sbin:" $(DESTDIR)$(PREFIX)/etc/systemd/system/lifepo4wered-daemon.service
build/modules-load.conf:
	echo "i2c-dev" > build/modules-load.conf
install: all install-init-$(USE_SYSTEMD) build/modules-load.conf
	install -D -p build/liblifepo4wered.so $(DESTDIR)$(PREFIX)/lib/liblifepo4wered.so
	install -D -p build/lifepo4wered-cli $(DESTDIR)$(PREFIX)/bin/lifepo4wered-cli
	install -D -p build/lifepo4wered-daemon $(DESTDIR)$(PREFIX)/sbin/lifepo4wered-daemon
	install -D -p build/modules-load.conf $(DESTDIR)$(PREFIX)/etc/modules-load.d/lifepo4wered.conf

clean:
	rm -rf build
