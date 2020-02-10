/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * simplified printf
 *
 * - does not handle positional parameter for width and precision
 * - does not handle float data type
 * - does not handle 64 bits data type
 * - does not handle wchar
 * - does not handle %c
 * - does not handle %p
 */

#include "kernel/types.h"
#include "kernel/stdarg.h"

#include "kernel/utl/printf.h"
#include "kernel/utl/string.h"

/*
 * Val
 */
union val_t {
	uint64_t u64;
	uint32_t u32;
	uint16_t u16;
	uint8_t u8;
	int64_t i64;
	int32_t i32;
	int16_t i16;
	int8_t i8;
	const char* s;
};

/*
 * Fmt specifier
 */
struct fmt_t {
	uint64_t width;
	uint64_t precision;
	uint16_t size;
	uint32_t base;
	uint8_t sign;
	uint8_t flags;
};

static const char* itoa(uint64_t val,char* buffer,size_t len,size_t* plen,uint32_t base);

#define MAX_WIDTH	4000000000UL
#define MAX_PRECISION	4000000000UL

#define FLAG_ZERO       0x01
#define FLAG_LEFT       0x02
#define FLAG_ALT        0x04
#define FLAG_SPACE      0x08
#define FLAG_PLUS       0x10
/*
 * Char classes
 */
#define CC_DEFAULT	0
#define CC_SPACE 	1
#define CC_NUMBER	2
#define CC_PERCENT	3
#define CC_ASTERISK	4
#define CC_DOLLAR	5
#define CC_PLUS		6
#define CC_MINUS	7
#define CC_DOT		8
#define CC_ZERO		9
#define CC_DIGIT	10
#define CC_LONG		11
#define CC_HALF		12
#define CC_SIGNED	13
#define CC_UNSIGNED	14
#define CC_STRING	15
#define CC_POINTER	16


#define BGN_CHARMAP(c) static uint8_t c[] = {
#define CHARMAP(c,s) [ c - ' '] = s,
#define END_CHARMAP(c) };

#define BGN_DECL_STATE() typedef enum {
#define STATE(state) STATE_##state,
#define END_DECL_STATE() } state_t;

#define BGN_JUMP_TABLE(state) static state_t JT_##state[] = {
#define END_JUMP_TABLE() };
 
#define BGN_DECL_LABEL(x) static void* x[] = {
#define DECLARE_LABEL(state) [ STATE_##state ] = &&LBL_##state,
#define END_DECL_LABEL() };

#define LABEL(state) LBL_##state:

#define SET_STATE(v,state) v = JT_##state
/*
 * Tab to map character to character class
 */
BGN_CHARMAP(cclass)
	CHARMAP(' ',CC_SPACE)
	CHARMAP('%',CC_PERCENT)
	CHARMAP('0',CC_ZERO)
	CHARMAP('1',CC_DIGIT)
	CHARMAP('2',CC_DIGIT)
	CHARMAP('3',CC_DIGIT)
	CHARMAP('4',CC_DIGIT)
	CHARMAP('5',CC_DIGIT)
	CHARMAP('6',CC_DIGIT)
	CHARMAP('7',CC_DIGIT)
	CHARMAP('8',CC_DIGIT)
	CHARMAP('9',CC_DIGIT)
	CHARMAP('d',CC_SIGNED)
	CHARMAP('i',CC_SIGNED)
	CHARMAP('u',CC_UNSIGNED)
	CHARMAP('x',CC_UNSIGNED)
	CHARMAP('X',CC_UNSIGNED)
	CHARMAP('o',CC_UNSIGNED)
	CHARMAP('p',CC_POINTER)
	CHARMAP('s',CC_STRING)
	CHARMAP('h',CC_HALF)
	CHARMAP('H',CC_HALF)
	CHARMAP('l',CC_LONG)
	CHARMAP('L',CC_LONG)
	CHARMAP('+',CC_PLUS)
	CHARMAP('-',CC_MINUS)
	CHARMAP('#',CC_NUMBER)
	CHARMAP('.',CC_DOT)
	CHARMAP('*',CC_ASTERISK)
