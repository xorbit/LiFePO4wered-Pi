NOTE: This was contributed by Mauricio José Flórez Moncayo of
SMARTCHIP S.A.S. (www.techchip.com) and has not been tested by me.

A common problem when using the the LiFePO4wered/Pi and related products with
single board computers other than the Raspberry Pi is that most other boards
do not bring down the TX line after the kernel is done shutting down.  The
LiFePO4wered/Pi uses the fact that the TX line goes low as an indication that
the system has completed shutdown and it is safe to remove power.

While current firmware (2018 and later) has a feature to cut power to the host
a certain amount of time after shutdown was initiated (see PI_SHDN_TO), it
is definitely more desirable to be able to detect shutdown and remove power
based on that.

The Linux kernel provides a feature to make this possible called
"gpio-poweroff", and on most ARM based systems it can be enabled using a
custom device tree overlay.

Mauricio made such an overlay to enable gpio-poweroff on PA13 (hardware pin 8)
of the Orange Pi Plus 2e, which will likely work on other Allwinner H3 based
boards as well.  It was only tested by him on that board though, and it may
be necessary to change the pin definition on other H3 boards if they connect
different chip pins to the GPIO header.

This will only work with 4.1x and better kernels, not on legacy 3.x kernels.
At the time of creation he tested it using the Armbian nightly with kernel 4.14
and applied the custom overlay following the instructions found at this link:

https://docs.armbian.com/Hardware_Allwinner_overlays/#using-custom-overlays

One problem with enabling gpio-poweroff on the pin is that the user cannot use
the UART on this pin at the same time, but that may not be a problem because
H3 has 2 more UARTS on the 40 pins headers that can be activated at any time.

