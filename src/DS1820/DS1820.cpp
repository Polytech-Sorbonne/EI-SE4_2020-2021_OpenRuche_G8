/*
 * Dallas' DS1820 family temperature sensor.
 * This library depends on the OneWire library (Dallas' 1-Wire bus protocol implementation)
 * available at <http://developer.mbed.org/users/hudakz/code/OneWire/>
 *
 * Example of use:
 * 
 * Single sensor.
 *
 * #include "mbed.h"
 * #include "DS1820.h"
 * 
 * Serial      pc(USBTX, USBRX);
 * DigitalOut  led(LED1);
 * OneWire     oneWire(D8);    // substitute D8 with actual mbed pin name connected 1-wire bus
 * float       temp = 0;
 * int         result = 0;
 * 
 * int main()
 * {
 *     pc.printf("\r\n--Starting--\r\n");
 *     if (ds1820.begin()) {
 *         while (1) {
 *             ds1820.startConversion();   // start temperature conversion from analog to digital
 *             ThisThread::sleep_for(1000);// let DS1820 complete the temperature conversion
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
 *         ThisThread::sleep_for(1000);        // let DS1820s complete the temperature conversion
 *         for(int i = 0; i < sensors_found; i++) {
 *             if(ds1820[i]->isPresent())
 *                 pc.printf("temp[%d] = %3.1f%cC\r\n", i, ds1820[i]->read(), 176);     // read temperature
 *         }
 *     }
 * }
 * 
 */
 
#include "DS1820.h"

#define DEBUG 0

//* Initializing static members
uint8_t DS1820::lastAddr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
/**
 * @brief   Constructs a generic DS1820 sensor
 * @note    begin() must be called to detect and initialize the actual model
 * @param   pin: Name of data pin
 * @retval
 */
DS1820::DS1820(PinName pin, int sample_point_us /* = 13 */) {
    oneWire = new OneWire(pin, sample_point_us);
    present = false;
    model_s = false;
}

/**
 * @brief   Constructs a generic DS1820 sensor
 * @note    begin() must be called to detect and initialize the actual model
 * @param   pin: Name of data pin
 * @retval
 */
DS1820::DS1820(OneWire* wire) :
    oneWire(wire) {
    present = false;
    model_s = false;
}

/**
 * @brief   Detects and initializes the actual DS1820 model
 * @note
 * @param
 * @retval  true:   if a DS1820 family sensor was detected and initialized
            false:  otherwise
 */
bool DS1820::begin(void) {
#if DEBUG
    printf("lastAddr =");
    for(uint8_t i = 0; i < 8; i++) {
        printf(" %x", lastAddr[i]);
    }
    printf("\r\n");
#endif
    if(!oneWire->search(lastAddr)) {
#if DEBUG
        printf("No addresses.\r\n");
#endif
        oneWire->reset_search();
        wait(0.250);
        return false;
    }
    
    for (int i = 0; i < 8; i++)
        addr[i] = lastAddr[i];

#if DEBUG
    printf("ROM =");
    for(uint8_t i = 0; i < 8; i++) {
        printf(" %x", addr[i]);
    }
    printf("\r\n");
#endif

    if(OneWire::crc8(addr, 7) == addr[7]) {
        present = true;

        // the first ROM byte indicates which chip
        switch(addr[0]) {
        case 0x10:
            model_s = true;
#if DEBUG
            printf("DS18S20 or old DS1820\r\n");
#endif            
            break;

        case 0x28:
            model_s = false;
#if DEBUG
            printf("DS18B20\r\n");
#endif            
            break;

        case 0x22:
            model_s = false;
#if DEBUG
            printf("DS1822\r\n");
#endif            
            break;

        default:
            present = false;
#if DEBUG
            printf("Device doesn't belong to the DS1820 family\r\n");
#endif            
            return false;
        }
        return true;
    }
    else {
#if DEBUG    
        printf("Invalid CRC!\r\n");
#endif    
        return false;
    }
}

