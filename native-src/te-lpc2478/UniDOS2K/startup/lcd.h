#ifndef _LCD_H_
#define _LCD_H_

#define CRSR_PIX_32     0
#define CRSR_PIX_64     1
#define CRSR_ASYNC      0
#define CRSR_FRAME_SYNC 2

void LCD_Init(unsigned int scraddr, unsigned int *pallete);

void LCD_Ctrl(int en);

void LCD_SetPallete(const unsigned int *pPallete);

void LCD_Cursor_Cfg(int Cfg);

void LCD_Cursor_En(int cursor);

void LCD_Cursor_Dis(int cursor);

void LCD_Move_Cursor(int x, int y);

void LCD_Copy_Cursor(const unsigned int *pCursor, int cursor, int size);

#endif
