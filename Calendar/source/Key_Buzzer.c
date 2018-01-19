#include "config.h"
#include "app.h"
#include "Key_Buzzer.h"

/*4位独立按键映射*/      // 回车键  上键  下键  ESC键
uint8 code KeyCodeMap[4] = { 0x0d, 0x26, 0x28, 0x1b };

/*4位独立按键当前状态*/
uint8 pdata KeySta[4] = { 1, 1, 1, 1 };

/*按键检测*/
void KeyDriver() {
  uint8 i;
  static uint8 pdata backup[4] = { 1, 1, 1, 1 };  // 4位独立按键备份值
  for (i = 0; i < 4; i++) {                       // 循环检测4个独立按键
    if (backup[i] != KeySta[i]) {                 // 检测按键
      if (backup[i] != 0) {                       // 如果按键按下
        KeyAction(KeyCodeMap[i]);                 // 调用按键动作函数
      }
      backup[i] = KeySta[i];                      // 刷新备份值
    }
  }
}

/*按键扫描*/
void KeyScan() {
  uint8 i;
  static uint8 flag = 0;                                // 消抖计数
  static uint8 keybuf[4] = { 0xff, 0xff, 0xff, 0xff};   // 4位独立按键扫描缓冲区
  // 将4个独立按键值移入缓冲区
  keybuf[0] = (keybuf[0] << 1) | KEY_S2;
  keybuf[1] = (keybuf[1] << 1) | KEY_S3;
  keybuf[2] = (keybuf[2] << 1) | KEY_S4;
  keybuf[3] = (keybuf[3] << 1) | KEY_S5;
  flag++;           // 间隔5ms扫描一次
  if (flag == 4) {  // 4次就是20ms 完成消抖
    flag = 0;
    for (i = 0; i < 4; i++) {
      if ((keybuf[i] & 0x0f) == 0x00) {
        KeySta[i] = 0;  // 如果4次扫描值都为0 即按下状态
      } else if ((keybuf[i] & 0x0f) == 0x0f) {
        KeySta[i] = 1;  // 如果4次扫描值都为1 即弹起状态
      }
    }
  }
}

/*蜂鸣器控制*/
void staBuzzer(bit e) { BUZZER = ~e; }
