#ifdef TARGET_LPC11XX_11CXX

#include "WakeInterruptIn.h"

WakeInterruptIn* WakeInterruptIn::objects[NUM_CHANNEL] = {0};

WakeInterruptIn::WakeInterruptIn(PinName pin) : DigitalIn(pin)
{
    if (pin < P1_0)
        channel = (pin >> PIN_SHIFT) & 0xF;
    else if (pin == P1_0)
        channel = 12;
    else
        error("Pin is not valid for WakeInterruptIn");

    objects[channel] = this;
    switch (channel) {
        case 0:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler0);
            break;
        case 1:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler1);
            break;
        case 2:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler2);
            break;
        case 3:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler3);
            break;
        case 4:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler4);
            break;
        case 5:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler5);
            break;
        case 6:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler6);
            break;
        case 7:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler7);
            break;
        case 8:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler8);
            break;
        case 9:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler9);
            break;
        case 10:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler10);
            break;
        case 11:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler11);
            break;
        case 12:
            NVIC_SetVector((IRQn_Type)channel, (uint32_t)handler12);
            break;
    }
}

#endif