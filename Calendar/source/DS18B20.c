#include "config.h"
#include "DS18B20.h"

/*us延时函数*/
void Delay_us(uint8 us) {
  while (us--) continue;
}

/*初始化DS18B20并获取存在脉冲*/
bit Get_DS18B20_ACK() {
  bit ack;
  EA = 0;                         // 关总中断
  DS18B20_IO = 1;
  _nop_();
  DS18B20_IO = 0;
  Delay_us(75);                   // 拉低总线并保持480us以上复位
  DS18B20_IO = 1;                 // 释放总线
  Delay_us(11);                   // 延时等待18B20发回存在信号
  ack = DS18B20_IO;
  while (!DS18B20_IO) continue;   // 等待存在脉冲结束
  _nop_();
  EA = 1;                         // 开总中断
  return ack;
}

/*写一个字节*/
void DS18B20_W_Byte(uint8 dat) {
  uint8 i;
  EA = 0;                         // 关总中断
  for (i = 0; i < 8; i++) {
    DS18B20_IO = 0;
    _nop_();
    DS18B20_IO = dat & 0x01;
    Delay_us(10);                 // 需保持60~120us
    DS18B20_IO = 1;               // 释放总线
    _nop_();
    dat >>= 1;
  }
  EA = 1;                         // 开总中断
}

/*读一个字节*/
uint8 DS18B20_R_Byte() {
  uint8 i, tmp, dat;
  EA = 0;                         // 关总中断
  for (i = 0; i < 8; i++) {
    DS18B20_IO = 0;
    _nop_();
    DS18B20_IO = 1;
    Delay_us(1);
    tmp = DS18B20_IO;
    Delay_us(10);
    dat = (tmp << 7)|(dat >> 1);
    _nop_();
  }
  EA = 1;                         // 开总中断
  return dat;
}

/*启动转换温度*/
bit Start18B20() {
  if (Get_DS18B20_ACK() == 0) {
    DS18B20_W_Byte(0xcc);         // 跳过ROM操作
    DS18B20_W_Byte(0x44);         // 启动一次温度转换
    return 1;                     // 操作成功 返回1
  }
  return 0;                       // 操作失败 返回0
}

/*获取温度*/
bit Get18B20Temp(int16 *temp) {
  uint8 LSB, MSB;
  if (Get_DS18B20_ACK() == 0) {
    DS18B20_W_Byte(0xcc);         // 跳过ROM操作
    DS18B20_W_Byte(0xbe);         // 发送读取暂存器指令
    LSB = DS18B20_R_Byte();       // 读取温度值低字节
    MSB = DS18B20_R_Byte();       // 读取温度值高字节
    *temp = ((int16)MSB << 8) + LSB;
    return 1;                     // 操作成功 返回1
  }
  return 0;                       // 操作成功 返回0
}
