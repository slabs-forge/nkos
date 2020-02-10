/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * printf emulation
 */

#include "loader/utl/unistd.h"
#include "loader/utl/stdio.h"
#include "loader/utl/string.h"
#include "loader/utl/conv.h"

#define CHR_FLG		"#+ -0"
#define CHR_FMT 	"sxpudi%"	
#define CHR_MOD		"h"

#define STAT_INI	0
#define STAT_FLG	1
#define STAT_WID	2
#define STAT_PRE	3
#define STAT_MOD	4
#define STAT_FMT	5
#define STAT_END	6
#define STAT_ERR	7

#define FLAG_ZERO	0x01
#define FLAG_LEFT	0x02
#define FLAG_ALT	0x04
#define FLAG_SPACE	0x08
#define FLAG_PLUS	0x10

#define PFMT_WREAD	0xFFFFFFFF
#define PFMT_PREAD	0xFFFFFFFF

union val_t {
	unsigned int u32;
	unsigned short u16;
	unsigned char u8;
	int i32;
	short i16;
	char i8;
	const char* s;
};

struct fmt_t;
struct buf_t;

typedef int (*callback_t)(struct buf_t*,struct fmt_t*);

struct fmt_t {
	size_t w;	// Width
	size_t p;  	// Precision
	size_t s;	// Size
	size_t f;	// Flags
	callback_t m;	// Print method
	union val_t v;	// Value
};

struct buf_t {
	int fd;
	va_list ap;
	const char* s;
	const char* p;
	size_t c;	
};

struct file_t {
	int fd;
};

static struct file_t fstd[] = {
	{ .fd = 0 }
};

void* stdout = &(fstd[0]);

static void pad(struct buf_t*,int c, size_t len);

int print_d(struct buf_t*,struct fmt_t*);
int print_u(struct buf_t*,struct fmt_t*);
int print_x(struct buf_t*,struct fmt_t*);
int print_s(struct buf_t*,struct fmt_t*);

int read_fmt(struct buf_t* pbuf);
int read_fmt_ini(struct buf_t* pbuf,struct fmt_t* pfmt);
int read_fmt_flg(struct buf_t* pbuf,struct fmt_t* pfmt);
int read_fmt_wid(struct buf_t* pbuf,struct fmt_t* pfmt);
int read_fmt_pre(struct buf_t* pbuf,struct fmt_t* pfmt);
int read_fmt_mod(struct buf_t* pbuf,struct fmt_t* pfmt);
int read_fmt_fmt(struct buf_t* pbuf,struct fmt_t* pfmt);

callback_t g_cb[STAT_END] = {
	read_fmt_ini,
	read_fmt_flg,
	read_fmt_wid,
	read_fmt_pre,
	read_fmt_mod,
	read_fmt_fmt
};

/*
 * printf emulation
 */
void printf(const char* format,...) {
	va_list ap;

	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
}

/*
 * fprintf emulation
 */
void vfprintf(FILE* stream,const char* format,va_list ap) {
	struct buf_t buf;

	
	buf.fd =  ((struct file_t*)stream)->fd ;
	buf.ap = ap;
	buf.s = format;
	buf.p = format;
	buf.c = 0;

	while (*buf.p != 0) {
		if (*buf.p == '%') {
			write(buf.fd, buf.s, buf.c);
			buf.s = buf.p++;
			buf.c = 1;

			if (read_fmt(&buf) == STAT_ERR) {
				write(buf.fd, buf.s,buf.c);
			}
			buf.s = buf.p;
			buf.c = 0;
		} else {
			buf.p++;
			buf.c++;
		}
	}
	write(buf.fd, buf.s, buf.c);
}

/*
 * Read format spec
 */
