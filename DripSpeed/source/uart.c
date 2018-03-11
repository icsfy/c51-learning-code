#include "uart.h"

void UartInit(void)		//9600bps@11.0592MHz
{
	PCON &= 0x7F;		//波特率不倍速
  
//  SM0 = 0;
//	SM1 = 1; //10位异步收发数据
//	REN = 1; //允许接收
	SCON = 0x50;		//8位数据,可变波特率
  
//	AUXR &= 0xBF;		//定时器1时钟为Fosc/12,即12T
//	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xFD;		//设定定时初值
	TH1 = 0xFD;		//设定定时器重装值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
  
  TI = 1;     //使用printf
//  ES = 1;     //串行中断开启
//  EA = 1;     //总中断允许
}

/*
 * putchar (basic version): expands '\n' into CR LF
 */
char putchar (char c)  {
  if (c == '\n')  {
    while (!TI);
    TI = 0;
    SBUF = 0x0d;                         /* output CR  */
  }
  while (!TI);
  TI = 0;
  return (SBUF = c);
}
