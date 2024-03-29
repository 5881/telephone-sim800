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
 * 27 июля
 * вернул как было
*****************************************************************************/ 

/*
+=============================================================================+
| includes
+=============================================================================+
*/

#include <stdarg.h>     // (...) parameter handling
#include <stdlib.h>     //NULL pointer definition
#include <libopencm3/stm32/usart.h>
#include "rprintf.h"
/*
+=============================================================================+
| defines
+=============================================================================+
*/

#define UART USART1
 
/*
+=============================================================================+
| local declarations
+=============================================================================+
*/


static void putc_strg(char);          // the put() function for sprintf()
char *SPRINTF_buffer;          
int scanf_buff_size = 64;	//chang this if needed
static int vfprintf_(void (*)(uint32_t, char), const char *format, va_list arg); //generic print
static void long_itoa (long, int, int, void (*) (uint32_t, char)); //heavily used by printf
static int rsscanf(const char* str, const char* format, va_list ap);//used by rscanf
/*
+=============================================================================+
| sample main()  file
+=============================================================================+
*/
/*
int main(void)
{
	char my_buff[12];
    int i;
	
		Usart1Init();
		printf_(" USART1 Test\r\n");

	    printf_("Enter your family name: ");
	    rscanf ("%s",my_buff);
	    printf_("%s\r\n",my_buff);
	    printf_("Enter your age: ");
	    rscanf ("%d",&i);
	    printf_("%d\r\n",i);
	    printf_("Mr. %s, %d years old.\r\n",my_buff,i);
	    printf_ ("Enter a hexadecimal number:\r\n");
	    rscanf("%x",&i);
	    printf_("You have entered %x (%d).\r\n",i,i);
while(1);
return 0;
}


*/


/*
+=============================================================================+
| global functions
+=============================================================================+
*/
int printf_(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vfprintf_((&usart_send_blocking), format, arg);
	va_end(arg);

	return 0;
}

int sprintf_(char *buffer, const char *format, ...)
{
	va_list arg;

	SPRINTF_buffer=buffer;	 //Pointer auf einen String in Speicherzelle abspeichern

	va_start(arg, format);
	vfprintf_((&putc_strg), format, arg);
	va_end(arg);

	*SPRINTF_buffer ='\0';             // append end of string

	return 0;
}


//Reads data from usart1  and stores them according to parameter format
//into the locations given by the additional arguments, as if
// scanf was used

// Reduced version of scanf (%d, %x, %c, %n are supported)
// %d dec integer (E.g.: 12)
// %x hex integer (E.g.: 0xa0)
// %b bin integer (E.g.: b1010100010)
// %n hex, de or bin integer (e.g: 12, 0xa0, b1010100010)
// %c any character
//buffer support 12 bytes

int rscanf(const char* format, ...){
	va_list args;
	va_start( args, format );
	int count = 0;
	char ch = 0;
	char buffer[scanf_buff_size];

	SPRINTF_buffer = buffer;


	while(count <= scanf_buff_size )//get string
	{
	count++;
	ch = usart_recv_blocking(UART);
	if(ch != '\n' && ch != '\r') *SPRINTF_buffer++ = ch;
	else
	break;
	}
	*SPRINTF_buffer = '\0';//end of string

	SPRINTF_buffer = buffer;
	count =  rsscanf(SPRINTF_buffer, format, args);
	va_end(args);
	return count;

}

/**
 * @def debug(format...)
 * @brief prints the timestamp, file name, line number, printf-formated @a format string and the
 * optional parameters to stdout
 *
 * The output looks like this:<br>
 * <pre>
 * 12345     filename.c[123]: format string
 * ^    ^    ^          ^
 * |    |    |          line number
 * |    |    +--------- file name
 * |    +-------------- tab character
 * +------------------- timestamp (ms since reset)
 * </pre>
 *
 * */
