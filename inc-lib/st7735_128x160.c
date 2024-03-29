/*Библиотека для работы с tft дисплеем 128x128 st7735
 * v 0.9
 * использована libopencm3
 * 16 mar 2019
 * Alexander Belyy
 * 
 * v0.91
 * добавлено масштабирование текста х2
 * v0.92
 * добавлен вывод побайтно как в терминале 12 мая 2019
 * v0.93
 * добавлена поддержка backspase \b 27 июля 2019
 * v0.94
 * Привёл к пристойному виду. 28 июля 2019
 */
#include <stdint.h>
#include <string.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "st7735_128x160.h"

/*********************************************************************
 * Секция переменных
 *********************************************************************/
uint16_t pos=0;
static uint16_t txt_color=GREEN;
static uint16_t bg_color=BLACK;

static const char ASCII[][5] =
{
 {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c ¥
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ←
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f →
,{0x7e, 0x11, 0x11, 0x11, 0x7e}//A  0x80
,{0x7f, 0x49, 0x49, 0x49, 0x33}//Б  0x81
,{0x7f, 0x49, 0x49, 0x49, 0x36}//В  0x82
,{0x7f, 0x01, 0x01, 0x01, 0x03}//Г  0x83
,{0xe0, 0x51, 0x4f, 0x41, 0xff}//Д  0x84
,{0x7f, 0x49, 0x49, 0x49, 0x41}//E  0x85
,{0x77, 0x08, 0x7f, 0x08, 0x77}//Ж  0x86
,{0x41, 0x49, 0x49, 0x49, 0x36}//З  0x87
,{0x7f, 0x10, 0x08, 0x04, 0x7f}//И  0x88
,{0x7c, 0x21, 0x12, 0x09, 0x7c}//Й  0x89
,{0x7f, 0x08, 0x14, 0x22, 0x41}//K  0x8A
,{0x20, 0x41, 0x3f, 0x01, 0x7f}//Л  0x8B
,{0x7f, 0x02, 0x0c, 0x02, 0x7f}//M  0x8C
,{0x7f, 0x08, 0x08, 0x08, 0x7f}//H  0x8D
,{0x3e, 0x41, 0x41, 0x41, 0x3e}//O  0x8E
,{0x7f, 0x01, 0x01, 0x01, 0x7f}//П  0x8F
,{0x7f, 0x09, 0x09, 0x09, 0x06}//P  0x90
,{0x3e, 0x41, 0x41, 0x41, 0x22}//C  0x91
,{0x01, 0x01, 0x7f, 0x01, 0x01}//T  0x92
,{0x47, 0x28, 0x10, 0x08, 0x07}//У  0x93
,{0x1c, 0x22, 0x7f, 0x22, 0x1c}//Ф  0x94
,{0x63, 0x14, 0x08, 0x14, 0x63}//X  0x95
,{0x7f, 0x40, 0x40, 0x40, 0xff}//Ц  0x96
,{0x07, 0x08, 0x08, 0x08, 0x7f}//Ч  0x97
,{0x7f, 0x40, 0x7f, 0x40, 0x7f}//Ш  0x98
,{0x7f, 0x40, 0x7f, 0x40, 0xff}//Щ  0x99
,{0x01, 0x7f, 0x48, 0x48, 0x30}//Ъ  0x9A
,{0x7f, 0x48, 0x30, 0x00, 0x7f}//Ы  0x9B
,{0x00, 0x7f, 0x48, 0x48, 0x30}//Э  0x9C
,{0x22, 0x41, 0x49, 0x49, 0x3e}//Ь  0x9D
,{0x7f, 0x08, 0x3e, 0x41, 0x3e}//Ю  0x9E
,{0x46, 0x29, 0x19, 0x09, 0x7f}//Я  0x9F
,{0x20, 0x54, 0x54, 0x54, 0x78}//a  0xA0
,{0x3c, 0x4a, 0x4a, 0x49, 0x31}//б  0xA1
,{0x7c, 0x54, 0x54, 0x28, 0x00}//в  0xA2
,{0x7c, 0x04, 0x04, 0x04, 0x0c}//г  0xA3
,{0xe0, 0x54, 0x4c, 0x44, 0xfc}//д  0xA4
,{0x38, 0x54, 0x54, 0x54, 0x18}//e  0xA5
,{0x6c, 0x10, 0x7c, 0x10, 0x6c}//ж  0xA6
,{0x44, 0x44, 0x54, 0x54, 0x28}//з  0xA7
,{0x7c, 0x20, 0x10, 0x08, 0x7c}//и  0xA8
,{0x7c, 0x41, 0x22, 0x11, 0x7c}//й  0xA9
,{0x7c, 0x10, 0x28, 0x44, 0x00}//к  0xAA
,{0x20, 0x44, 0x3c, 0x04, 0x7c}//л  0xAB
,{0x7c, 0x08, 0x10, 0x08, 0x7c}//м  0xAC
,{0x7c, 0x10, 0x10, 0x10, 0x7c}//н  0xAD
,{0x38, 0x44, 0x44, 0x44, 0x38}//o  0xAE
,{0x7c, 0x04, 0x04, 0x04, 0x7c}//п  0xAF
,{0x7C, 0x14, 0x14, 0x14, 0x08}//p  0xE0
,{0x38, 0x44, 0x44, 0x44, 0x20}//c  0xE1
,{0x04, 0x04, 0x7c, 0x04, 0x04}//т  0xE2
,{0x0C, 0x50, 0x50, 0x50, 0x3C}//у  0xE3
,{0x30, 0x48, 0xfc, 0x48, 0x30}//ф  0xE4
,{0x44, 0x28, 0x10, 0x28, 0x44}//x  0xE5
,{0x7c, 0x40, 0x40, 0x40, 0xfc}//ц  0xE6
,{0x0c, 0x10, 0x10, 0x10, 0x7c}//ч  0xE7
,{0x7c, 0x40, 0x7c, 0x40, 0x7c}//ш  0xE8
,{0x7c, 0x40, 0x7c, 0x40, 0xfc}//щ  0xE9
,{0x04, 0x7c, 0x50, 0x50, 0x20}//ъ  0xEA
,{0x7c, 0x50, 0x50, 0x20, 0x7c}//ы  0xEB
,{0x7c, 0x50, 0x50, 0x20, 0x00}//ь  0xEC
,{0x28, 0x44, 0x54, 0x54, 0x38}//э  0xED
,{0x7c, 0x10, 0x38, 0x44, 0x38}//ю  0xEE
,{0x08, 0x54, 0x34, 0x14, 0x7c}//я  0xEF
};

 
 
 
 
 /********************************************************************
  * Секция основных функций
  ********************************************************************/
void st7735_set_printf_color(uint16_t textcolor, uint16_t bgcolor){
	txt_color=textcolor;
	bg_color=bgcolor;	
}


void st7735_sendcmd(unsigned char cmd){
	gpio_clear(STPORT, DC);
	spi_send(SPI,cmd);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	}
void st7735_senddata(unsigned char data){
	gpio_set(STPORT, DC);
	spi_send(SPI,data);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	}

void st7735_init(void){
	uint32_t i;
	
	//for(i=0;i<0xfff;i++)__asm__("nop");
	gpio_set(STPORT,RST);
	gpio_clear(STPORT,STCS);
	for(i=0;i<0xfff;i++)__asm__("nop");
	gpio_clear(STPORT,RST);
	for(i=0;i<0xfff;i++)__asm__("nop");
	gpio_set(STPORT,RST);
	
	st7735_sendcmd(ST7735_SWRESET);//0x01
	for(i=0;i<0x5fff;i++)__asm__("nop");
	
	st7735_sendcmd(ST7735_SLPOUT);//0x11
	for(i=0;i<0x5fff;i++)__asm__("nop");
	
	st7735_sendcmd(ST7735_COLMOD);
	st7735_senddata(0x5);
	st7735_sendcmd(0x26);
	st7735_senddata(0x2);
	
	st7735_sendcmd(ST7735_FRMCTR1);
	st7735_senddata(0x00);
	st7735_senddata(0x06);
	st7735_senddata(0x03);
	
	st7735_sendcmd(ST7735_MADCTL);
#ifdef HORIZONTAL
	st7735_senddata(MV|MX); //mv set def is 0
#else
	st7735_senddata(0x0);
#endif
	//st7735_sendcmd(ST7735_DISSET5);
	//st7735_senddata(0x15);
	//st7735_senddata(0x02);

	
	st7735_sendcmd(ST7735_CASET);
	st7735_senddata(0x00);
	st7735_senddata(0x00);
	st7735_senddata(0x00);
	st7735_senddata(0x7f);

	st7735_sendcmd(ST7735_RASET);
	st7735_senddata(0x00);
	st7735_senddata(0x00);
	st7735_senddata(0x00);
	st7735_senddata(0xa0);

	
	
	st7735_sendcmd(ST7735_NORON);
	for(i=0;i<0xfff;i++)__asm__("nop");
	
	st7735_sendcmd(ST7735_DISPON);
	for(i=0;i<0xfff;i++)__asm__("nop");
	
		
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(GPIOA,STCS);
}

void st7735_wake(void){
	//st7735_init();
	gpio_clear(STPORT,STCS);
	st7735_sendcmd(ST7735_DISPON);
	gpio_set(STPORT,STCS);
	gpio_set(GPIOB,GPIO0);
	}

void st7735_sleep(void){
	gpio_clear(STPORT,STCS);
	st7735_sendcmd(ST7735_DISPOFF);
	gpio_set(STPORT,STCS);
	gpio_clear(GPIOB,GPIO0);
	}

void st7735_addrset(unsigned char sx,unsigned char sy,
								unsigned char ex,unsigned char ey){
	st7735_sendcmd(ST7735_CASET); // Column address set 2a
	gpio_set(GPIOA,DC);
	spi_send(SPI,0x00);
	spi_send(SPI,sx);
	spi_send(SPI,0x00);
	spi_send(SPI,ex);

	while (SPI_SR(SPI) & SPI_SR_BSY);

	st7735_sendcmd(ST7735_RASET);   // Row address set 2b
	gpio_set(GPIOA,DC);
	spi_send(SPI,0x00);
	spi_send(SPI,sy);
	spi_send(SPI,0x00);
	spi_send(SPI,ey);
	
	while (SPI_SR(SPI) & SPI_SR_BSY);
	st7735_sendcmd(ST7735_RAMWR);   // Memory write			
	}

void st7735_clear(uint16_t color){
	unsigned char BCH,BCL;
	BCL = color & 0xFF;
	BCH = color>>8;
	gpio_clear(STPORT,STCS);
	//for(uint8_t i=0;i<0xff;i++)__asm__("nop");

#ifdef HORIZONTAL
	st7735_addrset(0,0,160,128);
#else
	st7735_addrset(0,0,128,160);
#endif
	gpio_set(STPORT,DC);
	for(uint16_t i=0;i<128*160;i++){
		spi_send(SPI,BCH);
		spi_send(SPI,BCL);
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);
	}

//debug func

void st7735_draw_s_char(unsigned char x,unsigned char y,char chr,
								uint16_t color, uint16_t bg_color){
									
	unsigned char i,j;
	unsigned char CH,CL,BCH,BCL;
	unsigned char buffer[6];
	CL = color & 0xFF;
	CH = color>>8;
	BCL = bg_color & 0xFF;
	BCH = bg_color>>8;
	
	memcpy(buffer,&ASCII[chr-0x20],5);
	buffer[5]=0;
	
	gpio_clear(STPORT,STCS);
	st7735_addrset(x,y,x+5,y+7);
	gpio_set(GPIOA,DC);
	for(unsigned char i=0;i<8;i++){
		for(unsigned char j=0;j<6;j++){
			if((buffer[j] & (1<<i))){
				spi_send(SPI,CH);
				spi_send(SPI,CL);
				}
				else{
				spi_send(SPI,BCH);
				spi_send(SPI,BCL);
				}
			}
		}
	
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);

}

