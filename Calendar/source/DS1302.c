#include "config.h"
#include "DS1302.h"

/*向DS1302写一字节数据*/
void DS1302_W_Byte(uint8 dat) {
  uint8 i;
  for (i = 0; i < 8; i++) {
    TSCLK = 0;
    TIO = dat & 0x01;
    TSCLK = 1;
    dat >>= 1;
  }
}

/*向DS1302读一字节数据*/
uint8 DS1302_R_Byte() {
  uint8 i, dat;
  for (i = 0; i < 8; i++) {
    TSCLK = 0;
    dat >>= 1;
    if (TIO) dat |= 0x80;
    TSCLK = 1;
  }
  return dat;
}

/*向DS1302写数据*/
void DS1302_W_DAT(uchar cmd, uchar dat) {
  TRST = 0;
  TRST = 1;
  DS1302_W_Byte(cmd);
  DS1302_W_Byte(dat);
  TRST = 0;
}

/*向DS1302读数据*/
/*
uchar DS1302_R_DAT(uchar cmd) {
  uchar dat;
  TRST = 0;
  TRST = 1;
  DS1302_W_Byte(cmd);
  dat = DS1302_R_Byte();
  TRST = 0;
  return dat;
}*/

/*DS1302清除写保护*/
void DS1302_Clear_WP() { DS1302_W_DAT(0x8e, 0x00); }

/*DS1302设置写保护*/
void DS1302_Set_WP() { DS1302_W_DAT(0x8e, 0x80); }

/*DS1302突发模式写数据*/
void DS1302_Burst_Write(uint8 *dat) {
  uint8 i;
  DS1302_Clear_WP();
  TRST = 0;
  TSCLK = 0;
  TRST = 1;
  DS1302_W_Byte(0xbe);
  for (i = 0; i < 8; i++) {
    DS1302_W_Byte(dat[i]);
  }
  DS1302_Set_WP();
}

/*DS1302突发模式读数据*/
void DS1302_Burst_Read(uint8 *dat) {
  uint8 i;
  DS1302_Clear_WP();
  TRST = 0;
  TSCLK = 0;
  TRST = 1;
  DS1302_W_Byte(0xbf);
  for (i = 0; i < 8; i++) {
    dat[i] = DS1302_R_Byte();
  }
  DS1302_Set_WP();
}

/*获取DS1302时钟日历数据*/
void GetRealTime(struct sTime *time) {
  uint8 buf[8];
  DS1302_Burst_Read(buf);
  time->year = buf[6] + 0x2000;
  time->mon  = buf[4];
  time->day  = buf[3];
  time->hour = buf[2];
  time->min  = buf[1];
  time->sec  = buf[0];
  time->week = buf[5];
}

/*设置DS1302时钟日历数据*/
void SetRealTime(struct sTime *time) {
  uint8 buf[8];
  buf[7] = 0;
  buf[6] = time->year;
  buf[5] = time->week;
  buf[4] = time->mon;
  buf[3] = time->day;
  buf[2] = time->hour;
  buf[1] = time->min;
  buf[0] = time->sec;
  DS1302_Burst_Write(buf);
}

/*初始化DS1302并设置默认时间*/
void Init_DS1302() {
  struct sTime code initTime = { 0x2018, 0x01, 0x18, 0x23, 0x59, 0x40, 0x04 };
  SetRealTime(&initTime);
}