END_CHARMAP()

/*
 * Convert char to char class
 */
static inline uint8_t get_class(char c) {
	if (c < 0 || c < ' ' || (c - ' ') > sizeof(cclass)/sizeof(uint8_t)) {
		return CC_DEFAULT;
	}
	return cclass[c - ' '];
}

/*
 * States
 */
BGN_DECL_STATE()
	STATE(normal)
	STATE(fmtini)
	STATE(percent)
	STATE(width)
	STATE(dot)
	STATE(prec)
	STATE(pwidth)
	STATE(pprec)
	STATE(flags)
	STATE(m_half)
	STATE(m_long)
	STATE(fmt_u)
	STATE(fmt_d)
	STATE(fmt_s)
END_DECL_STATE()

/*
 * Normal State
 */
BGN_JUMP_TABLE(normal)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(fmtini)	/* CC_PERCENT */
	STATE(normal)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(normal)	/* CC_DOT */
	STATE(normal)	/* CC_ZERO */
	STATE(normal)	/* CC_DIGIT */
	STATE(normal)	/* CC_LONG */
	STATE(normal)	/* CC_HALF */
	STATE(normal)  	/* CC_SIGNED */
	STATE(normal)	/* CC_UNSIGNED */
	STATE(normal)   /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Format specifier init
 */
BGN_JUMP_TABLE(fmtini)
	STATE(normal)	/* CC_DEFAULT */
	STATE(flags)	/* CC_SPACE */
	STATE(flags)	/* CC_NUMBER */
	STATE(percent)	/* CC_PERCENT */
	STATE(pwidth)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(flags)	/* CC_PLUS */
	STATE(flags)	/* CC_MINUS */
	STATE(dot)	/* CC_DOT */
	STATE(flags)	/* CC_ZERO */
	STATE(width)	/* CC_DIGIT */
	STATE(m_long)	/* CC_LONG */
	STATE(m_half)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Flags
 */
BGN_JUMP_TABLE(flags)
	STATE(normal)	/* CC_DEFAULT */
	STATE(flags)	/* CC_SPACE */
	STATE(flags)	/* CC_NUMBER */
	STATE(percent)	/* CC_PERCENT */
	STATE(pwidth)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(flags)	/* CC_PLUS */
	STATE(flags)	/* CC_MINUS */
	STATE(prec)	/* CC_DOT */
	STATE(flags)	/* CC_ZERO */
	STATE(width)	/* CC_DIGIT */
	STATE(m_long)	/* CC_LONG */
	STATE(m_half)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Width
 */
BGN_JUMP_TABLE(width)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(normal)	/* CC_PERCENT */
	STATE(normal)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(dot)	/* CC_DOT */
	STATE(width)	/* CC_ZERO */
	STATE(width)	/* CC_DIGIT */
	STATE(m_long)	/* CC_LONG */
	STATE(m_half)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Handle parametrized witdh
 */
BGN_JUMP_TABLE(pwidth)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(normal)	/* CC_PERCENT */
	STATE(normal)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(dot)	/* CC_DOT */
	STATE(normal)	/* CC_ZERO */
	STATE(normal)	/* CC_DIGIT */
	STATE(m_long)	/* CC_LONG */
	STATE(m_half)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Precision trigger (.)
 */
BGN_JUMP_TABLE(dot)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(normal)	/* CC_PERCENT */
	STATE(pprec)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(normal)	/* CC_DOT */
	STATE(prec)	/* CC_ZERO */
	STATE(prec)	/* CC_DIGIT */
	STATE(m_long)	/* CC_LONG */
	STATE(m_half)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Precision
 */
BGN_JUMP_TABLE(prec)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(normal)	/* CC_PERCENT */
	STATE(normal)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(normal)	/* CC_DOT */
	STATE(prec)	/* CC_ZERO */
	STATE(prec)	/* CC_DIGIT */
	STATE(m_long)	/* CC_LONG */
	STATE(m_half)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Parametrized Precision
 */