int read_fmt(struct buf_t* pbuf) {
	struct fmt_t fmt;

	int s = STAT_INI;

	fmt.w = 0;
	fmt.p = 0;
	fmt.f = 0;
	fmt.m = 0;
	fmt.s = 32;

	while (*pbuf->p && s!=STAT_END && s != STAT_ERR) {
		if (s >= STAT_INI && s <= STAT_FMT) {
			s = g_cb[s](pbuf,&fmt);
		} else {
			pbuf->p++;
			pbuf->c++;
			s = STAT_ERR;
		}
	}
	if (s == STAT_END && fmt.m != 0) {
		fmt.m(pbuf,&fmt);
	}
	return s;
}

/*
 * fmt format
 */
int read_fmt_fmt(struct buf_t* pbuf,struct fmt_t* pfmt) {
	int s = STAT_FMT;

	switch(*pbuf->p) {
	case 's':
		pfmt->m = print_s;
		s = STAT_END;
		break;
	case 'u':
		pfmt->m = print_u;
		s = STAT_END;
		break;
	case 'd':
		pfmt->m = print_d;
		s = STAT_END;
		break;
	case 'x':
		pfmt->m = print_x;
		s = STAT_END;
		break;
	case 'p':
		pfmt->m = print_x;
		pfmt->f |= FLAG_ALT ;
		s = STAT_END;
		break;
	default:
		s = STAT_ERR;
	}

	if (s != STAT_ERR) {
		pbuf->p++;
		pbuf->c++;
	}

	return s;
}

/*
 * fmt modifier
 */
int read_fmt_mod(struct buf_t* pbuf,struct fmt_t* pfmt) {
	int s = STAT_MOD;
	
	switch(*pbuf->p) {
	case 'h':
		if (pfmt->s != 32 && pfmt->s != 16) {
			s = STAT_ERR;
		} else {
			pfmt->s >>= 1;
		}
		break;
	default:
		if (strchr(CHR_FMT, *pbuf->p) != 0) {
			s = STAT_FMT;
		} else {
			s = STAT_ERR;
		}
	}

	if (s == STAT_MOD) {
		pbuf->p++;
		pbuf->c++;
	}

	return s;
}

/*
 * fmt precision
 */
int read_fmt_pre(struct buf_t* pbuf,struct fmt_t* pfmt) {
	int s = STAT_PRE;

	if (*pbuf->p == '*') {
		if (pfmt->p != 0) {
			s = STAT_ERR;
		} else {
			pfmt->p = PFMT_PREAD;
			pbuf->p++;
			pbuf->c++;
		}
	} else if (*pbuf->p >= '0' && *pbuf->p <='9') {
		if (pfmt->p == PFMT_PREAD) {
			s = STAT_ERR;
		} else {
			pfmt->p *= 10;
			pfmt->p += (*pbuf->p - '0');
			pbuf->p++;
			pbuf->c++;
		}
	} else if (strchr(CHR_MOD, *pbuf->p) != 0) {
		s = STAT_MOD;
	} else if (strchr(CHR_FMT, *pbuf->p) != 0) {
		s = STAT_FMT;
	} else {
		s = STAT_ERR;
	}

	return s;
}

/*
 * fmt wid
 */
int read_fmt_wid(struct buf_t* pbuf,struct fmt_t* pfmt) {
	int s = STAT_WID;

	if (*pbuf->p == '*') {
		if (pfmt->w == 0) {
			pfmt->w = PFMT_WREAD;
			pbuf->p++;
			pbuf->c++;
		} else {
			s = STAT_ERR;
		}
	} else if (*pbuf->p >= '0' && *pbuf->p <='9') {
		if (pfmt->w == PFMT_WREAD) {
			s = STAT_ERR;
		} else {
			pfmt->w *= 10;
			pfmt->w += (*pbuf->p - '0');
			pbuf->p++;
			pbuf->c++;
		}
	} else if (strchr(CHR_FMT,*pbuf->p) != 0) {
		s = STAT_FMT;
	} else if (strchr(CHR_MOD,*pbuf->p) != 0) {
		s = STAT_MOD;
	} else if (*pbuf->p == '.') {
		pbuf->p++;
		pbuf->c++;
		s = STAT_PRE;
	} else {
		s = STAT_ERR;
	}

	return s;
}

