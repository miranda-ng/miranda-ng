#include "Mra.h"
#include "MraSendQueue.h"

void hmac_sha1(BYTE *text,size_t text_len,BYTE *key,size_t key_len,BYTE *digest)
{
	mir_sha1_ctx context;
	BYTE k_ipad[65];	/* inner padding - key XORd with ipad */
	BYTE k_opad[65];	/* outer padding - key XORd with opad */
	BYTE tk[MIR_SHA1_HASH_SIZE];
	/* if key is longer than 64 bytes reset it to key=SHA1(key) */
	if (key_len>64)
	{
		mir_sha1_ctx tctx;

		mir_sha1_init(&tctx);
		mir_sha1_append(&tctx,key,key_len);
		mir_sha1_finish(&tctx,(BYTE*)&tk);

		key=tk;
		key_len=MIR_SHA1_HASH_SIZE;
	}

	/*
	* the HMAC_SHA1 transform looks like:
	*
	* SHA1(K XOR opad, SHA1(K XOR ipad, text))
	*
	* where K is an n byte key
	* ipad is the byte 0x36 repeated 64 times
	* opad is the byte 0x5c repeated 64 times
	* and text is the data being protected
	*/

	/* start out by storing key in pads */
	memmove(&k_ipad,key,key_len);
	memmove(&k_opad,key,key_len);
	memset(&k_ipad[key_len], 0, (sizeof(k_ipad)-key_len));
	memset(&k_opad[key_len], 0 , (sizeof(k_opad)-key_len));

	/* XOR key with ipad and opad values */
	for (size_t i=0;i<(64/sizeof(ULONGLONG));i++)
	{
		((ULONGLONG*)k_ipad)[i]^=0x3636363636363636;
		((ULONGLONG*)k_opad)[i]^=0x5C5C5C5C5C5C5C5C;
	}
	/* perform inner SHA1 */
	mir_sha1_init(&context);				/* init context for 1st pass */
	mir_sha1_append(&context,k_ipad,64);		/* start with inner pad */
	mir_sha1_append(&context,text,text_len);	/* then text of datagram */
	mir_sha1_finish(&context,digest);		/* finish up 1st pass */
	/* perform outer SHA1 */
	mir_sha1_init(&context);							/* init context for 2nd pass */
	mir_sha1_append(&context,k_opad,64);					/* start with outer pad */
	mir_sha1_append(&context,(BYTE*)digest,MIR_SHA1_HASH_SIZE);	/* then results of 1st hash */
	mir_sha1_finish(&context,digest);					/* finish up 2nd pass */

	bzero(k_ipad,sizeof(k_ipad));
	bzero(k_opad,sizeof(k_opad));
	bzero(tk,sizeof(tk));
}
