Pyldin-601 emulator for Linux, WIN32, PS2, OS X, PSP
Copyright (c) Sasha Chukov <sash@pdaXrom.org>, Yura Kuznetsov 2000-2007

Keyboard:

PrtSc      	- Screenshot
ScrollLock 	- Toggle window and fullscreen mode
PauseBreak 	- Reset
L-WinKey(Linux) - Кир/Lat
L-Cmd(OS X)	- Кир/Lat

PSP Joystick:
Up,Down,	- Cursor keys
Left, Right
Triangle	- Escape
Circle		- Return
Cross		- Space
Square		- TAB
Start		- Reset
Select		- Screenshot
Home		- Shutdown
Left trigger	- Floppy menu
Right trigger	- Virtual screen keyboard

Usage:
	pyldin [-h][-t][-i][-p <file|system|covox>][-d <Rom/Floppy files directory>][boot floppy image]

-h        - this help
-t        - set date&time from host
-i        - show cpu performance
-p <type> - function of printer port:
            file   - output to file
            system - output to default system printer
            covox  - output to unsigned 8bit DAC (COVOX emulation)
-d <dir>  - path to directory with Rom/Floppy images
