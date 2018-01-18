#include <reg52.h>

typedef unsigned char uchar;
typedef unsigned int uint;

sbit we = P2^7;
sbit du = P2^6;

uchar code leddata[20] = {
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
  //"A"   "B"   "C"   "D"   "E"   "F"   "H"   "L"   "n"   "u"
  0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x76, 0x38, 0x37, 0x3e
};

void delay(uint z)
{
	uint x, y;
	for(x = z; x > 0; x--)
	  for(y = 114; y > 0; y--);
}

uchar KeyScan()
{
	//uchar cord_l, cord_h;
	uchar key;
	P3 = 0xf0;
	if((P3 & 0xf0) != 0xf0){
		delay(5);
		if((P3 & 0xf0) != 0xf0){
			/*
			cord_l = P3 & 0xf0;
			P3 = 0x0f;
			cord_h = P3 & 0x0f;
			return (cord_l + cord_h);
			*/
			key = P3 & 0xf0;
			P3 = key | 0x0f;
			return P3;
		}
	}
  return 0;
}

void KeyPro()
{
	switch(KeyScan()){
		case 0xee: P0 = leddata[0]; break;
		case 0xde: P0 = leddata[1]; break;
		case 0xbe: P0 = leddata[2]; break;
		case 0x7e: P0 = leddata[3]; break;
		
		case 0xed: P0 = leddata[4]; break;
		case 0xdd: P0 = leddata[5]; break;
		case 0xbd: P0 = leddata[6]; break;
		case 0x7d: P0 = leddata[7]; break;
		
		case 0xeb: P0 = leddata[8]; break;
		case 0xdb: P0 = leddata[9]; break;
		case 0xbb: P0 = leddata[10]; break;
		case 0x7b: P0 = leddata[11]; break;
		
		case 0xe7: P0 = leddata[12]; break;
		case 0xd7: P0 = leddata[13]; break;
		case 0xb7: P0 = leddata[14]; break;
		case 0x77: P0 = leddata[15]; break;
	}
	if(!KeyScan()){
		P3 = P3 | 0x0f;
		switch(P3 & 0x0f){
			case 0x0e: P0 = leddata[16]; break;
			case 0x0d: P0 = leddata[17]; break;
			case 0x0b: P0 = leddata[18]; break;
			case 0x07: P0 = leddata[19]; break;
		}
	}
}

void main()
{
	we = 1;
	P0 = 0;
	we = 0;
	
	du = 1;
	P0 = 0x40;
	while(1){
		KeyPro();
	}
}