BGN_JUMP_TABLE(pprec)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(normal)	/* CC_PERCENT */
	STATE(normal)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(normal)	/* CC_DOT */
	STATE(normal)	/* CC_ZERO */
	STATE(normal)	/* CC_DIGIT */
	STATE(m_long)	/* CC_LONG */
	STATE(m_half)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Half modifier
 */
BGN_JUMP_TABLE(m_half)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(normal)	/* CC_PERCENT */
	STATE(normal)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(normal)	/* CC_DOT */
	STATE(normal)	/* CC_ZERO */
	STATE(normal)	/* CC_DIGIT */
	STATE(normal)	/* CC_LONG */
	STATE(normal)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Long modifier
 */
BGN_JUMP_TABLE(m_long)
	STATE(normal)	/* CC_DEFAULT */
	STATE(normal)	/* CC_SPACE */
	STATE(normal)	/* CC_NUMBER */
	STATE(normal)	/* CC_PERCENT */
	STATE(normal)	/* CC_ASTERISK */
	STATE(normal)	/* CC_DOLLAR */
	STATE(normal)	/* CC_PLUS */
	STATE(normal)	/* CC_MINUS */
	STATE(normal)	/* CC_DOT */
	STATE(normal)	/* CC_ZERO */
	STATE(normal)	/* CC_DIGIT */
	STATE(normal)	/* CC_LONG */
	STATE(normal)	/* CC_HALF */
	STATE(fmt_d)    /* CC_SIGNED */
	STATE(fmt_u)    /* CC_UNSIGNED */
	STATE(fmt_s)    /* CC_STRING */
	STATE(normal)   /* CC_POINTER */
END_JUMP_TABLE()

/*
 * Unsigned to alpha conversions
 */
static const char* itoa(uint64_t val,char* buffer,size_t len,size_t* plen,uint32_t base) {
	static const char* digit = "0123456789abcdef";

	char *p = 0;

	p = buffer + len;
	*--p=0;
	*plen = 0;

	if (val == 0) {
		(*plen)++;
		*--p = '0';
	}

	while (val != 0 && p != buffer) {
		(*plen)++;
		*--p = digit[val % base];
		val = val / base;
	}	

	return val == 0 ? p : 0;
}
	

