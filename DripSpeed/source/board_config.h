#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

#include <reg52.h>
#include <intrins.h>

typedef unsigned char uchar;  // 8位无符号型
typedef unsigned int uint;    // 16位无符号型

/* 8个LED灯 */
sbit LED1 = P1^0;
sbit LED2 = P1^1;
sbit LED3 = P1^2;
sbit LED4 = P1^3;
sbit LED5 = P1^4;
sbit LED6 = P1^5;
sbit LED7 = P1^6;
sbit LED8 = P1^7;

/* 七段数码管 */
//#define SEVEN_SEGMENT_COMMON_ANODE  // 共阳极七段数码管
#define SEVEN_SEGMENT_COMMON_CATHODE  // 共阴极七段数码管
#define DIGITAL_DB P0         // 数据线
sbit WELA = P2^7;             // 位选端
sbit DULA = P2^6;             // 段选端

/* 5V有源蜂鸣器 */
sbit Buzzer = P2^3;           // 蜂鸣器引脚

#endif /* __BOARD_CONFIG_H */
