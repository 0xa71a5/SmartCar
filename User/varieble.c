#include "varieble.h"

uint16_t H_Cnt=0;			//��¼���ж���
uint32_t V_Cnt=0;			//��¼���жϴ���

uint8_t img1[row_num][col_num];
uint8_t img2[row_num][col_num];
uint8_t *imgadd;

uint8_t Lx[row_num];                       //�����������ĵ��к�
uint8_t Rx[row_num];                      //�����������ĵ��к�
uint8_t* L_Start;
uint8_t* L_End;
uint8_t* R_Start;
uint8_t* R_End;

const uint8_t offset[row_num]= {              //ÿһ�е�lp1,lp2ɨ��ƫ����
  40,    40,    40,    40,    40,    40,    40,    40,    40,    40,
  40,    40,    40,    40,    40,    38,    38,    38,    38,    38,
  35,    35,    35,    35,    35,    29,    28,    27,    25,    25,
  24,    24,    24,    24,    23,    23,    23,    23,    22,    22,
  22,    22,    21,    21,    21,    21,    21,    20,    20,    20,
};