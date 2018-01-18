#include <reg52.h>
#include <intrins.h>

#define uchar unsigned char
#define uint unsigned int
#define AT24C02_ADDR 0xa0
sbit SCL = P2^1;
sbit SDA = P2^0;
sbit we = P2^7;
sbit du = P2^6;
uchar count = 0;
uchar time5s = 0;
uchar code leddata[]={ 
                0x3F,  //"0"
                0x06,  //"1"
                0x5B,  //"2"
                0x4F,  //"3"
                0x66,  //"4"
                0x6D,  //"5"
                0x7D,  //"6"
                0x07,  //"7"
                0x7F,  //"8"
                0x6F,  //"9"
                0x77,  //"A"
                0x7C,  //"B"
                0x39,  //"C"
                0x5E,  //"D"
                0x79,  //"E"
                0x71,  //"F"
                0x76,  //"H"
                0x38,  //"L"
                0x37,  //"n"
                0x3E,  //"u"
                0x73,  //"P"
                0x5C,  //"o"
                0x40,  //"-"
                         };

void delay5us()
{
	_nop_();
}

void delay(uint z)
{
	uchar i;
	while(z--)
		for(i = 114; i > 0; i--);
}

void display(uchar num)
{
	P0 = 0xfe;
	we = 1;
	we = 0;
	du = 1;
	P0 = leddata[num/10];
	du = 0;
	delay(5);
	P0 = 0xfd;
	we = 1;
	we = 0;
	du = 1;
	P0 = leddata[num%10];
	du = 0;
	delay(5);
}

void I2C_INIT()
{
	SDA = 1;
	_nop_();
	SCL = 1;
	_nop_();
}

void I2C_Start()
{
	SCL = 1;
	_nop_();
	SDA = 1;
	delay5us();
	SDA = 0;
	delay5us();
}

void I2C_Stop()
{
	SDA = 0;
	_nop_();
	SCL = 1;
	delay5us();
	SDA = 1;
	delay5us();
}

bit Test_ACK()
{
	SCL = 1;
	delay5us();
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

void Master_ACK(bit ack)
{
	SCL = 0;
	_nop_();
	SDA = ~ack;
	_nop_();
	SCL = 1;
	delay5us();
	SCL = 0;
	_nop_();
	SDA = 1;
	_nop_();
}

void I2C_Send_byte(uchar byte)
{
	uchar i;
	SCL = 0;
	_nop_();
	for(i = 0; i < 8; i++){
		SDA = (byte & 0x80);
		_nop_();
		SCL = 1;
		delay5us();
		SCL = 0;
		_nop_();
		byte <<= 1;
	}
	SDA = 1;
	_nop_();
}

uchar I2C_Read_byte()
{
	uchar i, byte = 0;
	SCL = 0;
	_nop_();
	for(i = 0; i < 8; i++){
		byte <<= 1;
		SCL = 1;
		delay5us();
		byte |= SDA;
		SCL = 0;
		_nop_();
	}
	return byte;
}

uchar I2C_TransmitByte(uchar ADDR, DAT)
{
	I2C_Start();
	I2C_Send_byte(AT24C02_ADDR+0);
	if(!Test_ACK()){
		return 0;
	}
	I2C_Send_byte(ADDR);
	if(!Test_ACK()){
		return 0;
	}
	I2C_Send_byte(DAT);
	if(!Test_ACK()){
		return 0;
	}
	I2C_Stop();
	return 1;
}

uchar I2C_ReceiveByte(uchar ADDR)
{
	uchar dat;
	I2C_Start();
	I2C_Send_byte(AT24C02_ADDR+0);
	if(!Test_ACK()){
		return 0;
	}
	I2C_Send_byte(ADDR);
	Master_ACK(0);
	I2C_Start();
	I2C_Send_byte(AT24C02_ADDR+1);
	if(!Test_ACK()){
		return 0;
	}
	dat = I2C_Read_byte();
	Master_ACK(0);
	I2C_Stop();
	return dat;
}

void fun1()
{
	I2C_INIT();
	if(!I2C_TransmitByte(255, 0xf0)){
		P1 = 0;
		while(1);
	}
	delay(5);
	P1 = I2C_ReceiveByte(255);
	while(1);
}

void Timer0Init()
{
	TMOD = 0x01;
	TL0 = 0x00;
	TH0 = 0x4c;
	TR0 = 1;
	ET0 = 1;
	EA = 1;
}

void timer0() interrupt 1
{
	TL0 = 0x00;
	TH0 = 0x4c;
	count++;
	if(count == 100){
		count = 0;
		time5s = 1;
	}
}

void fun2()
{
	uchar dat;
	Timer0Init();
	I2C_INIT();
	dat = I2C_ReceiveByte(233);
	while(1){
		if(time5s){
			time5s = 0;
	    dat = (dat+1)%100;
	    if(!I2C_TransmitByte(233, dat))
				P1 = 0;
			else
				P1 = 0xff;
		}
	  display(dat);
	}
}

void main()
{
	//fun1(); // 写&读EEPROM
	fun2(); // 读取EEPROM 5秒+1 存入EEPROM并数码管显示
}