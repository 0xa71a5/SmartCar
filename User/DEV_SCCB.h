#ifndef __DEV_SCCB_H__
#define __DEV_SCCB_H__
#include "chlib_k.h"
   
#define ADR_OV7670      0x42


//����SCCB�豸��ַ
#define SCCB_DEV_ADR    ADR_OV7670
//����SCL��SDA������
#define SCCB_SCL        PCout(0) 
#define SCCB_SDA_O      PCout(3) 
#define SCCB_SDA_I      PCin(3) 
//����SDA�������
#define SCCB_SDA_OUT()  BITBAND_REG(PTC->PDDR, 3)=1
#define SCCB_SDA_IN()   BITBAND_REG(PTC->PDDR, 3)=0


#define SCCB_DELAY()	LPLD_SCCB_Delay(5000)	


uint8_t LPLD_SCCB_WriteReg(uint16_t, uint8_t);
uint8_t LPLD_SCCB_ReadReg(uint8_t, uint8_t*, uint16_t);

#endif 
