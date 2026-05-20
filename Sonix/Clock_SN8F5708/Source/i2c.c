#include "i2c.h"

#define I2C_ERROR  0xFF

/*-----------------------------------------------------------
 * I2C_Init - enable hardware I2C module
 *-----------------------------------------------------------*/
void I2C_Init(void)
{
    P1M  &= 0x30;       /* P1.0-P1.3, P1.6-P1.7: input mode */
    I2CCON |= 0x40;     /* Enable I2C module */
}

/*-----------------------------------------------------------
 * I2C_Start - generate START condition
 *-----------------------------------------------------------*/
void I2C_Start(void)
{
    I2CCON |= 0x20;                         /* Send START */
    I2CCON &= 0xF7;                         /* Clear SI flag to proceed */
    while((I2CCON & 0x08) != 0x08);        /* Wait for SI flag */
    I2CCON &= 0xDF;                         /* Clear START bit */
}

/*-----------------------------------------------------------
 * I2C_Stop - generate STOP condition
 *-----------------------------------------------------------*/
void I2C_Stop(void)
{
    I2CCON |= 0x10;     /* Send STOP */
    I2CCON &= 0xF7;
}

/*-----------------------------------------------------------
 * I2C_TxData - transmit one byte
 *-----------------------------------------------------------*/
void I2C_TxData(uint8_t data)
{
    if(I2CSTA == 0x08 || I2CSTA == 0x10 ||
       I2CSTA == 0x18 || I2CSTA == 0x28)
    {
        I2CDAT = data;
        I2CCON &= 0xF7;                     /* Clear SI to send */
        while((I2CCON & 0x08) != 0x08);    /* Wait for SI */
    }
}

/*-----------------------------------------------------------
 * I2C_RdData - receive one byte
 *   ack: I2C_ACK(1) = send ACK, I2C_NACK(0) = send NACK
 *-----------------------------------------------------------*/
uint8_t I2C_RdData(bit ack)
{
    if(I2CSTA == 0x40 || I2CSTA == 0x50)
    {
        if(ack) I2CCON |= 0x04;   /* ACK  */
        else    I2CCON &= 0xFB;   /* NACK */

        I2CCON &= 0xF7;
        while((I2CCON & 0x08) != 0x08);
        return I2CDAT;
    }
    return I2C_ERROR;
}

/*-----------------------------------------------------------
 * EP_WriteByte - write one byte to EEPROM
 *   dev_addr : 0xA0
 *   reg_addr : EEPROM memory address
 *   data     : byte to write
 *-----------------------------------------------------------*/
void EP_WriteByte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    I2C_Start();
    I2C_TxData(dev_addr);    /* Device address + WRITE */
    I2C_TxData(reg_addr);    /* Memory address */
    I2C_TxData(data);        /* Data */
    I2C_Stop();
}

/*-----------------------------------------------------------
 * EP_RdRead - read one byte from EEPROM
 *   dev_addr : 0xA0
 *   reg_addr : EEPROM memory address
 *   returns  : byte read
 *-----------------------------------------------------------*/
uint8_t EP_RdRead(uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t val;
    I2C_Start();
    I2C_TxData(dev_addr & 0xFE);   /* Device address + WRITE (set bit0=0) */
    I2C_TxData(reg_addr);           /* Memory address */
    I2C_Start();                    /* Repeated START */
    I2C_TxData(dev_addr | 0x01);   /* Device address + READ */
    val = I2C_RdData(I2C_NACK);    /* Read byte, send NACK (last byte) */
    I2C_Stop();
    return val;
}
