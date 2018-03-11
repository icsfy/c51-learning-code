#include "digital_display.h"

#ifdef SEVEN_SEGMENT_COMMON_ANODE     /* ������ */
uchar code table[]={
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
  0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x7f, 0xff
};
#endif
#ifdef SEVEN_SEGMENT_COMMON_CATHODE   /* ������ */
uchar code table[] = {
  //"0"   "1"   "2"   "3"   "4"   "5"   "6"   "7"   "8"   "9"   "." "OFF"
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x80, 0x00
};
#endif

// �����λѡֵ    //  ��1λ   2λ   3λ   4λ   5λ   6λ   7λ   8λ
uchar code T_COM[] = {0xfe, 0xfd, 0xfb, 0xf7};//, 0xef, 0xdf, 0xbf, 0x7f};

static void delay(uchar t) { while(t--); }

// ��ʾ��n������ܣ�ֵΪnum, С����dot
static void display_n(uchar n, num)
{
  DULA = WELA = 0;  // ��֤��ѡ��λѡ�ر�
  DIGITAL_DB = T_COM[n];  // λѡֵ
  WELA = 1; // ��λѡ
  WELA = 0; // �ر�λѡ
  DIGITAL_DB = table[num];  // ��ѡֵ
//  if(dot) DIGITAL_DB |= table[10]; // С����
  DULA = 1; // �򿪶�ѡ
  DULA = 0; // �رն�ѡ
  delay(1); // ���Ե������������
  // �ر������
  DIGITAL_DB = table[11];
  DULA = 1;
  DULA = 0;
}

/* ����� �趯̬��ʾ */
void Display(uint num)
{
  display_n(0, num/1000);
  display_n(1, num/100%10);
  display_n(2, num/10%10);
  display_n(3, num%10);
}
