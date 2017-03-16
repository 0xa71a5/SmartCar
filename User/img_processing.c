
#include "varieble.h"
#include "img_processing.h"

#include "oled.h"
#include "uart.h"
int frameCount=0;
int lastMiddlePoint=col_num/2;
int scanRowBegin=20;//一共50行
int scanRowEnd=25;

int middleX=0;
float ratio=2.0;
float angle_kd=0.5;
float angle_kp=2.0;
float angle_error=0;
float angle_lasterror=0;
float angle_kp_high=5.8;
float angle_kp_low=2.5;


extern float speedValue;
extern uint16_t lastCounterValue;
extern uint16_t deltValue;
extern int16_t QDvalue;
extern int8_t QDdirection;
extern uint32_t millis;

extern Queue fifoData;
extern int deltBias;

int deltMiddleWidth;
int whiteCount=1;
int blackCount=1;
bool doWeRun=true;
/*
* @name		searchline_OV7620
* @description	To get the racing track from the imgadd.
* @inputval	None
* @outputval	None
* @retval 	0
*/

/*
* @name			dispimage
* @description	Display the image or racing track on OLED screen.
* @inputval		None
* @outputval	None
* @retval 		0
*/
unsigned char display_col[158]={0,0,1,2,3,4,4,5,6,7,8,8,9,10,11,12,12,13,14,
                                 15,16,17,17,18,19,20,21,21,22,23,24,25,25,26,27,
                                 28,29,29,30,31,32,33,34,34,35,36,37,38,38,39,40,
                                 41,42,42,43,44,45,46,46,47,48,49,50,51,51,52,53,
                                 54,55,55,56,57,58,59,59,60,61,62,63,64,64,65,66,
                                 67,68,68,69,70,71,72,72,73,74,75,76,76,77,78,79,
                                 80,81,81,82,83,84,85,85,86,87,88,89,89,90,91,92,
                                 93,93,94,95,96,97,98,98,99,100,101,102,102,103,104,
                                 105,106,106,107,108,109,110,110,111,112,113,114,115,
                                 115,116,117,118,119,119,120,121,122,123,123,124,125,126,127};



int getImageFeature(void)
{
  //50x158
  //First ,get the middle points of lower rows
  int i,j;
  volatile int lowPoints[5][2];
  int lowPointsGrayscale[5][2];
  int lowMiddlePoint[5];
  int raw_num[5*col_num];
  int temp;
  int row_count=0;
  int thresholdBlack=99;
  bool flagL,flagR;
  int firstMiddle;
 
  int deltWidth[5]={1000,1000,1000,1000,1000};
  
  temp=lastMiddlePoint;//this is the middle of cols
  firstMiddle=lastMiddlePoint;

  
  for(i=scanRowBegin;i<scanRowEnd;i++)
  {
    flagL=false;
    flagR=false;
    for(j=temp;j>=0;j--)
    {
      if(imgadd[i*col_num+j]<whiteRoad)
      {
        if(j==0)break;
        if(imgadd[i*col_num+j-1]<thresholdBlack)
        {
          lowPoints[row_count][0]=j;
          lowPointsGrayscale[row_count][0]=imgadd[i*col_num+j];
       //   printf("(%d ",imgadd[i*col_num+j]);
          flagL=true;
          break;
        }
      }
    }
    for(j=temp;j<col_num;j++)
    {
      if(j==col_num-1)break;
      if(imgadd[i*col_num+j]<whiteRoad)
      {
        if(imgadd[i*col_num+j+1]<thresholdBlack)
        {
          lowPoints[row_count][1]=j;
          lowPointsGrayscale[row_count][1]=imgadd[i*col_num+j];
       //   printf(" %d)\n",imgadd[i*col_num+j]);
          flagR=true;
          break;
        }
      }
    }
    if(!flagL){lowPoints[row_count][0]=0;lowPointsGrayscale[row_count][0]=-1;}
    if(!flagR){lowPoints[row_count][1]=col_num-1;lowPointsGrayscale[row_count][1]=-1;}
    
    lowMiddlePoint[row_count]=(lowPoints[row_count][0]+lowPoints[row_count][1])/2;
    deltWidth[row_count]=lowPoints[row_count][1]-lowPoints[row_count][0];
    row_count++;
  }
  //printf("%d %d : %d\n %d %d : %d\n %d %d : %d\n %d %d : %d\n %d %d : %d\n",lowPoints[0][0],lowPoints[0][1],lowMiddlePoint[0],lowPoints[1][0],lowPoints[1][1],lowMiddlePoint[1],lowPoints[2][0],lowPoints[2][1],lowMiddlePoint[2],lowPoints[3][0],lowPoints[3][1],lowMiddlePoint[3],lowPoints[4][0],lowPoints[4][1],lowMiddlePoint[4]);
  //printf("%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n",lowPointsGrayscale[0][0],lowPointsGrayscale[0][1],lowPointsGrayscale[1][0],lowPointsGrayscale[1][1],lowPointsGrayscale[2][0],lowPointsGrayscale[2][1],lowPointsGrayscale[3][0],lowPointsGrayscale[3][1],lowPointsGrayscale[4][0],lowPointsGrayscale[4][1]);
  //进行中值滤波
  //获取中点
  
  firstMiddle=lowMiddlePoint[0]+lowMiddlePoint[1]+lowMiddlePoint[2]+lowMiddlePoint[3]+lowMiddlePoint[4];
  firstMiddle/=5;
  if(firstMiddle<10)
    lastMiddlePoint=10;
  else if(firstMiddle>col_num-10)
    lastMiddlePoint=col_num-10;
  else
    lastMiddlePoint=firstMiddle;
 // printf("Middle:%d\n",firstMiddle);
  
  deltMiddleWidth=deltWidth[0]+deltWidth[1]+deltWidth[2]+deltWidth[3]+deltWidth[4];
  deltMiddleWidth=deltMiddleWidth/5;
  if(deltMiddleWidth<15)//这里是黑线间隔的阈值  需要实际测量的
  {
    i=(scanRowBegin+2)*col_num;
    blackCount=1;
    whiteCount=1;
    #define blackBlock 0
    #define whiteBlock 1
    uint8_t lastBlock=blackBlock;
    for(int j=0;j<col_num;j++)
    {
       if(imgadd[i+j]<thresholdBlack)//black
       {
         if(lastBlock==whiteBlock)
         {
           blackCount++;
           lastBlock=blackBlock;
         }
       }
       else//white
       {
         if(lastBlock==blackBlock)
         {
           whiteCount++;
           lastBlock=whiteBlock;
         }
       }
       if(whiteCount>=9&&blackCount>=9)
        doWeRun=false;
       else
         doWeRun=true;
    }
  }
  else
  {
    doWeRun=true;//这里有可能要注释掉  看最后的刹车速度
  }
  
  return firstMiddle;
}


