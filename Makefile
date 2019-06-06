
help:
	-echo "Make goals:"
	-echo "  all     - build programs"
	-echo "  install - install programs to $$PREFIX"
	-echo "  inst    - install programs and update system"
	-echo "  clean   - delete generated files"
all:
	python3 build.py

	test -d debian && \
	sed "s:DAEMON_DIRECTORY:${PREFIX}/sbin:" < initscript > debian/lifepo4wered.init
inst:
	env PREFIX=${DESTDIR}/usr bash INSTALL.sh
install:
	env PREFIX=${DESTDIR}/usr NO_SYSTEM_INSTALL=nope bash INSTALL.sh
clean:
	rm -rf build
	-rm -f debian/lifepo4wered.init
