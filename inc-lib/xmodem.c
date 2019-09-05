//драйвер пересылки файлов xmodem

#include "xmodem.h"
#include "25q32.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#ifdef KEYBORD44
#include "4x4key.h"
#endif
static void usart2_init(void);
static void usart2_deinit(void);

static void usart2_init(void){
	// Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1.
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART2);
	// Enable the USART2 interrupt. 
	//nvic_enable_irq(NVIC_USART2_IRQ);
	/* Setup GPIO pin GPIO_USART1_TX/GPIO9 on GPIOA for transmit. */
	//PA9 TX,PA10 RX
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
	              GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
				  GPIO_CNF_INPUT_FLOAT, GPIO_USART2_RX);
	/* Setup UART parameters. */
	usart_set_baudrate(USART2, 115200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX_RX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
	
	//usart_enable_rx_interrupt(USART2);
	/* Finally enable the USART. */
	usart_enable(USART2); //экономим электричество
	}

static void usart2_deinit(void){
	usart_disable(USART2); //экономим электричество
	rcc_periph_clock_disable(RCC_USART2);
	}

void xmodem_to_flash(uint32_t addr){
	unsigned char buf[132];
	uint32_t byte=0;
	uint8_t lastlen,ch;
	usart2_init();
	//printf_("\r\nTo start load file at addres 0x%x press the button\r\n",addr);
	//for(uint32_t i=0;i<0x15fffff;i++) __asm__("nop");
	
#ifdef KEYBORD44
	//get_key()
#else
	//while(gpio_get(GPIOB,GPIO0));
#endif

	//printf_("\r\nstart\r\n");
	
	usart_send_blocking(USARTX,NAK);		// к приёму готов
	//usart_send_blocking(UART,ACK);		// к приёму готов
	while(1){
	ch = usart_recv_blocking(USARTX); //принимаем первый символ
	if (ch==SOH){					//если начало передачи 
		//if(byte) w25_write(addr+byte,buf+2,128);//предидущий пакет полный
		
		for(uint8_t i=0;i<131;i++){ //принимаем пакет
			ch = usart_recv_blocking(USARTX);
			buf[i]=ch;}
		//w25_write(addr+byte,buf+2,128);//предидущий пакет полный
		lastlen=129;
		while(buf[lastlen]==EOF) lastlen--; //отбрасываем EOF
		lastlen-=1;							//находим длинну пакета
		w25_write(addr+byte,buf+2,lastlen); //пишем
		byte+=lastlen; 
		usart_send_blocking(USARTX,ACK);//подтверждаем
		continue;
		}
	if(ch==EOT){//последний пакет
		//lastlen=129;
		//while(buf[lastlen]==EOF) lastlen--; //отбрасываем EOF
		//lastlen-=2;							//находим длинну пакета
		//w25_write(addr+byte,buf+2,lastlen); //пишем
		usart_send_blocking(USARTX,ACK);		// подтверждаем
		break;
		}
	}
	usart2_deinit();
}
