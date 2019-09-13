/*********************************************************************
 * Библиотека работы с клавиатурой 4x4 на 74HC165
 * с эмуляцией SPI
 * v 0.9
 * добавлена поддержка многократных нажатий
 * раскладка теперь хранится массивом
 * добавлена визуализация набора
 * 27 июля 2019
 *********************************************************************/
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include "st7735_128x160.h"
#include "4x4key.h"


static void timer_start(void);
static void timer_stop(void);
static char keymap(uint16_t key_code, uint8_t count);
static char keymap2(uint8_t count, uint8_t ch_map);
static void show_char_map(uint8_t ch_map);

uint16_t wait_key_counter=0;
uint8_t wait_key=0;
//раскладка клавиатуры
static const char char_map[] =
{
'\n','~','!',' ','1','4','7','\b','2','5','8','0','3','6','9','*',
'\n',':','#','=','a','j','s','\b','d','m','v','.','g','p','x','+',
'\n',';','=','&','b','k','t','\b','e','n','w',',','h','q','y','-',
'\n','@','\\','$','c','l','u','\b','f','o','8','"','i','r','z','?',

'\n','~','!',' ','1','4','7','\b','2','5','8','0','3','6','9','*',
'\n',':','#','=','\xa0','\xa9','\xe2','\b','\xa3','\xac','\xe5','\xeb','\xa6','\xaf','\xe8','\xee',
'\n',';','/','&','\xa1','\xaa','\xe3','\b','\xa4','\xad','\xe6','\xec','\xa7','\xe0','\xe9','\xef',
'\n','@','\\','$','\xa2','\xab','\xe4','\b','\xa5','\xae','\xe7','\xed','\xa8','\xe1','\xea','.',

' ','.',',','?','*','#','$','&','\\','/','=','"','[',']','{','}'
};
 
 /**********************************************************************
 * systic таймер используется для работы с клавиатурой
 * кнопок всего 16 а надо больше
 * поэтому реализуем многократное нажатие
 **********************************************************************/
void systick_init(void){
	//72MHz / 8 => 9000000  counts per second 
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	//9000000/9000 = 1000 overflows per second - every 1ms one interrupt
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(8999);
	//systick_interrupt_enable();
	//Start counting.
	systick_counter_enable();
}	
	
	
 
 //инициализация таймера
 void sys_tick_handler(void)
{
	// We call this handler every 1ms so 1000ms = 1s on/off. 
	//if (wait_key) {
	wait_key_counter++;
	//	if(wait_key_counter > 1000){wait_key_counter=0; wait_key=0;}
		//systick_counter_disable();
	//} else if (wait_key_counter)wait_key_counter=0;
}
 
 
 
 
 
 
 
static void delay(uint32_t d){
	 d*=3600;
	 while(d){ d--; __asm__("nop");};
	 }
 
uint16_t read_key(void){
	 uint16_t data;
	 
	 gpio_clear(HC165_PORT, HC165_CS);//включить тактирование
	 gpio_clear(HC165_PORT, HC165_PL);//записать значение в сдвиговый регистр
	 //delay(10);
	 gpio_set(HC165_PORT, HC165_PL);
	 for(uint8_t i=0;i<16;i++){
		data<<=1;
		if(gpio_get(HC165_PORT, HC165_Q7)) data|=1; 
		gpio_set(HC165_PORT,HC165_CLK);
		//delay(10);
		gpio_clear(HC165_PORT,HC165_CLK);
		}
	 gpio_set(HC165_PORT,HC165_CS);
	 data=~data;
	 return data;
	 }
 
 
 
// void gpio_test(void){
//	 gpio_toggle(HC165_PORT, HC165_CS|HC165_CLK|HC165_PL);
//	 delay(0xffff);
//	 }
	 
static char keymap2(uint8_t count, uint8_t ch_map){
	uint16_t ch;
	ch=ch_map*64+count;
	return char_map[ch];
	}

