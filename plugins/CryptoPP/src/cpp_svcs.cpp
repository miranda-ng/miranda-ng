#include "commonheaders.h"

const unsigned char IV[] = "SIMhell@MIRANDA!";

// encrypt string using KeyX, return encoded string as ASCII or NULL
LPSTR __cdecl cpp_encrypt(pCNTX ptr, LPCSTR szPlainMsg)
{
	ptr->error = ERROR_NONE;
	pSIMDATA p = (pSIMDATA)ptr->pdata;

	uint8_t dataflag = 0;
	size_t slen = strlen(szPlainMsg);

	LPSTR szMsg;
	if (ptr->features & FEATURES_GZIP) {
		size_t clen;
		szMsg = (LPSTR)cpp_gzip((uint8_t*)szPlainMsg, slen, clen);
		if (clen >= slen) {
			free(szMsg);
			szMsg = _strdup(szPlainMsg);
		}
		else {
			slen = clen;
			dataflag |= DATA_GZIP;
		}
	}
	else szMsg = _strdup(szPlainMsg);

	string ciphered;

	CBC_Mode<AES>::Encryption enc(p->KeyX, Tiger::DIGESTSIZE, IV);
	StreamTransformationFilter cbcEncryptor(enc, new StringSink(ciphered));

	cbcEncryptor.Put((uint8_t*)szMsg, slen);
	cbcEncryptor.MessageEnd();

	free(szMsg);

	unsigned clen = (unsigned)ciphered.length();
	if (ptr->features & FEATURES_CRC32) {
		uint8_t crc32[CRC32::DIGESTSIZE];
		memset(crc32, 0, sizeof(crc32));
		CRC32().CalculateDigest(crc32, (uint8_t*)ciphered.data(), clen);
		ciphered.insert(0, (LPSTR)&crc32, CRC32::DIGESTSIZE);
		ciphered.insert(0, (LPSTR)&clen, 2);
	}
	if (ptr->features & FEATURES_GZIP)
		ciphered.insert(0, (LPSTR)&dataflag, 1);

	clen = (unsigned)ciphered.length();
	mir_free(ptr->tmp);
	if (ptr->features & FEATURES_BASE64)
		ptr->tmp =  mir_base64_encode(ciphered.data(), clen);
	else {
		char *base16 = base16encode(ciphered.data(), clen);
		ptr->tmp = mir_strdup(base16);
		free(base16);
	}

	return ptr->tmp;
}


// decrypt string using KeyX, return decoded string as ASCII or NULL
LPSTR __cdecl cpp_decrypt(pCNTX ptr, LPCSTR szEncMsg)
{
	ptrA ciphered;

	try {
		ptr->error = ERROR_SEH;
		pSIMDATA p = (pSIMDATA)ptr->pdata;

		size_t clen = strlen(szEncMsg);

		if (ptr->features & FEATURES_BASE64)
			ciphered = (LPSTR)mir_base64_decode(szEncMsg, &clen);
		else
			ciphered = base16decode(szEncMsg, &clen);

		LPSTR bciphered = ciphered;

		uint8_t dataflag = 0;
		if (ptr->features & FEATURES_GZIP) {
			dataflag = *ciphered;
			bciphered++; clen--; // cut GZIP flag
		}
		if (ptr->features & FEATURES_CRC32) {
			int len = *(uint16_t*)bciphered;
			bciphered += 2; clen -= 2; // cut CRC32 length

			if ((int)clen - CRC32::DIGESTSIZE < len) { // mesage not full
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("cpp_decrypt: error bad_len");
#endif
				ptr->error = ERROR_BAD_LEN;
				return nullptr;
			}

			uint8_t crc32[CRC32::DIGESTSIZE];
			memset(crc32, 0, sizeof(crc32));

			CRC32().CalculateDigest(crc32, (uint8_t*)(bciphered + CRC32::DIGESTSIZE), len);

			if (memcmp(crc32, bciphered, CRC32::DIGESTSIZE)) { // message is bad crc
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("cpp_decrypt: error bad_crc");
#endif
				ptr->error = ERROR_BAD_CRC;
				return nullptr;
			}
			bciphered += CRC32::DIGESTSIZE; // cut CRC32 digest
			clen = len;
		}

		string unciphered;

		CBC_Mode<AES>::Decryption dec(p->KeyX, Tiger::DIGESTSIZE, IV);
		StreamTransformationFilter cbcDecryptor(dec, new StringSink(unciphered));

		cbcDecryptor.Put((uint8_t*)bciphered, clen);
		cbcDecryptor.MessageEnd();

		mir_free(ptr->tmp);

		if (dataflag & DATA_GZIP) {
			size_t clen2 = clen;
			LPSTR res = (LPSTR)cpp_gunzip((uint8_t*)unciphered.data(), unciphered.length(), clen2);
			ptr->tmp = mir_strndup(res, clen2);
			free(res);
		}
		else
			ptr->tmp = mir_strdup(unciphered.c_str());

		ptr->error = ERROR_NONE;
		return ptr->tmp;
	}
	catch (...) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("cpp_decrypt: error seh");
#endif
		mir_free(ptr->tmp); ptr->tmp = nullptr;
		return nullptr;
	}
}


