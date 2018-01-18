#include <reg52.h>
#include <stdio.h>
typedef unsigned char INT8U;
typedef unsigned char uchar;
typedef unsigned int INT16U;
typedef unsigned int uint;

uchar IRtime;
uchar IRcord[4];
uchar IRdata[33];
bit IRok;

void init()
{
	TMOD |= 0x02; // 定时器0 8位自动重装
	TH0 = TL0 = 0;
	EA = 1; // 开总中断
	ET0 = 1; // 开定时器0中断
	TR0 = 1; // 启动定时器0
	
	IT0 = 1; // 设置外部中断0跳变沿触发方式
	EX0 = 1; // 开外部中断0中断
	
	TMOD |= 0x20; // 定时器1 8为自动重装
	TH1 = TL1 = 0xfd; // 比特率9600
	SM1 = 1; // 串口10位异步收发
	TR1 = 1; // 启动定时器1
}

void timer0() interrupt 1
{
	IRtime++; // 277.76us
}

void int0() interrupt 0
{
	static uchar i;
	static bit startflag;
	if(startflag){
		if((IRtime < 53)&&(IRtime > 32))
			i = 0;
		IRdata[i] = IRtime;
		IRtime = 0;
		i++;
		if(i == 33){
			IRok = 1;
			i = 0;
		}
	} else {
		IRtime = 0;
		startflag = 1;
	}
}

void IRcordpro()
{
	uchar i, j, k = 1;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 8; j++){
			IRcord[i] >>= 1;
			if(IRdata[k] > 5)
				IRcord[i] |= 0x80;
			k++;
		}
	}
}

void decode(uchar c)
{
	switch(c){
		case 0x45: TI = 1; printf("CH- "); break;
		case 0x46: TI = 1; printf("CH "); break;
		case 0x47: TI = 1; printf("CH+ "); break;
		
		case 0x44: TI = 1; printf("|<< "); break;
		case 0x40: TI = 1; printf(">>| "); break;
		case 0x43: TI = 1; printf(">|| "); break;
		
		case 0x07: TI = 1; printf("- "); break;
		case 0x15: TI = 1; printf("+ "); break;
		case 0x09: TI = 1; printf("EQ "); break;
		
		case 0x16: TI = 1; printf("0 "); break;
		case 0x19: TI = 1; printf("100+ "); break;
		case 0x0D: TI = 1; printf("200+ "); break;
		
		case 0x0C: TI = 1; printf("1 "); break;
		case 0x18: TI = 1; printf("2 "); break;
		case 0x5E: TI = 1; printf("3 "); break;
		
		case 0x08: TI = 1; printf("4 "); break;
		case 0x1C: TI = 1; printf("5 "); break;
		case 0x5A: TI = 1; printf("6 "); break;
		
		case 0x42: TI = 1; printf("7 "); break;
		case 0x52: TI = 1; printf("8 "); break;
		case 0x4A: TI = 1; printf("9 "); break;
	}
}

void main()
{
	//uchar i;
	init();
	while(1){
		if(IRok){
			IRcordpro();
			IRok = 0;
			/*
			for(i = 0; i< 4; i++){
				SBUF = IRcord[i];
				while(!TI);
				TI = 0;
			}
			*/
			decode(IRcord[2]);
		}
	}
}