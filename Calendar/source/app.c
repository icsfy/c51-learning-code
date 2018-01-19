#include "config.h"
#include "app.h"
#include "main.h"
#include "LCD1602.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "Key_Buzzer.h"
#include "lunar.h"

bit alarm_on = 0;         // 闹钟开关标志位
uint8 s_index = 0;        // 设置位索引
struct sTime cur_time;    // 当前日期时间
uint8 alarm_min = 0x30;   // 闹钟时间的分钟数
uint8 alarm_hour = 0x06;  // 闹钟时间的小时数
uint8 pdata set_dat;      // 设置时间日期数据缓存

/*星期显示字符*/
uint8 code *week[8] = {
  "    ", "Mon ", "Tues", "Wed ", "Thur", "Fri ", "Sat ", "Sun "
};

/*闹钟开关显示字符表*/
uint8 code *AlarmSta[2] = { "OFF", "ON " };

/*十进制转BCD码*/
uchar DEC2BCD(uchar dec) { return (dec / 10 * 16) + (dec % 10); }

/*BCD码转十进制*/
uchar BCD2DEC(uchar bcd) { return (bcd / 16 * 10) + (bcd % 16); }

/*刷新时间数据并显示*/
void RefreshTime() {
  uint8 pdata time[10];
  GetRealTime(&cur_time);                   // 获取当前日期/时间
  time[0] = (cur_time.hour >> 4) + '0';     // 小时十位
  time[1] = (cur_time.hour & 0x0f) + '0';   // 小时个位
  time[2] = ':';
  time[3] = (cur_time.min >> 4) + '0';      // 分钟十位
  time[4] = (cur_time.min & 0x0f) + '0';    // 分钟个位
  time[5] = ':';
  time[6] = (cur_time.sec >> 4) + '0';      // 秒钟十位
  time[7] = (cur_time.sec & 0x0f) + '0';    // 秒钟个位
  time[8] = '\0';
  LCD1602_Dis_Str(0, 1, time);              // 显示时间
}

/*刷新日期数据并显示*/
void RefreshDate(uint8 ops) {
  uint8 pdata date[12];
  static uint8 backup = 0;
  if ((backup != cur_time.day) || (ops != 0)) {
    date[0] = ((cur_time.year >> 12) & 0x0f) + '0';  // 年份千位
    date[1] = ((cur_time.year >> 8) & 0x0f) + '0';   // 年份百位
    date[2] = ((cur_time.year >> 4) & 0x0f) + '0';   // 年份十位
    date[3] = ((cur_time.year) & 0x0f) + '0';        // 年份个位
    date[4] = '-';
    date[5] = (cur_time.mon >> 4) + '0';             // 月份十位
    date[6] = (cur_time.mon & 0x0f) + '0';           // 月份个位
    date[7] = '-';
    date[8] = (cur_time.day >> 4) + '0';             // 日期十位
    date[9] = (cur_time.day & 0x0f) + '0';           // 日期个位
    date[10] = '\0';
    LCD1602_Dis_Str(0, 0, date);                     // 显示日期
    LCD1602_Dis_Str(12, 0, week[cur_time.week]);     // 显示星期
    backup = cur_time.day;
  }
}

/*刷新阴历*/
void RefreshLunar() {
  uint8 pdata date[12];
  GetRealTime(&cur_time);                           // 获取当前日期/时间
  Conversion(0, cur_time.year, cur_time.mon, cur_time.day);
  date[0] = 0x02 + '0';                             // 年份千位
  date[1] = 0x00 + '0';                             // 年份百位
  date[2] = ((SpDate.year >> 4) & 0x0f) + '0';      // 年份十位
  date[3] = (SpDate.year & 0x0f) + '0';             // 年份个位
  date[4] = '-';
  date[5] = (SpDate.mon >> 4) + '0';                // 月份十位
  date[6] = (SpDate.mon & 0x0f) + '0';              // 月份个位
  date[7] = '-';
  date[8] = (SpDate.day >> 4) + '0';                // 日期十位
  date[9] = (SpDate.day & 0x0f) + '0';              // 日期个位
  date[10] = '\0';
  LCD1602_Dis_Str(0, 0, "Lunar");                   // 阴历字符
  LCD1602_Dis_Str(6, 0, date);                      // 阴历
}

