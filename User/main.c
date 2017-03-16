#include "chlib_k.h"
#include "varieble.h"
#include "img_processing.h"
#include "oled.h"
#include "init.h"
#include "systick.h"
#include "pit.h"
#include "uart.h"

void send(void);
int getImageFeature(void);
#define delay(x) SYSTICK_DelayMs(x)
void speedPidInit(float target);
void speedPid(float target);


uint16_t lastCounterValue=0;
uint16_t deltValue=0;
int16_t QDvalue=0;
int8_t QDdirection;
float speedValue=0;
uint32_t millis=0;

bool doWeReset=false;
//speed range 0-400
float speedControlErrSum=0;
float speedControlKp=30;
float speedControlKi=1;

float speedControlCurrentSpeed;
float speedControlTargetSpeed;
float speedControlOutput;
float speedControlError;
float speedControlLasttime=0;

extern int middleX;
extern float angle_kd;
extern float angle_kp;
extern float angle_kp_high;
extern float angle_kp_low;

extern int deltMiddleWidth;
extern int whiteCount;
extern int blackCount;
extern bool doWeRun;

//fifo data
Queue fifoData;
uint32_t accCount=200;
int deltBias=0;

int maxTargetSpeed=70;
int minTargetSpeed=45;

int accSpeedLow[11]={5,10,15,20,20,20,25,30,30,40,45};
int accSpeedHigh[11]={5,10,15,20,20,25,30,35,40,45,50};


static void PIT0_Int(void)
{
  millis++;
}

void resetSystem()
{
    FTM_PWM_ChangeDuty(HW_FTM2, HW_FTM_CH0,750);//middle the servo
    FTM_PWM_ChangeDuty(HW_FTM0, HW_FTM_CH7,0);//stop motor
    doWeReset=true;
}

void InitAccSpeed()
{
   for(int i=0;i<11;i++)
  {
    accSpeedHigh[i]=maxTargetSpeed*(i+1)/11;
    accSpeedLow[i] =minTargetSpeed*(i+1)/11;
  }
}
void parse()
{
  char recvType;
  int recvValue=0;
  delay(2);
  uint32_t waitStart=millis;
  while(Serial_available()==0)
  {
    if(millis-waitStart>200)return;
  }
  if(Serial_available())
  {
    recvType=Serial_read();
    delay(4);
    while(Serial_available())
    {
      char data=Serial_read();
      delay(4);
      if(data>'9'||data<'0')continue;
      recvValue*=10;
      recvValue+=data-'0';      
    }
    switch(recvType)
    {
    case 'm':maxTargetSpeed=recvValue;printf("maxTargetSpeed set to %d\r\n",recvValue);InitAccSpeed();delay(1500);break;
    case 'n':minTargetSpeed=recvValue;printf("minTargetSpeed set to %d\r\n",recvValue);InitAccSpeed();delay(1500);break; 
    case 'q':speedControlKp=(float)recvValue*0.1;printf("Speed Kp set to %f\r\n",speedControlKp);delay(1500);break; 
    case 'w':speedControlKi=(float)recvValue*0.1;printf("Speed Ki set to %f\r\n",speedControlKi);delay(1500);break; 
    case 'a':angle_kp_high=(float)recvValue*0.1;printf("Servo Kp high set to %f\r\n",angle_kp_high);delay(1500);break; 
    case 'z':angle_kp_low=(float)recvValue*0.1;printf("Servo Kp low set to %f\r\n",angle_kp_low);delay(1500);break; 
    case 'd':angle_kd=(float)recvValue*0.1;printf("Servo Kd set to %f\r\n",angle_kd);delay(1500);break;   
    case 'r':printf("Reset system\r\n");resetSystem();break;
    }
  }
  else
  {
    
  }
}
int main()
{
resetFlag:;
  int anglePulse=500;
  char showInfo[8];
  DisableInterrupts;//��ʼ��֮ǰ�ȹص������ж�
  init();
  PIT_QuickInit(HW_PIT_CH0, 1000);//��ʼ����ʱ��0
  PIT_CallbackInstall(HW_PIT_CH0, PIT0_Int); //ע��ص�����    
  PIT_ITDMAConfig(HW_PIT_CH0, kPIT_IT_TOF,ENABLE); //����ģ��0ͨ���ж�

  
  UART_CallbackRxInstall(HW_UART3, readIntHandler);//����uart�жϺ���
  UART_ITDMAConfig(HW_UART3, kUART_IT_Rx, true);
  OLED_Init();
  //OLED_ShowString_1206(0, 0, (uint8_t *)"Hello", 1);
  OLED_Refresh_Gram();
  EnableInterrupts;
  //row=50 col=152
  //��ʾ����128x64
 // GPIO_QuickInit(HW_GPIOB, 22, kGPIO_Mode_IPU);
  InitFifo(&fifoData,10);//��ʼ�����ݶ��У��������������õĴ�������
  printf("Hello world \r\n");
  SYSTICK_DelayInit();
  
  printf("Enter to begin\n");
 
  for(int i=0;i<11;i++)
  {
    accSpeedHigh[i]=maxTargetSpeed*(i+1)/11;
    accSpeedLow[i] =minTargetSpeed*(i+1)/11;
  }
  
  while(Serial_available()==0);//�ȴ��������ʼ
  
  FTM_PWM_QuickInit(FTM0_CH7_PD07, kPWM_EdgeAligned, 2000, 1000);//���õ��pwm2000hzpwm ��ʼռ�ձ�10%
  FTM_PWM_QuickInit(FTM2_CH0_PB18, kPWM_EdgeAligned, 50, 750);//���ö��Ϊ50hz pwm  ��ʼ750us������
  FTM_QD_QuickInit(FTM1_QD_PHA_PB00_PHB_PB01, kFTM_QD_NormalPolarity, kQD_PHABEncoding ); //���ñ�������ʼ��
  
  speedPidInit(0);//��ʼ���ٶ�pid
  char input;
  int num=1;
  accCount=200;
  while(1){    //One period is about 9~10ms
    uint32_t record1=millis;
     dispimage();//OLED��ʾ 
     
     if(Serial_available())//���ܴ��ڴ��������ٶȿ���ָ��
    {
      int lastNum=num;
      num=0;
      while(Serial_available())
      {
        num*=10;
        uint8_t tmpData=Serial_read();
        if(tmpData==':')
        {
          parse();
          break;
        }
        num+=(tmpData-'0');
        delay(5);
        doWeRun=true;
      }
      if(num!=0&&lastNum==0)
        accCount=0;
    }
    
    if(doWeReset){doWeReset=false;goto resetFlag;}
        
     uint32_t record2=millis-record1;//����������л���ʱ��
     uint32_t bias=GetFifoBias(&fifoData);//��ȡ����ʱ���µ��е���ֵ�仯
     deltBias=bias-75;//��ȥƫ����
     
     if(num!=0)//���numΪ0  ˵�����ڴ�������������Ҫֹͣǰ��
     {
       if(abs(deltBias)<8)num=maxTargetSpeed;//��ʱƫ��ֵ��С ����˵����ֱ�� ���Կ��ԼӴ��ٶ�
       else       num=minTargetSpeed;//�����  ���Լ�С�ٶ�  43
     }
     
     if(doWeRun)
     {
       if(accCount>110)//�Ѿ������Ǽ��ٶȶ�
        speedPid(num);//�ٶ�pid����
       else//�𲽼���  ���ݼ���
       {
         if((accCount/10)<11)
         {
              speedPid(accSpeedHigh[accCount/10]);//ʹ�ý��ݼ���
         }
       }
     }
     else//��⵽�յ�  ֹͣǰ��
     {
       num=0;
       speedPid(num);
     }

     printf("%.2f,%.2f,75,%d,%d,%d\r\n",speedControlTargetSpeed,speedControlCurrentSpeed,bias,whiteCount,blackCount); 

     accCount++;
     
  } 
}