static char keymap(uint16_t keycode, uint8_t count){
	uint8_t i=0, shift=0;
	while(!(keycode&(1<<i))) i++;
	i=i+16*count; //реализация кодовых страниц
	//DEBUG
	//stprintf("i= %d, count= %d\r\n",i,count); 
	//DEBUG
	/*	
	switch (i){
		case 0: return '\n';
		case 1: return 'x';
		case 2: return '\a';
		case 3: return ' ';
		case 4: return '1';
		case 5: return '4';
		case 6: return '7';
		case 7: return '\b';
		case 8: return '2';
		case 9: return '5';
		case 10: return '8';
		case 11: return '0';
		case 12: return '3';
		case 13: return '6';
		case 14: return '9';
		case 15: return '*';
		case 20: return 'A';
		case 36: return 'a'; 
		}
		*/
		return char_map[i];
	}

static void timer_start(void){
	wait_key_counter=0;
	systick_interrupt_enable();
}
static void timer_stop(void){
	//останавливаем прерывания таймера
	systick_interrupt_disable();
	wait_key_counter=0;
}

char fast_get_key(){
	 //static	 //static uint8_t count=0;
	//uint8_t count=0;
	//static uint16_t old_keycode;
	uint16_t key_code;
	uint16_t old_keycode=0;
	char key;
	while(!key_code) key_code=read_key();
	key=keymap(key_code, 0);
	while(read_key()) __asm__("nop");
	//count=0;
#ifdef ECHO
	 echo(key);
#endif
	 return key;
	 }

static void show_char_map(uint8_t ch_map){
	switch(ch_map){
		case 0: 
			st7735_string_at(0,120,"ENG",RED,BLACK);
			break;
		case 1:
			st7735_string_at(0,120,"RUS",RED,BLACK);
			break;
	}
	
	}


char get_key(){
	 //static	 //static uint8_t count=0;
	uint8_t count=0;
	static uint8_t ch_map=0;
	uint8_t change_keymap=0;
	//static uint16_t old_keycode;
	uint16_t key_code;
	uint16_t old_keycode=0;
	char key;
	//клавиша 0x0002 переключение раскладок
	//переключаем раскладки и ждём другой код
	while(!key_code) key_code=read_key();
	do{ 
		if(key_code==2){
			if(ch_map<2) ch_map++; else ch_map=0;
			show_char_map(ch_map);
			while(key_code==2) key_code=read_key();
			//delay(500);
			} //переключение расклвадок
		while(!key_code) key_code=read_key();
	}while(key_code==2);
	
	key=keymap(key_code, count+ch_map*4);
	
	if(key=='\n')delay(500);
	else if (key=='\b')delay(500);
	else if(key==' '){
	#ifdef ECHO
		st7735_virt_sendchar(key);
	#endif
		delay(800);
		timer_start();
		old_keycode=key_code;
		do{
			key_code=read_key();
			if(key_code){//при нажати другой кнопки сразу возвращается она
				if(key_code == old_keycode){ 
					count++;
					if (count>15) count=0;
					wait_key_counter=0;
					}
			else {count=0; break;}
		key=keymap2(count,2);
		//виртуальный вывод
#ifdef ECHO
		systick_interrupt_disable();
		st7735_virt_sendchar(key);
		systick_interrupt_enable();
#endif
		delay(900);
		}
		}while(wait_key_counter<1000);
		timer_stop();	//останавливаем прерывания таймера
		}
		 	
	else{
#ifdef ECHO
		st7735_virt_sendchar(key);
#endif
		delay(800);
		
		timer_start();
		old_keycode=key_code;
		
		do{
		key_code=read_key();
		//while(read_key()) __asm__("nop");//ждать пока отпустят клавишу
		if(key_code){//при нажати другой кнопки сразу возвращается она
			if(key_code == old_keycode){ 
				count++;
				if (count>3) count=0;
				wait_key_counter=0;
				}
			else {count=0; break;}
		
		//key=keymap(old_keycode, count);
		key=keymap(key_code, count+ch_map*4);
		//визуализация
#ifdef ECHO
		systick_interrupt_disable();
		//st7735_draw_s_char(1,1,key,RED,BLACK);
		st7735_virt_sendchar(key);
		systick_interrupt_enable();
#endif
		delay(900);
		}
		}while(wait_key_counter<1000);
		timer_stop();	//останавливаем прерывания таймера
		//key_code=old_keycode;
	};
	//key=keymap(key_code, count);
	
	
	
	count=0;
#ifdef ECHO
	 echo(key);
#endif
	 
	 return key;
	 }
