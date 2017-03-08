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

int deltMiddleWidth=0;

uint16_t lastCounterValue=0;
uint16_t deltValue=0;
int16_t QDvalue=0;
int8_t QDdirection;
float speedValue=0;
uint32_t millis=0;

//speed range 0-400
float speedControlErrSum=0;
float speedControlKp=30;
float speedControlKi=1;

float speedControlCurrentSpeed;
float speedControlTargetSpeed;
float speedControlOutput;
float speedControlError;
float speedControlLasttime=0;

//fifo data
Queue fifoData;

bool doWeRun=true;
float ra=0;
int deltBias=0;
extern int servoError;
static void PIT0_Int(void)
{
  millis++;
}

int main()
{
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
  
  InitFifo(&fifoData,20);//��ʼ�����ݶ��У��������������õĴ�������
 
  
  printf("Hello world \r\n");

  
  SYSTICK_DelayInit();
  
  printf("Enter to begin\n");
  while(Serial_available()==0);//�ȴ��������ʼ
  
  FTM_PWM_QuickInit(FTM0_CH7_PD07, kPWM_EdgeAligned, 2000, 1000);//���õ��pwm2000hzpwm ��ʼռ�ձ�10%
  FTM_PWM_QuickInit(FTM2_CH0_PB18, kPWM_EdgeAligned, 50, 750);//���ö��Ϊ50hz pwm  ��ʼ750us������
  FTM_QD_QuickInit(FTM1_QD_PHA_PB00_PHB_PB01, kFTM_QD_NormalPolarity, kQD_PHABEncoding ); //���ñ�������ʼ��
  
  speedPidInit(0);//��ʼ���ٶ�pid
  char input;
  int num=0;
  while(1){    //One period is about 9~10ms
    uint32_t record1=millis;
     dispimage();//OLED��ʾ 
     if(Serial_available())//���ܴ��ڴ��������ٶȿ���ָ��
    {
      num=0;
      while(Serial_available())
      {
        num*=10;
        num+=(Serial_read()-'0');
        delay(5);
      }
    }
        
     uint32_t record2=millis-record1;//����������л���ʱ��
     uint32_t bias=GetFifoBias(&fifoData);//��ȡ����ʱ���µ��е���ֵ�仯
     deltBias=bias-75;//��ȥƫ����
     
     if(num!=0)//���numΪ0  ˵�����ڴ�������������Ҫֹͣǰ��
     {
       if(abs(deltBias)<8)num=70;//��ʱƫ��ֵ��С ����˵����ֱ�� ���Կ��ԼӴ��ٶ�  65,40
       else       num=50;//�����  ���Լ�С�ٶ�
     }
     
     if(!doWeRun)
       num=0;
     
     speedPid(num);//�ٶ�pid����
     printf("%.2f %.2f  %.2f %d %d %d\r\n",speedControlTargetSpeed,speedControlCurrentSpeed,speedControlOutput,deltMiddleWidth,doWeRun,servoError);
     delay(2);
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