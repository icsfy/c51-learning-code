#ifndef _APP_H_
#define _APP_H_

uchar DEC2BCD(uchar dec);       // 十进制转BCD码
uchar BCD2DEC(uchar bcd);       // BCD码转十进制
void RefreshTime();             // 刷新时间数据并显示
void RefreshDate(uint8 ops);    // 刷新日期数据并显示
void KeyAction(uint8 keycode);  // 按键驱动
void AlarmMonitor();            // 监控闹钟
void RefreshLunar();            // 刷新阴历

#endif  // _APP_H_
