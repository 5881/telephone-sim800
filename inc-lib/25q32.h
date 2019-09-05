/*Библиотека для работы с flash памятью 25q32 4mb
 * v 0.0
 * использована libopencm3
 * 24 mar 2019
 * 
 * 28 jul v0.5
 * Alexander Belyy
 */


#ifndef FLASH25Q32_H
#define FLASH25Q32_H

#include "stdint.h"
#define SPI_25Q32 SPI2
#define PORT GPIOB
#define CS25 GPIO12

#define W25_WRITE_DISABLE 0x04
#define W25_WRITE_ENABLE 0x06
#define W25_READ_STATUS_1 0x05
#define W25_READ_STATUS_2 0x35
#define W25_READ_STATUS_3 0x15
#define W25_WRITE_STATUS_1 0x01
#define W25_WRITE_STATUS_2 0x31
#define W25_WRITE_STATUS_3 0x11
#define W25_CHIP_ERASE 0xc7 //0x60
#define W25_GET_DEV_ID 0x90
#define W25_GET_JEDEC_ID 0x9f
#define W25_ENABLE_RESET 0x66
#define W25_RESET 0x99
#define W25_PAGE_PROGRAMM 0x02
#define W25_READ 0x03
#define W25_FAST_READ 0x0B
#define W25_4K_ERASE 0x20
#define W25_32K_ERASE 0x52
#define W25_64K_ERASE 0xD8
#define W25_POWER_DOWN 0xB9
#define W25_POWER_UP 0xAB

#define SR1_BUSY 0x01


uint8_t w25_transfer(uint8_t data);
uint32_t w25_write_str(uint32_t addr, char *data);
unsigned char w25_read_sr1(void);
void w25_powerdown(void);
void w25_powerup(void);
void w25_rst(void);
void w25_wr_en(void);
void w25_wr_di(void);
void w25_read(uint32_t addr, unsigned char *data, uint32_t len);
void w25_read_str(uint32_t addr, uint8_t *data);
uint32_t w25_write(uint32_t addr, unsigned char *data, uint32_t len);
void w25_busy_wait(void);
void w25_erase_block(uint32_t addr, uint8_t cmd);
void w25_erase(void);
void w25_test(void);
#endif
