/* Includes ------------------------------------------------------------------*/
#include "MK60DZ10.h"
#include "sysinit.h"
/* Private variables ---------------------------------------------------------*/
int periph_clk_khz;

/**
* @name		System initialization
* @description	Configure the PLL,and set the frequency of core/system clock.
* @inputval 	None
* @outputval	None
* @retval 	None
*/
void sysinit (void)
{
  //ʹ�����ж˿ڵ�ʱ�ӡ����������ŵĸ��� ��
  //System Clock Gating Control Register 5
  //Use MASK to set the corresponding bit to 1,represents "enable".These bits control the clock gate to Port ? module.
  SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK );
  
  //ʹ�����໷������ϵͳʱ�ӡ�(��Ƶ��200MHz��K60��Ϊ�ȶ�����Ƶ��200MHz����ȶ����С�)
  PLL_INIT_200M();
  //System Clock Divider Register 1
  //periph_clk_khz = 200MHz / (0+1) = 200MHz.
  //periph_clk_khz = core_clk_khz /(((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1);  
}


/**
* @name		PLL initialization
* @description	Initialize the PLL to get the target frequency.
* @inputval 	None
* @outputval	None
* @retval 	None
*/
void PLL_INIT_200M(void)
{
  uint32_t temp_reg;
  
  //оƬ�ϵ�󣬻�λ��Ĭ�ϴ���FEIģʽ�������ƶ���FBEģʽ��Ŀ��ģʽ��PEE���� 
  //Multipurpose Clock Generator
  //MCG Control 2 Register
  //The MCG restricts transitions between modes.
  //High Gain Oscillator Select.
  //External Reference Select 
  MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK;
  //��ʼ��������ͷ�����״̬��������GPIO��
  //System Clock Gating Control Register 4
  //Enable the clock of LLWU(Low-Leakage Wake-up Unit) module.
  SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK;
  //Regulator Status and Control Register of Power Management Controller
  PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
  
  //ѡ���ⲿ���񣬲ο���Ƶ������IREFS�������ⲿ����
  //MCG Control 1 Register
  //CLKS(10b):External reference clock is selected.
  //FRDIV(011b):if RANGE=0,Divide Factor is 8;for all other RANGE values,Divide Factor is 256.
  MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);
  
  //�ȴ�ʱ���л����ⲿ�ο�ʱ�ӡ�
  //MCG Status Register
  // wait for oscillator to initialize.
  //This bit,which resets to 0,is set to 1 after the initialization cycles of the crystal oscillator clock have completed.
  while (!(MCG_S & MCG_S_OSCINIT0_MASK)){};  
  //When IREFST is 0,source of FLL reference clock is the external reference clock.Then exit the loop.
  while (MCG_S & MCG_S_IREFST_MASK){}  
  //CLKST bits indicate the current clock mode.
  //CLKST(10b):External reference clock is selected.
  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2){}
  
  //����FBEģʽ,�ⲿ����16M,4��Ƶ=4M��
  //MCG Control 5 Register
  //PRDIV(011b):Divide Factor is 4.
  MCG_C5 = MCG_C5_PRDIV0(0x03);                
  
  //ȷ��MCG_C6���ڸ�λ״̬����ֹLOLIE��PLL����ʱ�ӿ���������PLL VCO��Ƶ����
  //MCG Control 6 Register
  MCG_C6 = 0x0;
  
  //����FMC_PFAPR��ǰ��ֵ��
  //Flash Access Protection Register
  temp_reg = FMC_PFAPR;
  
  //ͨ��M&PFD��λM0PFD����ֹԤȡ���ܡ�
  //If thest bits is set to 1,Prefetching for this master is disable.
  FMC_PFAPR |= FMC_PFAPR_M7PFD_MASK | FMC_PFAPR_M6PFD_MASK | FMC_PFAPR_M5PFD_MASK
    | FMC_PFAPR_M4PFD_MASK | FMC_PFAPR_M3PFD_MASK | FMC_PFAPR_M2PFD_MASK
      | FMC_PFAPR_M1PFD_MASK | FMC_PFAPR_M0PFD_MASK;    
  
  //����ϵͳ��Ƶ����
  //MCG=PLL
  //System Clock Divider Register 1
  //Core/system clock:		Divide-by-1
  //The peripheral clock:	Divide-by-2
  //The FlexBus clock:		Divide-by-2
  //Flash clock:		Divide-by-8
  SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) 
    | SIM_CLKDIV1_OUTDIV3(1) | SIM_CLKDIV1_OUTDIV4(7);       
  
  //����д��FMC_PFAPR��ԭʼֵ��
  FMC_PFAPR = temp_reg; 
  
  //����VCO��Ƶ����ʹ��PLLΪ200MHz, LOLIE=0, PLLS=1, CME=0, VDIV=26
  //MCG Control 6 Register
  //If the PLLS is set,the FLL is disable in all modes.
  //VDIV(11010b):Multiply Factor is 50. 4*50MHz=200MHz.
  MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV0(0x1A);      
  //MCG Status Registe
  //PLLST bit indicates the clock source selected by PLLS.If PLLST is 1,source of PLLS clock is PLL clock.
  //wait for PLL status bit to set.
  while (!(MCG_S & MCG_S_PLLST_MASK)){};    
  //If the LOCK bit is 1,PLL is currently locked.
  //Wait for LOCK bit to set.
  while (!(MCG_S & MCG_S_LOCK0_MASK)){}; 
  
  //����PBEģʽ    
  //ͨ������CLKSλ������PEEģʽ
  // CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0
  //MCG Control 1 Register
  //CLKS(00b):Output of FLL or PLL is selected(depends on PLLS control bit).
  MCG_C1 &= ~MCG_C1_CLKS_MASK;
  
  //�ȴ�ʱ��״̬λ����
  //MCG Status Register
  //CLKST(11b):Output of the PLL is selected.
  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3){};
  
  //���ø���ʱ��Ϊ�ں�ʱ��
  //System Option Register 2
  //if TRACECLKSEL bit is 1,debug trace clock is core/system clock.
  SIM_SOPT2 |= SIM_SOPT2_TRACECLKSEL_MASK;	  
  //ʹ��FlexBusģ��ʱ��
  //System Clock Gating Control Register 7 
  //If FLEXBUS bit is 1,then the clock to FlexBus module enables. 
  SIM_SCGC7 |= SIM_SCGC7_FLEXBUS_MASK;
}

