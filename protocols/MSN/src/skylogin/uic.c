/*  
 * Skype Login
 * 
 * Based on:
 *   FakeSkype : Skype reverse engineering proof-of-concept client
 *               Ouanilo MEDEGAN (c) 2006   http://www.oklabs.net
 *   pyskype   : Skype login Python script by uunicorn
 *
 * Written by:   leecher@dose.0wnz.at (c) 2015 
 *
 * Module:       UIC generation
 *
 */
#include "common.h"
#include "random.h"
#include "skylogin.h"

#ifndef CRYPT_WOLFSSL
/* OpenSSL Base64_Encode */
#include <openssl/evp.h>
#include <openssl/buffer.h>

static int Base64_Encode(const uchar* in, uint inLen, uchar* out, uint* outLen)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;
	int ret=0;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, in, inLen);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	if (*outLen>(uint)bptr->length) *outLen=bptr->length; else ret=-1;
	memcpy(out, bptr->data, *outLen);
	BIO_free_all(b64);
	return ret;
}
#endif


Memory_U CreateUIC(Skype_Inst *pInst, const char *pszNonce, const char *pszSalt)
{
	Memory_U uic_pkt, uic={0};
	SHA_CTX CredCtx;
	uchar *p;
	uchar SignedChallenge[0x80] = {0};
	int cbSalt = strlen(pszSalt), cbNonce = strlen(pszNonce);

	if (!(uic_pkt.Memory = (uchar*)malloc(uic_pkt.MsZ = SHA_DIGEST_LENGTH + cbSalt + cbNonce)))
		return uic;
	SHA1_Init(&CredCtx);
	SHA1_Update(&CredCtx, pInst->LoginD.SignedCredentials.Memory, pInst->LoginD.SignedCredentials.MsZ);
	SHA1_Update(&CredCtx, pszSalt, cbSalt);
	SHA1_Final(uic_pkt.Memory, &CredCtx);
	p = uic_pkt.Memory + SHA_DIGEST_LENGTH;
	memcpy(p, pszSalt, cbSalt);
	p+=cbSalt;
	memcpy(p, pszNonce, cbNonce);
	BuildUnFinalizedDatas(uic_pkt.Memory, uic_pkt.MsZ, SignedChallenge);
	if (RSA_private_encrypt(sizeof(SignedChallenge), SignedChallenge, SignedChallenge, 
		pInst->LoginD.RSAKeys, RSA_NO_PADDING)<0)
		return uic;
	free (uic_pkt.Memory);
	if (!(uic.Memory = (uchar*)malloc(uic.MsZ = sizeof(SignedChallenge) + pInst->LoginD.SignedCredentials.MsZ + 4)))
	{
		uic.MsZ=0;
		return uic;
	}
	p = uic.Memory;
	*((uint*)p) = htonl(pInst->LoginD.SignedCredentials.MsZ);
	p+=sizeof(uint);
	memcpy(p, pInst->LoginD.SignedCredentials.Memory, pInst->LoginD.SignedCredentials.MsZ);
	p+=pInst->LoginD.SignedCredentials.MsZ;
	memcpy(p, SignedChallenge, sizeof(SignedChallenge));
	return uic;
}


int CreateUICString(Skype_Inst *pInst, const char *pszNonce, const char *pszSalt, char *pszOutUIC)
{
	Memory_U uic = CreateUIC(pInst, pszNonce, pszSalt);
	uint outlen = UICSTR_SIZE;

	if (uic.MsZ && uic.Memory)
	{	
		Base64_Encode(uic.Memory, uic.MsZ, (uchar*)pszOutUIC, &outlen);
		pszOutUIC[outlen]=0;
		free(uic.Memory);
	} else return 0;
	return outlen;
}

int GetCredentialsUIC(Skype_Inst *pInst, char *pszOutUIC)
{
	uint outlen = UICSTR_SIZE;

	if (pInst->LoginD.SignedCredentials.MsZ)
	{	
		Base64_Encode(pInst->LoginD.SignedCredentials.Memory, pInst->LoginD.SignedCredentials.MsZ, (uchar*)pszOutUIC, &outlen);
		pszOutUIC[outlen]=0;
	} else return 0;
	return outlen;
}