// encode message from ANSI into UTF8 if need
LPSTR __cdecl cpp_encodeA(HANDLE context, LPCSTR msg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return nullptr;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA)ptr->pdata;
	if (!p->KeyX) { ptr->error = ERROR_NO_KEYX; return nullptr; }

	LPSTR szNewMsg = nullptr;
	LPSTR szOldMsg = (LPSTR)msg;

	if (ptr->features & FEATURES_UTF8) {
		// ansi message: convert to unicode->utf-8 and encrypt.
		int slen = (int)strlen(szOldMsg) + 1;
		LPWSTR wstring = (LPWSTR)alloca(slen*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, szOldMsg, -1, wstring, slen*sizeof(wchar_t));
		// encrypt
		szNewMsg = cpp_encrypt(ptr, utf8encode(wstring));
	}
	else {
		// ansi message: encrypt.
		szNewMsg = cpp_encrypt(ptr, szOldMsg);
	}

	return szNewMsg;
}


// encode message from UTF8
LPSTR __cdecl cpp_encodeU(HANDLE context, LPCSTR msg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return nullptr;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA)ptr->pdata;
	if (!p->KeyX) { ptr->error = ERROR_NO_KEYX; return nullptr; }

	LPSTR szNewMsg = nullptr;
	LPSTR szOldMsg = (LPSTR)msg;

	if (ptr->features & FEATURES_UTF8) {
		// utf8 message: encrypt.
		szNewMsg = cpp_encrypt(ptr, szOldMsg);
	}
	else {
		// utf8 message: convert to ansi and encrypt.
		LPWSTR wstring = utf8decode(szOldMsg);
		int wlen = (int)wcslen(wstring) + 1;
		LPSTR astring = (LPSTR)alloca(wlen);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)szOldMsg, -1, astring, wlen, nullptr, nullptr);
		szNewMsg = cpp_encrypt(ptr, astring);
	}

	return szNewMsg;
}

// encode message from UNICODE into UTF8 if need
LPSTR __cdecl cpp_encodeW(HANDLE context, LPWSTR msg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return nullptr;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA)ptr->pdata;
	if (!p->KeyX) { ptr->error = ERROR_NO_KEYX; return nullptr; }

	LPSTR szNewMsg = nullptr;
	LPSTR szOldMsg = (LPSTR)msg;

	if (ptr->features & FEATURES_UTF8) {
		// unicode message: convert to utf-8 and encrypt.
		szNewMsg = cpp_encrypt(ptr, utf8encode((LPWSTR)szOldMsg));
	}
	else {
		// unicode message: convert to ansi and encrypt.
		int wlen = (int)wcslen((LPWSTR)szOldMsg) + 1;
		LPSTR astring = (LPSTR)alloca(wlen);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)szOldMsg, -1, astring, wlen, nullptr, nullptr);
		szNewMsg = cpp_encrypt(ptr, astring);
	}

	return szNewMsg;
}


