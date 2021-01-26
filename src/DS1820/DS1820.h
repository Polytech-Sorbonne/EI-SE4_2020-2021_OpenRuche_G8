#ifndef DS1820_H_
    #define DS1820_H_

    #include <OneWire.h>

/**
 * Dallas' DS1820 family temperature sensor.
 * This library depends on the OneWire library (Dallas' 1-Wire bus protocol implementation)
 * available at <http://developer.mbed.org/users/hudakz/code/OneWire/>
 *
 * Example of use:
 * 
 * @code
 * 
 * Single sensor.
 *
 * #include "mbed.h"
 * #include "DS1820.h"
 * 
 * Serial      pc(USBTX, USBRX);
 * DigitalOut  led(LED1);
 * DS1820      ds1820(D8);  // substitute D8 with actual mbed pin name connected to 1-wire bus
 * float       temp = 0;
 * int         result = 0;
 * 
 * int main()
 * {
 *     pc.printf("\r\n--Starting--\r\n");
 *     if (ds1820.begin()) {
 *         while (1) {
 *             ds1820.startConversion();   // start temperature conversion from analog to digital
 *             wait(1.0);                  // let DS1820 complete the temperature conversion
 *             result = ds1820.read(temp); // read temperature from DS1820 and perform cyclic redundancy check (CRC)
 *             switch (result) {
 *                 case 0:                 // no errors -> 'temp' contains the value of measured temperature
 *                     pc.printf("temp = %3.1f%cC\r\n", temp, 176);
 *                     break;
 * 
 *                 case 1:                 // no sensor present -> 'temp' is not updated
 *                     pc.printf("no sensor present\n\r");
 *                     break;
 * 
 *                 case 2:                 // CRC error -> 'temp' is not updated
 *                     pc.printf("CRC error\r\n");
 *             }
 * 
 *             led = !led;
 *         }
 *     }
 *     else
 *         pc.printf("No DS1820 sensor found!\r\n");
 * }
 * 
 * 
 * More sensors connected to the same 1-wire bus.
 * 
 * #include "mbed.h"
 * #include "DS1820.h"
 * 
 * #define     SENSORS_COUNT   64      // number of DS1820 sensors to be connected to the 1-wire bus (max 256)
 * 
 * Serial      pc(USBTX, USBRX);
 * DigitalOut  led(LED1);
 * OneWire     oneWire(D8);            // substitute D8 with actual mbed pin name connected to the DS1820 data pin
 * DS1820*     ds1820[SENSORS_COUNT];
 * int         sensors_found = 0;      // counts the actually found DS1820 sensors
 * float       temp = 0;
 * int         result = 0;
 * 
 * int main() {
 *     int i = 0;
 *     
 *     pc.printf("\r\n Starting \r\n");
 *     //Enumerate (i.e. detect) DS1820 sensors on the 1-wire bus
 *     for(i = 0; i < SENSORS_COUNT; i++) {
 *         ds1820[i] = new DS1820(&oneWire);
 *         if(!ds1820[i]->begin()) {
 *             delete ds1820[i];
 *             break;
 *         }
 *     }
 *     
 *     sensors_found = i;
 *     
 *     if (sensors_found == 0) {
 *         pc.printf("No DS1820 sensor found!\r\n");
 *         return -1;
 *     }
 *     else
 *         pc.printf("Found %d sensors.\r\n", sensors_found);
 *     
 *     while(1) {
 *         pc.printf("-------------------\r\n");
 *         for(i = 0; i < sensors_found; i++)
 *             ds1820[i]->startConversion();   // start temperature conversion from analog to digital       
 *         wait(1.0);                          // let DS1820s complete the temperature conversion
 *         for(int i = 0; i < sensors_found; i++) {
 *             if(ds1820[i]->isPresent())
 *                 pc.printf("temp[%d] = %3.1f%cC\r\n", i, ds1820[i]->read(), 176);     // read temperature
 *         }
 *     }
 * }
 * 
 * @endcode
 * 
 * Note: Don't forget to connect a 4.7k Ohm resistor 
 *       between the DS1820's data pin and the +3.3V pin
 *
 */
class   DS1820
{
    OneWire *oneWire;
    bool    present;    
    bool    model_s;
    uint8_t data[12];
    uint8_t addr[8];
    float   toFloat(uint16_t word);
    static  uint8_t lastAddr[8];
    
public:
    DS1820(PinName pin, int sample_point_us = 13);
//    DS1820(char model, PinName pin);
    DS1820(OneWire* wire);
    bool   begin(void);
    bool   isPresent();
    void   setResolution(uint8_t res);
    void   startConversion(void);
    float  read(void);
    uint8_t read(float& temp);
};
#endif /* DS1820_H_ */
