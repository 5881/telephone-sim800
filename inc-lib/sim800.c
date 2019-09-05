//функции специфичные для sim800, потом вынести в отдельный файл

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
/**********************************************************************
 * Библиотека работы с модулем sim800c
 * v0.1
 * 5 aug 2019
 * v0.2
 * 11 aug 2019 
 * Добавлено декодирование смс
 * Alexander Belyy 
 **********************************************************************/

//#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include "rprintf.h"
#include "st_printf.h"
#include "sim800.h"


//#define SMSDEBUG 1
extern char atcommand[];




void sim800_init(){
	rcc_periph_clock_enable(RCC_GPIOA);
	/* Set GPIO6/7 (in GPIO port B) to
	 *  'output push-pull' for the LEDs. */
	gpio_set_mode(SIM800_PORT, GPIO_MODE_OUTPUT_50_MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, SIM800_RST|SIM800_DTR);
	gpio_set_mode(SIM800_PORT, GPIO_MODE_INPUT,
	              GPIO_CNF_INPUT_FLOAT, SIM800_RI);
}

void sim800_sleep(void){
	for (uint32_t i=0;i<0xfffff;i++)__asm__("nop");
	gpio_set(SIM800_PORT,SIM800_DTR);
	}

void sim800_wake(void){
	gpio_clear(SIM800_PORT,SIM800_DTR);
	for (uint32_t i=0;i<0xfffff;i++)__asm__("nop");
	}
void sim800_take_call(void){
	sim800_wake();
	printf_("ATA\r\n");
	}

void sim800_ath(void){
	sim800_wake();
	printf_("ATH\r\n");
	sim800_sleep();
	}
void sim800_init_cmd(){
	sim800_wake();
	stprintf("\a");
	//st7735_sendchar('\a');//очищение экрана
	printf_("AT+CMGF=1\r\n");
	for (uint32_t i=0;i<0xffffff;i++)__asm__("nop");
	printf_("AT+CSCS=\"UCS2\"\r\n");
	for (uint32_t i=0;i<0xffffff;i++)__asm__("nop");
	printf_("AT+CSMP=17,167,0,8\r\n");
	for (uint32_t i=0;i<0xffffff;i++)__asm__("nop");
	
	sim800_sleep();
}
	
void call(){
	sim800_wake();
	char tel[12];
	char *ptr;
	stprintf("\aEnter the namber\n");
	set_scanf_mode(1);
	kscanf("%s",tel);
	set_scanf_mode(0);
	ptr=tel;
	if (*tel=='8') ptr=tel+1;
	if (*tel=='+') ptr=tel+2;
	stprintf("ATD+7%s;\n",ptr);
	printf_("ATD+7%s;\r\n",ptr);
	}
	

void sim800_power(void){
	gpio_set(SIM800_PORT,SIM800_RST);
	for (uint32_t i=0; i<0xffffff;i++)__asm__("nop");
	//for (uint32_t i=0; i<0xffffff;i++)__asm__("nop");
	//for (uint32_t i=0; i<0xffffff;i++)__asm__("nop");
	//for (uint32_t i=0; i<0xffffff;i++)__asm__("nop");
	gpio_clear(SIM800_PORT,SIM800_RST);

	}

void at_cli(void){
	sim800_wake();
	char bufer[32];
	char cmd[35]="at";
	char key;
	stprintf("\a");
	do{
		stprintf("Enter command\r\n>");
		kscanf("%s", bufer);
		if(bufer[0]!='a')printf_("at%s\r\n", bufer);
			else printf_("%s\r\n", bufer);
		bufer[0]=0x00;
		key=fast_get_key();
		stprintf("\a");
	}while(key!='\n');
	sim800_sleep();
	return;
	}
	

/***********************************************************************
 * Функция деродирования смс
 * ********************************************************************/
void parce_ussd_v0(char *in_data){
	//stprintf("ussd decode start\r\n");
	char data[512];
	uint16_t strlen=0;
	uint8_t search=1;
	char *b;
	char *e;
	b=strstr(in_data,", \"");
	e=strstr(in_data,"\",");
	strlen=e-b-3;
	//stprintf("b=%d,e=%d, len=%d\r\n",b,e,strlen);
	strncpy(data,b+3,strlen);
	data[strlen]=0;
	//stprintf("data: %s\r\n");
	utc2_to_cp866(data);
	stprintf("\aCUSD: %s\r\n",data);
}
 
void parcesms_v1(char *in_data){
	char tel[49];
	//char name[17];
	char date[25];
	char data[512];
	uint16_t smslen=0;
	uint8_t search=1;
	char *b;
	char *e;
	//while(!ok)__asm__("nop"); //без этого всё виснет намертво
	b=strstr(in_data,"\",\"");
	//strncpy(tel,b+3,12);
	strncpy(tel,b+3,48);
	//проверяем номер на размер как "MTS" "+7aaabbbcddd"
	for(uint8_t i=0;i<48;i++)if(tel[i]=='"'){tel[i]=0; search=0;};
	//tel[12]=0x00;
	tel[49]=0;
	utc2_to_cp866(tel);
	//если телефон в книге заменим телефон на имя.
	if(search)telbook_find_name(tel,tel);
	//stprintf("R:%s\r\n",tel);
	
	b=strstr(in_data,"\"\r\n");
	strncpy(date,b-20,20);
	date[20]=0x00;
	
	e=strstr(in_data, "\r\nOK");
	smslen=e-b-3; 
	strncpy(data,b+3,smslen);
	data[smslen]=0x00;
	//Отладочный вывод
	stprintf("tel:%s\r\n",tel);
	stprintf("date:%s\r\n",date);
	//stprintf("\r\ntext:%s",data);
	utc2_to_cp866(data);
	stprintf("text:\r\n%s",data);
	} 

