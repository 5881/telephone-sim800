/*
 * Мобильный телефон v0.8
 * 28 июля 2019
 * 11 августа 2019 теперь с смс
 * /

/**********************************************************************
 * Секция include и defines
**********************************************************************/
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/systick.h>
#include <stdlib.h>
#include "stdint.h"
#include "string.h"
#include "st7735_128x160.h"
#include "rprintf.h"
#include "4x4key.h"
#include "st_printf.h"
#include "25q32.h"
#include "xmodem.h"
#include "menu.h"
#include "sim800.h"

#define TELBOOK_BASE_ADDR 0

/**********************************************************************
 * Секция глобальных переменных
 *********************************************************************/
 
char usartrxbufer[1024];
//char atcommand[512];
//typedef struct telbook {
//	char name[16];
//	char tel[16];
//	} telbook;
telbook tb[10];
//uint16_t sms_number=1;

/**********************************************************************
 * Функции телефонной книги, затем вынести в отдельный файл
 **********************************************************************/

uint8_t telbook_get_number(char *number){
	//проверяем наличие записей в тел книге, если есть читаем первые 10
	//если нет предлагаем ввести номер
	// ' ' - отмена
	//
	telbook temp[10];
	char tel[13];
	int tel_num_count;
	uint16_t page=0;
	uint8_t count=0;
	char key;
	tel_num_count=telbook_rec_count();
	if(tel_num_count){
		do{
			stprintf("\atelbook v0.02\r\n");
			count=(tel_num_count-page*10<10)?(tel_num_count-page*10):10;
			if((tel_num_count-page*10)<0) count=0;
			read_telbook(TELBOOK_BASE_ADDR+page*320,temp,count);
			
			for(uint8_t i=0;i<count;i++) stprintf("%d %s %s\r\n",i,
										(temp+i)->tel,(temp+i)->name);
			stprintf("~ enter the number\r\n"
					"*- next page\r\n"
					"\\b - prev page\r\n" 
					" - exit\r\n");
			key=fast_get_key();
			if(key==' ') return 0;
			if(key=='*'){page++;continue;}
			if(key=='\b'){if(page)page--;continue;}
			if(key=='~'){
				stprintf("\aEnter the number\n");
				set_scanf_mode(1);
				kscanf("%s",tel);
				set_scanf_mode(0);
				strcpy(number,tel);
				return 2;};
			key=key-0x30;
			
			if(count && key>=0 && key<10){
				strcpy(number,(temp+key)->tel);
				return 1;}
			//else return 0;
		}while(1);
	}
	return 0;
	}

void read_telbook(uint32_t addr, telbook *tel_book,int count){
	 char *tel_from_flash;
	 tel_from_flash=tel_book;
	 w25_read(addr, tel_from_flash, 32*count);
	 }

void write_sms(void){
	char text[256];
	char tel[13];
	uint8_t ret_code;
	//char temp[12];
	stprintf("\aSMS writer v0.01\r\n"
			"Enter the sms text\r\n"
			">");
	kscanf("%s", text);
	ret_code=telbook_get_number(tel);
	if(!ret_code) return;
	fast_sms_send(text,tel);	
}

void fast_call(char *tel){
	char key;
	char *ptr;
	sim800_wake();
	//char tel[12];
	stprintf("\aDo you want call\r\n   %s\r\nPres return to call\r\n"
			"or any key to cancle.",tel);
	key=fast_get_key();
	if(key=='\n'){
		ptr=tel;
		if (*tel=='8') ptr=tel+1;
		if (*tel=='+') ptr=tel+2;
		stprintf("ATD+7%s;\n",ptr);
		printf_("ATD+7%s;\r\n",ptr);
		}
	}



int telbook_rec_count(void){
	int count=0;
	char temp;
	do{
		w25_read(count*32,&temp,1);//читаем первый байт каждой записи
		count++;
		
	}while(temp!=0xff);//если 0xff то запись пуста.
	//возвращаем номер первой пустой записи.
	return count-1;
}
//void get_telbook_name(uint16_t n, char *data){
//	w25_read_str(n-1,data);
//	}
uint16_t telbook_find_name(char *tel, char *name){
	uint8_t offset=0;
	uint8_t find=0;
	uint16_t n=0, i=0;
	telbook rec;
	if(tel[0]=='+') offset=2;
	if(tel[0]=='8') offset=1;
	n=telbook_rec_count();
	//stprintf("seach..");
	//for(i=0;i<0xfffff;i++)__asm__("nop");
	for(i=0;i<n;i++){
		w25_read(32*i,(uint8_t *)&rec,32);
		if(!strcmp(rec.tel+1,tel+offset)){
			//stprintf("s:%s\r\nb:%s\r\n",tel,rec.tel+1);
			strcpy(name,rec.name);
			find=1; //нашли
			break;
			}
	}
	return (find) ? i : 0;
	}