/**
 * @brief   Informs about presence of a DS1820 sensor.
 * @note    begin() shall be called before using this function
 *          if a generic DS1820 instance was created by the user. 
 *          No need to call begin() for a specific DS1820 instance.
 * @param
 * @retval  true:   when a DS1820 sensor is present
 *          false:  otherwise
 */
bool DS1820::isPresent(void) {
    return present;
}

/**
 * @brief   Sets temperature-to-digital conversion resolution.
 * @note    The configuration register allows the user to set the resolution
 *          of the temperature-to-digital conversion to 9, 10, 11, or 12 bits.
 *          Defaults to 12-bit resolution for DS18B20.
 *          DS18S20 allows only 9-bit resolution.
 * @param   res:    Resolution of the temperature-to-digital conversion in bits.
 * @retval
 */
void DS1820::setResolution(uint8_t res) {
    // keep resolution within limits
    if(res > 12)
        res = 12;
    if(res < 9)
        res = 9;      
    if(model_s)
        res = 9;
       
    oneWire->reset();
    oneWire->select(addr);
    oneWire->write_byte(0xBE);            // to read Scratchpad
    for(uint8_t i = 0; i < 9; i++)  // read Scratchpad bytes
        data[i] = oneWire->read_byte();   

    data[4] |= (res - 9) << 5;      // update configuration byte (set resolution)  
    oneWire->reset();
    oneWire->select(addr);
    oneWire->write_byte(0x4E);            // to write into Scratchpad
    for(uint8_t i = 2; i < 5; i++)  // write three bytes (2nd, 3rd, 4th) into Scratchpad
        oneWire->write_byte(data[i]);
}

/**
 * @brief   Starts temperature conversion
 * @note    The time to complete the converion depends on the selected resolution:
 *           9-bit resolution -> max conversion time = 93.75ms
 *          10-bit resolution -> max conversion time = 187.5ms
 *          11-bit resolution -> max conversion time = 375ms
 *          12-bit resolution -> max conversion time = 750ms
 * @param
 * @retval
 */
void DS1820::startConversion(void) {
    if(present) {
        oneWire->reset();
        oneWire->select(addr);
        oneWire->write_byte(0x44);    //start temperature conversion
    }
}

/**
 * @brief   Reads temperature from the chip's Scratchpad
 * @note
 * @param
 * @retval  Floating point temperature value
 */
float DS1820::read(void) {
    if(present) {
        oneWire->reset();
        oneWire->select(addr);
        oneWire->write_byte(0xBE);           // to read Scratchpad
        for(uint8_t i = 0; i < 9; i++)      // reading scratchpad registers
            data[i] = oneWire->read_byte();

        // Convert the raw bytes to a 16-bit unsigned value
        uint16_t*   p_word = reinterpret_cast < uint16_t * > (&data[0]);

#if DEBUG
        printf("raw = %#x\r\n", *p_word);
#endif            

        if(model_s) {
            *p_word = *p_word << 3;         // 9-bit resolution
            if(data[7] == 0x10) {

                // "count remain" gives full 12-bit resolution
                *p_word = (*p_word & 0xFFF0) + 12 - data[6];
            }
        }
        else {
            uint8_t cfg = (data[4] & 0x60); // default 12-bit resolution
            
            // at lower resolution, the low bits are undefined, so let's clear them
            if(cfg == 0x00)
                *p_word = *p_word &~7;      //  9-bit resolution
            else
            if(cfg == 0x20)
                *p_word = *p_word &~3;      // 10-bit resolution
            else
            if(cfg == 0x40)
                *p_word = *p_word &~1;      // 11-bit resolution
                                               
        }

        // Convert the raw bytes to a 16-bit signed fixed point value :
        // 1 sign bit, 7 integer bits, 8 fractional bits (two’s compliment
        // and the LSB of the 16-bit binary number represents 1/256th of a unit).
        *p_word = *p_word << 4;
        
        // Convert to floating point value
        return(toFloat(*p_word));
    }
    else
        return 0;
}