/*
 * fmt flg
 */
int read_fmt_flg(struct buf_t* pbuf,struct fmt_t* pfmt) {
	int s = STAT_FLG;

	if (*pbuf->p == '0') {
		pfmt->f |= FLAG_ZERO;
	} else if (*pbuf->p == '#') {
		pfmt->f |= FLAG_ALT;
	} else if (*pbuf->p == '-') {
		pfmt->f |= FLAG_LEFT;
	} else if (*pbuf->p == ' ') {
		pfmt->f |= FLAG_SPACE;
	} else if (*pbuf->p == '+') {
		pfmt->f |= FLAG_PLUS;
	} else if ( *pbuf->p >= '1' && *pbuf->p <= '9' ) {
		s = STAT_WID;
	} else if (*pbuf->p == '.') {
		pbuf->p++;
		pbuf->c++;
		s = STAT_PRE;
	} else if (strchr(CHR_MOD,*pbuf->p) != 0) {
		s = STAT_MOD;
	} else if (strchr(CHR_FMT,*pbuf->p) != 0) {
		s = STAT_FMT;
	} else {
		s = STAT_ERR;
	}

	if (s == STAT_FLG) {
		pbuf->p++;
		pbuf->c++;
	} else {
		if ((pfmt->f & FLAG_LEFT) == FLAG_LEFT) {
			pfmt->f &= (~FLAG_ZERO);
		}

		if ((pfmt->f & FLAG_PLUS) == FLAG_PLUS) {
			pfmt->f &= (~FLAG_SPACE);
		}
	}

	return s;
}

/*
 * fmt ini
 */
int read_fmt_ini(struct buf_t* pbuf,struct fmt_t* pfmt) {
	int s;

	if (strchr(CHR_FLG, *pbuf->p) != 0) {
		s = STAT_FLG;
	} else if (strchr("123456789*",*pbuf->p) != 0) {
		s = STAT_WID;
	} else if ( *pbuf->p == '.') {
		pbuf->p++;
		pbuf->c++;
		s = STAT_PRE;
	} else if (strchr(CHR_MOD,*pbuf->p) != 0 ) {
		s = STAT_MOD;
	} else if ( strchr(CHR_FMT, *pbuf->p) != 0) {
		s = STAT_FMT;
	} else {
		s = STAT_ERR;
		pbuf->p++;
		pbuf->c++;
	}
	return s;
}

/*
 * print method (decimal)
 */
