
/*********************************************************************
 * menu func
 * *******************************************************************/
 
#include "menu.h"
#include "st_printf.h"
#include "rprintf.h"
//#include "stdint.h"
#include "sim800.h"


extern telbook tb[];

void sms_menu(void){
	sim800_wake();
	//sim800_init_cmd();
	int sms_n=1;
	char key;
	char tel[15];
	stprintf("\aHELP\r\n"
			"1 - sms status\r\n"
			"2 - AT+CMGR=X\r\n"
			"3 - read all sms\r\n"
			"4 - del sms x\r\n"
			"5 - del all sms\r\n"
			"6 - save sms at addr\r\n"
			"7 - write sms\r\n"
			"8 - send test sms\r\n"
			"any key - to cancle\r\n");
	key=fast_get_key();
	stprintf("\a");
	switch (key){
			case '1':
				printf_("AT+CPMS?\r\n");
				break;
			case '2':
				stprintf("\aAT+CMGR=");
				set_scanf_mode(1);
				kscanf("%n",&sms_n);
				set_scanf_mode(0);
				printf_("AT+CMGR=%d\r\n",sms_n);
				//stprintf("\r\nwait...\r\n");
				stprintf("\aSMS %d\r\n",sms_n);
				break;
			case '3':
				sms_n=1;
				do{
				stprintf("\aSMS %d\r\n",sms_n);
				if(sms_n<1)sms_n=1;
				printf_("AT+CMGR=%d\r\n",sms_n);
				key=fast_get_key();
				if(key=='2') sms_n++;
				if(key=='5') sms_n--;
				if(key=='8'){//быстрое выключение
					st7735_sleep();
					w25_powerdown();
					standby();
					}
				}while (key!=' ');
				sms_menu();
				break;
			case '4':
				stprintf("\adel sms N\r\nAT+CMGD=");
				set_scanf_mode(1);
				kscanf("%n",&sms_n);
				set_scanf_mode(0);
				printf_("AT+CMGD=%d\r\n",sms_n);
				break;
			case '5':
				stprintf("\adel all readed sms\r\n ok?");
				key=fast_get_key();
				if(key=='\n') printf_("AT+CMGDA=\"DEL READ\"\r\n");
					else sms_menu();
				break;
			case '6':
				
				break;
			case '7':
				write_sms();
				break;
			case '8':
				telbook_get_number(tel);
				fast_sms_send("SIM800 telephone v0.11 sms hello world!",
											tel);
				break;
			case '9':
				
				break;
			}
	sim800_sleep();
	
	
}
 
 
 
 
void power_menu(void){
	 char key;
	uint32_t addr;
	stprintf("\aHELP\r\n"
			"1 - disp off 28h\r\n"
			"2 - standby\r\n"
			"3 - standby+28h\r\n"
			"4 - disp on\r\n"
			"5 - standby+logo\r\n"
			"6 - w25_powerdown\r\n"
			"7 - w25_powerup\r\n"
			"any key - to cancle\r\n");

	key=fast_get_key();
	switch (key){
			case '1':
				st7735_sleep();
				break;
			case '2':
				standby();
				break;
			case '3':
				st7735_sleep();
				w25_powerdown();
				standby();
				break;
			case '4':
				st7735_wake();
				break;
			case '5':
				img_from_flash_v3(0x30046);//RTS logo for CC
				w25_powerdown();
				standby();
				break;
			case '6':
				w25_powerdown();
				break;
			case '7':
				w25_powerup();
				break;
			default:
				return;
				break;
	}
}
	 
void erasing_page_dialog(uint8_t size){
	uint32_t addr;
	stprintf("enter addres of page\r\n:");
	kscanf("%n", &addr);
	w25_erase_block(addr, size);
	stprintf("\r\ndone!\r\n");
	}

