#include "stdafx.h"

void *buf;

static const char base64Fillchar = '='; // used to mark partial words at the end

// this lookup table defines the base64 encoding
char *base64EncodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const unsigned char base64DecodeTable[] = {
	99, 98, 98, 98, 98, 98, 98, 98, 98, 97,  97, 98, 98, 97, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //00 -29
	98, 98, 97, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 62, 98, 98, 98, 63, 52, 53,  54, 55, 56, 57, 58, 59, 60, 61, 98, 98,  //30 -59
	98, 96, 98, 98, 98, 0, 1, 2, 3, 4,   5, 6, 7, 8, 9, 10, 11, 12, 13, 14,  15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  //60 -89
	25, 98, 98, 98, 98, 98, 98, 26, 27, 28,  29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  //90 -119
	49, 50, 51, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //120 -149
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //150 -179
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //180 -209
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //210 -239
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98                                               //240 -255
};

int encodeLength(int inlen, char formatted)
{
	unsigned int i = ((inlen-1)/3*4+4+1);
	if (formatted) i+=inlen/54;
	return i;
}

char * encode(unsigned char *inbuf, unsigned int inlen, char formatted)
{
	int i = encodeLength(inlen, formatted), k = 17, eLen = inlen/3, j;
	malloc(i);
	char * curr = (char *)buf;
	for (i=0;i<eLen;i++)
	{
		// Copy next three bytes into lower 24 bits of int, paying attention to sign.
		j = (inbuf[0]<<16)|(inbuf[1]<<8)|inbuf[2]; inbuf+=3;
		// Encode the int into four chars
		*(curr++) = base64EncodeTable[ j>>18      ];
		*(curr++) = base64EncodeTable[(j>>12)&0x3f];
		*(curr++) = base64EncodeTable[(j>> 6)&0x3f];
		*(curr++) = base64EncodeTable[(j)&0x3f];
		if (formatted) { if ( !k) { *(curr++) = '\n'; k = 18; } k--; }
	}
	eLen = inlen-eLen*3; // 0 - 2.
	if (eLen == 1)
	{
		*(curr++) = base64EncodeTable[ inbuf[0]>>2      ];
		*(curr++) = base64EncodeTable[(inbuf[0]<<4)&0x3F];
		*(curr++) = base64Fillchar;
		*(curr++) = base64Fillchar;
	} else if (eLen == 2)
	{
		j = (inbuf[0]<<8)|inbuf[1];
		*(curr++) = base64EncodeTable[ j>>10      ];
		*(curr++) = base64EncodeTable[(j>> 4)&0x3f];
		*(curr++) = base64EncodeTable[(j<< 2)&0x3f];
		*(curr++) = base64Fillchar;
	}
	*(curr++) = 0;
	return (char *)buf;
}

unsigned int decodeSize(char * data)
{
	if ( !data) return 0;
	int size = 0;
	unsigned char c;
	//skip any extra characters (e.g. newlines or spaces)
	while (*data)
	{
		if (*data>255) { return 0; }
		c = base64DecodeTable[(unsigned char)(*data)];
		if (c<97) size++;
		else if (c == 98) { return 0; }
		data++;
	}
	if (size == 0) return 0;
	do { data--; size--; } while (*data == base64Fillchar); size++;
	return (unsigned int)((size*3)/4);
}

unsigned char decode(char * data, unsigned char *buf, int len)
{
	if ( !data) return 0;
	int i=0, p = 0;
	unsigned char d, c;
	for (;;)
	{

	#define BASE64DECODE_READ_NEXT_CHAR(c)                                              \
	do {                                                                        \
	if (data[i]>255) { c = 98; break; }                                        \
	c = base64DecodeTable[(unsigned char)data[i++]];                       \
	}while (c == 97);                                                             \
	if(c == 98) { return 0; }

		BASE64DECODE_READ_NEXT_CHAR(c)
			if (c == 99) { return 2; }
			if (c == 96)
			{
				if (p == (int)len) return 2;
				return 1;
			}

			BASE64DECODE_READ_NEXT_CHAR(d)
				if ((d == 99) || (d == 96)) { return 1; }
				if (p == (int)len) { return 0; }
				buf[p++] = (unsigned char)((c<<2)|((d>>4)&0x3));

				BASE64DECODE_READ_NEXT_CHAR(c)
					if (c == 99) { return 1; }
					if (p == (int)len)
					{
						if (c == 96) return 2;
						return 0;
					}
					if (c == 96) { return 1; }
					buf[p++] = (unsigned char)(((d<<4)&0xf0)|((c>>2)&0xf));

					BASE64DECODE_READ_NEXT_CHAR(d)
						if (d == 99) { return 1; }
						if (p == (int)len)
						{
							if (d == 96) return 2;
							return 0;
						}
						if (d == 96) { return 1; }
						buf[p++] = (unsigned char)(((c<<6)&0xc0)|d);
	}
}
#undef BASE64DECODE_READ_NEXT_CHAR

