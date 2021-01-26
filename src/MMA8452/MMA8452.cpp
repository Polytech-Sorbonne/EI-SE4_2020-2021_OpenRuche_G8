/**
@file MMA8452.cpp

@brief Member functions implementations

*/
#include "mbed.h"
#include "MMA8452.h"

MMA8452:: MMA8452(PinName sdaPin, PinName sclPin)
{
    i2c = new I2C(sdaPin,sclPin); // create new I2C instance and initialise
    i2c->frequency(400000);       // I2C Fast Mode - 400kHz
    leds = new BusOut(LED4,LED3,LED2,LED1);  // for debug
}

void MMA8452::init()
{

    i2c->frequency(400000); // set Fast Mode I2C frequency (5.10 datasheet)

    char data = readByteFromRegister(WHO_AM_I);  // p18 datasheet
    if (data != 0x2A) { // if correct ID not found, hand and flash error message
        error();
    }

    // put into STANDBY while configuring
    data = readByteFromRegister(CTRL_REG1); // get current value of register
    data &= ~(1<<0); // clear bit 0 (p37 datasheet)
    sendByteToRegister(data,CTRL_REG1);

    // Set output data rate, default is 800 Hz, will set to 100 Hz (clear b5, set b4/b3 - p37 datasheet)
    data = readByteFromRegister(CTRL_REG1);
    data &= ~(1<<5);
    data |=  (1<<4);
    data |=  (1<<3);
    sendByteToRegister(data,CTRL_REG1);

    //// Can also change default 2g range to 4g or 8g (p22 datasheet)
    data = readByteFromRegister(XYZ_DATA_CFG);
    data |=  (1<<0); // set bit 0 - 4g range
    sendByteToRegister(data,XYZ_DATA_CFG);

    // set ACTIVE
    data = readByteFromRegister(CTRL_REG1);
    data |= (1<<0);   // set bit 0 in CTRL_REG1
    sendByteToRegister(data,CTRL_REG1);

}

// read acceleration data from device
Acceleration MMA8452::readValues()
{
    // acceleration data stored in 6 registers (0x01 to 0x06)
    // device automatically increments register, so can read 6 bytes starting from OUT_X_MSB
    char data[6];
    readBytesFromRegister(OUT_X_MSB,6,data);

    char x_MSB = data[0];  // extract MSB and LSBs for x,y,z values
    char x_LSB = data[1];
    char y_MSB = data[2];
    char y_LSB = data[3];
    char z_MSB = data[4];
    char z_LSB = data[5];

    // [0:7] of MSB are 8 MSB of 12-bit value , [7:4] of LSB are 4 LSB's of 12-bit value
    // need to type-cast as numbers are in signed (2's complement) form (p20 datasheet)
    int x = (int16_t) (x_MSB << 8) | x_LSB;  // combine bytes
    x >>= 4;  // are left-aligned, so shift 4 places right to right-align
    int y = (int16_t) (y_MSB << 8) | y_LSB;
    y >>= 4;
    int z = (int16_t) (z_MSB << 8) | z_LSB;
    z >>= 4;

    // sensitivity is 1024 counts/g in 2g mode (pg 9 datasheet)
    //  "   "          512      "      4g     "
    //  "   "          256      "      8g     "
    Acceleration acc;
    
    acc.x = x/512.0;
    acc.y = y/512.0;
    acc.z = z/512.0;
    
    return acc;
}

// reads a byte from a specific register
char MMA8452::readByteFromRegister(char reg)
{
    int nack = i2c->write(MMA8452_W_ADDRESS,&reg,1,true);  // send the register address to the slave
    // true as need to send repeated start condition (5.10.1 datasheet)
    // http://www.i2c-bus.org/repeated-start-condition/
    if (nack)
        error();  // if we don't receive acknowledgement, flash error message

    char rx;
    nack = i2c->read(MMA8452_R_ADDRESS,&rx,1);  // read a byte from the register and store in buffer
    if (nack)
        error();  // if we don't receive acknowledgement, flash error message

    return rx;
}

// reads a series of bytes, starting from a specific register
void MMA8452::readBytesFromRegister(char reg,int numberOfBytes,char bytes[])
{

    int nack = i2c->write(MMA8452_W_ADDRESS,&reg,1,true);  // send the slave write address and the configuration register address
    // true as need to send repeated start condition (5.10.1 datasheet)
    // http://www.i2c-bus.org/repeated-start-condition/

    if (nack)
        error();  // if we don't receive acknowledgement, flash error message

    nack = i2c->read(MMA8452_R_ADDRESS,bytes,numberOfBytes);  // read bytes
    if (nack)
        error();  // if we don't receive acknowledgement, flash error message

}

// sends a byte to a specific register
void MMA8452::sendByteToRegister(char byte,char reg)
{
    char data[2];
    data[0] = reg;
    data[1] = byte;
    // send the register address, followed by the data
    int nack = i2c->write(MMA8452_W_ADDRESS,data,2);
    if (nack)
        error();  // if we don't receive acknowledgement, flash error message

}

void MMA8452::error()
{
    while(1) {
        leds->write(15);
        wait(0.1);
        leds->write(0);
        wait(0.1);
    }
}