void dispimage(void){
	uint16_t i=0, j=0;
        middleX=0;
        char showInfo[4];
        float output;
     
        
	//使用OLED画出摄像头的图像
	for(i = 0; i<row_num; i++){

		for(j = 0; j<col_num ;j++){
                  if(imgadd[i*col_num + j] > whiteRoad)
				OLED_DrawPoint(display_col[j],i+14,0);
			else
				OLED_DrawPoint(display_col[j],i+14,1);						
		}
	}
        
        
        i= scanRowBegin;
        for(j=0;j<col_num;j++)
          OLED_DrawPoint(display_col[j],i+14,1);
        
        i= scanRowEnd;
        for(j=0;j<col_num;j++)
          OLED_DrawPoint(display_col[j],i+14,1);
        
        
        middleX=getImageFeature();
        j=display_col[middleX];
        for(i=0;i<row_num;i++)
          OLED_DrawPoint(j,i+14,1);	
        
        
        
        InFifo(&fifoData,middleX);
        
        middleX=middleX-col_num/2;
        
        
        #define MAX_PULSE               940
        #define MIN_PULSE               760
        #define MIDDLE_PULSE            852
        
        if(abs(deltBias)<8)angle_kp=angle_kp_low;
        else if(abs(deltBias)<13) angle_kp=angle_kp_high;
        else angle_kp=angle_kp_high;
        
        angle_error=middleX;
        output=MIDDLE_PULSE+angle_kp*angle_error+angle_kd*(angle_error-angle_lasterror);

        if(output<MIN_PULSE)output=MIN_PULSE;
        if(output>MAX_PULSE)output=MAX_PULSE;
        
        
        FTM_QD_GetData(HW_FTM1, &QDvalue, &QDdirection);
        QDvalue=-QDvalue;
        uint16_t temp=QDvalue-lastCounterValue;
        if(!(temp>>15))deltValue=temp;
        lastCounterValue=QDvalue;
        speedValue=deltValue*3;//calculate speed
        
       // printf("%d %d %d\n",QDvalue,lastCounterValue,(int)speedValue);
        
        FTM_PWM_ChangeDuty(HW_FTM2, HW_FTM_CH0,(int)output);//设置舵机pwm输出
	OLED_Refresh_Gram();
}