int print_d(struct buf_t* b,struct fmt_t* f) {
	int sign = 1;
	char buffer[16];
	size_t w;
	size_t s;
	size_t n = 0;
	char sc;

	size_t ap;
	size_t aw;

	const char *p;
	
	if (f->w == PFMT_WREAD) {
		aw = va_arg(b->ap,unsigned int);
	} else {
		aw = f->w;
	}

	if (f->p == PFMT_PREAD) {
		ap = va_arg(b->ap,unsigned int);
	} else {
		ap = f->p;
	}

	if (f->s == 32) {
		f->v.u32 = va_arg(b->ap,unsigned int);		
		if (f->v.i32 < 0)  {
			f->v.i32 = -f->v.i32;
			sign = -1;
		}
		p = conv_uint32_d(f->v.u32,buffer,sizeof(buffer));
	} else if (f->s == 16) {
		f->v.u16 = va_arg(b->ap,unsigned short);		
		if (f->v.i16 < 0)  {
			f->v.i16 = -f->v.i16;
			sign = -1;
		}
		p = conv_uint16_d(f->v.u16,buffer,sizeof(buffer));
	} else if (f->s == 8) {
		f->v.u8 = va_arg(b->ap,unsigned char);		
		if (f->v.i8 < 0)  {
			f->v.i8 = -f->v.i8;
			sign = -1;
		}
		p = conv_uint8_d(f->v.u8,buffer,sizeof(buffer));
	}
	
	w = strlen(p);

	if (sign < 0) {
		s = 1;
		sc = '-';
	} else {
		if ((f->f & FLAG_PLUS) == FLAG_PLUS) {
			s = 1;
			sc = '+';
		} else if ((f->f & FLAG_SPACE) == FLAG_SPACE) {
			s = 1;
			sc = ' ';
		} else {
			s = 0;
		}
	}

	if ((f->f & FLAG_LEFT) == FLAG_LEFT) {
		// Output sign	
		if ( s > 0) {
			pad(b, sc, 1);
		}

		// Precision pading
		if (ap > w) {
			n = ap - w;
			pad(b, '0', ap - w);
		}

		// Output number
		write(b->fd, p , w);

		// See if extra pading needed
		if (aw > w+s+n) {
			pad(b, ' ', aw - w - s - n);
		}
	} else {
		if (ap > 0) {
			if (ap > w) {
				n = ap - w;
			}

			// Extra padding
			if (aw > w+n+s) {
				pad(b, ' ', aw - w - n -s);
			}

			// Sign
			if (s > 0) {
				pad(b, sc , 1);
			}

			// Precision padding
			if (n > 0) {
				pad(b, '0', n);
			}

			// Output number
			write(b->fd, p, w);
		} else {
			if ((f->f & FLAG_ZERO) == FLAG_ZERO) {
				// Output sign
				if (s > 0) {
					pad(b, sc , 1);
				}
				
				// Zero pad
				if (aw > w+s) {
					pad(b, '0',aw - w - s);
				}
			} else {
				// Space pad
				if (aw > w+s) {
					pad(b, ' ', aw -w -s);
				}

				// Output sign
				if (s > 0) {
					pad(b, sc,1);
				}

			}
			write(b->fd ,p ,w);
		}
	}

	return 0;
}

/*
 * print method (decimal unsigned)
 */
int print_u(struct buf_t* b,struct fmt_t* f) {
	char buffer[16];
	char pc;
	size_t w;
	size_t n = 0;

	size_t aw;
	size_t ap;

	const char *p;

	if (f->w == PFMT_WREAD) {
		aw = va_arg(b->ap,unsigned int);
	} else {
		aw = f->w;
	}

	if (f->p == PFMT_PREAD) {
		ap = va_arg(b->ap,unsigned int);
	} else {
		ap = f->p;
	}
	
	if (f->s == 32) {
		f->v.u32 = va_arg(b->ap,unsigned int);		
		p = conv_uint32_d(f->v.u32,buffer,sizeof(buffer));
	} else if (f->s == 16) {
		f->v.u16 = va_arg(b->ap,unsigned short);		
		p = conv_uint16_d(f->v.u16,buffer,sizeof(buffer));
	} else if (f->s == 8) {
		f->v.u8 = va_arg(b->ap,unsigned char);		
		p = conv_uint8_d(f->v.u8,buffer,sizeof(buffer));
	}
	
	w = strlen(p);

	if ((f->f & FLAG_LEFT) == FLAG_LEFT) {
		// Precision pading
		if (ap > w) {
			n = ap - w;
			pad(b, '0', ap-w);
		}

		// Output number
		write(b->fd, p, w);

		// See if extra pading needed
		if (aw > w+n) {
			pad(b, ' ',aw - w - n);
		}
	} else {
		if (ap > 0) {
			if (ap > w) {
				n = ap - w;
			}

			// Extra padding
			if (aw > w+n) {
				pad(b, ' ',aw - w - n );
			}

			// Precision padding
			if (n > 0) {
				pad(b, '0', n);
			}

			// Output number
			write(b->fd, p, w);
		} else {
			if (aw > w) {
				n = aw -w;
			}

			if ((f->f & FLAG_ZERO) == FLAG_ZERO && aw > w) {
				pc = '0';	
			} else if (aw > w){
				pc = ' ';
			}
			if (n >0) {
				pad(b, pc, n);
			}
			write(b->fd, p, w);
		}
	}

	return 0;
}