// decode message from UTF8 if need, return ANSIzUCS2z
LPSTR __cdecl cpp_decode(HANDLE context, LPCSTR szEncMsg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return nullptr;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA)ptr->pdata;
	if (!p->KeyX) { ptr->error = ERROR_NO_KEYX; return nullptr; }

	LPSTR szNewMsg = nullptr;
	LPSTR szOldMsg = cpp_decrypt(ptr, szEncMsg);

	if (szOldMsg) {
		if (ptr->features & FEATURES_UTF8) {
			// utf8 message: convert to unicode -> ansii
			LPWSTR wstring = utf8decode(szOldMsg);
			int wlen = (int)wcslen(wstring) + 1;
			szNewMsg = (LPSTR)mir_alloc(wlen*(sizeof(wchar_t)+2));			// work.zy@gmail.com
			WideCharToMultiByte(CP_ACP, 0, wstring, -1, szNewMsg, wlen, nullptr, nullptr);
			memcpy(szNewMsg + strlen(szNewMsg) + 1, wstring, wlen*sizeof(wchar_t));	// work.zy@gmail.com
		}
		else {
			// ansi message: convert to unicode
			int slen = (int)strlen(szOldMsg) + 1;
			szNewMsg = (LPSTR)mir_alloc(slen*(sizeof(wchar_t)+1));
			memcpy(szNewMsg, szOldMsg, slen);
			wchar_t* wstring = (LPWSTR)alloca(slen*sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, 0, szOldMsg, -1, wstring, slen*sizeof(wchar_t));
			memcpy(szNewMsg + slen, wstring, slen*sizeof(wchar_t));
		}
	}
	mir_free(ptr->tmp);
	return ptr->tmp = szNewMsg;
}

// decode message return UTF8z
LPSTR __cdecl cpp_decodeU(HANDLE context, LPCSTR szEncMsg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr)
		return nullptr;
	mir_free(ptr->tmp);
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA)ptr->pdata;
	if (!p->KeyX) { ptr->error = ERROR_NO_KEYX; return nullptr; }

	LPSTR szNewMsg = nullptr;
	LPSTR szOldMsg = cpp_decrypt(ptr, szEncMsg);

	if (szOldMsg) {
		if (ptr->features & FEATURES_UTF8) {
			// utf8 message: copy
			szNewMsg = mir_strdup(szOldMsg);
		}
		else {
			// ansi message: convert to utf8
			int slen = (int)strlen(szOldMsg) + 1;
			LPWSTR wstring = (LPWSTR)alloca(slen*sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, 0, szOldMsg, -1, wstring, slen*sizeof(wchar_t));
			szNewMsg = mir_strdup(utf8encode(wstring));
		}
	}
	return ptr->tmp = szNewMsg;
}

int __cdecl cpp_encrypt_file(HANDLE context, LPCSTR file_in, LPCSTR file_out)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return 0;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA)ptr->pdata;
	if (!p->KeyX) return 0;

	try {
		CBC_Mode<AES>::Encryption enc(p->KeyX, Tiger::DIGESTSIZE, IV);
		FileSource *f = new FileSource(file_in, true, new StreamTransformationFilter(enc, new FileSink(file_out)));
		delete f;
	}
	catch (...) {
		return 0;
	}
	return 1;
}

int __cdecl cpp_decrypt_file(HANDLE context, LPCSTR file_in, LPCSTR file_out)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return 0;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA)ptr->pdata;
	if (!p->KeyX) return 0;

	try {
		CBC_Mode<AES>::Decryption dec(p->KeyX, Tiger::DIGESTSIZE, IV);
		FileSource *f = new FileSource(file_in, true, new StreamTransformationFilter(dec, new FileSink(file_out)));
		delete f;
	}
	catch (...) {
		return 0;
	}
	return 1;
}
