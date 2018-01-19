#include "config.h"
#include "main.h"
#include "app.h"
#include "LCD1602.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "Key_Buzzer.h"

#define _MAIN_C

bit flag200ms = 0;                      // 200ms定时标志
bit flag1s = 0;                         // 1s定时标志
enum eStaSystem staSystem = E_NORMAL;   // 系统运行状态

/*刷新温度*/
void RefreshTemp(uint8 ops) {
  int16 temp;
  uint8 pdata str[8];
  static int16 backup = 0;
  Get18B20Temp(&temp);                    // 获取当前温度值
  Start18B20();                           // 启动下一次转换
  temp >>= 4;                             // 舍弃4bit小数位
  if ((backup != temp) || (ops != 0)) {   // 按需刷新液晶显示
    str[0] = (temp / 10) + '0';
    str[1] = (temp % 10) + '0';
    str[2] = '\'';
    str[3] = 'C';
    str[4] = '\0';
    LCD1602_Dis_Str(12, 1, str);
    backup = temp;
  }
}

/*初始化定时器0*/
void Init_Timer0() {
  TMOD = 0x01;  // 定时器0 16位计数工作模式
  TL0 = 0x00;
  TH0 = 0xee;   // 5ms
  ET0 = 1;      // 开定时器0中断
  TR0 = 1;      // 启动定时器0
}

void main() {
  EA = 1;                     // 开总中断
  Start18B20();               // 启动温度转换
  Init_Timer0();              // 初始化定时器0
  Init_LCD1602();             // 初始化LCD1602
  Init_DS1302();              // 初始化DS1302
  while (!flag1s) continue;   // 上电后延时1s
  flag1s = 0;
  RefreshTime();              // 刷新时间
  RefreshDate(1);             // 立即刷新日期
  RefreshTemp(1);             // 立即刷新温度显示
  while (1) {
    KeyDriver();              // 按键检测
    if (flag200ms) {
      flag200ms = 0;          // 清零标志位
      AlarmMonitor();         // 监控闹钟
      if (staSystem == E_NORMAL) {
        RefreshTime();        // 刷新时间
        RefreshDate(0);       // 刷新日期
      }
      if (staSystem == E_DIS_ALARM) {
        RefreshLunar();       // 刷新阴历
      }
    }
    if (flag1s && (staSystem == E_NORMAL)) {
      flag1s = 0;             // 清零标志位
      RefreshTemp(0);         // 刷新温度
    }
  }
}

/*定时器0中断服务程序*/
void Timer0() interrupt 1
{
  static uint8 tmr1s = 0;
  static uint8 tmr200ms = 0;
  TL0 = 0x00;
  TH0 = 0xee;               // 5ms
  tmr200ms++;               // 定时200ms
  if (tmr200ms >= 40) {
    tmr200ms = 0;
    flag200ms = 1;
    tmr1s++;                // 定时1s
    if (tmr1s >= 5) {
      tmr1s = 0;
      flag1s = 1;
    }
  }
  KeyScan();                // 按键扫描
}
