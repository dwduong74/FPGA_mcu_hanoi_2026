#ifndef _I2C_H_
#define _I2C_H_
#include "mcu.h"

/* I2C - hardware I2C module (SN8F5708)
 * SDA: P1.4  SCL: P1.5
 * EEPROM device address: 0xA0 (write), 0xA1 (read)
 */

#define I2C_ACK   1
#define I2C_NACK  0

void    I2C_Init(void);
void    I2C_Start(void);
void    I2C_Stop(void);
void    I2C_TxData(uint8_t data);
uint8_t I2C_RdData(bit ack);

/* EEPROM high-level functions */
void    EP_WriteByte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
uint8_t EP_RdRead(uint8_t dev_addr, uint8_t reg_addr);

#endif
