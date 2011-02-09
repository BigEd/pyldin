#include <inttypes.h>
#include "config.h"
#include "lcd.h"
#include "lcdconf.h"

void LCD_Init(unsigned int scraddr, unsigned int *pallete)
{
    // Assign pin:
    PINSEL0 &= BIN32(11111111,11110000,00000000,11111111);
    PINSEL0 |= BIN32(00000000,00000101,01010101,00000000); // P0.4(LCD0), P0.5(LCD1), P0.6(LCD8), P0.7(LCD9), P0.8(LCD16), P0.9(LCD17). 
    PINMODE0&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE0|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up. 

    PINSEL3 &= BIN32(11110000,00000000,00000000,11111111);
    PINSEL3 |= BIN32(00000101,01010101,01010101,00000000); // P1.20(LCD10), P1.21(LCD11), P1.22(LCD12), P1.23(LCD13), P1.24(LCD14), P1.25(LCD15), P1.26(LCD20), P1.27(LCD21), P1.28(LCD22), P1.29(LCD23).
    PINMODE3&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE3|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up.

    PINSEL4 &= BIN32(11110000,00110000,00000000,00000011); // P2.0(LCDPWR) - not used
    PINSEL4 |= BIN32(00000101,01001111,11111111,11111100); //               P2.1(LCDLE), P2.2(LCDDCP), P2.3(LCDFP), P2.4(LCDENAB), P2.5(LCDLP), P2.6(LCD4), P2.7(LCD5), P2.8(LCD6), P2.9(LCD7), P2.12(LCD18), P2.13(LCD19). 
    PINMODE4&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE4|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up.

    PINSEL9 &= BIN32(11110000,11111111,11111111,11111111);
    PINSEL9 |= BIN32(00001010,00000000,00000000,00000000); // P4.28(LCD2), P4.29(LCD3).
    PINMODE9&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE9|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up.

    PINSEL11&= BIN32(11111111,11111111,11111111,11110000);
    PINSEL11|= BIN32(00000000,00000000,00000000,00001101); // bit0=1 - LCD port is enabled.    bit1...3 = 110 TFT 16-bit. (1:5:5:5 mode) 
    //PINSEL11|= BIN32(00000000,00000000,00000000,00001011); // bit0=1 - LCD port is enabled.    bit1...3 = 101 TFT 16-bit. (5:6:5 mode)
    //PINSEL11|= BIN32(00000000,00000000,00000000,00001111); // bit0=1 - LCD port is enabled.    bit1...3 = 111 TFT 24-bit. (8:8:8 mode) 

    // SHUT, TPS61042.

    CONFIG_PINSEL(PINSEL7, 24, 0x00);//   PINSEL7_bit.P3_24 = 0x00;
    CONFIG_BIT(FIO3DIR, 24, 1);//  FIO3DIR_bit.P3_24 = 1;
    CONFIG_BIT(FIO3CLR, 24, 1);//  FIO3CLR_bit.P3_24 = 1;

    CONFIG_PINSEL(PINSEL1, 19, 0x00);//  PINSEL1_bit.P0_19 = 0x00;
    CONFIG_BIT(FIO0DIR, 19, 1);//  FIO0DIR_bit.P0_19  = 1;
    CONFIG_BIT(FIO0SET, 19, 1);//  FIO0SET_bit.P0_19  = 1;

    CONFIG_PINSEL(PINSEL4, 1, 0x00);//  PINSEL4_bit.P2_1  = 0x00;
    CONFIG_BIT(FIO2DIR, 1, 1);//  FIO2DIR_bit.P2_1  = 1;
    CONFIG_BIT(FIO2CLR, 1, 1);//  FIO2CLR_bit.P2_1  = 1;

    PCONP |= 0x00100000; // Power Control for CLCDC.
    CRSR_CTRL &= ~0x1;  // Disable cursor
    LCD_CTRL   = ((C_LCD_CTRL_WATERMARK << 16) | (C_LCD_CTRL_BEPO << 10) | (C_LCD_CTRL_BEBO << 9) | (C_LCD_CTRL_BGR << 8) | (C_LCD_CTRL_LcdTFT << 5) | (C_LCD_CTRL_LcdBpp << 1));
    LCD_CFG    = 1;
    LCD_POL    = ((C_LCD_POL_PCD_HI << 27) | (C_LCD_POL_BCD << 26) | (C_LCD_POL_CPL << 16) | (C_LCD_POL_IOE << 14) | (C_LCD_POL_IPC << 13)| (C_LCD_POL_IHS << 12) | (C_LCD_POL_IVS << 11)| (C_LCD_POL_ACB << 6) | (C_LCD_POL_CLKSEL << 5) | (C_LCD_POL_PCD_LO << 0));
    LCD_TIMH   = ((C_LCD_H_BACK_PORCH << 24) | (C_LCD_H_FRONT_PORCH << 16) | (C_LCD_H_PULSE <<  8) | (((C_LCD_H_SIZE / 16) - 1) << 2));
    LCD_TIMV   = ((C_LCD_V_BACK_PORCH << 24) | (C_LCD_V_FRONT_PORCH << 16) | (C_LCD_V_PULSE << 10) | C_LCD_V_SIZE);
    LCD_UPBASE = scraddr;
    LCD_LPBASE = scraddr;

    if (pallete)
	LCD_SetPallete(pallete);

    int i;
    for(i=0; i < 50000; i++) NOP;

    PINSEL11 = ( (6 << 1) | 1); // LCD 16 bit (1:5:5:5), LCD port is enabled
}

void LCD_Ctrl(int en)
{
    int i;
    if (en) {
	LCD_CTRL |= 1;
	for(i=0; i < 50000; i++) NOP;
	LCD_CTRL |= (1<<11);
    } else {
	LCD_CTRL |= ~(1<<11);
	for(i=0; i < 50000; i++) NOP;
	LCD_CTRL |= ~1;
    }
}

void LCD_SetPallete(const unsigned int *pPallete)
{
    int i;
    unsigned int *pDst = (unsigned int *)LCD_PAL;
    for (i = 0; i < 128; i++) *pDst++ = *pPallete++;
}

void LCD_Cursor_Cfg(int Cfg)
{
    CRSR_CFG = Cfg;
}

void LCD_Cursor_En(int cursor)
{
  /* Set Cursor and enable */
    CRSR_CTRL = ( cursor & 0x30 ) | 0x1;
}

void LCD_Cursor_Dis(int cursor)
{
    CRSR_CTRL &= ~(0x1);
    cursor = cursor;
}

void LCD_Move_Cursor(int x, int y)
{
    CRSR_XY = 0;
    CRSR_XY |= (x & 0x3FF);
    CRSR_XY |= ((y & 0x3FF) << 16);
}

void LCD_Copy_Cursor(const unsigned int *pCursor, int cursor, int size)
{
    int i;
    unsigned int *pDst = (unsigned int *)&CRSR_IMG;
    pDst += cursor * 64;

    for(i = 0; i < size ; i++) *pDst++ = *pCursor++;
}
