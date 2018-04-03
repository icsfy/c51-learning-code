#include "reg52.h"
#include "stdio.h"

//#define DEBUG_ON          //串口打印输出调试

//#define DEBUG_FIXTIME     //测试时间补偿

/* 晶振频率 */
#define MAIN_Fosc   11059200UL
//#define MAIN_Fosc   12000000UL

/* 七段数码管 */
#define DIGITAL_DB P0         // 数据线
sbit WELA = P2^7;             // 位选端
sbit DULA = P2^6;             // 段选端

/* 50ms定时器初值 */
#if MAIN_Fosc == 11059200UL
  #define T50MS_TL      0x00
  #define T50MS_TH      0x4C
  #define T50MS_VALUE   0x4C00
#elif MAIN_Fosc == 12000000UL
  #define T50MS_TL      0xB0
  #define T50MS_TH      0x3C
  #define T50MS_VALUE   0x3CB0
#endif

/* 定时器每计数一次的时间(us) */
#define TIM_PER_CNT     (12000000.0/MAIN_Fosc)

#ifdef DEBUG_ON
#include "uart.h"
#endif

typedef struct {
  char cnt50ms;
  int cnt1s;
  long remain;
} RecordType;

RecordType lastRec = {0, 0, 0};   //上一次记录值
RecordType newRec = {0, 0, 0};    //最新记录值

bit finish_flag = 0;      //脉宽捕获完成标志位
float pulse_width = 0;    //存储脉宽值

///* 共阳极七段数码管 */
//unsigned char code table[]={
//  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
//  0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x7f, 0xff
//};

/* 共阴极七段数码管 */
unsigned char code table[] = {
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x80, 0x00
};

/* 数码管位选值 */
unsigned char code T_COM[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};

void display_n(char n, num, dot);
void delay(char t) { while(t--); }

/* 16位定时器，50毫秒
 * 当 INT0(P3.2) 脚为高且 TR0=1 时定时器计时
 */
void Timer0Init(void)
{
  TMOD &= 0xF0;       //设置定时器模式
#ifdef DEBUG_FIXTIME
  TMOD |= 0x01;       //设置定时器模式
#else
  TMOD |= 0x09;       //设置定时器模式
#endif
  TL0 = T50MS_TL;     //设置定时初值
  TH0 = T50MS_TH;     //设置定时初值
  TF0 = 0;            //清除TF1标志
  TR0 = 1;            //定时器1开始计时
}