/**
 * @brief   Reads temperature from chip's scratchpad.
 * @note    Verifies data integrity by calculating cyclic redundancy check (CRC).
 *          If the calculated CRC dosn't match the one stored in chip's scratchpad register
 *          the temperature variable is not updated and CRC error code is returned.
 * @param   temp: The temperature variable to be updated by this routine.
 *                (It's passed as reference to floating point.)
 * @retval  error code:
 *              0 - no errors ('temp' contains the temperature measured)
 *              1 - sensor not present ('temp' is not updated)
 *              2 - CRC error ('temp' is not updated)
 */
uint8_t DS1820::read(float& temp) {
    if(present) {
        oneWire->reset();
        oneWire->select(addr);
        oneWire->write_byte(0xBE);               // to read Scratchpad
        for(uint8_t i = 0; i < 9; i++)          // reading scratchpad registers
            data[i] = oneWire->read_byte();

        if(oneWire->crc8(data, 8) != data[8])    // if calculated CRC does not match the stored one
        {
#if DEBUG
            for(uint8_t i = 0; i < 9; i++)
                printf("data[%d]=0x%.2x\r\n", i, data[i]);
#endif            
            return 2;                           // return with CRC error
        }

        // Convert the raw bytes to a 16bit unsigned value
        uint16_t*   p_word = reinterpret_cast < uint16_t * > (&data[0]);

#if DEBUG
        printf("raw = %#x\r\n", *p_word);
#endif

        if(model_s) {
            *p_word = *p_word << 3;         // 9 bit resolution,  max conversion time = 750ms
            if(data[7] == 0x10) {

                // "count remain" gives full 12 bit resolution
                *p_word = (*p_word & 0xFFF0) + 12 - data[6];
            }

            // Convert the raw bytes to a 16bit signed fixed point value :
            // 1 sign bit, 7 integer bits, 8 fractional bits (two's compliment
            // and the LSB of the 16bit binary number represents 1/256th of a unit).
            *p_word = *p_word << 4;
            // Convert to floating point value
            temp = toFloat(*p_word);
            return 0;   // return with no errors
        }
        else {
            uint8_t cfg = (data[4] & 0x60); // default 12bit resolution, max conversion time = 750ms

            // at lower resolution, the low bits are undefined, so let's clear them
            if(cfg == 0x00)
                *p_word = *p_word &~7;      //  9bit resolution, max conversion time = 93.75ms
            else
            if(cfg == 0x20)
                *p_word = *p_word &~3;      // 10bit resolution, max conversion time = 187.5ms
            else
            if(cfg == 0x40)
                *p_word = *p_word &~1;      // 11bit resolution, max conversion time = 375ms

            // Convert the raw bytes to a 16bit signed fixed point value :
            // 1 sign bit, 7 integer bits, 8 fractional bits (two's complement
            // and the LSB of the 16bit binary number represents 1/256th of a unit).
            *p_word = *p_word << 4;
            // Convert to floating point value
            temp = toFloat(*p_word);
            return 0;   // return with no errors
        }
    }
    else
        return 1;   // error, sensor is not present
}

/**
 * @brief   Converts a 16-bit signed fixed point value to floating point value
 * @note    The 16-bit unsigned integer represnts actually
 *          a 16-bit signed fixed point value:
 *          1 sign bit, 7 integer bits, 8 fractional bits (two’s complement
 *          and the LSB of the 16-bit binary number represents 1/256th of a unit).       
 * @param   16-bit unsigned integer
 * @retval  Floating point value
 */
float DS1820::toFloat(uint16_t word) {
    if(word & 0x8000)
        return (-float(uint16_t(~word + 1)) / 256.0f);
    else
        return (float(word) / 256.0f);
}

