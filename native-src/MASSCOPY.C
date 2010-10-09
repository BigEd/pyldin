/********************************************
 *					    *
 *    Diskette mass production program	    *
 *					    *
 *	  (c) Eagle software, 1989	    *
 *					    *
 ********************************************/

/* This program must be compiled using the COMPACT model */

#include <bios.h>
#include <alloc.h>
#include <mem.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define TEXT(x,y) textbackground(x); textcolor(y)

#define ENTER 13
#define ESC 27
#define SPACE 32

#define TX 20
#define TY 16

const int MaxHeads = 2;
const int MinHeads = 1;
const int MaxTracks = 40;
const int MaxSectors = 9;
const int MinSectors = 8;
const int MaxBytes = 512;

char *track_buffers[80];		/* Max possible 40 tracks by 2 heads */

char fmt_buff [] = {0,0,5,2, 0,0,1,2, 0,0,6,2,
			0,0,2,2, 0,0,7,2, 0,0,3,2,
			0,0,8,2, 0,0,4,2, 0,0,9,2};

int BPS, SPT, Heads, Tracks, TotalSectors;
int two_drives = 0;
int errflg = 0;

int drive;
char *buffer;

char far **parms = MK_FP (0,0x78);

/******************************************************************************/

void fatal_error ( char *s)
{
	TEXT(RED, BLACK);
	gotoxy (10, 10); cprintf ("Fatal error: %s", s);  putch('\a');
	getch();
	TEXT(BLACK,WHITE);
	clrscr();
	exit (1);
}

void quit()
{
	TEXT(BLACK,WHITE);
	clrscr();
	exit (0);
}

void error ( char *s)
{
	TEXT(RED,BLACK);
	gotoxy (24,10); cprintf ("%s", s); putch ('\a');
	while (bioskey(1)) bioskey(0);
	getch();
	biosdisk(0,drive,0,0,1,SPT,buffer);		/* reset controller */
	TEXT(BLUE,WHITE);
	gotoxy (24,10); clreol();
}

/******************************************************************************/

int	trackread(int drive, int head, int track, char *buff)
{
	int result;

	gotoxy (TX+track, TY+head);
	TEXT(BLACK,LIGHTGREEN);
	putch('R');
	putch('\b');

	result = biosdisk( 2, drive, head, track, 1, SPT, buff);

	if (result)
	{
		gotoxy (TX+track, TY+head);
		TEXT(BLACK,LIGHTRED|BLINK); putch('*');
	}
	return (result);
}


int	trackwrite(int drive, int head, int track, char *buff)
{
	int result;

	gotoxy (TX+track, TY+head);
	TEXT(BLACK,LIGHTRED);
	putch('W');
	putch('\b');

	result = biosdisk( 3, drive, head, track, 1, SPT, buff);

	if (result)
	{
		gotoxy (TX+track, TY+head);
		TEXT(BLACK,LIGHTRED|BLINK); putch('*');
	}
	return (result);
}


int	trackformat(int drive, int head, int track)
{
	int result;
	int sector_no;

	for (sector_no = 0; sector_no < SPT; sector_no++)
	   { fmt_buff[sector_no*4] = track; fmt_buff[sector_no*4+1] = head;}


	gotoxy (TX+track, TY+head);
	TEXT(BLACK,LIGHTMAGENTA);
	putch('F');
	putch('\b');

	result = biosdisk( 5, drive, head, track, 1, SPT, fmt_buff);

	if (result)
	{
		gotoxy (TX+track, TY+head);
		TEXT(BLACK,LIGHTRED|BLINK); putch('*');
	}
	return (result);
}



int	trackverify(int drive, int head, int track, char *buff)
{
	int result, result1;

	gotoxy (TX+track, TY+head);
	TEXT(BLACK,LIGHTGREEN);
	putch('R');
	putch('\b');

	result = biosdisk( 2, drive, head, track, 1, SPT, buffer);
	result1 = memcmp(buffer, buff, SPT*BPS);

	gotoxy (TX+track, TY+head);

	if (result || result1)
		{ TEXT(BLACK,LIGHTRED|BLINK); putch('*');}
	else
		{ TEXT(BLACK, WHITE); putch ((errflg)?',':'.'); }

	return ( (result) ? result : result1 );
}


/******************************************************************************/


/******************************************************************************/