//end debug func

void st7735_drawchar(unsigned char x,unsigned char y,char chr,
								uint16_t color, uint16_t bg_color){
	unsigned char i,j;
	unsigned char CH,CL,BCH,BCL;
	unsigned char buffer[6];
	//добавлена поддержка кирилицы
	unsigned char c=(chr<0xe0) ? chr - 0x20 : chr - 0x50;
	
	CL = color & 0xFF;
	CH = color>>8;
	BCL = bg_color & 0xFF;
	BCH = bg_color>>8;
	
	memcpy(buffer,&ASCII[c],5);
	buffer[5]=0;
	st7735_addrset(x,y,x+5,y+7);
	gpio_set(GPIOA,DC);
	for(unsigned char i=0;i<8;i++){
		for(unsigned char j=0;j<6;j++){
			if((buffer[j] & (1<<i))){
				spi_send(SPI,CH);
				spi_send(SPI,CL);
				}
				else{
				spi_send(SPI,BCH);
				spi_send(SPI,BCL);
				}
			}
		}
	
}

void st7735_drawchar_x2(unsigned char x,unsigned char y,char chr,
								uint16_t color, uint16_t bg_color){
	unsigned char i,j;
	unsigned char CH,CL,BCH,BCL;
	unsigned char buffer[6];
	//добавлена поддержка кирилицы
	unsigned char c=(chr<0xe0) ? chr - 0x20 : chr - 0x50;
	
	uint16_t temp;
	CL = color & 0xFF;
	CH = color>>8;
	BCL = bg_color & 0xFF;
	BCH = bg_color>>8;
	
	memcpy(buffer,&ASCII[c],5);
	buffer[5]=0;
	st7735_addrset(x,y,x+11,y+15);
	gpio_set(GPIOA,DC);
	
	
	for(uint8_t j=0;j<8;j++){
		for(uint8_t k=0;k<2;k++){
			for(uint8_t i=0;i<6;i++){
				if(buffer[i]&(1<<j)) {//если окрашено
					spi_send(SPI,CH);
					spi_send(SPI,CL);
					spi_send(SPI,CH);
					spi_send(SPI,CL);}
					else {			//фон
					spi_send(SPI,BCH);
					spi_send(SPI,BCL);
					spi_send(SPI,BCH);
					spi_send(SPI,BCL);}
				}
		}
	}
}

