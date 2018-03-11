#include "digital_display.h"

#ifdef SEVEN_SEGMENT_COMMON_ANODE     /* 共阳极 */
uchar code table[]={
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
  0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x7f, 0xff
};
#endif
#ifdef SEVEN_SEGMENT_COMMON_CATHODE   /* 共阴极 */
uchar code table[] = {
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x80, 0x00
};
#endif

// 数码管位选值    //  第1位   2位   3位   4位   5位   6位   7位   8位
uchar code T_COM[] = {0xfe, 0xfd, 0xfb, 0xf7};//, 0xef, 0xdf, 0xbf, 0x7f};

static void delay(uchar t) { while(t--); }

// 显示第n个数码管，值为num, 小数点dot
static void display_n(uchar n, num)
{
  DULA = WELA = 0;  // 保证段选与位选关闭
  DIGITAL_DB = T_COM[n];  // 位选值
  WELA = 1; // 打开位选
  WELA = 0; // 关闭位选
  DIGITAL_DB = table[num];  // 段选值
//  if(dot) DIGITAL_DB |= table[10]; // 小数点
  DULA = 1; // 打开段选
  DULA = 0; // 关闭段选
  delay(1); // 可以调整数码管亮度
  // 关闭数码管
  DIGITAL_DB = table[11];
  DULA = 1;
  DULA = 0;
}

/* 数码管 需动态显示 */
void Display(uint num)
{
  display_n(0, num/1000);
  display_n(1, num/100%10);
  display_n(2, num/10%10);
  display_n(3, num%10);
}