main (int argc, char *argv[])
{
	int track_no, head_no;
	int count = 0;
	char ch;
	int arg_no;

	directvideo = 1;
	drive = 0;
	two_drives = 0;

	for (arg_no = 1; arg_no < argc; arg_no++)
	{
	    if (strcmp(argv[arg_no], "/B") == 0) two_drives = 1;
	    else if (strcmp(argv[arg_no], "/b") == 0) two_drives =1;
	    else {puts ("\nOptions: /B - use both floppy drives");
		  exit (0);}
	}

	if (argc == 2) if (strcmp(argv[1], "/2") == 0) two_drives = 1;

	TEXT(BLUE,WHITE);
	clrscr();
	gotoxy(18,2); cputs("     Diskette MassCopy.   Version 1.01");
	gotoxy(18,3); cputs("    (C) 1989 Eagle software & Ivo Nenov");
	gotoxy(18,4); cputs("Software Research & Development Lab., Sofia");

	if ((buffer = malloc (MaxBytes * MaxSectors)) == NULL)
		fatal_error ("No enough memory");


again:
	TEXT(GREEN,WHITE);
	gotoxy (25,20); cprintf ("Insert ORIGINAL diskette in drive A:");
	gotoxy (25,21); cprintf ("and press ENTER when ready ...      ");
	do if ((ch = getch()) == ESC) quit(); while (ch != ENTER);
	TEXT(BLUE,WHITE);
	gotoxy (25,20); clreol(); gotoxy(25,21); clreol();


	if (biosdisk(2,0,0,0,1,1,buffer))
		{error ("Error reading Original diskette"); goto again;}

	Heads = *((int *)(buffer+26));
	SPT =  *((int *)(buffer+24));
	TotalSectors = *((int *)(buffer+19));
	BPS = *((int *)(buffer+11));
	Tracks = MaxTracks;

	if ( BPS != MaxBytes) fatal_error ("Invalid media: SectorSize <> $200");
	if ( (SPT < MinSectors) && (SPT > MaxSectors)) fatal_error ("Invalid media: Sectors per track less than 8 or more than 9");
	if ( (Heads < MaxHeads) && (Heads > MaxHeads)) fatal_error ("Invalid media: Number of heads > 2");
	if ( SPT * Heads * MaxTracks != TotalSectors) fatal_error ("Invalid media: Number of Cylinders <> 40");

	gotoxy (20, 14); cputs ("0000000000111111111122222222223333333333");
	gotoxy (20, 15); cputs ("0123456789012345678901234567890123456789");
	gotoxy (12, 16); cputs ("Side 0");
	if (Heads == 2) {gotoxy (12, 17); cputs ("Side 1");}


	for (track_no = 0; track_no < Tracks * Heads; track_no++)
		if ((track_buffers[track_no] = malloc ( BPS * SPT)) == NULL)
			fatal_error ("No enough memory");

	for (track_no = 0; track_no < Tracks; track_no++)
		for (head_no = 0; head_no < Heads; head_no++)
			if (trackread(0, head_no, track_no, track_buffers[track_no * Heads + head_no]))
				{ error ("Error reading ORIGINAL diskette"); goto again; }

	for (track_no = 0; track_no < Tracks; track_no++)
		for (head_no = 0; head_no < Heads; head_no++)
			if (trackverify(0, head_no, track_no, track_buffers[track_no * Heads + head_no]))
				{ error ("Error comparing ORIGINAL diskette"); goto again; }

	(*parms)[4] = SPT;    /* force BIOS */

loop:
	if (two_drives) drive = 1 - drive;

	gotoxy (14, 14); TEXT(BLUE,WHITE); cprintf ("%c:", drive?'B':'A');

	TEXT(YELLOW,BLACK);
	gotoxy (25,20); cprintf ("Insert NEW diskette in drive %c:", (drive)?'B':'A');
	gotoxy (25,21); cprintf ("and press SPACE when ready ... ");
	if (!(two_drives && count))
	   do {
		   while (bioskey(1)) bioskey(0);
	       if ((ch = getch()) == ESC) quit();
	   } while (ch != SPACE);
	TEXT(BLACK,WHITE);
	gotoxy (TX,TY);   cprintf ("                                        ");
	if (Heads == 2)
		{ gotoxy (TX,TY+1); cprintf ("                                        ");}
	TEXT(BLUE,WHITE);
	gotoxy (25,20); clreol(); gotoxy(25,21); clreol();


	biosdisk(0,drive,0,0,1,SPT,buffer);		/* reset controller */

	for (track_no = 0; track_no < Tracks; track_no++)
		for (head_no = 0; head_no < Heads; head_no++)
		{
		errflg = 0;
		retry:
			if (trackformat(drive, head_no, track_no))
				if (errflg) { error ("Error formatting TARGET diskette"); goto loop; }
				else {errflg = 1; goto retry;}

			if (trackwrite(drive, head_no, track_no, track_buffers[track_no * Heads + head_no]))
				if (errflg) { error ("Error writing TARGET diskette"); goto loop; }
				else {errflg = 1; goto retry;}

			if (trackverify(drive, head_no, track_no, track_buffers[track_no * Heads + head_no]))
				if (errflg) { error ("Error verifying TARGET diskette"); goto loop; }
				else {errflg = 1; goto retry;}
		}

	TEXT (BLUE,WHITE); gotoxy(30,24); cprintf ("Total number of copies: %d",++count);
	goto loop;
}