void telbook_rec_add(void){
	telbook new_rec;
	int count=0;
	uint32_t addr;
	char key;
	count=telbook_rec_count();
	addr=count*32;
	stprintf("\aAdd contact as %d\r\nEnter the name\r\n:",count);
	kscanf("%s",new_rec.name);
	stprintf("\r\ntel:");
	set_scanf_mode(1);//fast numbers only mode
	kscanf("%s",new_rec.tel);
	set_scanf_mode(0);//normal mode
	
	stprintf("\r\nsave? Return to yes.");
	key=fast_get_key();
	if(key=='\n') w25_write(addr,(uint8_t *)&new_rec,32);
	}

void telbook_rec_del(void){
	int count;
	int N;
	count=telbook_rec_count();
	if(!count)return;
	//telbook tb[10];
	//tb=(telbook*)malloc(32*count);
	
	stprintf("\r\nEnter N to del\r\n:");
	set_scanf_mode(1);
	kscanf("%d", &N);
	set_scanf_mode(0);
	for(int i=N-1;i<count;i++)tb[i]=tb[i+1];
	w25_erase_block(0,0x20);
	
	for(uint32_t i=0; i<0xfffff;i++)__asm__("nop");
	w25_write(0,(uint8_t*)tb,32*(count-1));
	stprintf("\r\nrec %d was delated",N);
	//free(tb);
	
}



/**********************************************************************
 * Секция настройки переферии
**********************************************************************/



void keybord_setup(void){
	
	 /* Enable GPIOB clock. */
	rcc_periph_clock_enable(RCC_GPIOB);
	/* Set GPIO6/7 (in GPIO port B) to
	 *  'output push-pull' for the LEDs. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, HC165_PL|HC165_CLK|HC165_CS);
	//Q7 это выход 74HC165
	gpio_set_mode(HC165_PORT, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_FLOAT, HC165_Q7);
	gpio_set(HC165_PORT,HC165_CS|HC165_PL);
	gpio_clear(HC165_PORT,HC165_CLK);
	 
 }


static void gpio_setup(void){
	/* Enable GPIOB clock. */
	rcc_periph_clock_enable(RCC_GPIOB);
	/* Set GPIO6/7 (in GPIO port B) to
	 *  'output push-pull' for the LEDs. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO1|GPIO0);
	
	
	//на pb0 весит подсветка дисплея
	gpio_set(GPIOB,GPIO0);
	}

	
static void spi1_setup(void){
	//spi1 - display
	/* Enable SPI1 Periph and gpio clocks */
	rcc_periph_clock_enable(RCC_SPI1);
	rcc_periph_clock_enable(RCC_GPIOA);
	/* Configure GPIOs:
	 * SCK=PA5
	 * DC=PA6
	 * MOSI=PA7 
	 * STCS PA1
	 * RST PA4
	 * LED PB0 -PWM
	 */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5|GPIO7);
    //gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
    //        GPIO_CNF_INPUT_FLOAT, GPIO6);        
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO4|GPIO6|GPIO1);
	              
  /* Reset SPI, SPI_CR1 register cleared, SPI is disabled */
	spi_reset(SPI1);
  /* Set up SPI in Master mode with:
   * Clock baud rate: 1/64 of peripheral clock frequency
   * Clock polarity: Idle High
   * Clock phase: Data valid on 2nd clock pulse
   * Data frame format: 8-bit
   * Frame format: MSB First
   */
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_2, 
					SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
					SPI_CR1_CPHA_CLK_TRANSITION_1,
					SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
	//spi_set_full_duplex_mode(SPI1);
  /*
   * Set NSS management to software.
   *
   * Note:
   * Setting nss high is very important, even if we are controlling 
   * the GPIO
   * ourselves this bit needs to be at least set to 1, otherwise the spi
   * peripheral will not send any data out.
   */
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
  /* Enable SPI1 periph. */
	spi_enable(SPI1);
	gpio_set(GPIOA,GPIO1);
	}

