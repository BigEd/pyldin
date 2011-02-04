#define C_LCD_H_SIZE           320
#define C_LCD_H_PULSE          30
#define C_LCD_H_FRONT_PORCH    20
#define C_LCD_H_BACK_PORCH     38

#define C_LCD_V_SIZE           240
#define C_LCD_V_PULSE          3
#define C_LCD_V_FRONT_PORCH    4
#define C_LCD_V_BACK_PORCH     15

#define C_LCD_CTRL_LcdBpp      4 //100 - 16pbb
#define C_LCD_CTRL_LcdTFT      1
#define C_LCD_CTRL_BGR         1
#define C_LCD_CTRL_BEBO        0
#define C_LCD_CTRL_BEPO        0
#define C_LCD_CTRL_WATERMARK   0

#define C_LCD_POL_PCD_LO       2
#define C_LCD_POL_CLKSEL       0
#define C_LCD_POL_ACB          1
#define C_LCD_POL_IVS          1
#define C_LCD_POL_IHS          1
#define C_LCD_POL_IPC          1
#define C_LCD_POL_IOE          1
#define C_LCD_POL_CPL          (C_LCD_H_SIZE - 1)
#define C_LCD_POL_BCD          0
#define C_LCD_POL_PCD_HI       0
