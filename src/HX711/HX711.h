/*
 * FILE: HX711.h
 * 
 * VERSION: 0.1
 * PURPOSE: HX711 weight library for Nucleo STM32
 * AUTHOR: Bertrand Bouvier
 * LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 * DATASHEET: http://www.dfrobot.com/image/data/SEN0160/hx711_english.pdf
 * URL: 
 *
 * HISTORY:
 * 24/05/2015 - Bertrand Bouvier - Original version
 * see HX711.cpp
 *
 * SPECIAL THANKS:
 *  Inspiré du travail de Weihong Guan (@aguegu)
 *  https://github.com/aguegu/Arduino
 *  http://aguegu.net/
 *
 *  Inspiré du travail de bodge
 *  https://github.com/bogde/HX711
 *
 */
 
#ifndef HX711_H
#define HX711_H
 
#include "mbed.h"
 
 
class HX711
{
 
public:
    HX711(PinName pinData, PinName pinSck,uint8_t gain = 128);
    ~HX711();
    int getValue(void);
    int averageValue(uint8_t times);
    void setOffset(int offset);
    void setScale(float scale);
    float getGram();
    void setGain(uint8_t gain);
    void powerDown();
    void powerUp();
    void tare(uint8_t times = 10);
 
 
private:
    PinName _pinData;
    PinName _pinSck;
    int _offset;
    float _scale;
    uint8_t _gain; //[128|32|64]

 
};
 
#endif