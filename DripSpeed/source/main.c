#include <reg52.h>
#include "board_config.h"
#include "digital_display.h"
#include "uart.h"

#define USE_ESP8266

#ifdef USE_ESP8266
#include "esp8266.h"
//#define UPLOAD_PERIOD 1     // 每隔1秒发送一次数据
//uchar upload_time = 0;
bit flag_need_upload = 0;
#endif

#define RECORD_BUF_SIZE 5   // 记录5个值求平均值得到液滴速度
#define COUNT_TIMEOUT 5     // 5s没有液滴就报警

typedef struct {
  uchar cnt50ms;
  uint cnt1s;
  uint remain;
} RecordType;

uchar cnt50ms = 0;    // 50ms计数
uint cnt1s = 0;       // 1s计数

float speed = 0;          // 液滴速度

RecordType lastRec = {0, 0, 0};     // 上一次记录
RecordType newRec = {0, 0, 0};      // 最新记录
uint buffRec[RECORD_BUF_SIZE];      // 两次记录之间的时间间隔 ms为单位
uchar record_index = 0;             // buff记录索引
bit flag_cal_speed = 0;             // 当buff满之后可以开始计算速度
bit flag_record_updated = 0;        // 是否有新的液滴记录

void Timer0Init(void)		//50毫秒@11.0592MHz
{
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x00;		//设置定时初值
	TH0 = 0x4C;		//设置定时初值
	TF0 = 0;		//清除TF0标志
  
  ET0 = 1;    //开定时器0中断
//  EA = 1;     //总中断允许
}

//External interrupt1 service routine
void exint1() interrupt 2
{
  if (!TR0) {
    TR0 = 1;		//定时器0开始计时
  } else {
    lastRec = newRec;
    flag_record_updated = 1;
    newRec.cnt50ms = cnt50ms;
    newRec.cnt1s = cnt1s;
    newRec.remain = ((uint)TH0 << 8) + TL0 - 0x4C00;
  }
}

void timer0() interrupt 1
{
  TL0 = 0x00;		//设置定时初值
	TH0 = 0x4C;		//设置定时初值
  cnt50ms++;
  if (cnt50ms == 20)  //50ms*20=1000ms
  {
    cnt50ms = 0;
    cnt1s++;
#ifdef USE_ESP8266
//    upload_time++;
    flag_need_upload = 1;
#endif
  }
  if (cnt1s - newRec.cnt1s > COUNT_TIMEOUT) {
    speed = 0;
    Buzzer = 0;   // 蜂鸣器报警
    EA = 0;
  }
}

void main(void)
{
  uchar i;
  UartInit();       // 串口初始化
#ifdef USE_ESP8266
  ESP8266_Init();   // ESP8266初始化
#endif
  Timer0Init();     // 定时器0初始化
  
  IT1 = 1;        // 外部中断1下降沿触发
  EX1 = 1;        // 允许外部中断1中断
  EA = 1;         // 总中断允许
  
  while(1) {
    /* 数码管显示液滴速度 */
    Display(speed);
#ifdef USE_ESP8266
    /* 上传液滴速度 */
//    if (upload_time >= UPLOAD_PERIOD) {
    if (flag_need_upload) {
      ESP8266_PostJsonData(speed);
//      upload_time = 0;
      flag_need_upload = 0;
    }
#endif
    /* 更新液滴记录buffer */
    if (flag_record_updated) {
      flag_record_updated = 0;
      buffRec[record_index] = (newRec.cnt1s - lastRec.cnt1s) * 1000 +
                            (newRec.cnt50ms - lastRec.cnt50ms) * 50 +
                            (newRec.remain - lastRec.remain) * (1/(11059200.0/12)*1000);
//      buffRec[record_index] = (newRec.cnt1s*1000 + newRec.cnt50ms*50 + newRec.remain * (1/(11059200.0/12)*1000))
//          - (lastRec.cnt1s*1000 + lastRec.cnt50ms*50 + lastRec.remain * (1/(11059200.0/12)*1000));
//      P1 = ~record_index;
//      printf("\n Got buffRec[%bd]: %d\n", record_index, buffRec[record_index]);
      record_index = (record_index + 1) % RECORD_BUF_SIZE;
      if (record_index == 0) {
        flag_cal_speed = 1;
      }
      /* 更新液滴速度 */
      if (flag_cal_speed) {
        speed = 0;
        for (i = 0; i < RECORD_BUF_SIZE; i++) {
          speed += buffRec[i];
        }
        speed = (RECORD_BUF_SIZE / speed) * 60000;
//      printf("\n speed = %f \n", speed);
      }
    }
  }
}
