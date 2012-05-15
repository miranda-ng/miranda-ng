#include "commonheaders.h"

const unsigned char IV[] = "SIMhell@MIRANDA!";


// encrypt string using KeyX, return encoded string as ASCII or NULL
LPSTR __cdecl cpp_encrypt(pCNTX ptr, LPCSTR szPlainMsg) {

   	ptr->error = ERROR_NONE;
	pSIMDATA p = (pSIMDATA) ptr->pdata;

	BYTE dataflag = 0;
	int clen,slen;
	slen = strlen(szPlainMsg);

	LPSTR szMsg;
	if(ptr->features & FEATURES_GZIP) {
		szMsg = (LPSTR) cpp_gzip((BYTE*)szPlainMsg,slen,clen);
		if(clen>=slen) {
		    free(szMsg);
		    szMsg = _strdup(szPlainMsg);
		}
		else {
		    slen = clen;
		    dataflag |= DATA_GZIP;
		}
	}
	else
	    szMsg = _strdup(szPlainMsg);

	string ciphered;

	CBC_Mode<AES>::Encryption enc(p->KeyX,Tiger::DIGESTSIZE,IV);
	StreamTransformationFilter cbcEncryptor(enc,new StringSink(ciphered));

	cbcEncryptor.Put((PBYTE)szMsg, slen);
	cbcEncryptor.MessageEnd();

	free(szMsg);

	clen = (int) ciphered.length();
	if(ptr->features & FEATURES_CRC32) {
		BYTE crc32[CRC32::DIGESTSIZE];
		memset(crc32,0,sizeof(crc32));
		CRC32().CalculateDigest(crc32, (BYTE*)ciphered.data(), clen);
		ciphered.insert(0,(LPSTR)&crc32,CRC32::DIGESTSIZE);
		ciphered.insert(0,(LPSTR)&clen,2);
	}
	if(ptr->features & FEATURES_GZIP) {
		ciphered.insert(0,(LPSTR)&dataflag,1);
	}
	clen = (int) ciphered.length();

	SAFE_FREE(ptr->tmp);
	if(ptr->features & FEATURES_BASE64) {
		ptr->tmp = base64encode(ciphered.data(),clen);
	}
	else {
		ptr->tmp = base16encode(ciphered.data(),clen);
	}

	return ptr->tmp;
}


// decrypt string using KeyX, return decoded string as ASCII or NULL
LPSTR __cdecl cpp_decrypt(pCNTX ptr, LPCSTR szEncMsg) {

	LPSTR ciphered = NULL;

	try {
		ptr->error = ERROR_SEH;
		pSIMDATA p = (pSIMDATA) ptr->pdata;

		int clen = strlen(szEncMsg);

		if(ptr->features & FEATURES_BASE64)
			ciphered = base64decode(szEncMsg,&clen);
		else
			ciphered = base16decode(szEncMsg,&clen);

		LPSTR bciphered = ciphered;

		BYTE dataflag=0;
		if(ptr->features & FEATURES_GZIP) {
			dataflag = *ciphered;
			bciphered++; clen--; // cut GZIP flag
		}
		if(ptr->features & FEATURES_CRC32) {
			int len;
			__asm {
				mov esi,[bciphered];
				xor eax,eax;
				mov ax,word ptr [esi];
				mov [len], eax;
			}
			bciphered+=2; clen-=2; // cut CRC32 length

			if(clen-CRC32::DIGESTSIZE<len) { // mesage not full
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("cpp_decrypt: error bad_len");
#endif
				free(ciphered);
				ptr->error = ERROR_BAD_LEN;
				return NULL;
			}

			BYTE crc32[CRC32::DIGESTSIZE];
			memset(crc32,0,sizeof(crc32));

			CRC32().CalculateDigest(crc32, (PBYTE)(bciphered+CRC32::DIGESTSIZE), len);

			if(memcmp(crc32,bciphered,CRC32::DIGESTSIZE)) { // message is bad crc
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("cpp_decrypt: error bad_crc");
#endif
				free(ciphered);
				ptr->error = ERROR_BAD_CRC;
				return NULL;
			}
			bciphered+=CRC32::DIGESTSIZE; // cut CRC32 digest
			clen=len;
		}

		string unciphered;

		CBC_Mode<AES>::Decryption dec(p->KeyX,Tiger::DIGESTSIZE,IV);
		StreamTransformationFilter cbcDecryptor(dec,new StringSink(unciphered));

		cbcDecryptor.Put((PBYTE)bciphered,clen);
		cbcDecryptor.MessageEnd();

		free(ciphered);
		SAFE_FREE(ptr->tmp);

		if(dataflag & DATA_GZIP) {
			ptr->tmp = (LPSTR) cpp_gunzip((PBYTE)unciphered.data(),unciphered.length(),clen);
			ptr->tmp[clen] = 0;
		}
		else {
			ptr->tmp = (LPSTR) strdup(unciphered.c_str());
		}

		ptr->error = ERROR_NONE;
		return ptr->tmp;
	}
	catch (...) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("cpp_decrypt: error seh");
#endif
		free(ciphered);
		SAFE_FREE(ptr->tmp);
		return NULL;
	}
}


