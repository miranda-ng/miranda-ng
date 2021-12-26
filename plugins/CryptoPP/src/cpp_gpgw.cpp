#include "commonheaders.h"


HMODULE hgpg;
HRSRC	hRS_gpg;
uint8_t *pRS_gpg;

int   __cdecl _gpg_init(void);
int   __cdecl _gpg_done(void);
int   __cdecl _gpg_open_keyrings(LPSTR, LPSTR);
int   __cdecl _gpg_close_keyrings(void);
void  __cdecl _gpg_set_log(LPCSTR);
void  __cdecl _gpg_set_tmp(LPCSTR);
LPSTR __cdecl _gpg_get_error(void);
int   __cdecl _gpg_size_keyid(void);
int   __cdecl _gpg_select_keyid(HWND, LPSTR);
LPSTR __cdecl _gpg_encrypt(LPCSTR, LPCSTR);
LPSTR __cdecl _gpg_decrypt(LPCSTR);
LPSTR __cdecl _gpg_get_passphrases();
void  __cdecl _gpg_set_passphrases(LPCSTR);

int __cdecl gpg_init()
{
	hgpg = g_plugin.getInst();
	return _gpg_init();
}

int __cdecl gpg_done()
{
	int r = 0;
	if (hgpg) {
		r = _gpg_done();
		hgpg = nullptr;
	}
	return r;
}

int __cdecl gpg_open_keyrings(LPSTR ExecPath, LPSTR HomePath)
{
	return _gpg_open_keyrings(ExecPath, HomePath);
}

int __cdecl gpg_close_keyrings()
{
	return _gpg_close_keyrings();
}

void __cdecl gpg_set_log(LPCSTR LogPath)
{
	_gpg_set_log(LogPath);
}

void __cdecl gpg_set_tmp(LPCSTR TmpPath)
{
	_gpg_set_tmp(TmpPath);
}

LPSTR __cdecl gpg_get_error()
{
	return _gpg_get_error();
}

LPSTR __cdecl gpg_encrypt(pCNTX ptr, LPCSTR szPlainMsg)
{
	ptr->error = ERROR_NONE;
	pGPGDATA p = (pGPGDATA)ptr->pdata;

	LPSTR szEncMsg = _gpg_encrypt(szPlainMsg, (LPCSTR)p->gpgKeyID);
	mir_free(ptr->tmp);
	if (!szEncMsg) {
		return ptr->tmp = nullptr;
	}
	else {
		ptr->tmp = mir_strdup(szEncMsg);
		LocalFree((LPVOID)szEncMsg);
		return ptr->tmp;
	}
}

LPSTR __cdecl gpg_decrypt(pCNTX ptr, LPCSTR szEncMsg)
{
	ptr->error = ERROR_NONE;

	LPSTR szPlainMsg = _gpg_decrypt(szEncMsg);
	mir_free(ptr->tmp);
	ptr->tmp = mir_strdup(szPlainMsg);
	LocalFree((LPVOID)szPlainMsg);

	return ptr->tmp;
}

LPSTR __cdecl gpg_encode(HANDLE context, LPCSTR szPlainMsg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr)
		return nullptr;
	pGPGDATA p = (pGPGDATA)cpp_alloc_pdata(ptr);
	if (!p->gpgKeyID) {
		ptr->error = ERROR_NO_GPG_KEY;
		return nullptr;
	}

	// utf8 message: encrypt.
	LPSTR szUtfMsg;
	if (ptr->mode & MODE_GPG_ANSI) {
		LPWSTR wszMsg = utf8decode(szPlainMsg);
		int wlen = (int)wcslen(wszMsg) + 1;
		szUtfMsg = (LPSTR)alloca(wlen);
		WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szUtfMsg, wlen, nullptr, nullptr);
	}
	else {
		szUtfMsg = (LPSTR)szPlainMsg;
	}
	return gpg_encrypt(ptr, szUtfMsg);
}


LPSTR __cdecl gpg_decode(HANDLE context, LPCSTR szEncMsg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr)
		return nullptr;

	LPSTR szNewMsg = nullptr;
	LPSTR szOldMsg = gpg_decrypt(ptr, szEncMsg);

	if (szOldMsg) {
		if (!is_7bit_string(szOldMsg) && !is_utf8_string(szOldMsg)) {
			szNewMsg = mir_utf8encode(szOldMsg);
		}
		else {
			szNewMsg = mir_strdup(szOldMsg);
		}
	}
	mir_free(ptr->tmp);
	return ptr->tmp = szNewMsg;
}

int __cdecl gpg_set_key(HANDLE context, LPCSTR RemoteKey)
{
	/*
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return 0;
	ptr->error = ERROR_NONE;

	if (!_gpg_check_key(RemoteKey)) return 0;

	SAFE_FREE(ptr->pgpKey);
	ptr->pgpKey = (uint8_t *) malloc(strlen(RemoteKey)+1);
	strcpy((LPSTR)ptr->pgpKey,RemoteKey);

	return 1;
	*/
	return 0;
}

int __cdecl gpg_set_keyid(HANDLE context, LPCSTR RemoteKeyID)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr)
		return 0;
	pGPGDATA p = (pGPGDATA)cpp_alloc_pdata(ptr);
	ptr->error = ERROR_NONE;

	SAFE_FREE(p->gpgKeyID);
	p->gpgKeyID = (uint8_t*)_strdup(RemoteKeyID);

	return 1;
}

int __cdecl gpg_size_keyid()
{
	return _gpg_size_keyid();
}

int __cdecl gpg_select_keyid(HWND hDlg, LPSTR szKeyID)
{
	return _gpg_select_keyid(hDlg, szKeyID);
}

LPSTR __cdecl gpg_get_passphrases()
{
	return _gpg_get_passphrases();
}

void __cdecl gpg_set_passphrases(LPCSTR buffer)
{
	_gpg_set_passphrases(buffer);
}
