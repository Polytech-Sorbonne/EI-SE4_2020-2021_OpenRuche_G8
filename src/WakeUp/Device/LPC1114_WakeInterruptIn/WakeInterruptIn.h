/*
Most mbed targets can be woken up from deepsleep by regular InterruptIn.
On the LPC1114 this has to be done by different interrupts. This
simple library helps with that
*/

//Only include it for correct target. This allows me to re-use it in WakeUp lib
#ifdef TARGET_LPC11XX_11CXX

#ifndef WAKEINTERRUPTIN_H
#define WAKEINTERRUPTIN_H

#include "mbed.h"
#define NUM_CHANNEL     13

/** Class to wake LPC1114 from deepsleep
*
* Use like regular InterruptIn, it works not only 
* to wake up from sleep but also just like InterruptIn.
* Only you can NOT attach both rising and falling edge interrupts.
*/
class WakeInterruptIn : DigitalIn {
    public:
    
    /** Constructor
    *
    * Pins which are allowed are all pins from Port 0 and P1_0
    *
    * @param pin Pin to use as WakeInterruptIn pin
    */
    WakeInterruptIn(PinName pin);
        
    ~WakeInterruptIn() {
        disable();
        objects[channel] = NULL;
    }
    
    /* Attach rising edge interrupt
    *
    * Attaching a member function can be done the regular way too
    *
    * @argument fptr Pointer to function to call, NULL to disable interrupt
    */
    void rise(void (*fptr)(void)) {
        fpointer.attach(fptr);
        LPC_SYSCON->STARTAPRP0 |= (1 << channel);
        if (fptr == NULL)
            disable();
        else
            enable();
    }  
    template<typename T>
    void rise(T* tptr, void (T::*mptr)(void)) {
        fpointer.attach(tptr, mptr);
        LPC_SYSCON->STARTAPRP0 |= (1 << channel);
        if (fptr == NULL)
            disable();
        else
            enable();
    }
        
    /* Attach falling edge interrupt
    *
    * Attaching a member function can be done the regular way too
    *
    * @argument fptr Pointer to function to call, NULL to disable interrupt
    */
    void fall(void (*fptr)(void)) {
        fpointer.attach(fptr);
        LPC_SYSCON->STARTAPRP0 &= ~(1 << channel);
        if (fptr == NULL)
            disable();
        else
            enable();
    }  
    template<typename T>
    void fall(T* tptr, void (T::*mptr)(void)) {
        fpointer.attach(tptr, mptr);
        LPC_SYSCON->STARTAPRP0 &= ~(1 << channel);
        if (fptr == NULL)
            disable();
        else
            enable();
    }
   
    
    
    private:
    uint8_t channel;
    FunctionPointer fpointer;
    
    void enable(void) {
        LPC_SYSCON->STARTRSRP0CLR = 1 << channel;
        LPC_SYSCON->STARTERP0 |= (1 << channel);
        NVIC_EnableIRQ((IRQn_Type)channel);
    }
    
    void disable(void) {
        LPC_SYSCON->STARTERP0 &= ~(1 << channel);
        NVIC_DisableIRQ((IRQn_Type)channel);
    }
    
    void handle( void ) {
        LPC_SYSCON->STARTRSRP0CLR = 1 << channel;
        fpointer.call();
    }
    
    static WakeInterruptIn* objects[NUM_CHANNEL];
    static void handler0(void) { objects[0]->handle(); }
    static void handler1(void) { objects[1]->handle(); }
    static void handler2(void) { objects[2]->handle(); }
    static void handler3(void) { objects[3]->handle(); }
    static void handler4(void) { objects[4]->handle(); }
    static void handler5(void) { objects[5]->handle(); }
    static void handler6(void) { objects[6]->handle(); }
    static void handler7(void) { objects[7]->handle(); }
    static void handler8(void) { objects[8]->handle(); }
    static void handler9(void) { objects[9]->handle(); }
    static void handler10(void) { objects[10]->handle(); }
    static void handler11(void) { objects[11]->handle(); }
    static void handler12(void) { objects[12]->handle(); }
    
};
#endif
#endif