static void spi2_setup(void){
	//spi1 - flash
	rcc_periph_clock_enable(RCC_SPI2);
	rcc_periph_clock_enable(RCC_GPIOB);
	/* Configure GPIOs:
	 * SCK=PB13
	 * MOSI=PB15
	 * MISO PB14 
	 * CS PB12
	 */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO13|GPIO15);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
            GPIO_CNF_INPUT_FLOAT, GPIO14);        
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
	              
  /* Reset SPI, SPI_CR1 register cleared, SPI is disabled */
	spi_reset(SPI2);
  /* Set up SPI in Master mode with:
   * Clock baud rate: 1/64 of peripheral clock frequency
   * Clock polarity: Idle High
   * Clock phase: Data valid on 2nd clock pulse
   * Data frame format: 8-bit
   * Frame format: MSB First
   */
	spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_4, 
					SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
					SPI_CR1_CPHA_CLK_TRANSITION_1,
					SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
	//spi_set_full_duplex_mode(SPI2);
  /*
   * Set NSS management to software.
   *
   * Note:
   * Setting nss high is very important, even if we are controlling 
   * the GPIO
   * ourselves this bit needs to be at least set to 1, otherwise the spi
   * peripheral will not send any data out.
   */
	spi_enable_software_slave_management(SPI2);
	spi_set_nss_high(SPI2);
  /* Enable SPI1 periph. */
	spi_enable(SPI2);
	gpio_set(GPIOB,GPIO12);
	}
/*
static void pwm_init(void){
	//LED PB2 -PWM
	rcc_periph_clock_enable(RCC_TIM3);
	rcc_periph_clock_enable(RCC_AFIO);

	// PB0 == TIM3.CH3 
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ,   // High speed
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO0); // GPIOB0=TIM3.CH3
	timer_disable_counter(TIM3);
	
	timer_set_mode(TIM3,TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
													TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM3,2);
   // Only needed for advanced timers:
   // timer_set_repetition_counter(TIM2,0);
   timer_enable_preload(TIM3);
   timer_set_period(TIM3,33000);
   timer_reset_output_idle_state(
   timer_disable_oc_output(TIM3,TIM_OC3);
   timer_set_oc_mode(TIM3,TIM_OC3,TIM_OCM_PWM1);
   timer_enable_oc_output(TIM3,TIM_OC3);

	timer_set_oc_value(TIM3,TIM_OC3,30000);
	timer_enable_counter(TIM3);
	}
*/
//static void i2c_setup(void){
//	/* Enable clocks for I2C2 and AFIO. */
//	rcc_periph_clock_enable(RCC_I2C1);
//	rcc_periph_clock_enable(RCC_AFIO);
//	/* Set alternate functions for the SCL and SDA pins of I2C2. */
//	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
//		      GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
//		      GPIO_I2C1_SCL|GPIO_I2C1_SDA);
//	//SDA PB7
//	//SCL PB6
//	/* Disable the I2C before changing any configuration. */
//	i2c_peripheral_disable(I2C1);
//	/* APB1 is running at 36MHz. */
//	i2c_set_clock_frequency(I2C1, I2C_CR2_FREQ_36MHZ);
//	/* 400KHz - I2C Fast Mode */
//	i2c_set_fast_mode(I2C1);
//	/*
//	 * fclock for I2C is 36MHz APB2 -> cycle time 28ns,
//	 * 										 low time at 400kHz
//	 * incl trise -> Thigh = 1600ns; CCR = tlow/tcycle = 0x1C,9;
//	 * Datasheet suggests 0x1e.
//	 */
//	i2c_set_ccr(I2C1, 0x1e);
	/*
	 * fclock for I2C is 36MHz -> cycle time 28ns, rise time for
	 * 400kHz => 300ns and 100kHz => 1000ns; 300ns/28ns = 10;
	 * Incremented by 1 -> 11.
	 */
//	i2c_set_trise(I2C1, 0x0b);
	/* If everything is configured -> enable the peripheral. */
//	i2c_peripheral_enable(I2C1);
//}


static void usart1_setup(void){
	/* Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1. */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART1);
	/* Enable the USART1 interrupt. */
	nvic_enable_irq(NVIC_USART1_IRQ);
	//PA9 TX,PA10 RX
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
	              GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
				  GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);
	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	
	usart_enable_rx_interrupt(USART1);
	//Finally enable the USART.
	usart_enable(USART1);
	}
	

	
	
	
/**********************************************************************
 * Прерывания
 **********************************************************************/