/*
 * print method (hexadecimal)
 */
int print_x(struct buf_t* b,struct fmt_t* f) {
	char buffer[16];
	size_t w;
	size_t n = 0;
	size_t s = 0;

	size_t aw;
	size_t ap;

	const char *p;

	if (f->w == PFMT_WREAD) {
		aw = va_arg(b->ap,unsigned int);
	} else {
		aw = f->w;
	}

	if (f->p == PFMT_PREAD) {
		ap = va_arg(b->ap,unsigned int);
	} else {
		ap = f->p;
	}
	
	if (f->s == 32) {
		f->v.u32 = va_arg(b->ap,unsigned int);		
		p = conv_uint32_h(f->v.u32,buffer,sizeof(buffer));
	} else if (f->s == 16) {
		f->v.u16 = va_arg(b->ap,unsigned short);		
		p = conv_uint16_h(f->v.u16,buffer,sizeof(buffer));
	} else if (f->s == 8) {
		f->v.u8 = va_arg(b->ap,unsigned char);		
		p = conv_uint8_h(f->v.u8,buffer,sizeof(buffer));
	}
	
	w = strlen(p);

	if ((f->f & FLAG_ALT) == FLAG_ALT) {
		s = 2;
	}

	if ((f->f & FLAG_LEFT) == FLAG_LEFT) {
		if (s > 0) {
			write(b->fd, "0x", 2);
		}

		// Precision pading
		if (ap > w) {
			n = ap - w;
			pad(b, '0',ap-w);
		}

		// Output number
		write(b->fd, p, w);

		// See if extra pading needed
		if (aw > w+n+s) {
			pad(b, ' ', aw - w - n - s);
		}
	} else {
		if (ap > 0) {
			if (ap > w) {
				n = ap - w;
			}

			// Extra padding
			if (aw > w+n+s) {
				pad(b, ' ', aw - w - n - s);
			}

			if (s > 0) {
				write(b->fd, "0x", 2);
			}

			// Precision padding
			if (n > 0) {
				pad(b, '0', n);
			}

			// Output number
			write(b->fd, p, w);
		} else {
			if (aw > w) {
				n = aw -w;
			}

			if ((f->f & FLAG_ZERO) == FLAG_ZERO ) {
				if (s > 0) {
					write(b->fd, "0x", 2);
				}

				if (n > 0) {
					pad(b, '0', n);
				}
			} else if (aw > w){
				if (n > 0) {
					pad(b, ' ', n);
				}

				if (s > 0) {
					write(b->fd, "0x", 2);
				}
			} else {
				if (s > 0) {
					write(b->fd, "0x", 2);
				}
			}
			write(b->fd, p, w);
		}
	}

	return 0;
}


/*
 * print method (string)
 */
int print_s(struct buf_t* b,struct fmt_t* f) {
	size_t w;
	size_t n = 0;

	size_t ap;
	size_t aw;

	if (f->w == PFMT_WREAD) {
		aw = va_arg(b->ap,unsigned int);
	} else {
		aw = f->w;
	}

	if (f->p == PFMT_PREAD) {
		ap = va_arg(b->ap,unsigned int);
	} else {
		ap = f->p;
	}

	f->v.s = va_arg(b->ap,const char*);

	w = strlen(f->v.s);

	if (ap > 0 && w > ap) {
		w = ap;
	}

	if (aw > w) {
		n = aw - w;
	}

	if ((f->f & FLAG_LEFT) == FLAG_LEFT) {
		write(b->fd, f->v.s, w);	
		pad(b, ' ',n);
	} else {
		pad(b, ' ', n);
		write(b->fd, f->v.s, w);	
	}
}

/*
 * Pad with len character c
 */
void pad(struct buf_t* b,int c, size_t len) {
	char *p = (char*) &c;
	
	int i;
	for (i = 0 ; i < len; i++) {
		write(b->fd,p,1);
	}
}
