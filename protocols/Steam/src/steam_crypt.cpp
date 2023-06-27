#include "stdafx.h"

#include <openssl/rsa.h>

MBinBuffer RsaEncrypt(const char *pszModulus, const char *exponent, const char *data)
{
	BIGNUM *N = BN_new(), *E = BN_new();
	BN_hex2bn(&N, pszModulus);
	BN_hex2bn(&E, exponent);

	auto rsa = RSA_new();
	RSA_set0_key(rsa, N, E, NULL);

	MBinBuffer ret(RSA_size(rsa));
	memset(ret.data(), 0, ret.length());
	RSA_public_encrypt((int)mir_strlen(data), (BYTE*)data, ret.data(), rsa, RSA_PKCS1_PADDING);

	RSA_free(rsa);
	return ret;
}