void usart1_isr(void){
	static uint16_t i=0;
	static rcv_cmd_ok=0;
	char c;
	c=usart_recv_blocking(USART1);
	if(rcv_cmd_ok) rcv_cmd_ok=0;
	if(i>1023)i=0;//защита от переполнения буфера
	
	//gpio_toggle(GPIOB,GPIO1);
	usartrxbufer[i++]=c;
	//if(c=='\n'){
		//usartrxbufer[i]=0x00;
		//i=0;
		//rcv_at_cmd();
		//}
	if(c=='\n'){
		usartrxbufer[i]=0x00;
		//ищем патерн завершенной команды
		if(i>5){
		
		if(strstr(usartrxbufer-5+i,"OK\r\n")) rcv_cmd_ok=1;
		else if(!strncmp(usartrxbufer,"\r\n+CMTI: ",7)) rcv_cmd_ok=1;
		else if(!strncmp(usartrxbufer,"\r\n+CUSD: ",7)) rcv_cmd_ok=1;
		else if(!strncmp(usartrxbufer,"\r\n+CME ",6)) rcv_cmd_ok=1;
		else if(!strncmp(usartrxbufer, "\r\nNO CARIER\r\n",9)) rcv_cmd_ok=1;
		else if(strstr(usartrxbufer-20+i, "RING\r\n")) rcv_cmd_ok=1;
		else if(strstr(usartrxbufer-20+i, "NO CARIER\r\n")) rcv_cmd_ok=1;
		};
		//если нашли вызываем декодер команд
		
		if(rcv_cmd_ok){
			//stprintf("\arcv command OK\r\n");
			i=0;
			rcv_at_cmd_v2();
			}
		}
	//отладочный вывод
	//st7735_sendchar(c);
	}


void rcv_at_cmd_v2(){
	//декодер пока умеет только смс
	//stprintf("parse start\r\n");
	char *b;
	char *e;
	uint8_t l=0;
	char temp_string;
	char temp[3];
	temp[2]=0;
	//stprintf("%s",usartrxbufer);
	if(!strncmp(usartrxbufer,"\r\n+CMTI:",7)){
		b=strpbrk(usartrxbufer,"0123456789");
		l=strspn(b,"01234567890");
		strncpy(temp,b,l);
		temp[2]=0;
		printf_("AT+GMGR=%s\r\n",temp);
		}
	else if(!strncmp(usartrxbufer,"\r\n+CMGR:",7)){
		//stprintf("SMS rcv\r\n");
		parcesms_v1(usartrxbufer);
		}
	else if(!strncmp(usartrxbufer, "\r\n+CUSD:",7)){
		//stprintf("%s",usartrxbufer);
		parce_ussd_v0(usartrxbufer);
		}
		else stprintf("\aAT%s", usartrxbufer+2);
			
	}
/*
void rcv_at_cmd(){
	//static uint16_t i=0;
	static uint8_t ok=0;
	if(ok){atcommand[0]=0x00; ok=0;}
	if(strstr(usartrxbufer, "OK\r\n")) ok=1;
	if(strstr(usartrxbufer, "RING\r\n")) ok=1;
	if(strstr(usartrxbufer, "NO CARIER\r\n")) ok=1;
	if(strstr(usartrxbufer, "ERROR")) ok=1;
	strcat(atcommand, usartrxbufer);
	if(ok){
		if(strstr(atcommand,"+CMGR:")){parcesms_v0(); return;}
		stprintf("\aAT%s", atcommand+2);
		//at_parcer();			
		}
			
	}
*/

/**********************************************************************
 * Секция основных функций
 **********************************************************************/
 



void standby(void){
	SCB_SCR|=SCB_SCR_SLEEPDEEP;
	PWR_CR|=PWR_CR_PDDS;
	PWR_CR|=PWR_CR_CWUF;
	
	__asm__("WFI");
	}



//функции специфичные для устройства
void write_to_flash(void){
	uint32_t addr;
	printf_("\r\nWrite to flash\r\nEnther the addres\r\n");
	rscanf("%n",&addr);
	printf_("\r\nWrite to flash at 0x%x\r\nChose the file\r\n",addr);
	//usart_disable_rx_interrupt(USART2);
	xmodem_to_flash(addr);
	stprintf("\r\done!\r\n");
	//usart_enable_rx_interrupt(USART2);
	}

