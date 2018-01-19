#ifndef _MAIN_H_
#define _MAIN_H_

/*系统运行状态枚举*/
enum eStaSystem { E_NORMAL, E_SET_TIME, E_SET_ALARM, E_DIS_ALARM };

#ifndef _MAIN_C
extern enum eStaSystem staSystem;
#endif

void RefreshTemp(uint8 ops);    // 刷新温度
void Init_Timer0();             // 初始化定时器0

#endif  // _MAIN_H_
