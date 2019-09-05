
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "25q32.h"

uint8_t w25_transfer(uint8_t data){
	while (!(SPI_SR(SPI_25Q32) & SPI_SR_TXE));
	SPI_SR(SPI_25Q32)&=~SPI_SR_TXE;
	SPI_DR(SPI_25Q32) = data;
	while (!(SPI_SR(SPI_25Q32) & SPI_SR_RXNE));
	SPI_SR(SPI_25Q32)&=~SPI_SR_RXNE;
	return SPI_DR(SPI_25Q32);
	
	}

void w25_powerdown(void){
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_POWER_DOWN);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
}

void w25_powerup(void){
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_POWER_UP);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
    for(uint32_t i=0;i<0xffff;i++)__asm__("nop");
}	

void w25_rst(){
    gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_ENABLE_RESET);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
    gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_RESET);
    gpio_set(PORT, CS25);
	}

void w25_wr_en(){
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_WRITE_ENABLE);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
    //w25_busy_wait();
	}

void w25_wr_di(){
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_WRITE_DISABLE);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
	}

void w25_read(uint32_t addr, unsigned char *data, uint32_t len){
	unsigned char addr1,addr2,addr3, a;
	addr1=addr>>16;
	addr2=(addr>>8)&0xff;
	addr3=addr&0xff;
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_FAST_READ);
    //spi_send(SPI_25Q32,W25_READ);
	spi_send(SPI_25Q32,addr1);
	spi_send(SPI_25Q32,addr2);
	spi_send(SPI_25Q32,addr3);
	//этот изврат нужен чтобы попасть в тайминги
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
	spi_xfer(SPI_25Q32,0x01);
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
	spi_xfer(SPI_25Q32,0x01);
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    while(len--)*data++=spi_xfer(SPI_25Q32,len&0xff); 
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
	};

void w25_read_str(uint32_t addr, uint8_t *data){
	unsigned char addr1,addr2,addr3,temp;
	addr1=addr>>16;
	addr2=(addr>>8)&0xff;
	addr3=addr&0xff;
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_FAST_READ);
    //spi_send(SPI_25Q32,W25_READ);
	spi_send(SPI_25Q32,addr1);
	spi_send(SPI_25Q32,addr2);
	spi_send(SPI_25Q32,addr3);
	//этот изврат нужен чтобы попасть в тайминги
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
	spi_xfer(SPI_25Q32,0x01);
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
	spi_xfer(SPI_25Q32,0x01);
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    do{//temp=SPI_25Q32_xfer(SPI_25Q321,0x00);
		temp=spi_xfer(SPI_25Q32,0xaa);
		*data=temp;
		data++;
	}while(temp);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
	};
/*
void w25_write(uint32_t addr, unsigned char *data, uint8_t len){
	w25_wr_en();
	unsigned char addr1,addr2,addr3;
	addr1=addr>>16;
	addr2=(addr>>8)&0xff;
	addr3=addr&0xff;
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_PAGE_PROGRAMM);
	spi_send(SPI_25Q32,addr1);
	spi_send(SPI_25Q32,addr2);
	spi_send(SPI_25Q32,addr3);
    //while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    do spi_send(SPI_25Q32,*data++); while(--len);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
	};
*/

uint32_t w25_write(uint32_t addr, unsigned char *data, uint32_t len){
	uint8_t temp;
	while(len){
		for(uint32_t i=0;i<0xffff;i++)__asm__("nop");
		w25_wr_en();
		for(uint32_t i=0;i<0xff;i++)__asm__("nop");
		unsigned char addr1,addr2,addr3;
		addr1=addr>>16;
		addr2=(addr>>8)&0xff;
		addr3=addr&0xff;
		gpio_clear(PORT, CS25);
		//gpio_toggle(GPIOB,GPIO1);
		spi_send(SPI_25Q32,W25_PAGE_PROGRAMM);
		spi_send(SPI_25Q32,addr1);
		spi_send(SPI_25Q32,addr2);
		spi_send(SPI_25Q32,addr3);
		while(len){ //пишем блоками по 256 байт
			spi_send(SPI_25Q32,*data++);
			len--;
			if(!(++addr&0xff)) break;
			}
		
		while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
		gpio_set(PORT, CS25);
		}
	return addr; //возвращаем первый свободный адрес
	};

uint32_t w25_write_str(uint32_t addr, char *data){
	//запись строки в flash
	do{
		w25_wr_en();
		unsigned char addr1,addr2,addr3;
		addr1=addr>>16;
		addr2=(addr>>8)&0xff;
		addr3=addr&0xff;
		gpio_clear(PORT, CS25);
		spi_send(SPI_25Q32,W25_PAGE_PROGRAMM);
		spi_send(SPI_25Q32,addr1);
		spi_send(SPI_25Q32,addr2);
		spi_send(SPI_25Q32,addr3);
		while(*data){ //пишем блоками по 256 байт
			spi_send(SPI_25Q32,*data);
			data++;
			if(!(++addr&0xff)) break;
		}
		if(!(*data)) spi_send(SPI_25Q32,0x00); //пишем конец строки
		while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
		gpio_set(PORT, CS25);
		w25_busy_wait();
		}while(*data);
	return addr; //возвращаем первый свободный адрес
	};


unsigned char w25_read_sr1(void){
	unsigned char sr1;
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,W25_READ_STATUS_1);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    sr1=spi_xfer(SPI_25Q32,0x33);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
    for(uint32_t i=0;i<0xff;i++)__asm__("nop");
	return sr1;
	};
	
void w25_busy_wait(void){
	//unsigned int sr1;
	while(w25_read_sr1()&SR1_BUSY);
	}
	
void w25_erase_block(uint32_t addr, uint8_t cmd){
	unsigned char addr1,addr2,addr3;
	w25_wr_en();
	if(cmd==W25_4K_ERASE)addr&=~(1024*4-1);
	if(cmd==W25_32K_ERASE)addr&=~(1024*32-1);
	if(cmd==W25_64K_ERASE)addr&=~(1024*64-1);
	
	addr1=addr>>16;
	addr2=(addr>>8)&0xff;
	addr3=addr&0xff;
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,cmd);
	spi_send(SPI_25Q32,addr1);
	spi_send(SPI_25Q32,addr2);
	spi_send(SPI_25Q32,addr3);
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
   // w25_busy_wait();
	}

void w25_erase(void){
	w25_wr_en();
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,0xC7);
	while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
	}
void w25_test(void){
	unsigned char id,id2;
	gpio_clear(PORT, CS25);
    spi_send(SPI_25Q32,0x90);
    spi_send(SPI_25Q32,0x0);
    spi_send(SPI_25Q32,0x0);
    spi_send(SPI_25Q32,0x0);

    //while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    spi_send(SPI_25Q32,0x0);
    id=spi_read(SPI_25Q32);
    spi_send(SPI_25Q32,0x0);
    id2=spi_read(SPI_25Q32);
    while (SPI_SR(SPI_25Q32) & SPI_SR_BSY);
    gpio_set(PORT, CS25);
	//return sr1;
	}


