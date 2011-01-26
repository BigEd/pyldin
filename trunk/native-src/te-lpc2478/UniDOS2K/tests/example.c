/* This example is released 2010 by Alexander Graf to the public domain with no
 * rights reserved. */

/*-
 * This is an example program for the pab bootloader.
 *
 * To create a loadable ELF, you have to consider:
 * - The program must be an ELF of type ET_EXEC for EM_ARM machine.
 * - The program must completely be inside the external RAM of the device.  No
 *   sections are allowed to be outside the external RAM.  The internal RAM is
 *   reserved for the bootloader itself.
 *
 * The ELF contains a function which is an entry point (referred to as
 * _start()).  This is the function which is initially called by the bootloader.
 * When this function is called,
 * - r0, i.e. in C, the first argument of the function, contains a pointer to
 *   the command line argument, if specified.  Elsewise, 0.
 * - lr, which is loaded to pc when _start() does return, is set to the reset
 *   function in the bootloaders interrupt vector (which persists, as it is in
 *   the device's ROM).
 * - sp is actually useful.  The bootloader automatically sets it to the end of
 *   the internal RAM.
 * - All the other register contents are not defined to have a special meaning.
 *   They may have some reasonable values, but they are not intently set to this
 *   values, so they possibly will change e.g. when changing pab's compiler
 *   flags.  Do not rely on the contents of this registers!
 *
 * Also, do not rely on the hardware being properly set up, even the hardware
 * which is needed by the bootloader.  It is the best way to reinitalize
 * everything, considering that reinitalizing external RAM could cause problems.
 * This example does not reinitalize everything, just to be more portable and
 * easier to understand.
 *
 * In general, it is useful to link your program with -N, as they are smaller
 * both on-disk and on-memory (-N may have some disadvantages, though).
 *
 * To compile this example, do:
 *  arm-elf-gcc -nostdlib -N -Ttext 0xa0000000 -o example doc/example.c
 * (in case your external RAM starts at 0xa0000000)
 *
 * To try this example,
 * 0. Compile this example (see above).
 * 1. Search an unneeded MMC or SPI-capable SD on your desk, your floor, your
 *    card slots, under your sofa or somewhere else.  Then, make it to your
 *    /dev/mmcblk0 (or sdb, da1, or whatever)
 * 2. fdisk /dev/mmcblk0, add one partition with type 0x83 (Linux).  We take
 *    partition number 0, i.e. the first partition.  Actually, this step is
 *    optional.
 * 3. mkfs.ext2 /dev/mmcblk0p1
 * 4. mount /dev/mmcblk0p1 /somewhere
 * 5. cp this-example /somewhere/example
 * 6. umount /somewhere
 * 7. Mount MMC in your ARM board.
 * 8. Flash pab to your processors flash.
 * 9. Boot the board with something connected to UART0.  On the console, type
 *    "load mmc|part:0|ext2:example" to boot it.
 *
 * This example does (should do):
 * 1. Print "It works!".
 * 2. Print either the command line argument, or that no command line argument
 *    was given.
 * 3. Return from _start(), so that the device is resetted.
 *
 * If it does not work, congratulations!  You either discovered a bug, or did
 * something (terribly) wrong.
 */

/* UART stuff.  Quick & dirty driver - good enough for this example */

#define UART0_BASE	0xe000c000	/* NOTE: Check this before trying. */
#define LSR	(*(volatile unsigned char *)	(UART0_BASE+0x14))
#define THR	(*(volatile char *)		(UART0_BASE+0x0 ))
#define THRE	(1 << 5)	/* THR empty */

static void uart_putchar(char c)
{
	while (!(LSR & THRE)) ;
	THR = c;
}

static void uart_putstring(const char *s)
{
	do {
		uart_putchar(*s);
	} while (*++s);
}

/* Now the real part of this example follows. */

void _start(char *elfarg)
{
	uart_putstring("It works!\n");

	if (elfarg) {
		uart_putstring("Command line argument: \"");
		uart_putstring(elfarg);
		uart_putstring("\"\n\n");
	} else
		uart_putstring("No command line argument was given.\n\n");
}
