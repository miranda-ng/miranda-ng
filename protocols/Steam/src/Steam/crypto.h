#ifndef _STEAM_CRYPTO_H_
#define _STEAM_CRYPTO_H_

#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

namespace SteamWebApi
{
	class CryptoApi : public BaseApi
	{
	public:

		class RsaKey : public Result
		{
			friend CryptoApi;

		private:
			std::string modulus;
			std::string exponent;
			time_t timestamp;

		public:
			RsaKey() : timestamp(0) { }

			const char * GetModulus() const
			{
				return modulus.c_str();
			}

			const char * GetExponent() const
			{
				return exponent.c_str();
			}

			time_t GetTimestamp() const
			{
				return timestamp;
			}

			int GetEncryptedSize() const
			{
				BIGNUM *n = BN_new();
				if (!BN_hex2bn(&n, modulus.c_str()))
				return NULL;

				BIGNUM *e = BN_new();
				if (!BN_hex2bn(&e, exponent.c_str()))
					return NULL;

				RSA *rsa = RSA_new();
				rsa->n = n;
				rsa->e = e;

				int size = RSA_size(rsa);

				RSA_free(rsa);
				
				return size;
			}

			int Encrypt(BYTE *data, int dataSize, BYTE *encrypted) const
			{
				BIGNUM *n = BN_new();
				if (!BN_hex2bn(&n, modulus.c_str()))
				return NULL;

				BIGNUM *e = BN_new();
				if (!BN_hex2bn(&e, exponent.c_str()))
					return NULL;

				RSA *rsa = RSA_new();
				rsa->n = n;
				rsa->e = e;

				if (RSA_public_encrypt(dataSize, data, encrypted, rsa, RSA_PKCS1_PADDING) < 0)
				{
					RSA_free(rsa);
					return 1;
				}

				RSA_free(rsa);
				return 0;
			}
		};

		static void GetRsaKey(HANDLE hConnection, const char *username, RsaKey *rsaKey)
		{
			rsaKey->success = false;

			HttpRequest request(hConnection, REQUEST_GET, STEAM_COMMUNITY_URL "/mobilelogin/getrsakey");
			request.AddParameter("username", username);
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node;
			if (!root) return;

			node = json_get(root, "success");
			if (!json_as_bool(node)) return;

			node = json_get(root, "publickey_mod");
			rsaKey->modulus = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "publickey_exp");
			rsaKey->exponent = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "timestamp");
			rsaKey->timestamp = _atoi64(ptrA(mir_u2a(json_as_string(node))));

			rsaKey->success = true;
		}
	};
}


#endif //_STEAM_CRYPTO_H_