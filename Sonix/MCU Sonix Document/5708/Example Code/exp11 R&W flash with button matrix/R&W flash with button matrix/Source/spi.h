#ifndef _SPI_
#define _SPI_
#include "mcu.h"
#define O_IO_CS								P07     			// 片选口
#define WAIT_TIME							450						// 最大死等时长 
//----------------------------------------------------------------------------------------------//
void spi_master_init (void);
uint8_t spi_WtRd_Byte (uint8_t cByte);
//指令表
#define W25Q_WRITE_ENABLE			0x06 
#define W25Q_WRITE_DISABLE		0x04 
#define W25Q_READ_STATUS1 		0x05
#define W25Q_READ_STATUS2 		0x35
#define W25Q_READ_DATA				0x03 
#define W25Q_SECTOR_ERASE			0x20 
#define W25Q_PAGE_PROGRAM			0x02
//----------------------------------------------------------------------------------------------//
bit w25q_busy(void);        
void w25q_read_id (void);
void w25q_erase_sector(uint16_t iAddr);
void w25q_write_256bytes(uint32_t lWriteAddr,uint8_t* p_cBuffer,uint16_t iNumByteToWrite); 
void w25q_read_data(uint32_t lReadAddr,uint8_t* p_cBuffer,uint16_t iNumByteToRead); 
#endif