#include "DEV_SCCB.h"

static uint8_t LPLD_SCCB_Start(void);
static void LPLD_SCCB_Stop(void);
static void LPLD_SCCB_Ack(void);
static void LPLD_SCCB_NoAck(void);
static uint8_t LPLD_SCCB_WaitAck(void);
static void LPLD_SCCB_SendByte(uint8_t);
static uint8_t LPLD_SCCB_ReceiveByte(void);
static void LPLD_SCCB_Delay(uint16_t);


/*
 * LPLD_SCCB_WriteReg
 * дSCCB�豸�Ĵ���
 * 
 * ����:
 *    reg_addr--�Ĵ�����ַ
 *    data--��д���� 
 *
 * ���:
 *    1-�ɹ�
 *    0-ʧ��
 */
uint8_t LPLD_SCCB_WriteReg(uint16_t reg_addr , uint8_t data)
{		
  if(!LPLD_SCCB_Start())
  {
    return 0;
  }
  LPLD_SCCB_SendByte( SCCB_DEV_ADR );         
  if( !LPLD_SCCB_WaitAck() )
  {
    LPLD_SCCB_Stop(); 
    return 0;
  }
  LPLD_SCCB_SendByte((uint8_t)(reg_addr & 0x00FF));   
  LPLD_SCCB_WaitAck();	
  LPLD_SCCB_SendByte(data);
  LPLD_SCCB_WaitAck();   
  LPLD_SCCB_Stop(); 
  return 1;
}									 




/******************************************************************************************************************
* ��������SCCB_ReadByte
* ����  ����ȡһ������
* ����  ��- data: ��Ŷ������� 	- length: ����������	- reg_addr: ��������ַ		 - DeviceAddress: ��������
* ���  ������Ϊ:=1�ɹ�����,=0ʧ��
* ע��  ����        
**********************************************************************************************************************/ 
/*
 * LPLD_SCCB_ReadReg
 * ��SCCB�豸�Ĵ���
 * 
 * ����:
 *    reg_addr--�Ĵ�����ַ
 *    *data--����������ݵ�ַ
 *    length--��ȡ����
 *
 * ���:
 *    1-�ɹ�
 *    0-ʧ��
 */          
uint8_t LPLD_SCCB_ReadReg(uint8_t reg_addr, uint8_t* data, uint16_t length)
{	
  if(!LPLD_SCCB_Start())
  {
    return 0;
  }
  LPLD_SCCB_SendByte( SCCB_DEV_ADR ); 
  if( !LPLD_SCCB_WaitAck() )
  {
    LPLD_SCCB_Stop(); 
    return 0;
  }
  LPLD_SCCB_SendByte( reg_addr ); 
  LPLD_SCCB_WaitAck();	
  LPLD_SCCB_Stop(); 
  
  if(!LPLD_SCCB_Start())
  {
    return 0;
  }
  LPLD_SCCB_SendByte( SCCB_DEV_ADR + 1 ); 
  if(!LPLD_SCCB_WaitAck())
  {
    LPLD_SCCB_Stop(); 
    return 0;
  }
  while(length)
  {
    *data = LPLD_SCCB_ReceiveByte();
    if(length == 1)
    {
      LPLD_SCCB_NoAck();
    }
    else
    {
      LPLD_SCCB_Ack(); 
    }
    data++;
    length--;
  }
  LPLD_SCCB_Stop();
  return 1;
}

/*
 * LPLD_SCCB_Start
 * SCCB��ʼ�źţ��ڲ�����
 */
static uint8_t LPLD_SCCB_Start(void)
{
  SCCB_SDA_O=1;
  SCCB_SCL=1;
  SCCB_DELAY();
  
  SCCB_SDA_IN();
  if(!SCCB_SDA_I)
  {
    SCCB_SDA_OUT();	
    return 0;
  }
  SCCB_SDA_OUT();	
  SCCB_SDA_O=0;
  
  SCCB_DELAY();
  
  SCCB_SDA_IN();
  if(SCCB_SDA_I) 
  {
    SCCB_SDA_OUT();
    return 0;
  }
  SCCB_SDA_OUT();
  SCCB_SDA_O=0;

  return 1;
}

/*
 * LPLD_SCCB_Stop
 * SCCBֹͣ�źţ��ڲ�����
 */
static void LPLD_SCCB_Stop(void)
{
  SCCB_SCL=0;
  SCCB_SDA_O=0;
  SCCB_DELAY();
  
  SCCB_SCL=1;
  SCCB_SDA_O=1;
  SCCB_DELAY();
}

/*
 * LPLD_SCCB_Stop
 * SCCBӦ���źţ��ڲ�����
 */
static void LPLD_SCCB_Ack(void)
{	
  SCCB_SCL=0;
  SCCB_DELAY();
  
  SCCB_SDA_O=0;
  SCCB_DELAY();
  
  SCCB_SCL=1;
  SCCB_DELAY();
  
  SCCB_SCL=0;
  SCCB_DELAY();
}

/*
 * LPLD_SCCB_NoAck
 * SCCB��Ӧ���źţ��ڲ�����
 */
static void LPLD_SCCB_NoAck(void)
{	
  SCCB_SCL=0;
  SCCB_DELAY();
  SCCB_SDA_O=1;
  SCCB_DELAY();
  SCCB_SCL=1;
  SCCB_DELAY();
  SCCB_SCL=0;
  SCCB_DELAY();
}

/*
 * LPLD_SCCB_WaitAck
 * SCCB�ȴ�Ӧ���źţ��ڲ�����
 */
static uint8_t LPLD_SCCB_WaitAck(void) 	
{
  SCCB_SCL=0;
  SCCB_DELAY();
  SCCB_SDA_O=1;	
  SCCB_DELAY();
  
  SCCB_SCL=1;
  
  SCCB_SDA_IN();
  SCCB_DELAY();
  
  if(SCCB_SDA_I)
  {
    SCCB_SDA_OUT();
    SCCB_SCL=0;
    return 0;
  }
  SCCB_SDA_OUT();
  SCCB_SCL=0;
  return 1;
}

/*
 * LPLD_SCCB_SendByte
 * SCCB�������ݣ��ڲ�����
 */
static void LPLD_SCCB_SendByte(uint8_t data) 
{
  uint8_t i=8;
  while(i--)
  {
    SCCB_SCL=0;
    SCCB_DELAY();
    if(data&0x80)
    {
      SCCB_SDA_O=1; 
    }
    else 
    {
      SCCB_SDA_O=0;   
    }
    data<<=1;
    SCCB_DELAY();
    SCCB_SCL=1;
    SCCB_DELAY();
  }
  SCCB_SCL=0;
}

/*
 * LPLD_SCCB_SendByte
 * SCCB�������ݣ��ڲ�����
 */
static uint8_t LPLD_SCCB_ReceiveByte(void)  
{ 
  uint8_t i=8;
  uint8_t ReceiveByte=0;
  
  SCCB_SDA_O=1;	
  SCCB_DELAY();
  
  SCCB_SDA_IN();	
  
  while(i--)
  {
    ReceiveByte<<=1;      
    SCCB_SCL=0;
    SCCB_DELAY();
    
    SCCB_SCL=1;
    SCCB_DELAY();	
    
    if(SCCB_SDA_I)
    {
      ReceiveByte|=0x01;
    }
    
  }
  SCCB_SDA_OUT();
  SCCB_SCL=0;
  
  return ReceiveByte;
}

/*
 * LPLD_SCCB_SendByte
 * SCCB��ʱ�������ڲ�����
 */
static void LPLD_SCCB_Delay(uint16_t i)
{	
  while(i) 
    i--; 
}