void send()//����ͼ����λ��
{
      uint32_t i,j;
      i=row_num/2;
        for(j=0;j<col_num;j++)
        {
            printf("%d ",(int)(imgadd[i*col_num+j]));
        }
           printf("\n");
}

void speedPidInit(float target)
{ 
  if(target<0)target=0;
  if(target>400)target=400;
  speedControlTargetSpeed=target;
}

void speedPid(float target)//�ٶ�pid����
{
  float speedRatio=10.0;
  if(target==0)
  {
    speedControlTargetSpeed=target;
    speedControlCurrentSpeed=target;
    FTM_PWM_ChangeDuty(HW_FTM0, HW_FTM_CH7,0);
    return;
  }
    speedControlTargetSpeed=target;
    FTM_QD_GetData(HW_FTM1, &QDvalue, &QDdirection);
    QDvalue=-QDvalue;
    uint16_t temp=QDvalue-lastCounterValue;
    if(!(temp>>15))deltValue=temp;
    lastCounterValue=QDvalue;
    float deltTime=millis-speedControlLasttime;
    if(deltTime!=0)
    speedControlCurrentSpeed=speedRatio*deltValue/deltTime;//calculate speed

   speedControlError=speedControlTargetSpeed-speedControlCurrentSpeed;
   
   //speedControlOutput=speedControlOutput+speedControlError*speedControlKi;
   speedControlOutput=speedControlKp*speedControlError+speedControlKi*speedControlErrSum;
   speedControlErrSum+=speedControlError;
   
   if(speedControlOutput<0)speedControlOutput=0;
   if(speedControlOutput>3000)speedControlOutput=3000;
   FTM_PWM_ChangeDuty(HW_FTM0, HW_FTM_CH7,speedControlOutput);
   speedControlLasttime=millis;
   
  // printf("%d %d %d %d\r\n",speedControlTargetSpeed,speedControlCurrentSpeed,speedControlError,speedControlOutput);
}