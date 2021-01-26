#if defined(TARGET_Freescale)

#include "WakeUp.h"
#include "us_ticker_api.h"

FunctionPointer WakeUp::callback;
float WakeUp::cycles_per_ms = 1.0;

static uint16_t remainder_count;
static uint32_t oldvector;
static uint8_t oldPSR;

//See if we have a 32kHz crystal on the clock input
//Check if the DMX32 bit is set, not perfect, but most cases will work
static inline bool is32kXtal(void) {
    return (MCG->C4 & MCG_C4_DMX32_MASK);
}

void restore(void);

void WakeUp::set_ms(uint32_t ms)
{
    /* Clock the timer */
    SIM->SCGC5 |= 0x1u;
    
    //Check if it is running, in that case, store current values
    remainder_count = 0;
    if (NVIC_GetVector(LPTimer_IRQn) != (uint32_t)WakeUp::irq_handler) {
        oldvector = NVIC_GetVector(LPTimer_IRQn);
        oldPSR = LPTMR0->PSR;
        
        if (LPTMR0->CSR & LPTMR_CSR_TIE_MASK) {
            //Write first to sync value
            LPTMR0->CNR = 0;
            uint16_t countval = LPTMR0->CNR;
            if (countval < LPTMR0->CMR)
                remainder_count = countval - LPTMR0->CMR;
        }
    }
    
    LPTMR0->CSR = 0;

    if (ms != 0) {
        /* Set interrupt handler */
        NVIC_SetVector(LPTimer_IRQn, (uint32_t)WakeUp::irq_handler);
        NVIC_EnableIRQ(LPTimer_IRQn);
        
        uint32_t counts;
        //Set clock
        if (is32kXtal()) {
            SIM->SOPT1 &= ~SIM_SOPT1_OSC32KSEL_MASK;    //Put RTC/LPTMR on 32kHz external. 
            #ifdef OSC0
            OSC0->CR |= OSC_CR_EREFSTEN_MASK;
            #else
            OSC->CR |= OSC_CR_EREFSTEN_MASK;
            #endif
            LPTMR0->PSR = LPTMR_PSR_PCS(2);
            counts = (uint32_t)((float)ms * 32.768f);
        } else {
            //Clock from the 1kHz LPO
            LPTMR0->PSR = LPTMR_PSR_PCS(1);
            counts = (uint32_t)((float)ms * cycles_per_ms);
        }
        
        //If no prescaler is needed
        if (counts <= 0xFFFF) 
            LPTMR0->PSR |= LPTMR_PSR_PBYP_MASK;
        else {        //Otherwise increase prescaler until it fits
            counts >>= 1;
            uint32_t prescaler = 0;
            while (counts > 0xFFFF) {
                counts >>= 1;
                prescaler++;
            }
            LPTMR0->PSR |= LPTMR_PSR_PRESCALE(prescaler);
        }
        LPTMR0->CMR = counts;        

        LPTMR0->CSR = LPTMR_CSR_TIE_MASK;
        LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;
    } else {
        restore();
    }

}


void WakeUp::irq_handler(void)
{
    // write 1 to TCF to clear the LPT timer compare flag
    LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;
    restore();
    callback.call();
}

void WakeUp::calibrate(void)
{
    if (!is32kXtal()) {
        wait_us(1);     //Otherwise next wait might overwrite our settings
        cycles_per_ms = 1.0;
        set_ms(1100);
        wait_ms(100);
    
        //Write first to sync value
        LPTMR0->CNR = 0;
        uint32_t ticks = LPTMR0->CNR;
        cycles_per_ms = ticks / 100.0;
        set_ms(0);
    }
}

void restore(void){
    /* Reset */
    LPTMR0->CSR = 0;
    
    /* Set interrupt handler */
    NVIC_SetVector(LPTimer_IRQn, oldvector);
    NVIC_EnableIRQ(LPTimer_IRQn);
    
    /* Clock at (1)MHz -> (1)tick/us */
    LPTMR0->PSR = oldPSR;

    if (remainder_count) {  
        /* Set the compare register */
        LPTMR0->CMR = remainder_count;
        
        /* Enable interrupt */
        LPTMR0->CSR |= LPTMR_CSR_TIE_MASK;
        
        /* Start the timer */
        LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;
    }
}

#endif