#include <reg52.h>

typedef unsigned int uint;
typedef unsigned char uchar;

sbit WE = P2^7; // 数码管位选
sbit DU = P2^6; // 数码管段选
sbit TSCLK = P1^0; // 时钟线
sbit TIO   = P1^1; // 数据线
sbit TRST  = P1^2; // 使能端

// 数码管段选值
uchar code table[] = {
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
  //"A"   "B"   "C"   "D"   "E"   "F"   "-"   "."  "OFF"
  0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x40, 0x80, 0x00
};

// 数码管位选值    //第1位   2位   3位   4位   5位   6位   7位   8位
uchar code T_COM[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};

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
void Display(uchar Hour, Min, Sec)
{
  display_n(0, Hour / 10, 0);
  display_n(1, Hour % 10, 1);
  display_n(2, Min / 10, 0);
  display_n(3, Min % 10, 1);
  display_n(4, Sec / 10, 0);
  display_n(5, Sec % 10, 0);
}

void Write_DS1302_DAT(uchar cmd, uchar dat)
{
  uchar i;
  TRST = 0;
  TSCLK = 0;
  TRST = 1;
  for(i = 0; i < 8; i++){
    TSCLK = 0;
    TIO = cmd & 0x01;
    TSCLK = 1;
    cmd >>= 1;
  }
  for(i = 0; i < 8; i++){
    TSCLK = 0;
    TIO = dat & 0x01;
    TSCLK = 1;
    dat >>= 1;
  }
}

uchar Read_DS1302_DAT(uchar cmd)
{
  uchar i, dat;
  TRST = 0;
  TSCLK = 0;
  TRST = 1;
  for(i = 0; i < 8; i++){
    TSCLK = 0;
    TIO = cmd & 0x01;
    TSCLK = 1;
    cmd >>= 1;
  }
  for(i = 0; i < 8; i++){
    TSCLK = 0;
    dat >>= 1;
    if(TIO)
      dat |= 0x80;
    TSCLK = 1;
  }
  return dat;
}

uchar dec2bcd(uchar dec)
{
  return ((dec / 10) * 16 + (dec % 10));
}

uchar bcd2dec(uchar bcd)
{
  return ((bcd / 16) * 10 + (bcd % 16));
}

void main()
{
  uchar i;
  uchar Sec, Min, Hour;
  Write_DS1302_DAT(0x8e, 0); // 清除写保护
  Write_DS1302_DAT(0x80, dec2bcd(30)); // 30秒
  Write_DS1302_DAT(0x82, dec2bcd(15)); // 15分
  Write_DS1302_DAT(0x84, dec2bcd(19)); // 19时
  Write_DS1302_DAT(0x8e, 0x80); // 开写保护
  while(1){
    Write_DS1302_DAT(0x8e, 0); // 清除写保护
    Sec = bcd2dec(Read_DS1302_DAT(0x81)); // 读秒寄存器
    Min = bcd2dec(Read_DS1302_DAT(0x83)); // 读分寄存器
    Hour = bcd2dec(Read_DS1302_DAT(0x85)); // 读时寄存器
    Write_DS1302_DAT(0x8e, 0x80); // 开写保护
    for(i = 0; i < 50; i++){
      Display(Hour, Min, Sec);
    }
  }
}