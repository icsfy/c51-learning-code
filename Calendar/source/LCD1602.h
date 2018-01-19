#ifndef _LCD1602_H_
#define _LCD1602_H_

void LCD1602_Set_Cursor(uint8 x, uint8 y);            // LCD1602设置光标显示位置
void LCD1602_Open_Cursor();                           // LCD1602打开光标显示
void LCD1602_Close_Cursor();                          // LCD1602关闭光标显示
void LCD1602_Clear_Screen();                          // LCD1602清屏
void Init_LCD1602();                                  // LCD1602初始化
void LCD1602_Dis_Char(uint8 x, uint8 y, uint8 dat);   // LCD1602写一个字节数据
void LCD1602_Dis_Str(uint8 x, uint8 y, uint8 *str);   // LCD1602写字符串

#endif  // _LCD1602_H_
