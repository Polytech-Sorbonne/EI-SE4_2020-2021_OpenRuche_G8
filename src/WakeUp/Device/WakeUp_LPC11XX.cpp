/**
See homepage of this lib for LPC11xx special treatment
**/

#ifdef TARGET_LPC11XX_11CXX

#include "WakeUp.h"
#include "WakeInterruptIn.h"
#include "pinmap.h"
#include "toolchain.h"

//Pin used, allowed pins = P0_1 (dp24, default), P0_8 (dp1) and P0_9 (dp2)
//By defining WakeUpPin in for example your main.cpp this can be overridden
WEAK PinName WakeUpPin = dp24;
extern PinName WakeUpPin;

WakeInterruptIn IRQ_in(WakeUpPin);
PwmOut pulse_out(WakeUpPin);

FunctionPointer WakeUp::callback;
float WakeUp::cycles_per_ms = 20.0;

static uint32_t old_clk_sel = ~0;
static uint32_t SYSAHBCLKCTRL;
static uint32_t TCR, PR, MR3;
static LPC_TMR_TypeDef *WakeUpTimer;
static uint32_t SYSAHBCLKCTRL_Sleep;
static uint8_t WakeUpTimer_Match;

static inline void restore(void);


void WakeUp::set_ms(uint32_t ms)
{
    if (old_clk_sel == ~0) {                                //Only during first run
        old_clk_sel = LPC_SYSCON->MAINCLKSEL;
        SYSAHBCLKCTRL = LPC_SYSCON->SYSAHBCLKCTRL;
        
        switch(WakeUpPin) {
            case dp24:
                WakeUpTimer = LPC_TMR32B0;
                SYSAHBCLKCTRL_Sleep = 0x15 | (1<<9);
                WakeUpTimer_Match = 2;
                break;
            case dp1:
                WakeUpTimer = LPC_TMR16B0;
                SYSAHBCLKCTRL_Sleep = 0x15 | (1<<7);
                WakeUpTimer_Match = 0;
                break;
            case dp2:
                WakeUpTimer = LPC_TMR16B0;
                SYSAHBCLKCTRL_Sleep = 0x15 | (1<<7);
                WakeUpTimer_Match = 1;
                break;
            default:
                error("Invalid WakeUp pin, choose dp1, dp2 or dp24");
        }            
    }
        
    if (ms != 0) {        
        if (LPC_SYSCON->SYSAHBCLKCTRL != SYSAHBCLKCTRL_Sleep)    //Always when it is different from sleep settings
            SYSAHBCLKCTRL = LPC_SYSCON->SYSAHBCLKCTRL;

        LPC_SYSCON->PDRUNCFG &= ~PDRUNCFG_WDTOSC_PD;
        LPC_SYSCON->PDSLEEPCFG = 0x000018B7 | (LPC_SYSCON->PDRUNCFG & (PDRUNCFG_WDTOSC_PD | PDRUNCFG_BOD_PD)); 
        
        //Set oscillator for 20kHz
        LPC_SYSCON->WDTOSCCTRL = 14 | (1<<5);
        
        //Store old PWM
        TCR = WakeUpTimer->TCR;
        PR = WakeUpTimer->PR;
        MR3 = WakeUpTimer->MR3;
        
        //Setup PWM
        WakeUpTimer->TCR = TMR16B0TCR_CRST;
        uint32_t ticks = (float)ms * cycles_per_ms;
        
        //whatever timer it is, we treat it as 16-bit (with PR that is 32-bit still, do the math, it is enough for this)
        WakeUpTimer->PR = ticks >> 16;
        WakeUpTimer->MR[WakeUpTimer_Match] = ticks / ((ticks >> 16) + 1);
        WakeUpTimer->MR3 = 0xFFFF;
                
        IRQ_in.rise(irq_handler);
        
        //Disable most peripherals
        LPC_SYSCON->SYSAHBCLKCTRL = SYSAHBCLKCTRL_Sleep;
        
        //Switch clock to WD OSC
        LPC_SYSCON->MAINCLKSEL = 0x2;
        LPC_SYSCON->MAINCLKUEN = 0;
        LPC_SYSCON->MAINCLKUEN = MAINCLKUEN_ENA;
        
        //Enable PWM:
        WakeUpTimer->TCR = TMR16B0TCR_CEN;
    } else {
        //Else restore normal settings
        restore();
    }
    
}

void WakeUp::irq_handler(void)
{    
    restore();    
    callback.call();
}

void WakeUp::calibrate(void)
{
    //Save current pin function
    __IO uint32_t *reg = (__IO uint32_t*)(LPC_IOCON_BASE + (dp24 & 0xFF));
    uint32_t old_pinfun = *reg;
    
    //Set oscillator for 20kHz
    LPC_SYSCON->PDRUNCFG &= ~PDRUNCFG_WDTOSC_PD;
    LPC_SYSCON->WDTOSCCTRL = 14 | (1<<5);
    
    //Direct WDT to the CLKOUT pin (dp24), sample it back
    DigitalIn din(dp24);
    Timer timer;
    
    LPC_SYSCON->CLKOUTDIV = 1;
    LPC_SYSCON->CLKOUTCLKSEL = 0x2;
    LPC_SYSCON->CLKOUTUEN = 0;
    LPC_SYSCON->CLKOUTUEN = CLKOUTUEN_ENA;
    pin_function(dp24, 1);
    
    int count = 0;
    timer.start();
    while (timer.read_ms() < 100) {
        while (din.read() == 0);
        while (din.read() == 1);
        count++;
    }
    cycles_per_ms = (float)count / 100.0f;
    
    //Set old pin function back, disable CLKOUT
    *reg = old_pinfun;
    LPC_SYSCON->CLKOUTDIV = 0;
}

static inline void restore(void) {
        
    WakeUpTimer->MR[WakeUpTimer_Match] = 0xFFFFFFFF;

    if (old_clk_sel == 3)                           //Was running on PLL
        while(LPC_SYSCON->SYSPLLSTAT != SYSPLLSTAT_LOCK);
    
    if (old_clk_sel < 4) {  //If valid setting
        LPC_SYSCON->MAINCLKSEL = old_clk_sel;
        LPC_SYSCON->MAINCLKUEN = 0;
        LPC_SYSCON->MAINCLKUEN = MAINCLKUEN_ENA;
    }
    
    IRQ_in.rise(NULL);
    
    LPC_SYSCON->SYSAHBCLKCTRL = SYSAHBCLKCTRL; 
    
    WakeUpTimer->MR3 = MR3;
    WakeUpTimer->PR = PR;
    WakeUpTimer->TCR = TCR;
}

#endif
