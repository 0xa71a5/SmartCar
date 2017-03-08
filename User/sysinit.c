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
  //使能所有端口的时钟。以配置引脚的复用 。
  //System Clock Gating Control Register 5
  //Use MASK to set the corresponding bit to 1,represents "enable".These bits control the clock gate to Port ? module.
  SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK );
  
  //使用锁相环，配置系统时钟。(超频至200MHz，K60较为稳定，超频至200MHz亦可稳定运行。)
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
  
  //芯片上电后，或复位后，默认处于FEI模式。首先移动到FBE模式（目标模式：PEE）。 
  //Multipurpose Clock Generator
  //MCG Control 2 Register
  //The MCG restricts transitions between modes.
  //High Gain Oscillator Select.
  //External Reference Select 
  MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK;
  //初始化晶振后释放锁定状态的振荡器和GPIO。
  //System Clock Gating Control Register 4
  //Enable the clock of LLWU(Low-Leakage Wake-up Unit) module.
  SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK;
  //Regulator Status and Control Register of Power Management Controller
  PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
  
  //选择外部晶振，参考分频器，清IREFS来启动外部晶振。
  //MCG Control 1 Register
  //CLKS(10b):External reference clock is selected.
  //FRDIV(011b):if RANGE=0,Divide Factor is 8;for all other RANGE values,Divide Factor is 256.
  MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);
  
  //等待时钟切换到外部参考时钟。
  //MCG Status Register
  // wait for oscillator to initialize.
  //This bit,which resets to 0,is set to 1 after the initialization cycles of the crystal oscillator clock have completed.
  while (!(MCG_S & MCG_S_OSCINIT0_MASK)){};  
  //When IREFST is 0,source of FLL reference clock is the external reference clock.Then exit the loop.
  while (MCG_S & MCG_S_IREFST_MASK){}  
  //CLKST bits indicate the current clock mode.
  //CLKST(10b):External reference clock is selected.
  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2){}
  
  //进入FBE模式,外部晶振16M,4分频=4M。
  //MCG Control 5 Register
  //PRDIV(011b):Divide Factor is 4.
  MCG_C5 = MCG_C5_PRDIV0(0x03);                
  
  //确保MCG_C6处于复位状态，禁止LOLIE、PLL、和时钟控制器，清PLL VCO分频器。
  //MCG Control 6 Register
  MCG_C6 = 0x0;
  
  //保存FMC_PFAPR当前的值。
  //Flash Access Protection Register
  temp_reg = FMC_PFAPR;
  
  //通过M&PFD置位M0PFD来禁止预取功能。
  //If thest bits is set to 1,Prefetching for this master is disable.
  FMC_PFAPR |= FMC_PFAPR_M7PFD_MASK | FMC_PFAPR_M6PFD_MASK | FMC_PFAPR_M5PFD_MASK
    | FMC_PFAPR_M4PFD_MASK | FMC_PFAPR_M3PFD_MASK | FMC_PFAPR_M2PFD_MASK
      | FMC_PFAPR_M1PFD_MASK | FMC_PFAPR_M0PFD_MASK;    
  
  //设置系统分频器。
  //MCG=PLL
  //System Clock Divider Register 1
  //Core/system clock:		Divide-by-1
  //The peripheral clock:	Divide-by-2
  //The FlexBus clock:		Divide-by-2
  //Flash clock:		Divide-by-8
  SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) 
    | SIM_CLKDIV1_OUTDIV3(1) | SIM_CLKDIV1_OUTDIV4(7);       
  
  //重新写入FMC_PFAPR的原始值。
  FMC_PFAPR = temp_reg; 
  
  //设置VCO分频器，使能PLL为200MHz, LOLIE=0, PLLS=1, CME=0, VDIV=26
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
  
  //进入PBE模式    
  //通过清零CLKS位来进入PEE模式
  // CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0
  //MCG Control 1 Register
  //CLKS(00b):Output of FLL or PLL is selected(depends on PLLS control bit).
  MCG_C1 &= ~MCG_C1_CLKS_MASK;
  
  //等待时钟状态位更新
  //MCG Status Register
  //CLKST(11b):Output of the PLL is selected.
  while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3){};
  
  //设置跟踪时钟为内核时钟
  //System Option Register 2
  //if TRACECLKSEL bit is 1,debug trace clock is core/system clock.
  SIM_SOPT2 |= SIM_SOPT2_TRACECLKSEL_MASK;	  
  //使能FlexBus模块时钟
  //System Clock Gating Control Register 7 
  //If FLEXBUS bit is 1,then the clock to FlexBus module enables. 
  SIM_SCGC7 |= SIM_SCGC7_FLEXBUS_MASK;
}