int64_t kvprinto(struct out_t* out,const char* format,va_list ap) {
	BGN_DECL_LABEL(jumps)
		DECLARE_LABEL(normal)
		DECLARE_LABEL(fmtini)
		DECLARE_LABEL(flags)
		DECLARE_LABEL(percent)
		DECLARE_LABEL(width)
		DECLARE_LABEL(pwidth)
		DECLARE_LABEL(dot)
		DECLARE_LABEL(prec)
		DECLARE_LABEL(pprec)
		DECLARE_LABEL(m_long)
		DECLARE_LABEL(m_half)
		DECLARE_LABEL(fmt_u)
		DECLARE_LABEL(fmt_d)
		DECLARE_LABEL(fmt_s)
	END_DECL_LABEL()

	int64_t cr = 0;

	char tmp[64];

	struct fmt_t fmt;
	union val_t val;

	const char* v;

	uint8_t sigchr;
	uint64_t padlen;
	uint64_t siglen;
	uint64_t len;

	state_t* jt;

	auto void out_str(const char* p,size_t s) {
		for (size_t i = 0; i < s; i++) {
			out->output(p[i]);
		}
	}

	auto void out_pad(int c, size_t s) {
		for (size_t i = 0; i < s; i++) {
			out->output(c);
		}
	}

	auto void out_chr(int c) {
		out->output(c);
	}

	SET_STATE(jt,normal);

	for (const char* p = format; *p ; p++) {
		uint8_t cc = get_class(*p);

		goto *jumps[jt[cc]];

	/*
	 * Start of a format specifier
	 */
	LABEL(fmtini)
		SET_STATE(jt,fmtini);

		memset(&fmt,0,sizeof(fmt));
		fmt.size = 32;
		fmt.base = 10;

		continue;
	/*
	 * Hangle flags
	 */
	LABEL(flags)
		SET_STATE(jt,flags);

		if (*p == '0') {
			fmt.flags |= FLAG_ZERO;
		} else if (*p == '+') {
			fmt.flags |= FLAG_PLUS;
		} else if (*p == ' ') {
			fmt.flags |= FLAG_SPACE;
		} else if (*p == '-') {
			fmt.flags |= FLAG_LEFT;
		} else if (*p == '#') {
			fmt.flags |= FLAG_ALT;
		}
		continue;
	/*
	 * Handle width
	 */
	LABEL(width)	
		SET_STATE(jt,width);

		fmt.width *= 10;
		fmt.width += (*p - '0'); 

		if (fmt.width > MAX_WIDTH) goto failure;
		continue;
	/*
	 * Handle parametrized width
	 */
	LABEL(pwidth)
		SET_STATE(jt,pwidth);
		
		val.u64 = va_arg(ap,uint64_t);
		if (val.i64 < 0) {
			val.i64 = - val.i64;
			fmt.flags |= FLAG_LEFT;
		}
		fmt.width = val.u64;
		continue;
	/*
	 * Handle dot
	 */
	LABEL(dot)
		SET_STATE(jt,dot);
		continue;
	/*
	 * Handle parametrized precision
	 */
	LABEL(pprec)
		SET_STATE(jt,pprec);
		
		val.u64 = va_arg(ap,uint64_t);
		if (val.i64 > 0) {
			fmt.precision = val.u64;
		}
		continue;
	/*
	 * Handle precision
	 */
	LABEL(prec)
		SET_STATE(jt,prec);

		fmt.precision *= 10;
		fmt.precision += (*p - '0');

		if (fmt.precision > MAX_PRECISION) goto failure;
		continue;

	/*
	 * Handle half
	 */
	LABEL(m_half)
		SET_STATE(jt,m_half);
		fmt.size>>=1;
		continue;
	/*
	 * Handle long
	 */
	LABEL(m_long)
		SET_STATE(jt,m_long);
		fmt.size<<=1;
		continue;

	/*
 	 * Handle %%
 	 */
	LABEL(percent)
		SET_STATE(jt,normal);

	/*
	 * Normal characters
	 */
	LABEL(normal)
		out_chr(*p);
		continue;

	/*
	 * Handle signed format
	 */
	LABEL(fmt_d)
		padlen = 0;
		val.u64 = 0;

                if ((fmt.flags & FLAG_LEFT) == FLAG_LEFT) {
                        fmt.flags &= (~FLAG_ZERO);
                }

		if (fmt.size == 64) {
			val.u64 = va_arg(ap,uint64_t);
			if (val.i64 < 0) {
				fmt.sign = 1;
				val.i64 = - val.i64;
			}
		} else if (fmt.size == 16) {
			val.u16 = va_arg(ap,uint64_t);
			if (val.i16 < 0) {
				fmt.sign = 1;
				val.i16 = - val.i16;
			}
		} else if (fmt.size == 8) {
			val.u8 = va_arg(ap,uint64_t);
			if (val.i8 < 0) {
				fmt.sign = 1;
				val.i8 = - val.i8;
			}
		} else {
			val.u32 = va_arg(ap,uint64_t);
			if (val.i32 < 0) {
				fmt.sign = 1;
				val.i32 = - val.i32;
			}
		}

		if (fmt.sign == 1) {
			siglen = 1;
			sigchr = '-';
		} else {
			if ((fmt.flags & FLAG_PLUS) == FLAG_PLUS) {
				siglen = 1;
				sigchr = '+';
			} else if ((fmt.flags & FLAG_SPACE) == FLAG_SPACE) {
				siglen = 1;
				sigchr = ' ';
			} else {
				siglen = 0;
			}
		}

		v = itoa(val.u64,tmp,sizeof(tmp),&len,10);
		if (v == 0) goto failure;

		if ((fmt.flags & FLAG_LEFT) == FLAG_LEFT) {
			if (siglen > 0) out_chr(sigchr);
			if (fmt.precision > len) {
				padlen = fmt.precision - len;
				out_pad('0',padlen);
			}
			out_str(v,len);

			if (fmt.width > len + siglen + padlen) {
				out_pad(' ',fmt.width - len - siglen - padlen);
			}
		} else {
			if (fmt.precision > 0) {
				if (fmt.precision > len) padlen = fmt.precision - len;

				// Extra padding
				if (fmt.width > len + padlen + siglen) {
					out_pad(' ',fmt.width - len - padlen - siglen);
				}

				if (siglen > 0) out_chr(sigchr);
				if (padlen > 0) out_pad('0',padlen);
			} else {
				if (fmt.width > len + siglen) {
					padlen = fmt.width - len - siglen;
				}

				if ((fmt.flags & FLAG_ZERO) == FLAG_ZERO) {
					if (siglen > 0) out_chr(sigchr);
					if (padlen > 0) out_pad('0',padlen);
				} else {
					if (padlen > 0) out_pad(' ', padlen);
					if (siglen > 0) out_chr(sigchr);
				}
			}
			out_str(v,len);
		}

		SET_STATE(jt,normal);
		continue;
	/*
	 * Handle unsigned format
	 */
	LABEL(fmt_u)
		padlen = 0;

		val.u64 = 0;
		if (fmt.size == 64) {
			val.u64 = va_arg(ap,uint64_t);
		} else if (fmt.size == 16) {
			val.u16 = va_arg(ap,uint64_t) & 0xFFFF;
		} else if (fmt.size == 8) {
			val.u8 = va_arg(ap,uint64_t) & 0xFF;
		} else {
			val.u32 = (uint32_t)va_arg(ap,uint64_t);
		}

		if (*p == 'x' || *p == 'X') {
			fmt.base = 16;
		} else if (*p == 'o') {
			fmt.base = 8;
		}

		v = itoa(val.u64,tmp,sizeof(tmp),&len,fmt.base);
		if (v == 0) goto failure;

		if ((fmt.flags & FLAG_LEFT) == FLAG_LEFT) {
			if (fmt.precision > len) {
				padlen = fmt.precision - len;
				out_pad('0', padlen);
			}
			out_str(v,len);
			if (fmt.width > len + padlen) {
				out_pad(' ',fmt.width - len - padlen);
			}
		} else {
			if (fmt.precision > 0) {
				if (fmt.precision > len) padlen = fmt.precision - len;
				if (fmt.width > len + padlen) {
					out_pad(' ', fmt.width - len - padlen);
				}
				if (padlen > 0) out_pad('0', padlen);
			} else {
				if (fmt.width > len) padlen = fmt.width - len;
				if ((fmt.flags & FLAG_ZERO) == FLAG_ZERO && fmt.width > len) {
					sigchr = '0';
				} else if (fmt.width > len) {
					sigchr = ' ';
				}
				if (padlen) out_pad(sigchr,padlen);
			}
			out_str(v,len);
		}

		SET_STATE(jt,normal);
		continue;
	/*
	 * Handle string format
	 */
	LABEL(fmt_s)
		padlen = 0;
	
		val.s = va_arg(ap,const char*);
		len = strlen(val.s);

		if (fmt.precision > 0 && len > fmt.precision) {
			len = fmt.precision;
		}

		if (fmt.width > len) padlen = fmt.width - len;
		
		if ((fmt.flags & FLAG_LEFT) == FLAG_LEFT) {
			out_str(val.s,len);
			out_pad(' ',padlen);
		} else {
			out_pad(' ',padlen);
			out_str(val.s,len);
		}
		SET_STATE(jt,normal);
		continue;
	}

	return cr;
failure:
	return -1;
}

