
/**********************************************************************
 * Библиотека работы с модулем sim800c
 * v0.1
 * 5 aug 2019
 * Alexander Belyy 
 **********************************************************************/
#ifndef SIM800_H
#define DIM800_H


#define SIM800_PORT GPIOA
#define SIM800_RST GPIO0
#define SIM800_RI GPIO8
#define SIM800_DTR GPIO11
//#define SIM800_UART UART1

void fast_sms_send(char *text,char *tel);
void int_to_char(uint16_t ch, char *data);
void cp866_to_utc2(char *data);
void sim800_init();
void sim800_sleep(void);
void sim800_wake(void);
void sim800_take_call(void);
void sim800_ath(void);
void sim800_init_cmd();
void call();
void sim800_power(void);
void at_cli(void);
void parcesms_v0(void);
uint16_t char_to_int16(char *data);
void utc2_to_cp866(char *data);
#endif
