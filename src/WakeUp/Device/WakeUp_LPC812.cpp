#ifdef TARGET_LPC812

#include "WakeUp.h"

FunctionPointer WakeUp::callback;
float WakeUp::cycles_per_ms = 10.0;

void WakeUp::set_ms(uint32_t ms)
{
    //Enable clock to register interface:
    LPC_SYSCON->SYSAHBCLKCTRL |= 1<<9;

    //Clear the counter:
    LPC_WKT->CTRL |= 1<<2;
    if (ms != 0) {
        //Enable clock to register interface:
        LPC_SYSCON->SYSAHBCLKCTRL |= 1<<9;

        //Set 10kHz timer as source, and just to be sure clear status bit
        LPC_WKT->CTRL = 3;

        //Enable the 10kHz timer
        LPC_PMU->DPDCTRL |= (1<<2) | (1<<3);

        //Set interrupts
        NVIC_SetVector(WKT_IRQn, (uint32_t)WakeUp::irq_handler);
        NVIC_EnableIRQ(WKT_IRQn);

        //Load the timer
        LPC_WKT->COUNT = (uint32_t)((float)ms * cycles_per_ms);

    } else {
        //Disable clock to register interface:
        LPC_SYSCON->SYSAHBCLKCTRL &= ~(1<<9);

        //Disable the 10kHz timer
        LPC_PMU->DPDCTRL &= ~((1<<2) | (1<<3));
    }
}

void WakeUp::irq_handler(void)
{
    //Clear status
    LPC_WKT->CTRL |= 2;

    //Disable clock to register interface:
    LPC_SYSCON->SYSAHBCLKCTRL &= ~(1<<9);

    //Disable the 10kHz timer
    LPC_PMU->DPDCTRL &= ~((1<<2) | (1<<3));

    callback.call();
}

void WakeUp::calibrate(void)
{
    cycles_per_ms = 10.0;
    set_ms(1100);
    wait_ms(100);

    uint32_t prevread = LPC_WKT->COUNT;
    uint32_t read = LPC_WKT->COUNT;
    while( read != prevread) {
        prevread = read;
        read = LPC_WKT->COUNT;
    }

    uint32_t ticks = 11000 - read;

    cycles_per_ms = ticks / 100.0;
    set_ms(0);
}

#endif
