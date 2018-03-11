#include "uart.h"

void UartInit(void)		//9600bps@11.0592MHz
{
	PCON &= 0x7F;		//�����ʲ�����
  
//  SM0 = 0;
//	SM1 = 1; //10λ�첽�շ�����
//	REN = 1; //�������
	SCON = 0x50;		//8λ����,�ɱ䲨����
  
//	AUXR &= 0xBF;		//��ʱ��1ʱ��ΪFosc/12,��12T
//	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�����ʱ��1ģʽλ
	TMOD |= 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
	TL1 = 0xFD;		//�趨��ʱ��ֵ
	TH1 = 0xFD;		//�趨��ʱ����װֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
  
  TI = 1;     //ʹ��printf
//  ES = 1;     //�����жϿ���
//  EA = 1;     //���ж�����
}

/*
 * putchar (basic version): expands '\n' into CR LF
 */
char putchar (char c)  {
  if (c == '\n')  {
    while (!TI);
    TI = 0;
    SBUF = 0x0d;                         /* output CR  */
  }
  while (!TI);
  TI = 0;
  return (SBUF = c);
}
