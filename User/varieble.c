#include "varieble.h"

uint16_t H_Cnt=0;			//记录行中断数
uint32_t V_Cnt=0;			//记录场中断次数

uint8_t img1[row_num][col_num];
uint8_t img2[row_num][col_num];
uint8_t *imgadd;

uint8_t Lx[row_num];                       //左引导线中心点列号
uint8_t Rx[row_num];                      //右引导线中心点列号
uint8_t* L_Start;
uint8_t* L_End;
uint8_t* R_Start;
uint8_t* R_End;

const uint8_t offset[row_num]= {              //每一行的lp1,lp2扫描偏移量
  40,    40,    40,    40,    40,    40,    40,    40,    40,    40,
  40,    40,    40,    40,    40,    38,    38,    38,    38,    38,
  35,    35,    35,    35,    35,    29,    28,    27,    25,    25,
  24,    24,    24,    24,    23,    23,    23,    23,    22,    22,
  22,    22,    21,    21,    21,    21,    21,    20,    20,    20,
};