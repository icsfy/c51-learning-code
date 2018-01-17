#include <reg52.h>
#include <intrins.h>

typedef unsigned int uint;
typedef unsigned char uchar;

sbit WE = P2^7; // 数码管位选
sbit DU = P2^6; // 数码管段选
sbit DS = P2^2; // DSP18B20温度传感器

// 数码管段选值
uchar code table[] = {
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
  //"A"   "B"   "C"   "D"   "E"   "F"   "-"   "."  "OFF"
  0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x40, 0x80, 0x00
};

// 数码管位选值    //第1位   2位   3位   4位   5位   6位   7位   8位
uchar code T_COM[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};

/*us延时*/
void delay_us(uchar us)
{
  while(us--);
}

// 显示第n个数码管，值为num, 小数点dot
void display_n(uchar n, num, dot)
{
  DU = WE = 0;  // 保证段选与位选关闭
  P0 = T_COM[n];  // 位选值
  WE = 1; // 打开位选
  WE = 0; // 关闭位选
  P0 = table[num];  // 段选值
  if(dot) P0 |= 0x80; // 小数点
  DU = 1; // 打开段选
  DU = 0; // 关闭段选

  // 关闭数码管
  P0 = table[18];
  DU = 1;
  DU = 0;
}

// 数码管显示
void Display(uint num)
{
  display_n(0, num/100, 0);
  display_n(1, num/10%10, 1);
  display_n(2, num%10, 0);
}

bit DS_init()
{
  bit i;
  DS = 1;
  _nop_();
  DS = 0;
  delay_us(75); // 492.79us
  DS = 1;
  delay_us(5); // 44.09us
  i = DS;
  delay_us(12); // 88.96us
  return ~i;
}

void write_byte(uchar dat)
{
  uchar i = 8;
  while(i--){
    DS = 0;
    _nop_();
    DS = dat & 0x01;
    delay_us(12); // 88.96us
    DS = 1;
    _nop_();
    dat >>= 1;
  }
}

uchar read_byte()
{
  uchar tmp, dat, i = 8;
  while(i--){
    DS = 0;
    _nop_();
    DS = 1;
    delay_us(1); // 18.45us
    tmp = DS;
    delay_us(12); // 88.96us
    dat = (tmp << 7)|(dat>>1);
    _nop_();
  }
  return dat;
}

void main()
{
  uchar i, LSB, MSB;
  uint temp;
  //delay_us(1); // 44054-42209=18.45us;  18.45-6.41=12.04us;
  //delay_us(2); // 46549-44054=24.95us;  24.95-18.54=6.41us;
  //_nop_();     // 46658-46549=1.09us;

  /*
  if(!DS_init())
      P1 = 0;
  write_byte(0xcc); // 忽略ROM
  write_byte(0x4e); // 写暂存器
  write_byte(0x7f); // 写入TH
  write_byte(0xf7); // 写入TL
  write_byte(0x7f); // 写入配置寄存器 9-bit(0x1f) 10-bit(0x3f) 11-bit(0x5f) 12-bit(0x7f)
  if(!DS_init())
      P1 = 0;
  write_byte(0xcc); // 忽略ROM
  write_byte(0x48); // 拷贝寄存器至EEPROM
  //*/
  while(1){
    if(!DS_init())
      P1 = 0;
    write_byte(0xcc); // 忽略ROM
    write_byte(0x44); // 温度转换
    if(!DS_init())
      P1 = 0;
    write_byte(0xcc); // 忽略ROM
    write_byte(0xbe); // 读暂存器
    LSB = read_byte();
    MSB = read_byte();
    temp = MSB;
    temp = (temp<<8)|LSB;
    temp = temp * 0.0625 * 10 + 0.5;
    for(i=50;i;i--) Display(temp);
  }
}