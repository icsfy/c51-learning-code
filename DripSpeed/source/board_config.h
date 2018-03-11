#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

#include <reg52.h>
#include <intrins.h>

typedef unsigned char uchar;  // 8λ�޷�����
typedef unsigned int uint;    // 16λ�޷�����

/* 8��LED�� */
sbit LED1 = P1^0;
sbit LED2 = P1^1;
sbit LED3 = P1^2;
sbit LED4 = P1^3;
sbit LED5 = P1^4;
sbit LED6 = P1^5;
sbit LED7 = P1^6;
sbit LED8 = P1^7;

/* �߶������ */
//#define SEVEN_SEGMENT_COMMON_ANODE  // �������߶������
#define SEVEN_SEGMENT_COMMON_CATHODE  // �������߶������
#define DIGITAL_DB P0         // ������
sbit WELA = P2^7;             // λѡ��
sbit DULA = P2^6;             // ��ѡ��

/* 5V��Դ������ */
sbit Buzzer = P2^3;           // ����������

#endif /* __BOARD_CONFIG_H */
