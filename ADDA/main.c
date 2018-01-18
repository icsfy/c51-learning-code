#include <reg52.h>
#include <intrins.h>

#define MAIN_Fosc 11059200UL // 晶振频率
#define PCF8591_ADDR 0x90 // PCF8591地址
#define DAC_OUT_EN 0x40 // DAC输出使能

typedef unsigned int uint;
typedef unsigned int INT16U;
typedef unsigned char uchar;
typedef unsigned char INT8U;

sbit WE   = P2^7; // 数码管位选
sbit DU   = P2^6; // 数码管段选
sbit SCL  = P2^1; // I2C串行时钟
sbit SDA  = P2^0; // I2C串行数据
sbit BEEP = P2^3; // 蜂鸣器
sbit LED1 = P1^0;
sbit LED2 = P1^1;

// 数码管段选值
uchar code leddata[] = {
	//"0"		"1"		"2"		"3"		"4"		"5"		"6"		"7"		"8"		"9"
	0x3F,	0x06,	0x5B,	0x4F,	0x66,	0x6D,	0x7D,	0x07,	0x7F,	0x6F,
	//"A"		"B"		"C"		"D"		"E"		"F"		"H"		"L"		"n"		"P"
	0x77,	0x7C,	0x39,	0x5E,	0x79,	0x71,	0x76,	0x38,	0x37,	0x3E,
	//"P"		"O"		"-"		"."	"OFF"
	0x73,	0x5C,	0x40, 0x80, 0x00
};

// 数码管位选值
uchar code T_COM[] = {
	//"0"		"1"		"2"		"3"		"4"		"5"		"6"		"7"		"8"		"9"
	0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};

// ms延时
void Delay_Ms(INT16U ms)
{
    INT16U i;
	  do{
	      i = MAIN_Fosc / 96000; 
		    while(--i); // 96T per loop
    }while(--ms);
}

// 延时5us
void Delay5us()
{
	#if MAIN_Fosc == 11059200
		_nop_();
	#elif MAIN_Fosc == 12000000
		_nop_();
	#elif MAIN_Fosc == 22118400
		_nop_(); _nop_(); _nop_();
	#endif
}

// 显示第n个数码管，值为num
void display_1(uchar n, num)
{
	P0 = T_COM[n];
	WE = 1;
	WE = 0;
	DU = 1;
	P0 = leddata[num];
	DU = 0;
	Delay_Ms(1);
}

// 数码管显示
void Display(uchar num)
{
	uint _0_1mv = num * 195; // 精确到0.1mv
	display_1(0, _0_1mv/10000);
	display_1(1, _0_1mv/1000%10);
	display_1(2, _0_1mv/100%10);
	display_1(3, _0_1mv%100/10);
	display_1(0, 23); // 小数点
}

// I2C总线初始化
void I2C_INIT()
{
	SDA = 1;
	_nop_();
	SCL = 1;
	_nop_();
}

// I2C起始信号
void I2C_Start()
{
	SCL = 1;
	_nop_();
	SDA = 1;
	Delay5us();
	SDA = 0;
	Delay5us();
}

// I2C停止信号
void I2C_Stop()
{
	SDA = 0;
	_nop_();
	SCL = 1;
	Delay5us();
	SDA = 1;
	Delay5us();
}

// I2C主机发送应答，ack: 0(非应答), 1(应答)
void Master_ACK(bit ack)
{
	SCL = 0;
	_nop_();
	SDA = ~ack;
	_nop_();
	SCL = 1;
	Delay5us();
	SCL = 0;
	_nop_();
	SDA = 1;
	_nop_();
}

// I2C检测从机应答，0非应答，1应答
bit Test_ACK()
{
	SCL = 1;
	Delay5us();
	if(SDA){
		SCL = 0;
		_nop_();
		I2C_Stop();
		return 0;
	} else {
		SCL = 0;
		_nop_();
		return 1;
	}
}

// I2C发送一个字节
void I2C_Send_byte(uchar byte)
{
	uchar i;
	SCL = 0;
	_nop_();
	for(i = 0; i < 8; i++){
		SDA = (byte & 0x80);
		_nop_();
		SCL = 1;
		Delay5us();
		SCL = 0;
		_nop_();
		byte <<= 1;
	}
	SDA = 1;
	_nop_();
}

// I2C读取一个字节
uchar I2C_Read_byte()
{
	uchar i, byte = 0;
	SCL = 0;
	_nop_();
	for(i = 0; i < 8; i++){
		byte <<= 1;
		SCL = 1;
		Delay5us();
		byte |= SDA;
		SCL = 0;
		_nop_();
	}
	return byte;
}

bit DAC_Out(uchar DAT)
{
	I2C_Start();
	I2C_Send_byte(PCF8591_ADDR+0);
	if(!Test_ACK())
		return 0;
	I2C_Send_byte(DAC_OUT_EN);
	if(!Test_ACK())
		return 0;
	I2C_Send_byte(DAT);
	if(!Test_ACK())
		return 0;
	I2C_Stop();
	return 1;
}

bit ADC_Read(uchar CON, uchar *dat)
{
	I2C_Start();
	I2C_Send_byte(PCF8591_ADDR+0);
	if(!Test_ACK())
		return 0;
	I2C_Send_byte(CON);
	Master_ACK(0);
	I2C_Start();
	I2C_Send_byte(PCF8591_ADDR+1);
	if(!Test_ACK())
		return 0;
	*dat = I2C_Read_byte();
	Master_ACK(0);
	I2C_Stop();
	return 1;
}

void main()
{
	INT8U ADC_Value;
	I2C_INIT();
	while(1){
		if(!ADC_Read(0x00, &ADC_Value))
			LED1 = 0;
		if(!DAC_Out(ADC_Value))
			LED2 = 0;
		Display(ADC_Value);
		BEEP = (ADC_Value > 150) ? 0 : 1;
		Delay_Ms(5);
	}
}
