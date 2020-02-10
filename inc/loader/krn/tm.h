/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Terminal Manager includes
 *
 */

#ifndef __LOADER_KRN_TM_H__
#define __LOADER_KRN_TM_H__

#define TM_FG_BLACK 		0x00
#define TM_FG_BLUE		0x01
#define TM_FG_GREEN		0x02
#define TM_FG_CYAN		0x03
#define TM_FG_RED		0x04
#define TM_FG_MAGENTA 		0x05
#define TM_FG_BROWN		0x06
#define TM_FG_WHITE		0x07
#define TM_FG_GRAY		0x08
#define TM_FG_BRIGHT_BLUE	0x09
#define TM_FG_BRIGHT_GREEN	0x0A
#define TM_FG_BRIGHT_CYAN	0x0B
#define TM_FG_BRIGHT_RED	0x0C
#define TM_FG_BRIGHT_MAGENTA	0x0D
#define TM_FG_YELLOW		0x0E
#define TM_FG_BRIGHT_WHITE	0x0F
#define TM_BG_BLACK		0x00
#define TM_BG_BLUE		0x10
#define TM_BG_GREEN		0x20
#define TM_BG_CYAN		0x30
#define TM_BG_RED		0x40
#define TM_BG_MAGENTA 		0x50
#define TM_BG_BROWN		0x60
#define TM_BG_WHITE		0x70

#define TM_BLINK		0x80

typedef unsigned int TMID;

int tm_clear(TMID ht);

void tm_putchar(TMID ht,int c);
void tm_putstring(TMID ht,const char*);

void tm_scrollup(TMID ht);

#endif

