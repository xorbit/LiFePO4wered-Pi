
help:
	-echo "Make goals:"
	-echo "  all     - build programs"
	-echo "  install - install programs to $$PREFIX"
	-echo "  clean   - delete generated files"
all:
	python3 build.py
install:
	env PREFIX=${DESTDIR}/usr NO_SYSTEM_INSTALL=nope bash INSTALL.sh
clean:
	rm -rf build