void st7735_drawchar_x3(unsigned char x,unsigned char y,char chr,
								uint16_t color, uint16_t bg_color){
	unsigned char i,j;
	unsigned char CH,CL,BCH,BCL;
	unsigned char buffer[6];
	//добавлена поддержка кирилицы
	unsigned char c=(chr<0xe0) ? chr - 0x20 : chr - 0x50;
	
	uint16_t temp;
	CL = color & 0xFF;
	CH = color>>8;
	BCL = bg_color & 0xFF;
	BCH = bg_color>>8;
	
	memcpy(buffer,&ASCII[c],5);
	buffer[5]=0;
	st7735_addrset(x,y,x+17,y+23);
	gpio_set(GPIOA,DC);
	
	for(uint8_t j=0;j<8;j++){
		for(uint8_t k=0;k<3;k++){
			for(uint8_t i=0;i<6;i++){
				if(buffer[i]&(1<<j)) {//если окрашено
					spi_send(SPI,CH);
					spi_send(SPI,CL);
					spi_send(SPI,CH);
					spi_send(SPI,CL);
					spi_send(SPI,CH);
					spi_send(SPI,CL);}
					else {			//фон
					spi_send(SPI,BCH);
					spi_send(SPI,BCL);
					spi_send(SPI,BCH);
					spi_send(SPI,BCL);
					spi_send(SPI,BCH);
					spi_send(SPI,BCL);}
				}
		}
	}
}

