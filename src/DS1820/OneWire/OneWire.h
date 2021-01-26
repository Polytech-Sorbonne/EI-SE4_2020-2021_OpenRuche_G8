#ifndef OneWire_h
#define OneWire_h

#include <inttypes.h>
#include <mbed.h>

#if defined(TARGET_STM)
    #define MODE()      output(); \
                        mode(OpenDrain)
    #define OUTPUT()    // configured as output in the constructor and stays like that forever
#if defined(TARGET_STM32L072xx)
    #define PORT        ((GPIO_TypeDef *)(GPIOA_BASE + 0x0400 * STM_PORT(gpio.pin)))
    #define PINMASK     (1 << STM_PIN(gpio.pin))
    #define INPUT()     (PORT->MODER &= ~(GPIO_MODER_MODE0_0 << (STM_PIN(gpio.pin) * 2)))                              
    #define READ()      ((PORT->IDR & gpio.mask) != 0)
    #define WRITE(x)    (x == 1 ? PORT->BSRR = PINMASK : PORT->BRR = PINMASK)
#else
    #define INPUT()     (*gpio.reg_set = gpio.mask) // write 1 to open drain
    #define READ()      ((*gpio.reg_in & gpio.mask) != 0)
    #define WRITE(x)    write(x)
#endif
#else
    #define MODE()      mode(PullUp)
    #define INPUT()     input()
    #define OUTPUT()    output()
    #define READ()      read()
    #define WRITE(x)    write(x)
#endif

#ifdef TARGET_NORDIC
//NORDIC targets (NRF) use software delays since their ticker uses a 32kHz clock
    static uint32_t loops_per_us = 0;
    
    #define INIT_WAIT   init_soft_delay()
    #define WAIT_US(x)  for(int cnt = 0; cnt < (x * loops_per_us) >> 5; cnt++) {__NOP(); __NOP(); __NOP();}
    
void init_soft_delay( void ) {
    if (loops_per_us == 0) {
        loops_per_us = 1;
        Timer timey; 
        timey.start();
        ONEWIRE_DELAY_US(320000);                     
        timey.stop();
        loops_per_us = (320000 + timey.read_us() / 2) / timey.read_us();  
    }
}
#else
    #define INIT_WAIT
    #define WAIT_US(x)  wait_us(x)
#endif

// You can exclude certain features from OneWire.  In theory, this
// might save some space.  In practice, the compiler automatically
// removes unused code (technically, the linker, using -fdata-sections
// and -ffunction-sections when compiling, and Wl,--gc-sections
// when linking), so most of these will not result in any code size
// reduction.  Well, unless you try to use the missing features
// and redesign your program to not need them!  ONEWIRE_CRC8_TABLE
// is the exception, because it selects a fast but large algorithm
// or a small but slow algorithm.

// you can exclude onewire_search by defining that to 0
#ifndef ONEWIRE_SEARCH
#define ONEWIRE_SEARCH 1
#endif

// You can exclude CRC checks altogether by defining this to 0
#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 1
#endif

class OneWire : public DigitalInOut
{
    int _sample_point_us;
    int _out_to_in_transition_us;

#if ONEWIRE_SEARCH
    // global search state
    unsigned char ROM_NO[8];
    uint8_t LastDiscrepancy;
    uint8_t LastFamilyDiscrepancy;
    uint8_t LastDeviceFlag;
#endif

public:
    OneWire(PinName pin, int sample_point_us = 13);

    // Perform a 1-Wire reset cycle. Returns 1 if a device responds
    // with a presence pulse.  Returns 0 if there is no device or the
    // bus is shorted or otherwise held low for more than 250uS
    uint8_t reset(void);

    // Issue a 1-Wire rom select command, you do the reset first.
    void select(const uint8_t rom[8]);

    // Issue a 1-Wire rom skip command, to address all on bus.
    void skip(void);

    // Write a byte. If 'power' is one then the wire is held high at
    // the end for parasitically powered devices. You are responsible
    // for eventually depowering it by calling depower() or doing
    // another read or write.
    void write_byte(uint8_t v, uint8_t power = 0);

    void write_bytes(const uint8_t *buf, uint16_t count, bool power = 0);

    // Read a byte.
    uint8_t read_byte(void);

    void read_bytes(uint8_t *buf, uint16_t count);

    // Write a bit. The bus is always left powered at the end, see
    // note in write() about that.
    void write_bit(uint8_t v);

    // Read a bit.
    uint8_t read_bit(void);

    // Stop forcing power onto the bus. You only need to do this if
    // you used the 'power' flag to write() or used a write_bit() call
    // and aren't about to do another read or write. You would rather
    // not leave this powered if you don't have to, just in case
    // someone shorts your bus.
    void depower(void);

#if ONEWIRE_SEARCH
    // Clear the search state so that if will start from the beginning again.
    void reset_search();

    // Setup the search to find the device type 'family_code' on the next call
    // to search(*newAddr) if it is present.
    void target_search(uint8_t family_code);

    // Look for the next device. Returns 1 if a new address has been
    // returned. A zero might mean that the bus is shorted, there are
    // no devices, or you have already retrieved all of them.  It
    // might be a good idea to check the CRC to make sure you didn't
    // get garbage.  The order is deterministic. You will always get
    // the same devices in the same order.
    uint8_t search(uint8_t *newAddr);
#endif

#if ONEWIRE_CRC
    // Compute a Dallas Semiconductor 8 bit CRC, these are used in the
    // ROM and scratchpad registers.
    static uint8_t crc8(const uint8_t *addr, uint8_t len);

#if ONEWIRE_CRC16
    // Compute the 1-Wire CRC16 and compare it against the received CRC.
    // Example usage (reading a DS2408):
    //    // Put everything in a buffer so we can compute the CRC easily.
    //    uint8_t buf[13];
    //    buf[0] = 0xF0;    // Read PIO Registers
    //    buf[1] = 0x88;    // LSB address
    //    buf[2] = 0x00;    // MSB address
    //    WriteBytes(net, buf, 3);    // Write 3 cmd bytes
    //    ReadBytes(net, buf+3, 10);  // Read 6 data bytes, 2 0xFF, 2 CRC16
    //    if (!CheckCRC16(buf, 11, &buf[11])) {
    //        // Handle error.
    //    }     
    //          
    // @param input - Array of bytes to checksum.
    // @param len - How many bytes to use.
    // @param inverted_crc - The two CRC16 bytes in the received data.
    //                       This should just point into the received data,
    //                       *not* at a 16-bit integer.
    // @param crc - The crc starting value (optional)
    // @return True, iff the CRC matches.
    static bool check_crc16(const uint8_t* input, uint16_t len, const uint8_t* inverted_crc, uint16_t crc = 0);

    // Compute a Dallas Semiconductor 16 bit CRC.  This is required to check
    // the integrity of data received from many 1-Wire devices.  Note that the
    // CRC computed here is *not* what you'll get from the 1-Wire network,
    // for two reasons:
    //   1) The CRC is transmitted bitwise inverted.
    //   2) Depending on the endian-ness of your processor, the binary
    //      representation of the two-byte return value may have a different
    //      byte order than the two bytes you get from 1-Wire.
    // @param input - Array of bytes to checksum.
    // @param len - How many bytes to use.
    // @param crc - The crc starting value (optional)
    // @return The CRC16, as defined by Dallas Semiconductor.
    static uint16_t crc16(const uint8_t* input, uint16_t len, uint16_t crc = 0);
#endif
#endif
};

#endif

            