void data_menu(void){
	char key;
	uint32_t addr=0;
	char bufer[1024];
	stprintf("\aHELP\r\n"
			"1 - dump data at addr\r\n"
			"2 - hex dump at addr\r\n"
			"3 - write string at\r\n"
			"4 - load file to flash\r\n" 
			"5 - read str from addr\r\n"
			"6 - erising sector 4k\r\n"
			"7 - erising sector 32k\r\n"
			"8 - erising sector 64k\r\n"
			"9 - erising chip\r\n"
			"any key - to cancle\r\n");
	key=fast_get_key();
	switch (key){
			case '1':
				stprintf("\adump of page\r\n");
				stprintf("enter the addres:");
				kscanf("%n",&addr);
				dump_page(addr);
				break;
			case '2':
				stprintf("\adump of page\r\n");
				stprintf("enter the addres:");
				kscanf("%n",&addr);
				dump_page_hex(addr);
				break;
			case '3':
				stprintf("\aWrite string to flash\r\n");
				stprintf("enter start addres:");
				kscanf("%n",&addr);
				stprintf("\r\nenter string:\r\n");
				kscanf("%s",bufer);
				//l=kscanf("%s",bufer);
				//l=strlen(bufer);
				//w25_write(addr, bufer,l+1);
				w25_write_str(addr,bufer);
				stprintf("the string is writed\r\n");
				break;
			case '4':
				stprintf("\awrite xmodem data\r\n Enter the addres:");
				kscanf("%n",&addr);
				//usart_disable_rx_interrupt(USART1);
				//systick_interrupt_disable();
				xmodem_to_flash(addr);
				//systick_interrupt_enable();
				stprintf("\r\ndone!\r\n");
				//usart_enable_rx_interrupt(USART1);
				break;
			case '5':
				stprintf("\aRead str from addres\r\n");
				stprintf("enter the addres:");
				kscanf("%n",&addr);
				w25_read_str(addr, bufer);
				stprintf("\a%s",bufer);
				break;
			case '6':
				stprintf("\aerising 4k\r\n:");
				erasing_page_dialog(0x20);
				break;
			case '7':
				stprintf("\aerising 32kk\r\n:");
				erasing_page_dialog(0x52);
				break;
			case '8':
				stprintf("\aerising 64k\r\n:");
				erasing_page_dialog(0xd8);
				break;
			case '9':
				stprintf("\aerising chip\r\n");
				w25_erase();
				stprintf("done\r\n");
				return;
				break;
			default:
				main_help_menu();
				return;
				break;
	}
	
	
	return;
}

void img_menu(void){
	char key;
	uint32_t addr;
	stprintf("\aHELP\r\n"
			"a - load img at addr\r\n"
			"1 - load from std1\r\n"
			"2 - load from std1\r\n"
			"3 - load from std1\r\n"
			"4 - load from std1\r\n"
			"5 - load from std1\r\n"
			"6 - load from std1\r\n"
			"any key - to cancle\r\n");

	key=fast_get_key();
	switch (key){
			case 'a':
				stprintf("\aEnter addr. of img\r\n:");
				kscanf("%n", &addr);
				break;
			case '1':
				addr=0x10000+0x46;
				break;
			case '2':
				addr=0x20000+0x46;
				break;
			case '3':
				addr=0x30000+0x46;
				break;
			case '4':
				addr=0x40000+0x46;
				break;
			case '5':
				addr=0x50000+0x46;
				break;
			case '6':
				addr=0x60000+0x46;
				break;
			default:
				return;
				break;
	}
	img_from_flash_v3(addr);
	return;
	}
	