void st7735_string_x3_at(unsigned char x,unsigned char y,
				unsigned char *chr,	uint16_t color, uint16_t bg_color){
	gpio_clear(STPORT,STCS);
	//for (uint8_t i=0;i<50;i++) __asm__("nop");//задержка против глюков
	unsigned char i=0;
	while(*chr){
		if(*chr=='\n'){y+=24;chr++;i=0;}
		st7735_drawchar_x3(x+i,y,*chr++,color,bg_color);
		i+=17;
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);  
	}

void st7735_string_x2_at(unsigned char x,unsigned char y,
				unsigned char *chr,	uint16_t color, uint16_t bg_color){
	gpio_clear(STPORT,STCS);
	//for (uint8_t i=0;i<50;i++) __asm__("nop");
	unsigned char i=0;
	while(*chr){
		if(*chr=='\n'){y+=16;chr++;i=0;}
		st7735_drawchar_x2(x+i,y,*chr++,color,bg_color);
		i+=11;
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);  
	}



void st7735_string_at(unsigned char x,unsigned char y,
				unsigned char *chr,	uint16_t color, uint16_t bg_color){
	gpio_clear(STPORT,STCS);
	//for (uint8_t i=0;i<50;i++) __asm__("nop");
	unsigned char i=0;
	while(*chr){
		if(*chr=='\n'){y+=8;chr++;i=0;}
		st7735_drawchar(x+i,y,*chr++,color,bg_color);
		i+=6;
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);  
	}
	