/* 主函数，脉宽计算和显示 */
void main(void)
{
  char len, buff[10], i;
#ifdef DEBUG_ON
  UartInit();
#endif
  Timer0Init();
  IT0 = 1;    //设置外部中断0触发方式 (1:下降沿 0:低电平)
  EX0 = 1;    //使能外部中断0中断请求
  ET0 = 1;    //使能定时器0中断请求
  EA = 1;     //打开全局中断开关
  
  while(1) {
    if (finish_flag) {
      /* 取出 TL0 和 TH0 里面的值 */
      newRec.remain = ((TH0 << 8) | TL0) - T50MS_VALUE;
      /* 脉宽 = 最新计时值 - 上次计时值 */
      pulse_width = (newRec.remain*TIM_PER_CNT + newRec.cnt50ms*50000 + newRec.cnt1s*1000000) -
          (lastRec.remain*TIM_PER_CNT + lastRec.cnt50ms*50000 + lastRec.cnt1s*1000000);
      len = sprintf(buff, "%.3f", pulse_width/1000.0);
#ifdef DEBUG_ON
//      if (pulse_width <= 0) {
        printf("new: cnt1s: %d, cnt50ms: %bd, remain: %ld\n"
               "rec: cnt1s: %d, cnt50ms: %bd, remain: %ld\n",
               newRec.cnt1s, newRec.cnt50ms, newRec.remain,
               lastRec.cnt1s, lastRec.cnt50ms, lastRec.remain);
//      }
      printf("pulse width: %f\n", pulse_width);
#endif
      lastRec = newRec;
      finish_flag = 0;
    }
    /* 低电平时允许中断，准备开始捕获脉冲 */
    if (INT0 == 0) {
      EA = 1;
    }
#ifdef DEBUG_ON
    if (len > 9)
      printf("len: %bd, buff: %s\n", len, buff);
#endif
//    /* 显示方式1：4位显示 */
//    switch (len) {
//      case 5:   // "1.234"
//        display_n(0, buff[0] - '0', 1);
//        display_n(1, buff[2] - '0', 0);
//        display_n(2, buff[3] - '0', 0);
//        display_n(3, buff[4] - '0', 0);
//      case 6:   // "12.345"
//        display_n(0, buff[0] - '0', 0);
//        display_n(1, buff[1] - '0', 1);
//        display_n(2, buff[3] - '0', 0);
//        display_n(3, buff[4] - '0', 0);
//      case 7:   // "123.456"
//        display_n(0, buff[0] - '0', 0);
//        display_n(1, buff[1] - '0', 0);
//        display_n(2, buff[2] - '0', 1);
//        display_n(3, buff[4] - '0', 0);
//      case 8:   // "1234.567"
//        display_n(0, buff[0] - '0', 0);
//        display_n(1, buff[1] - '0', 0);
//        display_n(2, buff[2] - '0', 0);
//        display_n(3, buff[3] - '0', 1);
//    }
//    /* 显示方式2：4位显示 */
//    display_n(0, buff[0] - '0', len == 5);
//    display_n(1, buff[len > 5 ? 1 : 2] - '0', len == 6);
//    display_n(2, buff[len > 6 ? 2 : 3] - '0', len == 7);
//    display_n(3, buff[len > 7 ? 3 : 4] - '0', len == 8);
//    /* 显示方式3：8位显示 */
//    display_n(0, (long)pulse_width / 10000000, 0);
//    display_n(1, (long)pulse_width / 1000000 % 10, 0);
//    display_n(2, (long)pulse_width / 100000 % 10, 0);
//    display_n(3, (long)pulse_width / 10000 % 10, 0);
//    display_n(4, (long)pulse_width / 1000 % 10, 1);
//    display_n(5, (long)pulse_width / 100 % 10, 0);
//    display_n(6, (long)pulse_width / 10 % 10, 0);
//    display_n(7, (long)pulse_width % 10, 0);
    /* 显示方式4：自动位数显示 */
    for (i = 0; i < len - 1; i++) {
      display_n(7-i, buff[len-(i>2?(i+1):i)-1] - '0', i == 3);
    }
  }
}

/* 显示第n个数码管 */
void display_n(char n, num, dot)
{
  DULA = WELA = 0;                  // 保证段选与位选关闭
  DIGITAL_DB = T_COM[n];            // 位选值
  WELA = 1;                         // 打开位选
  WELA = 0;                         // 关闭位选
  DIGITAL_DB = table[num];          // 段选值
  if (dot) DIGITAL_DB |= table[10]; // 小数点
  DULA = 1;                         // 打开段选
  DULA = 0;                         // 关闭段选
  delay(30);                         // 可以调整数码管亮度
  DIGITAL_DB = table[11];           // 关闭数码管
  DULA = 1;
  DULA = 0;
}

///* 显示第n个数码管 */
//void display_n(char n, num, dot)
//{
//  P2 = T_COM[n];                //数码管位选
//  delay(1);
//  P0 = table[num];              //数码管段选
//  if (dot) P0 |= table[10];     //小数点
//  delay(10);
//  P0 = table[11];               //关闭数码管
//}

/* 外部中断服务函数 */
void exint0() interrupt 0
{
  if (TR0 == 0) {               //定时器还没有打开
    TR0 = 1;                    //打开定时器
    return;                     //退出中断
  }
  TR0 = 0;                      //关闭定时器
  EA = 0;                       //禁止所有中断请求
  finish_flag = 1;              //捕获完成
}

/* 定时器中断服务函数
 * 当定时器计数溢出时会被调用
 * 每计时 50ms 会产生溢出，计数值加 1
 */
void tm0_isr() interrupt 1
{
  EA = 0;                       //禁止所有中断请求
#ifdef DEBUG_FIXTIME
  TR0 = 1;
#else
  TR0 = 0;                      //关闭定时器
#endif
  TL0 = T50MS_TL + 0x14;        //时间补偿
  TH0 = T50MS_TH;
  newRec.cnt50ms++;
  if (newRec.cnt50ms == 20) {   // 50ms * 20 = 1000ms
    TL0 += 0x08;                //时间补偿
    newRec.cnt50ms = 0;
    newRec.cnt1s++;
  }
  EA = 1;                       //允许所有中断请求
  TR0 = 1;                      //打开定时器
}
