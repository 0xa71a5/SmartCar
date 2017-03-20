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
void gpiob_isr(uint32_t array);

float outputAngleValue;
uint16_t lastCounterValue=0;
uint16_t deltValue=0;
int16_t QDvalue=0;
int8_t QDdirection;
float speedValue=0;
uint32_t millis=0;
extern int middleX;
extern bool doWeRun;
extern int whiteCount;
extern int blackCount;
//speed range 0-400
float speedControlErrSum=0;
float speedControlKp=30;//30
float speedControlKi=1;
float speedControlKd=3;

float speedControlCurrentSpeed;
float speedControlTargetSpeed;
float speedControlOutput;
float speedControlError;
float speedControlLastError=0;
float speedControlLasttime=0;

int maxTargetSpeed=75;
int minTargetSpeed=40;

uint8_t buttonStatus=1;
//fifo data
Queue fifoData;
uint32_t accCount=200;
int deltBias=0;

extern int seg1;
extern int seg2;
extern int seg3;

int maxAccSpeed=75;//100
int minAccSpeed=45;//45
int accSpeedLow[11]={5,10,15,20,20,20,25,30,30,40,45};
int accSpeedHigh[11]={5,10,15,20,25,30,35,40,45,45,45};

float incCounter=0;
int incButton=0;
uint8_t incIndex=0;//0-6
//0:max speed     1: min speed    2:deltBiasSeg1   3:deltBiasSeg2    
float toSetValue1=3.5,toSetValue2=4.8;
uint32_t runningCounter=0;
static void PIT0_Int(void)
{
  millis++;
}

void setValues()
{
  char toDisplay[15]; 
  incCounter=(float)maxTargetSpeed/10;
  while(GPIO_ReadBit(HW_GPIOB, 10)==0)
  {
     maxTargetSpeed=incCounter*10;
     printf("maxTargetSpeed=%d\r\n",maxTargetSpeed);
     sprintf(toDisplay,"maxSpd:%d  ",maxTargetSpeed);
     OLED_ShowString_1206(0, 0,toDisplay, 1);
     OLED_Refresh_Gram();
     delay(50);
  }
  delay(200);
  while(GPIO_ReadBit(HW_GPIOB, 10)==1);
  
  incCounter=(float)minTargetSpeed/10;
  while(GPIO_ReadBit(HW_GPIOB, 10)==0)
  {
     minTargetSpeed=incCounter*10;
     printf("minTargetSpeed=%d\r\n",minTargetSpeed);
     sprintf(toDisplay,"minSpd:%d  ",minTargetSpeed);
     OLED_ShowString_1206(0, 0,toDisplay, 1);
     OLED_Refresh_Gram();
     delay(50);
  }
  delay(200);
  while(GPIO_ReadBit(HW_GPIOB, 10)==1);
  
  incCounter=(float)speedControlKp/10;
  while(GPIO_ReadBit(HW_GPIOB, 10)==0)
  {
     speedControlKp=incCounter*10;
     printf("v kp=%d\r\n",(int)speedControlKp);
     sprintf(toDisplay,"v kp:%d     ",(int)speedControlKp);
     OLED_ShowString_1206(0, 0,toDisplay, 1);
     OLED_Refresh_Gram();
     delay(50);
  }
  delay(200);
  while(GPIO_ReadBit(HW_GPIOB, 10)==1);
  
  
  incCounter=(float)seg1/10;
  while(GPIO_ReadBit(HW_GPIOB, 10)==0)
  {
     seg1=incCounter*10;
     printf("seg1=%d\r\n",seg1);
     sprintf(toDisplay,"seg1:%d  ",seg1);
     OLED_ShowString_1206(0, 0,toDisplay, 1);
     OLED_Refresh_Gram();
     delay(50);
  }
  delay(200);
  while(GPIO_ReadBit(HW_GPIOB, 10)==1);
  
  incCounter=(float)seg2/10;
  while(GPIO_ReadBit(HW_GPIOB, 10)==0)
  {
     seg2=incCounter*10;
     printf("seg2=%d\r\n",seg2);
     sprintf(toDisplay,"seg2:%d  ",seg2);
     OLED_ShowString_1206(0, 0,toDisplay, 1);
     OLED_Refresh_Gram();
     delay(50);
  }
  delay(200);
  while(GPIO_ReadBit(HW_GPIOB, 10)==1);
  
  incCounter=(float)seg3/10;
  while(GPIO_ReadBit(HW_GPIOB, 10)==0)
  {
     seg3=incCounter*10;
     printf("seg3=%d\r\n",seg3);
     sprintf(toDisplay,"seg3:%d  ",seg3);
     OLED_ShowString_1206(0, 0,toDisplay, 1);
     OLED_Refresh_Gram();
     delay(50);
  }
  delay(200);
  while(GPIO_ReadBit(HW_GPIOB, 10)==1);
  
  
  
  
}