//#define USE_DEBUG
#ifdef USE_DEBUG
	#define debug(format,...) {\
		printf_("%ul\t%s[%i]: ", millisec, __FILE__, __LINE__); /* print file name and line number */\
		printf_(format, ## __VA_ARGS__);               /* print format string and args */\
		printf_("\n"); \
	}
#else
	#define debug(format,...) ((void)0)
#endif /* USE_DEBUG */



/*
+=============================================================================+
| local functions
+=============================================================================+
*/
// putc_strg() is the putc()function for sprintf_()
static void putc_strg(char character)
{
	*SPRINTF_buffer = (char)character;	// just add the character to buffer
	 SPRINTF_buffer++;

}

/*--------------------------------------------------------------------------------+
 * vfprintf_()
 * Prints a string to stream. Supports %s, %c, %d, %ld %ul %02d %i %x  %lud  and %%
 *     - partly supported: long long, float (%l %f, %F, %2.2f)
 *     - not supported: double float and exponent (%e %g %p %o \t)
 *--------------------------------------------------------------------------------+
*/
static int vfprintf_(void (*putc)(uint32_t, char), const char* str,  va_list arp)
{
	int d, r, w, s, l;  //d=char, r = radix, w = width, s=zeros, l=long
	char *c;            // for the while loop only
	//const char* p;
#ifdef INCLUDE_FLOAT
	float f;
	long int m, w2;
#endif



	while ((d = *str++) != 0) {
		if (d != '%') {//if it is not format qualifier
			(*putc)(UART, d);
			continue;//get out of while loop
		}
		d = *str++;//if it is '%'get next char
		w = r = s = l = 0;
		if (d == '%') {//if it is % print silmpy %
			(*putc)(UART, d);
			d = *str++;
		}
		if (d == '0') {
			d = *str++; s = 1;  //padd with zeros
		}
		while ((d >= '0')&&(d <= '9')) {
			w += w * 10 + (d - '0');
			d = *str++;
		}
		if (s) w = -w;      //padd with zeros if negative

	#ifdef INCLUDE_FLOAT
		w2 = 0;
		if (d == '.')
			d = *str++;
		while ((d >= '0')&&(d <= '9')) {
			w2 += w2 * 10 + (d - '0');
			d = *str++;
		}
	#endif

		if (d == 's') {// if string
			c = va_arg(arp, char*); //get buffer addres
			//p = c;//debug
			while (*c)
				(*putc)(UART, *(c++));//write the buffer out
			continue;
		}


		//debug

			//while(*p) Usart1Put(*p++);

		if (d == 'c') {
			(*putc)(UART, (char)va_arg(arp, int));
			continue;
		}
		if (d == 'u') {     // %ul
			r = 10;
			d = *str++;
		}
		if (d == 'l') {     // long =32bit
			l = 1;
			if (r==0) r = -10;
			d = *str++;
		}
//		if (!d) break;
		if (d == 'u') r = 10;//     %lu,    %llu
		else if (d == 'd' || d == 'i') {if (r==0) r = -10;}  //can be 16 or 32bit int
		else if (d == 'X' || d == 'x') r = 16;               // 'x' added by mthomas
		else if (d == 'b') r = 2;
		else str--;                                         // normal character

	#ifdef INCLUDE_FLOAT
		if (d == 'f' || d == 'F') {
			f=va_arg(arp, double);
			if (f>0) {
				r=10;
				m=(int)f;
			}
			else {
				r=-10;
				f=-f;
				m=(int)(f);
			}
			long_itoa(m, r, w, (putc));
			f=f-m; m=f*(10^w2); w2=-w2;
			long_itoa(m, r, w2, (putc));
			l=3; //do not continue with long
		}
	#endif

		if (!r) continue;  //
		if (l==0) {
			if (r > 0){      //unsigned
				long_itoa((unsigned long)va_arg(arp, int), r, w, (putc)); //needed for 16bit int, no harm to 32bit int
			}
			else            //signed
				long_itoa((long)va_arg(arp, int), r, w, (putc));
		} else if (l==1){  // long =32bit
				long_itoa((long)va_arg(arp, long), r, w, (putc));        //no matter if signed or unsigned
		}
	}

	return 0;
}


