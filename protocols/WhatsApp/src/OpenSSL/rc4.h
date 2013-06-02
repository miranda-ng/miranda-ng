#ifndef HEADER_RC4_LOCL_H
#define HEADER_RC4_LOCL_H

typedef int RC4_INT;

typedef struct rc4_key_st
	{
	RC4_INT x,y;
	RC4_INT data[256];
	} RC4_KEY;

#ifdef  __cplusplus
extern "C" {
#endif

void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data);
void RC4(RC4_KEY *key, unsigned long len, const unsigned char *indata, unsigned char *outdata);

#ifdef  __cplusplus
}
#endif

#endif