// encode message from ANSI into UTF8 if need
LPSTR __cdecl cpp_encodeA(HANDLE context, LPCSTR msg) {

	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return NULL;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA) ptr->pdata;
	if(!p->KeyX) { ptr->error = ERROR_NO_KEYX; return NULL; }

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = (LPSTR) msg;

	if(ptr->features & FEATURES_UTF8) {
		// ansi message: convert to unicode->utf-8 and encrypt.
		int slen = strlen(szOldMsg)+1;
		LPWSTR wstring = (LPWSTR) alloca(slen*sizeof(WCHAR));
		MultiByteToWideChar(CP_ACP, 0, szOldMsg, -1, wstring, slen*sizeof(WCHAR));
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
LPSTR __cdecl cpp_encodeU(HANDLE context, LPCSTR msg) {

	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return NULL;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA) ptr->pdata;
	if(!p->KeyX) { ptr->error = ERROR_NO_KEYX; return NULL; }

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = (LPSTR) msg;

	if(ptr->features & FEATURES_UTF8) {
		// utf8 message: encrypt.
		szNewMsg = cpp_encrypt(ptr, szOldMsg);
	}
	else {
		// utf8 message: convert to ansi and encrypt.
		LPWSTR wstring = utf8decode(szOldMsg);
		int wlen = wcslen(wstring)+1;
		LPSTR astring = (LPSTR) alloca(wlen);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)szOldMsg, -1, astring, wlen, 0, 0);
		szNewMsg = cpp_encrypt(ptr, astring);
	}

	return szNewMsg;
}


// encode message from UNICODE into UTF8 if need
LPSTR __cdecl cpp_encodeW(HANDLE context, LPWSTR msg) {

	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return NULL;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA) ptr->pdata;
	if(!p->KeyX) { ptr->error = ERROR_NO_KEYX; return NULL; }

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = (LPSTR) msg;

	if(ptr->features & FEATURES_UTF8) {
		// unicode message: convert to utf-8 and encrypt.
		szNewMsg = cpp_encrypt(ptr, utf8encode((LPWSTR)szOldMsg));
	}
	else {
		// unicode message: convert to ansi and encrypt.
		int wlen = wcslen((LPWSTR)szOldMsg)+1;
		LPSTR astring = (LPSTR) alloca(wlen);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)szOldMsg, -1, astring, wlen, 0, 0);
		szNewMsg = cpp_encrypt(ptr, astring);
	}

	return szNewMsg;
}


// decode message from UTF8 if need, return ANSIzUCS2z
LPSTR __cdecl cpp_decode(HANDLE context, LPCSTR szEncMsg) {

	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return NULL;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA) ptr->pdata;
	if(!p->KeyX) { ptr->error = ERROR_NO_KEYX; return NULL; }

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = cpp_decrypt(ptr, szEncMsg);

	if(szOldMsg) {
		if(ptr->features & FEATURES_UTF8) {
			// utf8 message: convert to unicode -> ansii
			LPWSTR wstring = utf8decode(szOldMsg);
			int wlen = wcslen(wstring)+1;
			szNewMsg = (LPSTR) malloc(wlen*(sizeof(WCHAR)+2));			// work.zy@gmail.com
			WideCharToMultiByte(CP_ACP, 0, wstring, -1, szNewMsg, wlen, 0, 0);
			memcpy(szNewMsg+strlen(szNewMsg)+1, wstring, wlen*sizeof(WCHAR));	// work.zy@gmail.com
		}
		else {
			// ansi message: convert to unicode
			int slen = strlen(szOldMsg)+1;
			szNewMsg = (LPSTR) malloc(slen*(sizeof(WCHAR)+1));
			memcpy(szNewMsg,szOldMsg,slen);
			WCHAR* wstring = (LPWSTR) alloca(slen*sizeof(WCHAR));
			MultiByteToWideChar(CP_ACP, 0, szOldMsg, -1, wstring, slen*sizeof(WCHAR));
			memcpy(szNewMsg+slen,wstring,slen*sizeof(WCHAR));
		}
	}
	SAFE_FREE(ptr->tmp);
	ptr->tmp = szNewMsg;
	return szNewMsg;
}


// decode message return UTF8z
LPSTR __cdecl cpp_decodeU(HANDLE context, LPCSTR szEncMsg) {

	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return NULL;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA) ptr->pdata;
	if(!p->KeyX) { ptr->error = ERROR_NO_KEYX; return NULL; }

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = cpp_decrypt(ptr, szEncMsg);

	if(szOldMsg) {
		if(ptr->features & FEATURES_UTF8) {
			// utf8 message: copy
			szNewMsg = _strdup(szOldMsg);
		}
		else {
			// ansi message: convert to utf8
			int slen = strlen(szOldMsg)+1;
			LPWSTR wstring = (LPWSTR) alloca(slen*sizeof(WCHAR));
			MultiByteToWideChar(CP_ACP, 0, szOldMsg, -1, wstring, slen*sizeof(WCHAR));
			szNewMsg = _strdup(utf8encode(wstring));
		}
	}
	SAFE_FREE(ptr->tmp);
	ptr->tmp = szNewMsg;
	return szNewMsg;
}


int __cdecl cpp_encrypt_file(HANDLE context,LPCSTR file_in,LPCSTR file_out) {

	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return 0;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA) ptr->pdata;
	if(!p->KeyX) return 0;

	try{
		CBC_Mode<AES>::Encryption enc(p->KeyX,Tiger::DIGESTSIZE,IV);
		FileSource *f = new FileSource(file_in,true,new StreamTransformationFilter (enc,new FileSink(file_out)));
		delete f;
	}
	catch (...) {
		return 0;
	}
	return 1;
}


int __cdecl cpp_decrypt_file(HANDLE context,LPCSTR file_in,LPCSTR file_out) {

	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return 0;
	cpp_alloc_pdata(ptr); pSIMDATA p = (pSIMDATA) ptr->pdata;
	if(!p->KeyX) return 0;

	try{
		CBC_Mode<AES>::Decryption dec(p->KeyX,Tiger::DIGESTSIZE,IV);
		FileSource *f = new FileSource(file_in,true,new StreamTransformationFilter (dec,new FileSink(file_out)));
		delete f;
	}
	catch (...) {
		return 0;
	}
	return 1;
}


// EOF
