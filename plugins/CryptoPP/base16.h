#ifndef __BASE_16__
#define __BASE_16__

#define BERR		0xFF	/* Illegal char marker */
#define BEOF		0x7F	/* EOF marker (padding char or EOL) */

typedef unsigned char byte;

static const byte asciiToBin16[] =
	{ BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BEOF, BERR, BERR, BEOF, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	  0x08, 0x09, BERR, BERR, BERR, BEOF, BERR, BERR,
	  BERR, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR,
	  BERR, BERR, BERR, BERR, BERR, BERR, BERR, BERR
	};

static const byte binToAscii16[] = "0123456789ABCDEF";

char *base16encode(const char *, const int);
char *base16decode(const char *, int *);
char *base16decode(const char *);

#define encode16(data) binToAscii16[data]
#define decode16(data) asciiToBin16[data]

#endif
