#ifndef _LUNAR_H_
#define _LUNAR_H_

struct SPDATE {   // 阴历结构体
  uint8 year;     // 年
  uint8 mon;      // 月
  uint8 day;      // 日
};

#ifndef _LUNAR_C
extern struct SPDATE SpDate;  // 阴历
#endif

/*BCD阳历转为BCD阴历 c=0为21世纪 c=1为19世纪*/
void Conversion(bit c, uchar year, uchar mon, uchar day);

#endif  // _LUNAR_H_