void img_from_flash_v3(uint32_t addr){
	gpio_clear(STPORT,STCS);
	st7735_sendcmd(ST7735_MADCTL);
	st7735_senddata(1<<7);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(GPIOA,STCS);
	uint8_t bufer[4096];
	for(uint8_t i=0;i<10;i++){
		w25_read(addr+i*4096, bufer, 4096);
		st7735_drawimg(0,0+16*i,128,16, bufer);
		}
	gpio_clear(STPORT,STCS);
	st7735_sendcmd(ST7735_MADCTL);
	st7735_senddata(MV|MX);
	while (SPI_SR(SPI) & SPI_SR_BSY);
	gpio_set(STPORT,STCS);
	}

dump_page(uint32_t addr) {
	char buf[17];
	//addr&=~0xFF;		// Start on page boundary
	st7735_sendchar('\a');//очищение экрана
	for ( int x=0; x<16; ++x, addr += 16 ) {
		stprintf("%02X ",(unsigned)addr);
		w25_read(addr,buf,16);
		//for ( uint32_t offset=0; offset<16; ++offset )
		//	stprintf("%02X ",buf[offset]);
		for ( uint32_t offset=0; offset<16; ++offset ) {
	 		if ( buf[offset] < ' ' || buf[offset] >= 0x7F )
				st7735_sendchar('.');
			else	st7735_sendchar(buf[offset]);
		}
		//st7735_sendchar(UART,'\r');
		st7735_sendchar('\r');
		st7735_sendchar('\n');
	}
	return addr;
}

dump_page_hex(uint32_t addr) {
	char buf[9];
	//addr&=~0xFF;		// Start on page boundary
	st7735_sendchar('\a');//очищение экрана
	for ( int x=0; x<16; x++, addr += 8 ) {
		stprintf("%02X ",(unsigned)addr);
		w25_read(addr,buf,8);
		for ( uint32_t offset=0; offset<8; ++offset )
										stprintf("%02X",buf[offset]);
		stprintf("\r\n");
	}
	return addr;
}

dump_page_usart(uint32_t addr) {
	char buf[17];
	//addr&=~0xFF;		// Start on page boundary

	for ( int x=0; x<16; ++x, addr += 16 ) {
		stprintf("%06X ",(unsigned)addr);
		w25_read(addr,buf,16);
		for ( uint32_t offset=0; offset<16; ++offset )
			stprintf("%02X ",buf[offset]);
		for ( uint32_t offset=0; offset<16; ++offset ) {
			if ( buf[offset] < ' ' || buf[offset] >= 0x7F )
				usart_send_blocking(USART2,'.');
			else	usart_send_blocking(USART2,buf[offset]);
		}
		usart_send_blocking(USART2,'\r');
		usart_send_blocking(USART2,'\n');
	}
	return addr;
}

void d(void){
	for(uint32_t i=0;i<0xffff;i++)__asm__("nop");
}

void tel_book_init(void){
	char init_tst[96];
	w25_read(0x00,init_tst,96);
	for(uint8_t i=0;i<96;i++) if(init_tst[i]!=0xff) return;
	d();	
	w25_write_str(0x00, "aa");
	d();
	w25_write_str(0x10, "xxxxxxxxxxx");
	d();
	w25_write_str(0x20, "bb");
	d();
	w25_write_str(0x30, "8xxxxxxxxxx");
	d();
	w25_write_str(0x40, "cc");
	d();
	w25_write_str(0x50, "8xxxxxxxxxx");
	}




int main(void){
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	gpio_setup();
	//usart2_setup(); //xmodem
	usart1_setup();
	//i2c_setup();
	spi1_setup();
	spi2_setup();
	sim800_init();
	keybord_setup();
	systick_init();
	st7735_init();
	//pwm_init();
	w25_powerup();
	st7735_clear(BLACK);
	stprintf("start!\r\n");
	//Задаём цветовую схему
	 
	st7735_set_printf_color(GREEN,BLACK);
	
	st7735_drawline(0,0,160,128,RED);
	for(uint32_t i=0;i<0xfffff;i++)__asm__("nop");
	img_from_flash_v3(131142);
	//stprintf("Проверка работы с кирилицей\r\n");
	//st7735_string_at(10,10,"This is a test!!!",GREEN,BLACK);
	sim800_sleep();
	tel_book_init(); //тестовая телефонная книга
	while (1){	
		//command=usart_recv_blocking(UART);
		//for(uint32_t i=0xfffff;i;i--)__asm__("nop");
		get_keybord_cmd();
			};
	
		
return 0;
}
  
