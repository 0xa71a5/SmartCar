#ifndef VARIEBLE_H_
#define VARIEBLE_H_
#include "chlib_k.h"



#define row_num     50          //����ͷ�ɼ�����
	
#define col_num     152	        //����ͷ�ɼ�����

#define car_center    80        //��ģ����ֵ

#define P_WIDTH       8         //lp1����lp2ָ���֮����


#define BW_DELTA      50
#define whiteRoad     99//110
#define LINE_EDGE 	  2 

#define BLOCK_LEN     20

extern uint16_t H_Cnt;			//��¼���ж���
extern uint32_t V_Cnt;			//��¼���жϴ���

extern uint8_t img1[row_num][col_num];
extern uint8_t img2[row_num][col_num];
extern uint8_t *imgadd;

extern uint8_t Lx[row_num];                       //�����������ĵ��кţ��Ҳ���ʱΪcol_num
extern uint8_t Rx[row_num];                       //�����������ĵ��кţ��Ҳ���ʱΪ0
extern uint8_t* L_Start;
extern uint8_t* L_End;
extern uint8_t* R_Start;
extern uint8_t* R_End;

extern const uint8_t offset[ ];

#endif 