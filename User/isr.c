/*********************************************************/
//@demo
//@�̼��⣺����V2.4
//@author��th
//@2016.11.30
//@for seu2016 ����ͷ������
/*********************************************************/

#include "isr.h"
#include "MK60DZ10.h"
uint16_t vsync=0;
//�жϷ���������Ҫ�ɼ�������(�У���)�������޸�
//�����ں����м���led�Ŀ���������Ƿ�����ж�
//���������жϷ�������ִ����ʱ�����ݴ���/�����շ�
 void GPIO_ISR(uint32_t array)
{

    if(array & (1 << 7)) //���ж�
    {
      if(H_Cnt%2==1 && H_Cnt<100)
        {
          DMA_EnableRequest(HW_DMA_CH0);
        }
        H_Cnt++; 
      
    }

    if(array & (1 << 6)) //���ж�
    {
      H_Cnt = 0;
      if(V_Cnt<20)
      V_Cnt++;
      else{                                     //20��֮��ʼ�ɼ�
      vsync=1-vsync;                            //��ż���л�
      DMA_SetDestAddress(HW_DMA_CH0, vsync?(uint32_t)img1[0]:(uint32_t)img2[0]);
      imgadd=vsync?img2[0]:img1[0];
      }
    }
    

}
