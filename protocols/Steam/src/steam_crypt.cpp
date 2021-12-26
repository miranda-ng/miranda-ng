#include "stdafx.h"

#pragma comment(lib, "crypt32.lib")

int CSteamProto::RsaEncrypt(const char *pszModulus, DWORD &exponent, const char *data, uint8_t *encryptedData, DWORD &encryptedSize)
{
	uint32_t cchModulus = (uint32_t)mir_strlen(pszModulus);
	int result;
	HCRYPTKEY phKey = 0;
	HCRYPTPROV hCSP = 0;

	__try {
		// convert hex string to byte array
		DWORD cbLen = 0, dwSkip = 0, dwFlags = 0;
		if (!CryptStringToBinaryA(pszModulus, cchModulus, CRYPT_STRING_HEX, nullptr, &cbLen, &dwSkip, &dwFlags)) {
			result = GetLastError();
			__leave;
		}

		// allocate a new buffer.
		mir_ptr<uint8_t> pbBuffer((uint8_t *)mir_alloc(cbLen));
		if (!CryptStringToBinaryA(pszModulus, cchModulus, CRYPT_STRING_HEX, pbBuffer, &cbLen, &dwSkip, &dwFlags)) {
			result = GetLastError();
			__leave;
		}

		// reverse byte array, because of microsoft
		for (int i = 0; i < (int)(cbLen / 2); ++i) {
			uint8_t temp = pbBuffer[cbLen - i - 1];
			pbBuffer[cbLen - i - 1] = pbBuffer[i];
			pbBuffer[i] = temp;
		}

		if (!CryptAcquireContext(&hCSP, nullptr, nullptr, PROV_RSA_AES, CRYPT_SILENT) &&
			!CryptAcquireContext(&hCSP, nullptr, nullptr, PROV_RSA_AES, CRYPT_SILENT | CRYPT_NEWKEYSET)) {
			result = GetLastError();
			__leave;
		}

		// Move the key into the key container.
		uint32_t cbKeyBlob = sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY) + cbLen;
		mir_ptr<uint8_t> pKeyBlob((uint8_t *)mir_alloc(cbKeyBlob));

		// Fill in the data.
		PUBLICKEYSTRUC *pPublicKey = (PUBLICKEYSTRUC *)pKeyBlob.get();
		pPublicKey->bType = PUBLICKEYBLOB;
		pPublicKey->bVersion = CUR_BLOB_VERSION;  // Always use this value.
		pPublicKey->reserved = 0;                 // Must be zero.
		pPublicKey->aiKeyAlg = CALG_RSA_KEYX;     // RSA public-key key exchange. 

		// The next block of data is the RSAPUBKEY structure.
		RSAPUBKEY *pRsaPubKey = (RSAPUBKEY *)(pKeyBlob + sizeof(PUBLICKEYSTRUC));
		pRsaPubKey->magic = 0x31415352; // RSA1 // Use public key
		pRsaPubKey->bitlen = cbLen * 8;  // Number of bits in the modulus.
		pRsaPubKey->pubexp = exponent; // Exponent.

		// Copy the modulus into the blob. Put the modulus directly after the
		// RSAPUBKEY structure in the blob.
		uint8_t *pKey = (uint8_t *)(((uint8_t *)pRsaPubKey) + sizeof(RSAPUBKEY));
		memcpy(pKey, pbBuffer, cbLen);

		// Now import public key	
		if (!CryptImportKey(hCSP, pKeyBlob, cbKeyBlob, 0, 0, &phKey)) {
			result = GetLastError();
			__leave;
		}

		DWORD dataSize = (DWORD)mir_strlen(data);

		// if data is not allocated just renurn size
		if (encryptedData == nullptr) {
			// get length of encrypted data
			if (!CryptEncrypt(phKey, 0, TRUE, 0, nullptr, &encryptedSize, dataSize))
				result = GetLastError();
			__leave;
		}

		// encrypt password
		memcpy(encryptedData, data, dataSize);
		if (!CryptEncrypt(phKey, 0, TRUE, 0, encryptedData, &dataSize, encryptedSize)) {
			result = GetLastError();
			__leave;
		}

		// reverse byte array again
		for (int i = 0; i < (int)(encryptedSize / 2); ++i) {
			uint8_t temp = encryptedData[encryptedSize - i - 1];
			encryptedData[encryptedSize - i - 1] = encryptedData[i];
			encryptedData[i] = temp;
		}
	}
	__finally
	{
		result = 0;
	};

	if (phKey)
		CryptDestroyKey(phKey);

	if (hCSP)
		CryptReleaseContext(hCSP, 0);

	return 0;
}