/*监控闹钟*/
void AlarmMonitor() {
  staBuzzer(alarm_on &&
            (cur_time.hour == alarm_hour) && (cur_time.min == alarm_min));
}

/*刷新闹钟显示*/
void RefreshAlarm() {
  uint8 pdata str[6];
  str[0] = (alarm_hour >> 4) + '0';            // 闹钟小时数
  str[1] = (alarm_hour & 0x0f) + '0';
  str[2] = ':';
  str[3] = (alarm_min >> 4) + '0';             // 闹钟分钟数
  str[4] = (alarm_min & 0x0f) + '0';
  str[5] = '\0';
  LCD1602_Dis_Str(0, 1, "Alarm ");             // 显示提示标题
  LCD1602_Dis_Str(13, 1, AlarmSta[alarm_on]);  // 显示闹钟状态
  LCD1602_Dis_Str(6, 1, str);                  // 显示闹钟时间
}

/*S2按键功能函数*/
void SetShift() {
  if (staSystem == E_SET_TIME) {                               // 设置时间光标
    switch (s_index) {
      case 0: s_index = 1;  LCD1602_Set_Cursor(7, 1);  break;  // 光标位于秒钟处
      case 1: s_index = 2;  LCD1602_Set_Cursor(4, 1);  break;  // 光标位于分钟处
      case 2: s_index = 3;  LCD1602_Set_Cursor(1, 1);  break;  // 光标位于小时处
      case 3: s_index = 4;  LCD1602_Set_Cursor(12, 0); break;  // 光标位于星期处
      case 4: s_index = 5;  LCD1602_Set_Cursor(9, 0);  break;  // 光标位于日处
      case 5: s_index = 6;  LCD1602_Set_Cursor(6, 0);  break;  // 光标位于月处
      default: s_index = 0; LCD1602_Set_Cursor(3, 0);  break;  // 光标位于年处
    }
  } else if (staSystem == E_SET_ALARM) {                       // 设置闹钟光标
    switch (s_index) {
      case 0: s_index = 1; LCD1602_Set_Cursor(13, 1); break;   // 光标位于闹钟开关处
      case 1: s_index = 2; LCD1602_Set_Cursor(10, 1); break;   // 光标位于闹钟分钟处
      default: s_index = 0; LCD1602_Set_Cursor(7, 1); break;   // 光标位于闹钟小时处
    }
  } else if (staSystem == E_NORMAL) {     // 正常运行状态下切换为显示阴历/闹钟
    staSystem = E_DIS_ALARM;              // 把系统状态更新为显示阴历/闹钟
    LCD1602_Clear_Screen();               // 清屏
    RefreshLunar();                       // 显示阴历
    RefreshAlarm();                       // 显示闹钟
  } else if (staSystem == E_DIS_ALARM) {  // 显示阴历/闹钟状态下切回显示公历/时间
    staSystem = E_NORMAL;                 // 把系统状态更新为显示公历/时间
    LCD1602_Clear_Screen();               // 清屏
    RefreshTime();                        // 刷新时间
    RefreshDate(1);                       // 立即刷新日期
    RefreshTemp(1);                       // 立即刷新温度
  }
}