/*
void parcesms_v0(void){
	char tel[16];
	//char name[17];
	char date[25];
	char data[512];
	uint16_t smslen=0;
	uint8_t search=1;
	char *b;
	char *e;
	//while(!ok)__asm__("nop"); //без этого всё виснет намертво
	b=strstr(atcommand,"\",\"");
	strncpy(tel,b+3,12);
	//проверяем номер на размер как "MTS" "+7aaabbbcddd"
	for(uint8_t i=0;i<12;i++)if(tel[i]=='"'){tel[i]=0; search=0;};
	tel[12]=0x00;
	//если телефон в книге заменим телефон на имя.
	if(search)telbook_find_name(tel,tel);
	//stprintf("R:%s\r\n",tel);
	
	b=strstr(atcommand,"\"\r\n");
	strncpy(date,b-20,20);
	date[20]=0x00;
	
	e=strstr(atcommand, "\r\nOK");
	smslen=e-b-3;
	strncpy(data,b+3,smslen);
	data[smslen]=0x00;
	//Отладочный вывод
	stprintf("tel:%s\r\n",tel);
	stprintf("date:%s\r\n",date);
	//stprintf("\r\ntext:%s",data);
	utc2_to_cp866(data);
	stprintf("text:\r\n%s",data);
	} 
*/

void fast_sms_send(char *text,char *tel){
	char *p_tel;
	char u_tel[64]="+7";
	char temp[512];
	if(tel[0]=='8') p_tel=tel+1;//8xxxx... >> xxxxx...
	else if (tel[0]=='+') p_tel=tel+2;//+7xxxxx >> xxxxx...
	else p_tel=tel;//xxxxx... >> xxxxx...
	strcat(u_tel,p_tel);
	strcpy(temp,text);
	cp866_to_utc2(temp);
	cp866_to_utc2(u_tel);
	stprintf("\aSend sms\r\nAT+CMGS=\"%s\"\r\n%s\x1A",u_tel,temp);
	
	printf_("AT+CMGS=\"%s\"\r\n",u_tel);
	for (uint32_t i=0; i<0xffffff;i++)__asm__("nop");
	printf_("%s\x1A",temp);

	}



void int_to_char(uint16_t ch, char *data){
	static const char table[]="0123456789ABCDEF";
	uint8_t a;
	uint16_t c=ch;
	stprintf("\a");
	for(uint8_t i=0; i<4;i++){
		a=ch&0xf;
		stprintf("a=%x\r\n",a);
		ch>>=1;
		data[3-i]=table[a];
	}
	data[4]=0;
	stprintf("%x, str=%s",c,data);
	for (uint32_t i=0;i<0xffffff;i++)__asm__("nop");
}

uint16_t char_to_int16(char *data){
	uint16_t result=0;
	uint8_t temp=0;
	static const char table[]="0123456789ABCDEF";
	for(uint8_t i=0;i<4;i++){
		while(data[i]!=table[temp])temp++;
		result<<=4;
		result|=temp;
		temp=0;
		}
	return result;
	}

void cp866_to_utc2(char *data){
	int len=0;
	len=strlen(data);
	uint8_t ch;
	char temp[512];
	for(uint16_t i=0; i<len;i++){
		ch=data[i];
		if(ch<0x80){//int_to_char(ch,temp+i*4);
			snprintf(temp+i*4,5,"00%x",ch);
			continue;}
		if(ch>0x7f){//учитываем псевдографику
			if(ch>0xdf) ch=ch-0x70-0x30;
				else ch=ch-0x70;
			//int_to_char(ch,temp+i*4);
			snprintf(temp+i*4,5,"04%x",ch);
			continue;
		}
	}
	strcpy(data,temp);
	}

void utc2_to_cp866(char *data){
	uint8_t h,l;
	uint8_t j=0;
	int ch;
	int i=0;
	int len=0;
	//st7735_sendchar('\a');
	//stprintf("start decode\r\n");
	len=strlen(data);
	//stprintf("data len=%d\r\n",len);
	for(i=0;i<len-4;i+=4){
		ch=char_to_int16(data+i);
		//stprintf(" %d char is %x\r\n",i,ch); //для отладки
		h=ch>>8;
		l=ch&0xff;
		if(h==4){
			switch(l){
				case 0x1: l=0x85;
					break;
				case 0x51: l=0xa5;
					break;
				default:
					l=l-0x10+0x80;
					if(l>0xaf) l+=0x30; //поправка на псевдографику в cp866;
					break;
				}
			}
		data[j++]=l;
		//st7735_sendchar(l);
		//это отладочные ыункции
		//stprintf("result char is \"%c\"\r\n",l);
		//temp[j++]=l;
		//for(uint32_t j=0;j<0xfffff;j++)__asm__("nop");
		}
		//temp[j]=0;
		data[j]=0;
}