static void long_itoa (long val, int radix, int len, void (*putc) (uint32_t, char))
{
	char c, sgn = 0, pad = ' ';
	char s[20];
	int  i = 0;


	if (radix < 0) {
		radix = -radix;
		if (val < 0) {
			val = -val;
			sgn = '-';
		}
	}
	if (len < 0) {
		len = -len;
		pad = '0';
	}
	if (len > 20) return;
	do {
		c = (char)((unsigned long)val % radix); //cast!
		if (c >= 10) c += ('A'-10); //ABCDEF
		else c += '0';            //0123456789
		s[i++] = c;
		val = (unsigned long)val /radix; //cast!
	} while (val);
	if (sgn) s[i++] = sgn;
	while (i < len)
		s[i++] = pad;
	do
		(*putc)(UART, s[--i]);
	while (i);
}





//
// Reduced version of sscanf (%d, %x, %c, %n are supported)
// %d dec integer (E.g.: 12)
// %x hex integer (E.g.: 0xa0)
// %b bin integer (E.g.: b1010100010)
// %n hex, de or bin integer (e.g: 12, 0xa0, b1010100010)
// %c any character
//

static int rsscanf(const char* str, const char* format, va_list ap)
{
	//va_list ap;
	int value, tmp;
	int count;
	int pos;
	char neg, fmt_code;
	const char* pf;
	char* sval;
	//va_start(ap, format);

	for (pf = format, count = 0; *format != 0 && *str != 0; format++, str++)
	{
		while (*format == ' ' && *format != 0) format++;//
		if (*format == 0) break;

		while (*str == ' ' && *str != 0) str++;//increment pointer of input string
		if (*str == 0) break;

		if (*format == '%')//recognize how to format
		{
			format++;
			if (*format == 'n')
			{
                if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))//if in str sth like 0xff
                {
                    fmt_code = 'x';
                    str += 2;
                }
                else
                if (str[0] == 'b')
                {
                    fmt_code = 'b';
                    str++;
                }
                else
                    fmt_code = 'd';
			}
			else
				fmt_code = *format; //it is format letter

			switch (fmt_code)
			{
			case 'x':
			case 'X':
				for (value = 0, pos = 0; *str != 0; str++, pos++)
				{
					if ('0' <= *str && *str <= '9')
						tmp = *str - '0';
					else
					if ('a' <= *str && *str <= 'f')
						tmp = *str - 'a' + 10;
					else
					if ('A' <= *str && *str <= 'F')
						tmp = *str - 'A' + 10;
					else
						break;

					value *= 16;
					value += tmp;
				}
				if (pos == 0)
					return count;
				*(va_arg(ap, int*)) = value;
				count++;
				break;

            case 'b':
				for (value = 0, pos = 0; *str != 0; str++, pos++)
				{
					if (*str != '0' && *str != '1')
                        break;
					value *= 2;
					value += *str - '0';
				}
				if (pos == 0)
					return count;
				*(va_arg(ap, int*)) = value;
				count++;
				break;

			case 'd':
				if (*str == '-')
				{
					neg = 1;
					str++;
				}
				else
					neg = 0;
				for (value = 0, pos = 0; *str != 0; str++, pos++)
				{
					if ('0' <= *str && *str <= '9')
						value = value*10 + (int)(*str - '0');
					else
						break;
				}
				if (pos == 0)
					return count;
				*(va_arg(ap, int*)) = neg ? -value : value;
				count++;
				break;

			case 'c':
				*(va_arg(ap, char*)) = *str;
				count++;
				break;
			case 's':
				sval = va_arg(ap, char*);


				while(*str){
				 *sval++ = *str++;
				count++;
				}
				*sval = NULL;

				break;

			default:
				return count;
			}
		}
		else
		{
			if (*format != *str)//
				break;
		}
	}



	return count;
}
/******************************************************************************
* END OF FILE
******************************************************************************/
