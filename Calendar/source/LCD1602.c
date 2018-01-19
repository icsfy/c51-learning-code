#include "config.h"
#include "LCD1602.h"

/*LCD1602读写检测并等待*/
void LCD1602_Read_busy() {
  uint8 busy;
  LCD1602_RS = 0;
  LCD1602_RW = 1;
  LCD1602_DB = 0xff;  // 释放数据总线
  do {
    LCD1602_EN = 1;
    busy = LCD1602_DB & 0x80;
    LCD1602_EN = 0;
  } while (busy);
}

/*LCD1602写指令*/
void LCD1602_Write_Cmd(uchar cmd) {
  LCD1602_Read_busy();
  LCD1602_RS = 0;
  LCD1602_RW = 0;
  LCD1602_DB = cmd;
  LCD1602_EN = 1;
  LCD1602_EN = 0;
}

/*LCD1602写数据*/
void LCD1602_Write_Dat(uchar dat) {
  LCD1602_Read_busy();
  LCD1602_RS = 1;
  LCD1602_RW = 0;
  LCD1602_DB = dat;
  LCD1602_EN = 1;
  LCD1602_EN = 0;
}

/*LCD1602设置光标显示位置*/
void LCD1602_Set_Cursor(uint8 x, uint8 y) {
  LCD1602_Write_Cmd(0x80 | x | (y ? 0x40 : 0));
}

/*LCD1602打开光标显示*/
void LCD1602_Open_Cursor() { LCD1602_Write_Cmd(0x0f); }

/*LCD1602关闭光标显示*/
void LCD1602_Close_Cursor() { LCD1602_Write_Cmd(0x0c); }

/*LCD1602清屏*/
void LCD1602_Clear_Screen() { LCD1602_Write_Cmd(0x01); }

/*LCD1602初始化*/
void Init_LCD1602() {
  LCD1602_Write_Cmd(0x38);  // 设置16x2显示 5x7点阵 8位数据接口
  LCD1602_Close_Cursor();
  LCD1602_Write_Cmd(0x06);  // 读或写一个字符后 指针加一 光标加一
  LCD1602_Clear_Screen();
}

/*LCD1602写一个字节数据*/
void LCD1602_Dis_Char(uint8 x, uint8 y, uint8 dat) {
  LCD1602_Set_Cursor(x, y);
  LCD1602_Write_Dat(dat);
}

/*LCD1602写字符串*/
void LCD1602_Dis_Str(uint8 x, uint8 y, uint8 *str) {
  LCD1602_Set_Cursor(x, y);
  while (*str != '\0') {
    LCD1602_Write_Dat(*str++);
  }
}
