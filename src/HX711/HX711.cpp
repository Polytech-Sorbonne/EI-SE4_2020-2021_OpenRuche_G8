/*
 * FILE: HX711.cpp
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
 * see HX711.h
 *
 * SPECIAL THANKS:
 *  Inspiré du travail de Weihong Guan (@aguegu)
 *  https://github.com/aguegu/Arduino
 *  http://aguegu.net
 *  
 *  Inspiré du travail de bodge
 *  https://github.com/bogde/HX711
 *
 */
 
#include "HX711.h"
#include "mbed.h"
 
// #define SCALE_VALUE 259.79 //multiple propre à chaque hardware

#define SCALE_VALUE -43*259.79*2 //multiple propre à chaque hardware
 
HX711::HX711(PinName pinData, PinName pinSck, uint8_t gain)
{
    _pinData = pinData;
    _pinSck = pinSck;
    
    this->setGain(gain);
    
    DigitalOut sck(_pinSck);
    DigitalIn data(_pinData);
    
    sck = 1;        //Initialisation HX711
    wait_us(100);
    sck = 0;
    this->setScale(SCALE_VALUE);        //Réglage SCALE
    int poids_ruche_initial = 13200; //En grammes
    this->setOffset(poids_ruche_initial*609.9979);
    //8051961.00/13200
}
 
HX711::~HX711()
{
 
}
 
int HX711::averageValue(uint8_t times) //Calcule une moyenne sur plusieurs mesures 
{
    int sum = 0;
    for (int i = 0; i < times; i++)
    {
        sum += getValue();
    }
    //printf("average value:%d \r\n", sum/times);
    return sum / times;
}
 
int HX711::getValue() //Obtenir la valeur brut du controller
{
    int buffer; 
    buffer = 0 ;
    
    DigitalOut sck(_pinSck);
    DigitalIn data(_pinData);
    
    while (data.read()==1) //wait is ready
    ;
    
    for (uint8_t i = 24; i--;) //read 24 bit 1 per 1 and save to buffer
    {
        sck=1;
 
        buffer = buffer << 1 ;
    
        if (data.read())
        {
            buffer ++;
        }
        
        sck=0;
    }
    
    for (int i = 0; i < _gain; i++)
    {
        sck=1; 
        sck=0;
    }
    
    buffer = buffer ^ 0x800000;
 
    return buffer;
}
 
void HX711::setOffset(int offset)
{
    _offset = offset;
}
 
void HX711::setScale(float scale)
{
    _scale = scale;
}
 
float HX711::getGram()
{
    long val = (averageValue(2) - _offset);
    return (float) val / _scale;
}
 // gram = val/scale 
 // val = gram * scale 
 // ave - offset = gram * scale 
 // -offset = gram*scale -ave
 // offset = -gram*scale + ave
void HX711::setGain(uint8_t  gain) 
{
    switch (gain) 
    { 
        case 128:       // channel A, gain factor 128 
            _gain = 1; 
            break; 
            
        case 64:        // channel A, gain factor 64 
            _gain = 3; 
            break; 
            
        case 32:        // channel B, gain factor 32 
            _gain = 2; 
            break; 
    } 
    DigitalOut sck(_pinSck);
    sck = 0;
    getValue();
}
 
void HX711::powerDown() 
{
    DigitalOut sck(_pinSck);
    sck = 0;
    sck = 1;
}
 
void HX711::powerUp() 
{
    DigitalOut sck(_pinSck);
    sck = 0;
}
 
void HX711::tare(uint8_t times) 
{
    int sum = averageValue(times);
    setOffset(sum);
}
            