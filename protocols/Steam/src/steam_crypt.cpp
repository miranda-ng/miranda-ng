#include "stdafx.h"

#include <openssl/rsa.h>

MBinBuffer RsaEncrypt(const char *pszModulus, const char *exponent, const char *data)
{
	//pszModulus = "cb23284f3078f97f9667624b4f882cd7d68aefd6b22f136b5808dfc3ae19d6df7f278d71049a4d61d2bedb4fe958e84140e3ba261b80cf29b37d2aca3ab456cf26fbeca4eded69d51982b38f9ec1003c3e41b22c757150736d2df976908abfdc5da7c9bbc5f4626f6752c41141534867d14cddc4e4278aa456824bfe131880aaf17a125569a365f802af859107a9e916e2442ceff6ff2feb77c6dc0b87639c9f1b34f681f2383a599f8dca8f6558cc60cdb5318fe58888604d4b66ab5175e0dadf1deb499937cb090094adb46b52752954ffc915fbbf41999bb5c301c40e1f1a6e45c23bb10529a356d753ee0d42003d82bf3e5eb5556fa27e394780034dcf5b";

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
