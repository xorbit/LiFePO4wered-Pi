
PREFIX ?= /usr

help:
	-echo "Make goals:"
	-echo "  all     - build programs"
	-echo "  install - install programs to $$PREFIX"
	-echo "  inst    - install programs and update system"
	-echo "  clean   - delete generated files"
all:
	python3 build.py

install:
	env PREFIX=${PREFIX} NO_SYSTEM_INSTALL=nope bash INSTALL.sh
clean:
	rm -rf build
	-rm -f debian/lifepo4wered.init
