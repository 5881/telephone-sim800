/*********************************************************************
 * menu func
 * эта фигня стала занимать слишком много и я её вынес в отдельный файл
 * *******************************************************************/

#ifndef MENU_H
#define MENU_H
#include "stdint.h"
typedef struct telbook {
	char name[16];
	char tel[16];
	} telbook;
void telbook_menu_v2(void);
void sms_menu(void);
void power_menu(void);
void erasing_page_dialog(uint8_t size);
void data_menu(void);
void img_menu(void);
void sim800_menu(void);
void main_help_menu(void);
void get_keybord_cmd(void);
void telbook_menu(void);

#endif
