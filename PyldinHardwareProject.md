# Introduction #

This project is an emulator that running as a real hardware. The hardware is based on the [LPC2478](http://ics.nxp.com/microcontrollers/to/pip/LPC2478.html) microcontroller and runs special firmware. Currently, the system being debugged on the developer board([TE-LPC2478LCD3.5](http://www.terraelectronica.ru/news_made.php?ID=5)) extended 1 bit's beeper and PS/2 keyboard port.

| ![http://pyldin.googlecode.com/svn/wiki/pyldin-board-proto.jpg](http://pyldin.googlecode.com/svn/wiki/pyldin-board-proto.jpg) | ![http://pyldin.googlecode.com/svn/wiki/pyldin-proto.jpg](http://pyldin.googlecode.com/svn/wiki/pyldin-proto.jpg) |
|:------------------------------------------------------------------------------------------------------------------------------|:------------------------------------------------------------------------------------------------------------------|

# Details #

Supported devices are the following:
  * LCD display
  * PS/2 keyboard
  * 1bit's speaker
  * SD slot for memory cards as external disk drives (hot-swap)

# Piezo speaker and PS/2 keyboard for TE-LPC2478LCD3.5 board #

![http://pyldin.googlecode.com/svn/wiki/pyldin-tlelpc2478-mini.png](http://pyldin.googlecode.com/svn/wiki/pyldin-tlelpc2478-mini.png)

# How to use SD card as disk A(B) #

The system can use sd card as an external drive A (and B). To do this you must create primary partition(s) type FAT12 (linux fdisk partition type 1). The first matching partition is connected as drive A, the second as drive B. The maximum partition size is limited to 16MB. Hot-swap is supported.
Before use, do not forget to format the partition(s) to the filesystem FAT12 (linux mkdosfs -F 12 /dev/sdXXXN).