void sim800_menu(void){
	char key;
	sim800_wake();
	int a=1;
	//char temp[64]="*100#"
	//uint32_t addr;
	stprintf("\aHELP\r\n"
			"0 - AT command line\r\n"
			"1 - sim800 init\r\n"
			"2 - AT+CREG?\r\n"
			"3 - AT+CSQ\r\n"
			"4 - AT+CCLK?\r\n"
			"5 - AT+CBC\r\n"
			"6 - AT+CMGF=1\r\n"
			"7 - AT+CSCS=\"GSM\"\r\n"
			"8 - AT&W\r\n"
			"9 - AT+CUSD=1,\"*100#\"\r\n"
			"* - AT+COPS=?\r\n");

	key=fast_get_key();
	stprintf("\a");
	switch (key){
			case '0':
				at_cli();
				break;
			case '1':
				sim800_init_cmd();
				//stprintf("\aAT+CPAS\r\n");
				//printf_("AT+CPAS\r\n");
				//parcesms_v0();
				break;
			case '2':
				//stprintf("\aAT+CREG=?\r\n");
				printf_("AT+CREG=?\r\n");
				break;
			case '3':
				//stprintf("AT+CSQ\r\n");
				printf_("AT+CSQ\r\n");
				break;
			case '4':
				//stprintf("\aAT+CCLK?\r\n");
				printf_("AT+CCLK?\r\n");
				break;
			case '5':
				//stprintf("\aAT+CBC\r\n");
				printf_("AT+CBC\r\n");
				break;
			case '6':
				//stprintf("\aAT+CMGF=1\r\n");
				printf_("AT+CMGF=1\r\n");
				break;
			case '7':
				//stprintf("AT+CSCS=\"GSM\"\r\n");
				printf_("AT+CSCS=\"GSM\"\r\n");
				break;
			case '8':
				//stprintf("\aAT&W\r\n");
				printf_("AT&W\r\n");
				break;
			case '9':
				//printf_("AT+CUSD=1,\"*100#\"\r\n");
				printf_("AT+CUSD=1,\"002A0031003000300023\"\r\n");//usc2
				//stprintf("\r\nwait...\r\n");
				break;
			case '*':
				printf_("AT+COPS=?\r\n");
				break;
			//default:
				//return;
				//break;
	}
	sim800_sleep();
}

void main_help_menu(void){
	stprintf("\aHELP\r\n"
						//"# - erising chip\r\n"
						"a - ATA\r\n"
						"h - ATH\r\n"
						"1 - data menu\r\n"
						"2 - call menu \r\n"
						"3 - img menu\r\n"
						"4 - power menu\r\n"
						"5 - sim800 menu\r\n"
						"6 - gsm power\r\n"
						"7 - power off\r\n"
						"* - call\r\n"
						"0 - sms menu");
	
	}

	
void get_keybord_cmd(void){
	char bufer[64];
	uint32_t addr, l,n=4096;
	char key;
	key=fast_get_key();
	switch (key){
		case '1':
			data_menu();
			break;
		case '2':
			//telbook_menu();
			telbook_menu_v2();
			break;
		case '3':
			img_menu();
			break;
		case '4':
			power_menu();
			break;
		case '5':
			sim800_menu();
			break;
		case '6':
			main_help_menu();
			break;
		case '7':
			sim800_power();
			break;
		case '8':
			st7735_sleep();
			w25_powerdown();
			standby();
			break;
		case '9':
			w25_powerdown();
			standby();
			break;
		case '0':
			sms_menu();
			break;
		case '*':
			call();
			//telbook_menu();
			break;
		case '~':
			sim800_take_call();
			break;
		case '!':
			sim800_ath();
			break;
		}
	return;	
};

void telbook_menu_v2(void){
	char number[13];
	uint8_t ret_code;
	ret_code=telbook_get_number(number);
	if(ret_code==1) fast_call(number);
	if(ret_code==2) fast_call(number);
	}


void telbook_menu(void){
	int count=0;
	count=telbook_rec_count();
	if(!count){stprintf("Telbook not found\r\n");return;};
	char key;
	read_telbook(0,tb,count);
	stprintf("\aTel book redactor v0.01\r\n");
	for(uint8_t i=0;i<count;i++) stprintf("%d %s %s\r\n",i+1,(tb+i)->tel,
														(tb+i)->name);
	stprintf("~ - add rec\r\n"
			"! - del rec\r\n");
	key=fast_get_key();
		
	switch (key){
						case '~':
				telbook_rec_add();
				break;
			case '!':
				telbook_rec_del();
				break;
			default:
				//main_help_menu();
				return;
				break;
		}
	//free(tb);
	}
	
