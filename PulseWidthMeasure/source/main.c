#include "reg52.h"
#include "stdio.h"

//#define DEBUG_ON          //���ڴ�ӡ�������

//#define DEBUG_FIXTIME     //����ʱ�䲹��

/* ����Ƶ�� */
#define MAIN_Fosc   11059200UL
//#define MAIN_Fosc   12000000UL

/* �߶������ */
#define DIGITAL_DB P0         // ������
sbit WELA = P2^7;             // λѡ��
sbit DULA = P2^6;             // ��ѡ��

/* 50ms��ʱ����ֵ */
#if MAIN_Fosc == 11059200UL
  #define T50MS_TL      0x00
  #define T50MS_TH      0x4C
  #define T50MS_VALUE   0x4C00
#elif MAIN_Fosc == 12000000UL
  #define T50MS_TL      0xB0
  #define T50MS_TH      0x3C
  #define T50MS_VALUE   0x3CB0
#endif

/* ��ʱ��ÿ����һ�ε�ʱ��(us) */
#define TIM_PER_CNT     (12000000.0/MAIN_Fosc)

#ifdef DEBUG_ON
#include "uart.h"
#endif

typedef struct {
  char cnt50ms;
  int cnt1s;
  long remain;
} RecordType;

RecordType lastRec = {0, 0, 0};   //��һ�μ�¼ֵ
RecordType newRec = {0, 0, 0};    //���¼�¼ֵ

bit finish_flag = 0;      //��������ɱ�־λ
float pulse_width = 0;    //�洢����ֵ

///* �������߶������ */
//unsigned char code table[]={
//  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
//  0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x7f, 0xff
//};

/* �������߶������ */
unsigned char code table[] = {
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x80, 0x00
};

/* �����λѡֵ */
unsigned char code T_COM[] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f};

void display_n(char n, num, dot);
void delay(char t) { while(t--); }

/* 16λ��ʱ����50����
 * �� INT0(P3.2) ��Ϊ���� TR0=1 ʱ��ʱ����ʱ
 */
void Timer0Init(void)
{
  TMOD &= 0xF0;       //���ö�ʱ��ģʽ
#ifdef DEBUG_FIXTIME
  TMOD |= 0x01;       //���ö�ʱ��ģʽ
#else
  TMOD |= 0x09;       //���ö�ʱ��ģʽ
#endif
  TL0 = T50MS_TL;     //���ö�ʱ��ֵ
  TH0 = T50MS_TH;     //���ö�ʱ��ֵ
  TF0 = 0;            //���TF1��־
  TR0 = 1;            //��ʱ��1��ʼ��ʱ
}

/* ������������������ʾ */
void main(void)
{
  char len, buff[10], i;
#ifdef DEBUG_ON
  UartInit();
#endif
  Timer0Init();
  IT0 = 1;    //�����ⲿ�ж�0������ʽ (1:�½��� 0:�͵�ƽ)
  EX0 = 1;    //ʹ���ⲿ�ж�0�ж�����
  ET0 = 1;    //ʹ�ܶ�ʱ��0�ж�����
  EA = 1;     //��ȫ���жϿ���
  
  while(1) {
    if (finish_flag) {
      /* ȡ�� TL0 �� TH0 �����ֵ */
      newRec.remain = ((TH0 << 8) | TL0) - T50MS_VALUE;
      /* ���� = ���¼�ʱֵ - �ϴμ�ʱֵ */
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
    /* �͵�ƽʱ�����жϣ�׼����ʼ�������� */
    if (INT0 == 0) {
      EA = 1;
    }
#ifdef DEBUG_ON
    if (len > 9)
      printf("len: %bd, buff: %s\n", len, buff);
#endif
//    /* ��ʾ��ʽ1��4λ��ʾ */
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
//    /* ��ʾ��ʽ2��4λ��ʾ */
//    display_n(0, buff[0] - '0', len == 5);
//    display_n(1, buff[len > 5 ? 1 : 2] - '0', len == 6);
//    display_n(2, buff[len > 6 ? 2 : 3] - '0', len == 7);
//    display_n(3, buff[len > 7 ? 3 : 4] - '0', len == 8);
//    /* ��ʾ��ʽ3��8λ��ʾ */
//    display_n(0, (long)pulse_width / 10000000, 0);
//    display_n(1, (long)pulse_width / 1000000 % 10, 0);
//    display_n(2, (long)pulse_width / 100000 % 10, 0);
//    display_n(3, (long)pulse_width / 10000 % 10, 0);
//    display_n(4, (long)pulse_width / 1000 % 10, 1);
//    display_n(5, (long)pulse_width / 100 % 10, 0);
//    display_n(6, (long)pulse_width / 10 % 10, 0);
//    display_n(7, (long)pulse_width % 10, 0);
    /* ��ʾ��ʽ4���Զ�λ����ʾ */
    for (i = 0; i < len - 1; i++) {
      display_n(7-i, buff[len-(i>2?(i+1):i)-1] - '0', i == 3);
    }
  }
}

/* ��ʾ��n������� */
void display_n(char n, num, dot)
{
  DULA = WELA = 0;                  // ��֤��ѡ��λѡ�ر�
  DIGITAL_DB = T_COM[n];            // λѡֵ
  WELA = 1;                         // ��λѡ
  WELA = 0;                         // �ر�λѡ
  DIGITAL_DB = table[num];          // ��ѡֵ
  if (dot) DIGITAL_DB |= table[10]; // С����
  DULA = 1;                         // �򿪶�ѡ
  DULA = 0;                         // �رն�ѡ
  delay(30);                         // ���Ե������������
  DIGITAL_DB = table[11];           // �ر������
  DULA = 1;
  DULA = 0;
}

///* ��ʾ��n������� */
//void display_n(char n, num, dot)
//{
//  P2 = T_COM[n];                //�����λѡ
//  delay(1);
//  P0 = table[num];              //����ܶ�ѡ
//  if (dot) P0 |= table[10];     //С����
//  delay(10);
//  P0 = table[11];               //�ر������
//}

/* �ⲿ�жϷ����� */
void exint0() interrupt 0
{
  if (TR0 == 0) {               //��ʱ����û�д�
    TR0 = 1;                    //�򿪶�ʱ��
    return;                     //�˳��ж�
  }
  TR0 = 0;                      //�رն�ʱ��
  EA = 0;                       //��ֹ�����ж�����
  finish_flag = 1;              //�������
}

/* ��ʱ���жϷ�����
 * ����ʱ���������ʱ�ᱻ����
 * ÿ��ʱ 50ms ��������������ֵ�� 1
 */
void tm0_isr() interrupt 1
{
  EA = 0;                       //��ֹ�����ж�����
#ifdef DEBUG_FIXTIME
  TR0 = 1;
#else
  TR0 = 0;                      //�رն�ʱ��
#endif
  TL0 = T50MS_TL + 0x14;        //ʱ�䲹��
  TH0 = T50MS_TH;
  newRec.cnt50ms++;
  if (newRec.cnt50ms == 20) {   // 50ms * 20 = 1000ms
    TL0 += 0x08;                //ʱ�䲹��
    newRec.cnt50ms = 0;
    newRec.cnt1s++;
  }
  EA = 1;                       //���������ж�����
  TR0 = 1;                      //�򿪶�ʱ��
}
