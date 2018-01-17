#include <reg52.h>

typedef unsigned char uchar;
typedef unsigned int uint;

sbit RS = P3^5;
sbit RW = P3^6;
sbit EN = P3^4;

void Read_busy()
{
  uchar busy;
	RS = 0;
	RW = 1;
	P0 = 0xff;
	EN = 1;
  do {
    EN = 1;
    busy = P0 & 0x80;
    EN = 0;
  }	while(busy);
}

void Write_Cmd(uchar cmd)
{
	Read_busy();
	RS = 0;
	RW = 0;
	P0 = cmd;
	EN = 1;
	EN = 0;
}

void Write_Dat(uchar dat)
{
	Read_busy();
	RS = 1;
	RW = 0;
	P0 = dat;
	EN = 1;
	EN = 0;
}

void main()
{
	char i, *p;
	char s1[] = "Hello, world!";
	char s2[] = "Happy coding!";
	Write_Cmd(0x38); // 设置16x2显示 5x7点阵 8位数据接口
	Write_Cmd(0x0f); // 开显示 显示光标 光标闪烁
	Write_Cmd(0x01); // 清屏
	Write_Cmd(0x06); // 指针+1 光标+1
	Write_Cmd(0x80 | 0x01); // 数据地址指针
	p = s1;
	while(*p != '\0'){
	  Write_Dat(*p++);
		while(i--);
	}
	Write_Cmd(0x80 | 0x41); // 数据地址指针
	p = s2;
	while(*p != '\0'){
	  Write_Dat(*p++);
		while(i--);
	}
	while(1);
}