void gpiob_isr(uint32_t array)//中断函数
{
  if(array & (1 << 16)) 
  {
    if(!GPIO_ReadBit(HW_GPIOB, 17))//正旋了一格
    {
      incCounter+=0.1;
    }
    else//反旋了一格
    {
      incCounter-=0.1;
    }
  }
      if(array & (1 << 10))//按下了按键
    {
      incButton=1;
      incIndex=(incIndex+1)%6;
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
    case 'm':maxTargetSpeed=recvValue;printf("maxTargetSpeed set to %d\r\n",recvValue);delay(1500);break;
    case 'n':minTargetSpeed=recvValue;printf("minTargetSpeed set to %d\r\n",recvValue);delay(1500);break; 
    case 'q':speedControlKp=(float)recvValue*0.1;printf("Speed Kp set to %f\r\n",speedControlKp);delay(1500);break; 
    case 'w':speedControlKi=(float)recvValue*0.1;printf("Speed Ki set to %f\r\n",speedControlKi);delay(1500);break; 
    case 'e':speedControlKd=(float)recvValue*0.1;printf("Speed Kd set to %f\r\n",speedControlKd);delay(1500);break; 
    }
  }
  else
  {
    
  }
}

uint32_t buttonCount=0;
int main()
{
  int anglePulse=500;
  char showInfo[8];
  DisableInterrupts;//初始化之前先关掉所有中断
  init();
  PIT_QuickInit(HW_PIT_CH0, 1000);//初始化定时器0
  PIT_CallbackInstall(HW_PIT_CH0, PIT0_Int); //注册回调函数    
  PIT_ITDMAConfig(HW_PIT_CH0, kPIT_IT_TOF,ENABLE); //开启模块0通道中断

  
  UART_CallbackRxInstall(HW_UART3, readIntHandler);//设置uart中断函数
  UART_ITDMAConfig(HW_UART3, kUART_IT_Rx, true);
  OLED_Init();
  //OLED_ShowString_1206(0, 0, (uint8_t *)"Hello", 1);
  OLED_Refresh_Gram();
  EnableInterrupts;
  //row=50 col=152
  //显示屏是128x64
  GPIO_QuickInit(HW_GPIOB, 22, kGPIO_Mode_IPU);//button 4 
  GPIO_QuickInit(HW_GPIOB, 23, kGPIO_Mode_IPU);//button 3
  GPIO_QuickInit(HW_GPIOB, 20, kGPIO_Mode_IPU);//button 2
  GPIO_QuickInit(HW_GPIOB, 21, kGPIO_Mode_IPU);//button 1



  
  InitFifo(&fifoData,10);//初始化数据队列，用来存放连续获得的传感数据  20
 
  for(int i=0;i<11;i++)
  {
    accSpeedHigh[i]=maxAccSpeed*(i+1)/11;
    accSpeedLow[i]=minAccSpeed*(i+1)/11;
  }
  printf("Hello world \r\n");

  
  SYSTICK_DelayInit();
  printf("Enter to begin\n");
 
  while(GPIO_ReadBit(HW_GPIOB, 22)==0)
    {
      if(Serial_available())break;
    }
  delay(1000);
  //while(Serial_available()==0);//等待串口命令开始

  
  
  FTM_PWM_QuickInit(FTM0_CH7_PD07, kPWM_EdgeAligned, 2000, 0);//设置电机pwm2000hzpwm 初始占空比10% //正向
  FTM_PWM_QuickInit(FTM2_CH0_PB18, kPWM_EdgeAligned, 50, 750);//设置舵机为50hz pwm  初始750us高脉冲
  FTM_QD_QuickInit(FTM1_QD_PHA_PB00_PHB_PB01, kFTM_QD_NormalPolarity, kQD_PHABEncoding ); //设置编码器初始化
  //旋转编码器测试
  GPIO_QuickInit(HW_GPIOB, 16, kGPIO_Mode_IPU);
  GPIO_QuickInit(HW_GPIOB, 17, kGPIO_Mode_IPU);
  GPIO_CallbackInstall(HW_GPIOB, (GPIO_CallBackType)gpiob_isr);
  GPIO_ITDMAConfig(HW_GPIOB, 16, kGPIO_IT_FallingEdge, true);//下降沿沿触发
  GPIO_ITDMAConfig(HW_GPIOB, 17, kGPIO_IT_FallingEdge, true);//下降沿沿触发
  GPIO_QuickInit(HW_GPIOB, 10, kGPIO_Mode_IPU);
  GPIO_ITDMAConfig(HW_GPIOB, 10, kGPIO_IT_RisingEdge, true);
  //以上为旋转编码器测试
  speedPidInit(0);//初始化速度pid
  char input;
  int num=0;
  accCount=200;
  while(1){    //One period is about 9~10ms
    uint32_t record1=millis;
     dispimage();//OLED显示 
     while(GPIO_ReadBit(HW_GPIOB, 21)==1)
    {
      setValues();
    }
     uint8_t currentButtonStatus=GPIO_ReadBit(HW_GPIOB, 22);
    if(currentButtonStatus==1&&buttonCount==0)
    {
      num=1;
      delay(2000);
      buttonCount++;
      runningCounter=millis+10000;
    }
    if(currentButtonStatus==0)
    {
      num=0;
      buttonCount=0;
    }
     
    if(Serial_available())//接受串口传回来的速度控制指令
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
        doWeRun=true;
        delay(5);
      }
      if(num!=0&&lastNum==0)
        accCount=0;
    }
        
     uint32_t record2=millis-record1;//计算程序运行花费时间
     uint32_t bias=GetFifoBias(&fifoData);//获取连续时间下的中点数值变化
     deltBias=bias-75;//减去偏差量
     
     if(num!=0)//如果num为0  说明串口传过来的数据是要停止前进
     {
       if(abs(deltBias)<15)num=maxTargetSpeed;//此时偏差值较小 所以说明是直道 所以可以加大速度   100
       //else if(abs(deltBias)<13)num=90;
       //else if(abs(deltBias)<15)num=70;
       //else if(abs(deltBias)<20)num=60;
       else    num=minTargetSpeed;//是弯道  所以减小速度  43  
     }
     if(millis<runningCounter)
       doWeRun=true;
     if(doWeRun)
     {
       if(accCount>110)//已经不再是加速度段
        speedPid(num);//速度pid调控
       else//起步加速  阶梯加速
       {
         if((accCount/10)<11)
         {
            if(num<50)
              speedPid(accSpeedLow[accCount/10]);
            else
              speedPid(accSpeedHigh[accCount/10]);
         }
       }
     }
     else
     {
       num=0;
       speedPid(num);
     }
     //printf("%.2f %.2f  %.2f %d %d\r\n",speedControlTargetSpeed,speedControlCurrentSpeed,speedControlOutput,bias,millis-record1);
     printf("%.2f,%.2f,%d,0,%.2f,%.2f,%d,%d\r\n",speedControlTargetSpeed,speedControlCurrentSpeed,deltBias,outputAngleValue,speedControlOutput,blackCount,whiteCount); 
     //delay(2);
     accCount++;
  } 
}

void send()//发送图像到上位机
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
  if(target>500)target=500;
  speedControlTargetSpeed=target;
}

void speedPid(float target)//速度pid调控
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
   speedControlOutput=speedControlKp*speedControlError+speedControlKi*speedControlErrSum+speedControlKd*(speedControlError-speedControlLastError);
   speedControlErrSum+=speedControlError;
   speedControlLastError=speedControlError;
   if(speedControlOutput<0)speedControlOutput=0;
   if(speedControlOutput>5000)speedControlOutput=5000;
   FTM_PWM_ChangeDuty(HW_FTM0, HW_FTM_CH7,speedControlOutput);
   speedControlLasttime=millis;
   
  // printf("%d %d %d %d\r\n",speedControlTargetSpeed,speedControlCurrentSpeed,speedControlError,speedControlOutput);
}