/******************** (C) COPYRIGHT 2011 ������ӹ����� ********************
 * �ļ���       ��FTM.h
 * ����         ������PWM�����벶׽��������ͷ�ļ�
 *
 * ʵ��ƽ̨     ��
 * ��汾       ��
 * Ƕ��ϵͳ     ��
 *
 * ����         ��������ӹ�����
 * �Ա���       ��
 * ����֧����̳ ��
**********************************************************************************/

#ifndef _FTM_B_H_
#define _FTM_B_H_

extern volatile struct FTM_MemMap *FTMx[3];

typedef enum FTMn
{
    FTM0,
    FTM1,
    FTM2
} FTMn;

typedef enum CHn
{
    //   --FTM0--  --FTM1--  --FTM2--
    CH0,   //     PTC1      PTA8      PTA10
    CH1,   //     PTC2      PTA9      PTA11
    CH2,   //     PTC3       ��         ��
    CH3,   //     PTC4       ��         ��
    CH4,   //     PTD4       ��         ��
    CH5,   //     PTD5       ��         ��
    CH6,   //     PTD6       ��         ��
    CH7    //     PTD7       ��         ��
    // ����ʾ������
} CHn;



/*********************** PWM **************************/

#define FTM_PRECISON 100u     //����ռ�ձȾ��ȣ�100������Ϊ1%��1000u�򾫶�Ϊ0.1%������ռ�ձ� duty �βδ��룬��ռ�ձ�Ϊ duty/FTM_PRECISON

void  FTM_PWM_init(FTMn, CHn, u32 freq, u32 duty);  //��ʼ��FTM��PWM���ܲ�����Ƶ�ʡ�ռ�ձȡ�����ͨ�����ռ�ձȡ�ͬһ��FTM����ͨ����PWMƵ����һ���ģ���3��FTM

void  FTM_PWM_Duty(FTMn, CHn,         u32 duty);    //����ͨ��ռ�ձ�,ռ�ձ�Ϊ ��duty * ���ȣ� % ����� FTM_PRECISON ����Ϊ 1000 ��duty = 100 ����ռ�ձ� 100*0.1%=10%
void  FTM_PWM_freq(FTMn,    u32 freq);              //����FTM��Ƶ��

void FTM_QUAD_init(FTMn ftmn,CHn ch);
void FTM_SpeedMeasure_init(void);


/*********************** ���벶׽ **************************/

typedef enum Input_cfg
{
    Rising,               //�����ز�׽
    Falling,              //�½��ز�׽
    Rising_or_Falling     //�����ز�׽
} Input_cfg;


void FTM_Input_init(FTMn, CHn, Input_cfg);

#define FTM_IRQ_EN(FTMn,CHn)        FTM_CnSC_REG(FTMx[FTMn],CHn) |= FTM_CnSC_CHIE_MASK       //���� FTMn_CHn �ж�
#define FTM_IRQ_DIS(FTMn,CHn)       FTM_CnSC_REG(FTMx[FTMn],CHn) &= ~FTM_CnSC_CHIE_MASK      //�ر� FTMn_CHn �ж�








#endif  //_FTM_H_



