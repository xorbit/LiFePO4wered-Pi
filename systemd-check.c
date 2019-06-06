#include <stdlib.h>
#include <systemd/sd-daemon.h>
int main() {
	sd_notify(0, "STATUS=Test");
	exit(0);
}