void st7735_drawpixel(unsigned char x,unsigned char y,uint16_t Color){
	unsigned char CL,CH;
	CL = Color & 0xFF;
	CH = (Color>>8) & 0xFF;
	st7735_addrset(x,y,x+1,y+1);
	gpio_set(STPORT,DC);
	spi_send(SPI,CH);
	spi_send(SPI,CL);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	}
	
/**********************************************************************
 * рисование линий с помощью  Алгоритм Брезенхэма
 * https://ru.wikibooks.org/
 **********************************************************************/

void st7735_drawline(unsigned char x1, unsigned char y1,unsigned char x2,
						           unsigned char y2, uint16_t color){
	gpio_clear(STPORT,STCS); //перенёс сюда чтобы сделать задержку
    char deltax,deltay,signx,signy;
    deltax = x2 - x1;
    if(deltax<0) deltax*=-1;
    deltay = y2 - y1;
    if(deltay<0) deltay*=-1;
    signx = x1 < x2 ? 1 : -1;
    signy = y1 < y2 ? 1 : -1;
    //
    short int error = deltax - deltay, error2;
    //
    
    st7735_drawpixel(x2, y2,color);
		while(x1 != x2 || y1 != y2){
			st7735_drawpixel(x1, y1,color);
			error2 = error * 2;
			//
			if(error2 > -deltay){
				error -= deltay;
				x1 += signx;
				}
			if(error2 < deltax){
				error += deltax;
				y1 += signy;
				}
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);
	}


void st7735_drawcircle(unsigned char x0, unsigned char y0,
							unsigned char radius, uint16_t color) {
	gpio_clear(STPORT,STCS);//здесь для задержки
	short int x = 0;
	short int y = radius;
	short int delta = 1 - 2 * radius;
	short int error = 0;
	while(y >= 0) {
		st7735_drawpixel(x0 + x, y0 + y,color);
		st7735_drawpixel(x0 + x, y0 - y,color);
		st7735_drawpixel(x0 - x, y0 + y,color);
		st7735_drawpixel(x0 - x, y0 - y,color);
		error = 2 * (delta + y) - 1;
		if(delta < 0 && error <= 0) {
			++x;
			delta += 2 * x + 1;
			continue;
			}
		error = 2 * (delta - x) - 1;
		if(delta > 0 && error > 0) {
			--y;
			delta += 1 - 2 * y;
			continue;
			}
		++x;
		delta += 2 * (x - y);
		--y;
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);
	}

