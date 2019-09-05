
/** \file printf.c
 * Simplified printf() and sprintf() implementation - prints formatted string to
 * USART (or whereever). Most common % specifiers are supported. It costs you about
 * 3k FLASH memory - without floating point support it uses only 1k ROM!
 * \author Freddie Chopin, Martin Thomas, Marten Petschke and many others
 * \date 16.2.2012
 * reduced scanf() added by Andrzej Dworzynski on the base of reduced sscanf() written by
 * some nice gay from this post: http://www.fpgarelated.com/usenet/fpga/show/61760-1.php
 * thanks to everybody:)
 * \date 12.2.2013
 */

/*****************************************************************************
 * 5 января 2019
 * Немного подправил текст библиотеки, адоптировав под libopencm3
 * Выкинул лишние файлы, уместив всё в одном.
 * Shaman
*****************************************************************************/ 
#ifndef RPRINTF_H
#define RPRINTF_H

int printf_(const char *format, ...);
int sprintf_(char *buffer, const char *format, ...);
int rscanf(const char* format, ...);

#endif
