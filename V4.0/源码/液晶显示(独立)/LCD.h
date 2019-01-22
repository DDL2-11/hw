#ifndef __LCD_H_
#define __LCD_H_

	

void DelayMS(unsigned int msec);
void LCD_DLY_ms(unsigned int ms);
void LCD_WrDat(unsigned char dat);
void LCD_WrCmd(unsigned char cmd);
void LCD_Set_Pos(unsigned char x, unsigned char y);
void LCD_Fill(unsigned char bmp_dat);
void LCD_CLS(void);
void LCD_Init(void);
//void LCD_P8x16Str(unsigned char x, unsigned char y,unsigned char ch[]);
void LCD_P16x16Ch(unsigned char x, unsigned char y, unsigned char N);
void LCD_P6x8Str(unsigned char x, unsigned char y,unsigned char ch[]);
#endif

