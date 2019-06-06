
all:
	python3 build.py
install:
	env PREFIX=${DESTDIR}/usr NO_SYSTEM_INSTALL=nope bash INSTALL.sh
clean:
	rm -rf build