/*S5按键功能函数 切换系统运行状态*/
void SwitchSystemStatus() {
  if (staSystem == E_NORMAL) {            // 正常运行切换到时间设置
    staSystem = E_SET_TIME;               // 系统状态更新为设置时间
    s_index = 0;                          // 光标索引设置到秒钟上
    SetShift();                           // 显示光标位置
    LCD1602_Open_Cursor();                // LCD1602打开光标显示
  } else if (staSystem == E_DIS_ALARM) {  // 显示阴历/闹钟时 切换到设置闹钟
    staSystem = E_SET_ALARM;              // 系统状态更新为设置闹钟
    s_index = 0;                          // 光标索引设置到秒钟上
    SetShift();                           // 显示光标位置
    LCD1602_Open_Cursor();                // LCD1602打开光标显示
  } else if (staSystem == E_SET_TIME) {   // 设置公历/时钟状态下
    staSystem = E_NORMAL;                 // 系统状态更新为正常运行
    SetRealTime(&cur_time);               // 把设定时间写入实时时钟
    LCD1602_Close_Cursor();               // LCD1602关闭光标显示
    LCD1602_Clear_Screen();               // LCD1602清屏
    RefreshTime();                        // 刷新时间
    RefreshDate(1);                       // 立即刷新日期
    RefreshTemp(1);                       // 立即刷新温度
  } else {                                // 设置闹钟状态下
    staSystem = E_NORMAL;                 // 系统状态更新为正常运行
    RefreshAlarm();                       // 写入闹钟设定值
    LCD1602_Close_Cursor();               // LCD1602关闭光标显示
    LCD1602_Clear_Screen();               // LCD1602清屏
    RefreshTime();                        // 刷新时间
    RefreshDate(1);                       // 立即刷新日期
    RefreshTemp(1);                       // 立即刷新温度
  }
}

/*S3按键功能函数 数据加*/
void SetDateAlarmS3() {
  if (staSystem == E_SET_TIME) {          // 设置公历/时间
    switch (s_index) {
      case 1: {
        set_dat = BCD2DEC(cur_time.sec);  // 把BCD码转为十进制
        set_dat = (set_dat + 1) % 60;     // 限制秒钟不超过59秒
        cur_time.sec = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(6, 1, (cur_time.sec >> 4) + '0');
        LCD1602_Dis_Char(7, 1, (cur_time.sec & 0x0f) + '0');
        LCD1602_Set_Cursor(7, 1);         // 光标保持原位
        break;
      }
      case 2: {
        set_dat = BCD2DEC(cur_time.min);  // 把BCD码转为十进制
        set_dat = (set_dat + 1) % 60;     // 限制分钟不超过59分
        cur_time.min = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(3, 1, (cur_time.min >> 4) + '0');
        LCD1602_Dis_Char(4, 1, (cur_time.min & 0x0f) + '0');
        LCD1602_Set_Cursor(4, 1);         // 光标保持原位
        break;
      }
      case 3: {
        set_dat = BCD2DEC(cur_time.hour);  // 把BCD码转为十进制
        set_dat = (set_dat + 1) % 24;      // 限制小时不超过23时
        cur_time.hour = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(0, 1, (cur_time.hour >> 4) + '0');
        LCD1602_Dis_Char(1, 1, (cur_time.hour & 0x0f) + '0');
        LCD1602_Set_Cursor(1, 1);          // 光标保持原位
        break;
      }
      case 4: {                           // 限制星期不超过7
        cur_time.week = (cur_time.week % 7) + 1;
        LCD1602_Dis_Str(12, 0, week[cur_time.week]);
        LCD1602_Set_Cursor(12, 0);        // 光标保持原位
        break;
      }
      case 5: {
        set_dat = BCD2DEC(cur_time.day);  // 把BCD码转为十进制
        set_dat = (set_dat % 31) + 1;     // 限制日期不超过31日
        cur_time.day = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(8, 0, (cur_time.day >> 4) + '0');
        LCD1602_Dis_Char(9, 0, (cur_time.day & 0x0f) + '0');
        LCD1602_Set_Cursor(9, 0);         // 光标保持原位
        break;
      }
      case 6: {
        set_dat = BCD2DEC(cur_time.mon);  // 把BCD码转为十进制
        set_dat = (set_dat % 12) + 1;     // 限制月份不超过12月
        cur_time.mon = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(5, 0, (cur_time.mon >> 4) + '0');
        LCD1602_Dis_Char(6, 0, (cur_time.mon & 0x0f) + '0');
        LCD1602_Set_Cursor(6, 0);         // 光标保持原位
        break;
      }
      case 0: {
        set_dat = BCD2DEC(cur_time.year);  // 把BCD码转为十进制
        set_dat = (set_dat + 1) % 100;     // 限制年不超过99年
        cur_time.year = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(2, 0, (cur_time.year >> 4) + '0');
        LCD1602_Dis_Char(3, 0, (cur_time.year & 0x0f) + '0');
        LCD1602_Set_Cursor(3, 0);          // 光标保持原位
        break;
      }
    }
  } else if (staSystem == E_SET_ALARM) {  // 设置闹钟
    switch (s_index) {
      case 1: {
        alarm_on = ~alarm_on;
        LCD1602_Dis_Str(13, 1, AlarmSta[alarm_on]);
        LCD1602_Set_Cursor(13, 1);        // 光标保持原位
        break;
      }
      case 2: {
        set_dat = BCD2DEC(alarm_min);     // 把BCD码转为十进制
        set_dat = (set_dat + 1) % 60;     // 限制设置分钟不超过59分
        alarm_min = DEC2BCD(set_dat);     // 把十进制转为BCD码
        LCD1602_Dis_Char(9, 1, (alarm_min >> 4) + '0');
        LCD1602_Dis_Char(10, 1, (alarm_min & 0x0f) + '0');
        LCD1602_Set_Cursor(10, 1);        // 光标保持原位
        break;
      }
      case 0: {
        set_dat = BCD2DEC(alarm_hour);    // 把BCD码转为十进制
        set_dat = (set_dat + 1) % 24;     // 限制设置小时不超过23时
        alarm_hour = DEC2BCD(set_dat);    // 把十进制转为BCD码
        LCD1602_Dis_Char(6, 1, (alarm_hour >> 4) + '0');
        LCD1602_Dis_Char(7, 1, (alarm_hour & 0x0f) + '0');
        LCD1602_Set_Cursor(7, 1);         // 光标保持原位
        break;
      }
    }
  }
}

