/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Terminal Manager
 */

#include "loader/module.h"
#include "loader/krn/ios.h"
#include "loader/krn/tm.h"

#include "loader/utl/string.h"

#define NPAR 16

DECLARE_BUILTIN_MODULE("term",tm_init,50000);

struct term_t;

static void tm_do_sgr(struct term_t*);

static void tm_state_normal(struct term_t*,int);
static void tm_state_csi(struct term_t*,int);
static void tm_state_esc(struct term_t*,int);

struct term_t {
	char* addr;
	short px;
	short py;
	short mx;
	short my;
	char attr;
	void (*state)(struct term_t*,int);
	unsigned int par[NPAR];
	unsigned int npar;
};

struct term_t tm_desc[]={
	{ (char*)0x000b8000, 0, 0, 80, 25 , TM_FG_WHITE | TM_BG_BLACK, tm_state_normal}
};

#define TM_LAST	(sizeof(tm_desc)/sizeof(struct term_t))

/*
 * Initialisation
 */
void tm_init() {
	tm_clear(0);
}

int tm_clear(TMID ht) {
	struct term_t* t;
	size_t len;
	size_t i;

	if (ht>TM_LAST) {
		return -1;
	}		

	t = &tm_desc[ht];
	len = 2 * t->mx * t->my;

	for (i = 0;i < len ; i++) {
		t->addr[i] = (i%2 == 1 ? TM_FG_WHITE | TM_BG_BLACK : 0x00);
	}

	return 0;
}

void tm_scrollup(TMID ht) {
	struct term_t* t;
	size_t l1;

	if (ht>TM_LAST) {
		return;
	}		

	t = &tm_desc[ht];
	l1 = 2 * (t->mx) * (t->my-1);

	memmove(t->addr, t->addr + 2 * t->mx, l1);
	memset(t->addr + 2* t->mx * t->my - 2 * t->mx, 0, 2 * t->mx);
}

void tm_putchar(TMID ht,int c) {
	struct term_t* t;
	size_t i;

	if (ht>TM_LAST) {
		return;
	}		
	
	t = &tm_desc[ht];

	t->state(t,c);

	while (t->py >= t->my) {
		tm_scrollup(ht);
		t->py --;
	}
}

void tm_putstring(TMID ht,const char* src) {
	const char *p = src;
	while (*p != 0) {
		tm_putchar(ht,*p++);
	}
}

/*
 * Process normal characters
 */
static void tm_state_normal(struct term_t* t,int c) {
	size_t i;

        switch(c) {
        case '\r':
                t->px = 0;
                break;
        case '\n':
                t->px = 0;
                t->py++;
                break;
	case 27:
		t->state = tm_state_esc;
		break;
        default:
                i = 2 * (t->py * t->mx + t->px);
                t->addr[i+1] = t->attr;
                t->addr[i+0] = c;

                t->px++;
                if (t->px >= t->mx) {
                        t->px = 0;
                        t->py++;
                }
        }
}

/*
 * Process ESC sequence
 */
static void tm_state_esc(struct term_t* t,int c) {
	int i = 0;

	switch(c) {
		case '[':
			t->state = tm_state_csi;
			for ( i = 0 ; i < NPAR; i++) t->par[i] = 0;
			t->npar = 0;
			break;
		default:
			t->state = tm_state_normal;
			t->state(t,c);
	}
}

/*
 * Process CSI sequence
 */
static void tm_state_csi(struct term_t* t,int c) {
	switch(c) {
	case 'm':
		t->npar++;
		tm_do_sgr(t);
		t->state = tm_state_normal;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		t->par[t->npar] *= 10;
		t->par[t->npar] += (c - '0');
		break;
	case ';':
		t->npar ++;
		if (t->npar < NPAR) break;
	default:
		t->state = tm_state_normal;
		t->state(t,c);
	}
}

/*
 * Process CSI SGR
 */
static void tm_do_sgr(struct term_t* t) {
	int i;

	for (i = 0; i < t->npar ; i ++) {
		switch(t->par[i]) {
		case 0:
			t->attr = TM_FG_WHITE | TM_BG_BLACK;
			break;
		case 4:
			t->attr |= 0x08;
			break;
		case 5:
			t->attr |= 0x80;
			break;
		case 24:
			t->attr &= 0xF7;
			break;
		case 25:
			t->attr &= 0x7F;
			break;
		case 30:
			t->attr = (t->attr & 0xF8) | TM_FG_BLACK; 
			break;
		case 31:
			t->attr = (t->attr & 0xF8) | TM_FG_RED; 
			break;
		case 32:
			t->attr = (t->attr & 0xF8) | TM_FG_GREEN; 
			break;
		case 33:
			t->attr = (t->attr & 0xF8) | TM_FG_BROWN; 
			break;
		case 34:
			t->attr = (t->attr & 0xF8) | TM_FG_BLUE; 
			break;
		case 35:
			t->attr = (t->attr & 0xF8) | TM_FG_MAGENTA; 
			break;
		case 36:
			t->attr = (t->attr & 0xF8) | TM_FG_CYAN; 
			break;
		case 37:
			t->attr = (t->attr & 0xF8) | TM_FG_WHITE; 
			break;
		case 38:
			t->attr = (t->attr & 0xF0) | TM_FG_BRIGHT_WHITE; 
			break;
		case 39:
			t->attr = (t->attr & 0xF0) | TM_FG_WHITE; 
			break;
		case 40:
		case 49:
			t->attr = (t->attr & 0x8F) | TM_BG_BLACK;
			break;
		case 41:
			t->attr = (t->attr & 0x8F) | TM_BG_RED;
			break;
		case 42:
			t->attr = (t->attr & 0x8F) | TM_BG_GREEN;
			break;
		case 43:
			t->attr = (t->attr & 0x8F) | TM_BG_BROWN;
			break;
		case 44:
			t->attr = (t->attr & 0x8F) | TM_BG_BLUE;
			break;
		case 45:
			t->attr = (t->attr & 0x8F) | TM_BG_MAGENTA;
			break;
		case 46:
			t->attr = (t->attr & 0x8F) | TM_BG_CYAN;
			break;
		case 47:
			t->attr = (t->attr & 0x8F) | TM_BG_WHITE;
			break;
		}
	}
}