void st7735_drawhl(unsigned char x,unsigned char x1,
									unsigned char y,uint16_t color){
	unsigned char len,CL,CH;
	CL = color & 0xFF;
	CH = color>>8;
	st7735_addrset(x,y,x1,y);
	len = x1-x;
	gpio_set(STPORT,DC);
	for(unsigned char i=0;i<len;i++){
		spi_send(SPI,CH);
		spi_send(SPI,CL);
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
}

void st7735_drawvl(unsigned char x,unsigned char y,
									unsigned char y1,uint16_t color){
	unsigned char len,CL,CH;
	CL = color & 0xFF;
	CH = color>>8;
	st7735_addrset(x,y,x,y1);
	len = y1-y;
	gpio_set(STPORT,DC);
	for(unsigned char i=0;i<len;i++){
		spi_send(SPI,CH);
		spi_send(SPI,CL);
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
}

void st7735_drawrectangle(unsigned char x, unsigned char y,
				unsigned char dx, unsigned char dy, uint16_t color){
	gpio_clear(STPORT,STCS);
	//for (uint8_t i=0;i<50;i++) __asm__("nop");
	st7735_drawhl(x,x+dx,y,color);										
	st7735_drawvl(x+dx,y,y+dy,color);
	st7735_drawvl(x,y,y+dy,color);
	st7735_drawhl(x,x+dx,y+dy,color);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);					
	}
	
void st7735_drawimg(uint8_t x,uint8_t y, uint8_t w,uint8_t h, uint16_t *data){
	gpio_clear(STPORT,STCS);
	//for (uint8_t i=0;i<50;i++) __asm__("nop");
	st7735_addrset(x,y,x+w+1,y+h);
	//uint32_t size=h*w-1;
	uint32_t size=h*w;
	gpio_set(GPIOA,DC);
	
	for(uint32_t i=0;i<size;i++){
				spi_send(SPI,data[i]>>8);
				spi_send(SPI,data[i]&0xff);
		//spi_send(SPI,Oxff);
//spi_send(SPI,Oxff);
		}
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);	
}	
	
void st7735_drawgimg(uint8_t x,uint8_t y, uint8_t w,uint8_t h, uint8_t *data){
	gpio_clear(STPORT,STCS);
	for (uint8_t i=0;i<50;i++) __asm__("nop");
	st7735_addrset(x,y,x+w,y+h);
	uint32_t size=h*w*2;
	gpio_set(GPIOA,DC);
	
	for(uint32_t i=0;i<size+1;i++)spi_send(SPI,data[i]);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);	
}	


//задефайнить горизонтальный и вертикальный режимы
void st7735_sendchar(char ch){
	gpio_clear(STPORT,STCS);
	//for (uint8_t i=0;i<50;i++) __asm__("nop");
	//static uint16_t pos=0;
	uint16_t px,py;
#ifdef HORIZONTAL	
	if(ch=='\r'){
		pos-=pos%26;
	#ifdef DEBUG
		ch='R';}
	#else
		return;}
	#endif
	if(ch=='\n'){
		pos+=26;
		pos-=pos%26;
	#ifdef DEBUG
		ch='N';}
	#else
		return;}
	#endif	
		if(ch=='\a'){
		pos=0;
		st7735_clear(bg_color);
		return;}
	if(ch=='\b'){//backspase new!
	pos--;
	px=pos%26*6;
	py=pos/26*8;
	st7735_drawchar(px,py,0x20,txt_color,bg_color);
	return;}
	if(pos>416){
		pos=0;
		st7735_clear(bg_color);
		}
	px=pos%26*6;
	py=pos/26*8;
#else
	if(ch=='\r'){
		pos-=pos%21;
		return;}
	if(ch=='\n'){
		pos+=21;
		pos-=pos%21;
		return;}
	if(ch=='\a'){
		pos=0;
		st7735_clear(bg_color);
		return;}
	if(ch=='\b'){//backspase new!
	pos--;
	px=pos%21*6;
	py=pos/21*8;
	st7735_drawchar(px,py,' ',txt_color,bg_color);
	return;
	}
	if(pos>420){
		pos=0;
		st7735_clear(BLACK);}
	px=pos%21*6;
	py=pos/21*8;
#endif

	st7735_drawchar(px,py,ch,txt_color,bg_color);
	pos++;
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);
	//return;
	}

void st7735_virt_sendchar(char ch){
	gpio_clear(STPORT,STCS);
	//for (uint8_t i=0;i<50;i++) __asm__("nop");
	//static uint16_t pos=0;
	uint16_t px,py;
#ifdef HORIZONTAL	
	px=pos%26*6;
	py=pos/26*8;
#else
	px=pos%21*6;
	py=pos/21*8;
#endif
	if(ch>0x20)	st7735_drawchar(px,py,ch,RED,bg_color);
	//pos++;
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);
	//return;
	}