/*S4按键功能函数 数据减*/
void SetDateAlarmS4() {
  if (staSystem == E_SET_TIME) {          // 设置公历/时间
    switch (s_index) {
      case 1: {
        set_dat = BCD2DEC(cur_time.sec);  // 把BCD码转为十进制
        set_dat = (set_dat + 59) % 60;    // 秒钟减1限制为0~59秒
        cur_time.sec = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(6, 1, (cur_time.sec >> 4) + '0');
        LCD1602_Dis_Char(7, 1, (cur_time.sec & 0x0f) + '0');
        LCD1602_Set_Cursor(7, 1);         // 光标保持原位
        break;
      }
      case 2: {
        set_dat = BCD2DEC(cur_time.min);  // 把BCD码转为十进制
        set_dat = (set_dat + 59) % 60;    // 分钟减1限制为0~59分
        cur_time.min = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(3, 1, (cur_time.min >> 4) + '0');
        LCD1602_Dis_Char(4, 1, (cur_time.min & 0x0f) + '0');
        LCD1602_Set_Cursor(4, 1);         // 光标保持原位
        break;
      }
      case 3: {
        set_dat = BCD2DEC(cur_time.hour);  // 把BCD码转为十进制
        set_dat = (set_dat + 23) % 24;     // 小时减1限制为0~23时
        cur_time.hour = DEC2BCD(set_dat);  // 把十进制转为BCD码
        LCD1602_Dis_Char(0, 1, (cur_time.hour >> 4) + '0');
        LCD1602_Dis_Char(1, 1, (cur_time.hour & 0x0f) + '0');
        LCD1602_Set_Cursor(1, 1);          // 光标保持原位
        break;
      }
      case 4: {                           // 星期减1限制为1~7
        cur_time.week = (cur_time.week + 5) % 7 + 1;
        LCD1602_Dis_Str(12, 0, week[cur_time.week]);
        LCD1602_Set_Cursor(12, 0);        // 光标保持原位
        break;
      }
      case 5: {
        set_dat = BCD2DEC(cur_time.day);    // 把BCD码转为十进制
        set_dat = (set_dat + 29) % 31 + 1;  // 日期减1限制为1~31日
        cur_time.day = DEC2BCD(set_dat);    // 把十进制转为BCD码
        LCD1602_Dis_Char(8, 0, (cur_time.day >> 4) + '0');
        LCD1602_Dis_Char(9, 0, (cur_time.day & 0x0f) + '0');
        LCD1602_Set_Cursor(9, 0);           // 光标保持原位
        break;
      }
      case 6: {
        set_dat = BCD2DEC(cur_time.mon);    // 把BCD码转为十进制
        set_dat = (set_dat + 10) % 12 + 1;  // 月份减1限制为1~12月
        cur_time.mon = DEC2BCD(set_dat);    // 把十进制转为BCD码
        LCD1602_Dis_Char(5, 0, (cur_time.mon >> 4) + '0');
        LCD1602_Dis_Char(6, 0, (cur_time.mon & 0x0f) + '0');
        LCD1602_Set_Cursor(6, 0);           // 光标保持原位
        break;
      }
      case 0: {
        set_dat = BCD2DEC(cur_time.year);   // 把BCD码转为十进制
        set_dat = (set_dat + 99) % 100;     // 年减1限制为0~99年
        cur_time.year = DEC2BCD(set_dat);   // 把十进制转为BCD码
        LCD1602_Dis_Char(2, 0, (cur_time.year >> 4) + '0');
        LCD1602_Dis_Char(3, 0, (cur_time.year & 0x0f) + '0');
        LCD1602_Set_Cursor(3, 0);           // 光标保持原位
        break;
      }
    }
  } else if (staSystem == E_SET_ALARM) {  // 设置闹钟
    switch (s_index) {
      case 1: {
        alarm_on = ~alarm_on;
        LCD1602_Dis_Str(13, 1, AlarmSta[alarm_on]);
        LCD1602_Set_Cursor(13, 1);        // 光标保持原位
        break;
      }
      case 2: {
        set_dat = BCD2DEC(alarm_min);     // 把BCD码转为十进制
        set_dat = (set_dat + 59) % 60;    // 设置分钟减1限制为0~59分
        alarm_min = DEC2BCD(set_dat);     // 把十进制转为BCD码
        LCD1602_Dis_Char(9, 1, (alarm_min >> 4) + '0');
        LCD1602_Dis_Char(10, 1, (alarm_min & 0x0f) + '0');
        LCD1602_Set_Cursor(10, 1);        // 光标保持原位
        break;
      }
      case 0: {
        set_dat = BCD2DEC(alarm_hour);    // 把BCD码转为十进制
        set_dat = (set_dat + 23) % 24;    // 设置小时减1限制为0~23时
        alarm_hour = DEC2BCD(set_dat);    // 把十进制转为BCD码
        LCD1602_Dis_Char(6, 1, (alarm_hour >> 4) + '0');
        LCD1602_Dis_Char(7, 1, (alarm_hour & 0x0f) + '0');
        LCD1602_Set_Cursor(7, 1);         // 光标保持原位
        break;
      }
    }
  }
}

/*按键驱动*/
void KeyAction(uint8 keycode) {
  switch (keycode) {
    case 0x0d: {  // 按下S2 显示切换键 设置状态下为设置左移键
      SetShift();
      break;
    }
    case 0x26: {  // 按下S3 数据加
      SetDateAlarmS3();
      break;
    }
    case 0x28: {  // 按下S4 数据减
      SetDateAlarmS4();
      break;
    }
    case 0x1b: {  // 按下S5 设置键
      SwitchSystemStatus();
      break;
    }
  }
}