unsigned char *decode(char * data, int *outlen)
{
	if ( !data) { *outlen = 0; return (unsigned char*)""; }
	unsigned int len = decodeSize(data);
	if (outlen) *outlen = len;
	if ( !len) return NULL;
	malloc(len+1);
	if( !decode(data, (unsigned char*)buf, len)) { return NULL; }
	return (unsigned char*)buf;
}

int main()
{
	aes_context ctx;
	FILE *fin, *fout;

	int t = strlen(MY_KEY);
	int basecodedkey = encodeLength(strlen(MY_KEY), false);
	buf = malloc(basecodedkey + 1);
	unsigned char *tmpk = (unsigned char *)malloc(basecodedkey + 1);
	tmpk = (unsigned char *)encode((unsigned char*)MY_KEY, strlen(MY_KEY), false);
	tmpk[basecodedkey] = 0;
	char *output = (char*)malloc(basecodedkey + 17);
	strcpy(output, "#define MY_KEY \"");
	strcat(output, (const char*)tmpk);
	strcat(output, "\"");
	fout = fopen("key.h", "wb");
	fputs((const char*)output, fout);
	fclose(fout);
	free(buf);

	aes_set_key( &ctx, (BYTE*)MY_KEY, 128);

	//encrypt
	fin = fopen("keypair.crt", "rb");
	fseek(fin, 0, SEEK_END);
	long fileSize =	ftell(fin);
	rewind (fin);
	int needbyte = 16 - fileSize%16;
	int corsize = fileSize + needbyte;
	unsigned char *inBuf = (unsigned char*)malloc(corsize + 1);
	unsigned char *locbuf = (unsigned char*)malloc(corsize + 1);
	fread(inBuf, fileSize, 1, fin);
	fclose(fin);
	memset(inBuf+fileSize, 0, needbyte);
	inBuf[corsize] = 0;
	for (int i = 0; i < corsize; i+=16) {
		aes_encrypt(&ctx, inBuf+i, locbuf+i);
	}
	free(inBuf);
	locbuf[corsize] = 0; //cert should be null terminated
	int basecoded = encodeLength(corsize, false);
	buf = malloc(basecoded + 1);
	unsigned char *tmp = (unsigned char *)malloc(basecoded + 1);
	tmp = (unsigned char *)encode(locbuf, corsize, false);
	tmp[basecoded] = 0;
	free(locbuf);
	fout = fopen("keypair.bin", "wb");
	fputs((const char*)tmp, fout);
	fclose(fout);
	free(buf);
	//free(tmp); todo:fix

	//decrypt
	fin = fopen("keypair.bin", "rb");
	fseek(fin, 0, SEEK_END);
	long fileSizeD = ftell(fin);
	rewind(fin);
	unsigned char *inBufD = (unsigned char*)malloc(fileSizeD);
	fread(inBufD, fileSizeD, 1, fin);
	fclose(fin);
	inBufD[fileSizeD] = 0;
	int basedecoded = decodeSize((char*)inBufD);
	unsigned char *bufD = (unsigned char*)malloc(basedecoded + 1);
	unsigned char *tmpD = (unsigned char*)malloc(basedecoded + 1);
	decode((char*)inBufD, tmpD, basedecoded);
	for (int i = 0; i < basedecoded; i += 16) {
		aes_decrypt(&ctx, tmpD+i, bufD+i);
	}
	bufD[basedecoded] = 0; //cert should be null terminated
	//free(inBufD); todo:fix
	free(tmpD);
	fout = fopen("keypair.crt.decrypted", "wb");
	fputs((const char*)bufD, fout);
	fclose(fout);
	free(bufD);
            
	return 0;
}