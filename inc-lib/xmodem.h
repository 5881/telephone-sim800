//драйвер пересылки файлов xmodem

#ifndef XMODEM_H
#define XMODEM_H

#include "stdint.h"

#define USARTX USART2

#define KEYBORD44 1

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define EOF 0x1A


void xmodem_to_flash(uint32_t addr);

#endif
