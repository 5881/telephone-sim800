/*********************************************************************
 * Библиотека работы с клавиатурой 4x4 на 74HC165
 * с эмуляцией SPI
 * v 0.9
 * добавлена поддержка многократных нажатий
 * раскладка теперь хранится массивом
 * добавлена визуализация набора
 * 27 июля 2019
 *********************************************************************/
#ifndef KEY_H
#define KEY_H

#define HC165_PL GPIO9
#define HC165_Q7 GPIO7
#define HC165_CLK GPIO8
#define HC165_CS  GPIO6
#define HC165_PORT GPIOB
 
#define ECHO 1
 
#define echo(char) st7735_sendchar(char)
 
uint16_t read_key(void);
//char keymap(uint16_t, uint8_t);
void systick_init(void);
char get_key(void);
char fast_get_key(void);

#endif
