#ifndef _AES_H
#define _AES_H

typedef struct
{
	unsigned long int erk[64]; /* encryption round keys */
	unsigned long int drk[64]; /* decryption round keys */
	int nr; /* number of rounds */
} aes_context;

int aes_set_key(aes_context *ctx, unsigned char *key, int nbits);
void aes_encrypt(aes_context *ctx, unsigned char input[16], unsigned char output[16]);
void aes_decrypt(aes_context *ctx, unsigned char input[16], unsigned char output[16]);

#endif /* aes.h */
