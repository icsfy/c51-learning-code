#include <reg52.h>
#include "board_config.h"
#include "digital_display.h"
#include "uart.h"

#define USE_ESP8266

#ifdef USE_ESP8266
#include "esp8266.h"
//#define UPLOAD_PERIOD 1     // ÿ��1�뷢��һ������
//uchar upload_time = 0;
bit flag_need_upload = 0;
#endif

#define RECORD_BUF_SIZE 5   // ��¼5��ֵ��ƽ��ֵ�õ�Һ���ٶ�
#define COUNT_TIMEOUT 5     // 5sû��Һ�ξͱ���

typedef struct {
  uchar cnt50ms;
  uint cnt1s;
  uint remain;
} RecordType;

uchar cnt50ms = 0;    // 50ms����
uint cnt1s = 0;       // 1s����

float speed = 0;          // Һ���ٶ�

RecordType lastRec = {0, 0, 0};     // ��һ�μ�¼
RecordType newRec = {0, 0, 0};      // ���¼�¼
uint buffRec[RECORD_BUF_SIZE];      // ���μ�¼֮���ʱ���� msΪ��λ
uchar record_index = 0;             // buff��¼����
bit flag_cal_speed = 0;             // ��buff��֮����Կ�ʼ�����ٶ�
bit flag_record_updated = 0;        // �Ƿ����µ�Һ�μ�¼

void Timer0Init(void)		//50����@11.0592MHz
{
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TMOD |= 0x01;		//���ö�ʱ��ģʽ
	TL0 = 0x00;		//���ö�ʱ��ֵ
	TH0 = 0x4C;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
  
  ET0 = 1;    //����ʱ��0�ж�
//  EA = 1;     //���ж�����
}

//External interrupt1 service routine
void exint1() interrupt 2
{
  if (!TR0) {
    TR0 = 1;		//��ʱ��0��ʼ��ʱ
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
  TL0 = 0x00;		//���ö�ʱ��ֵ
	TH0 = 0x4C;		//���ö�ʱ��ֵ
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
    Buzzer = 0;   // ����������
    EA = 0;
  }
}

void main(void)
{
  uchar i;
  UartInit();       // ���ڳ�ʼ��
#ifdef USE_ESP8266
  ESP8266_Init();   // ESP8266��ʼ��
#endif
  Timer0Init();     // ��ʱ��0��ʼ��
  
  IT1 = 1;        // �ⲿ�ж�1�½��ش���
  EX1 = 1;        // �����ⲿ�ж�1�ж�
  EA = 1;         // ���ж�����
  
  while(1) {
    /* �������ʾҺ���ٶ� */
    Display(speed);
#ifdef USE_ESP8266
    /* �ϴ�Һ���ٶ� */
//    if (upload_time >= UPLOAD_PERIOD) {
    if (flag_need_upload) {
      ESP8266_PostJsonData(speed);
//      upload_time = 0;
      flag_need_upload = 0;
    }
#endif
    /* ����Һ�μ�¼buffer */
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
      /* ����Һ���ٶ� */
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
