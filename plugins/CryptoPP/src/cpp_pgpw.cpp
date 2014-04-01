#include "commonheaders.h"


HMODULE hpgpsdk;
UINT	pgpVer;
HRSRC	hRS_pgp;
PBYTE	pRS_pgp;

int   __cdecl _pgp_init(void);
int   __cdecl _pgp_done(void);
int   __cdecl _pgp_open_keyrings(LPSTR,LPSTR);
int   __cdecl _pgp_close_keyrings(void);
int   __cdecl _pgp_get_version(void);
LPSTR __cdecl _pgp_get_error(void);
int   __cdecl _pgp_size_keyid(void);
PVOID __cdecl _pgp_select_keyid(HWND,LPSTR);
LPSTR __cdecl _pgp_encrypt_keydb(LPCSTR,PVOID);
LPSTR __cdecl _pgp_decrypt_keydb(LPCSTR);
//int   __cdecl _pgp_check_key(LPCSTR);
LPSTR __cdecl _pgp_encrypt_key(LPCSTR,LPCSTR);
LPSTR __cdecl _pgp_decrypt_key(LPCSTR,LPCSTR);

int   (__cdecl *p_pgp_init)(void);
int   (__cdecl *p_pgp_done)(void);
int   (__cdecl *p_pgp_open_keyrings)(LPSTR,LPSTR);
int   (__cdecl *p_pgp_close_keyrings)(void);
int   (__cdecl *p_pgp_get_version)(void);
LPSTR (__cdecl *p_pgp_get_error)(void);
int   (__cdecl *p_pgp_size_keyid)(void);
PVOID (__cdecl *p_pgp_select_keyid)(HWND,LPSTR);
LPSTR (__cdecl *p_pgp_encrypt_keydb)(LPCSTR,PVOID);
LPSTR (__cdecl *p_pgp_decrypt_keydb)(LPCSTR);
//int   (__cdecl *p_pgp_check_key)(LPCSTR);
LPSTR (__cdecl *p_pgp_encrypt_key)(LPCSTR,LPCSTR);
LPSTR (__cdecl *p_pgp_decrypt_key)(LPCSTR,LPCSTR);

#define GPA(x)                                              \
{                                                           \
	*((PVOID*)&p##x) = (PVOID)GetProcAddress(mod, TEXT(#x)); \
	if (!p##x) return 0;                                     \
}

int load_pgpsdk_dll(HMODULE mod)
{
	GPA(_pgp_init);
	GPA(_pgp_done);
	GPA(_pgp_open_keyrings);
	GPA(_pgp_close_keyrings);
	GPA(_pgp_get_version);
	GPA(_pgp_get_error);
	GPA(_pgp_size_keyid);
	GPA(_pgp_select_keyid);
	GPA(_pgp_encrypt_keydb);
	GPA(_pgp_decrypt_keydb);
	GPA(_pgp_encrypt_key);
	GPA(_pgp_decrypt_key);

	return 1;
}

#undef GPA

#define GPA(x)                                                 \
{                                                              \
	*((PVOID*)&p##x) = (PVOID)GetProcAddress(mod, TEXT(#x)); \
	if (!p##x) {                                                \
	return 0;                                                   \
	}                                                           \
}

int load_pgpsdk_mem(HMODULE mod)
{
	GPA(_pgp_init);
	GPA(_pgp_done);
	GPA(_pgp_open_keyrings);
	GPA(_pgp_close_keyrings);
	GPA(_pgp_get_version);
	GPA(_pgp_get_error);
	GPA(_pgp_size_keyid);
	GPA(_pgp_select_keyid);
	GPA(_pgp_encrypt_keydb);
	GPA(_pgp_decrypt_keydb);
	GPA(_pgp_encrypt_key);
	GPA(_pgp_decrypt_key);
	return 1;
}

#undef GPA

int __cdecl pgp_init()
{
	if (!hPGPPRIV) {
		// create context for private pgp keys
		hPGPPRIV = (HANDLE)cpp_create_context(MODE_PGP | MODE_PRIV_KEY);
		pCNTX tmp = (pCNTX)hPGPPRIV;
		tmp->pdata = (PBYTE)malloc(sizeof(PGPDATA));
		memset(tmp->pdata, 0, sizeof(PGPDATA));
	}

	hpgpsdk = g_hInst;
	return 0;
}

int __cdecl pgp_done()
{
	int r = 0;
	pgpVer = 0;
	if (hpgpsdk) {
		r = p_pgp_done();
		hpgpsdk = 0;
	}
	return r;
}

int __cdecl pgp_open_keyrings(LPSTR PubRingPath, LPSTR SecRingPath)
{
	return p_pgp_open_keyrings(PubRingPath, SecRingPath);
}

int __cdecl pgp_close_keyrings()
{
	return p_pgp_close_keyrings();
}

int __cdecl pgp_get_version()
{
	return pgpVer;
}

LPSTR __cdecl pgp_get_error()
{
	return p_pgp_get_error();
}

LPSTR __cdecl pgp_encrypt(pCNTX ptr, LPCSTR szPlainMsg)
{
	ptr->error = ERROR_NONE;
	pPGPDATA p = (pPGPDATA)ptr->pdata;

	LPSTR szEncMsg;
	if (p->pgpKey)
		szEncMsg = p_pgp_encrypt_key(szPlainMsg, (LPCSTR)p->pgpKey);
	else
		szEncMsg = p_pgp_encrypt_keydb(szPlainMsg, p->pgpKeyID);
	if (!szEncMsg) {
		replaceStr(ptr->tmp, NULL);
		return 0;
	}

	replaceStr(ptr->tmp, mir_strdup(szEncMsg));
	LocalFree((LPVOID)szEncMsg);

	return ptr->tmp;
}

LPSTR __cdecl pgp_decrypt(pCNTX ptr, LPCSTR szEncMsg)
{
	ptr->error = ERROR_NONE;

	LPSTR szPlainMsg = p_pgp_decrypt_keydb(szEncMsg);
	if (!szPlainMsg) {
		ptr = get_context_on_id(hPGPPRIV); // find private pgp keys
		if (ptr) {
			pPGPDATA p = (pPGPDATA)ptr->pdata;
			if (p->pgpKey)
				szPlainMsg = p_pgp_decrypt_key(szEncMsg, (LPCSTR)p->pgpKey);
		}
		if (!szPlainMsg) {
			replaceStr(ptr->tmp, NULL);
			return NULL;
		}
	}

	replaceStr(ptr->tmp, mir_strdup(szPlainMsg));
	LocalFree((LPVOID)szPlainMsg);
	return ptr->tmp;
}

LPSTR __cdecl pgp_encode(HANDLE context, LPCSTR szPlainMsg)
{
	pCNTX ptr = get_context_on_id(context); if (!ptr) return NULL;
	pPGPDATA p = (pPGPDATA)cpp_alloc_pdata(ptr);
	if (!p->pgpKeyID && !p->pgpKey) {
		ptr->error = ERROR_NO_PGP_KEY;
		return NULL;
	}

	// utf8 message: encrypt.
	return pgp_encrypt(ptr, szPlainMsg);
}

LPSTR __cdecl pgp_decode(HANDLE context, LPCSTR szEncMsg)
{
	pCNTX ptr = get_context_on_id(context);
	if (!ptr) return NULL;

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = pgp_decrypt(ptr, szEncMsg);

	if (szOldMsg) {
		if (!is_7bit_string(szOldMsg) && !is_utf8_string(szOldMsg)) {
			int slen = (int)strlen(szOldMsg) + 1;
			LPWSTR wszMsg = (LPWSTR)alloca(slen*sizeof(WCHAR));
			MultiByteToWideChar(CP_ACP, 0, szOldMsg, -1, wszMsg, slen*sizeof(WCHAR));
			szNewMsg = _strdup(utf8encode(wszMsg));
		}
		else szNewMsg = _strdup(szOldMsg);
	}
	replaceStr(ptr->tmp, szNewMsg);
	return szNewMsg;
}

int __cdecl pgp_set_priv_key(LPCSTR LocalKey)
{
	return pgp_set_key(hPGPPRIV, LocalKey);
}

int __cdecl pgp_set_key(HANDLE context, LPCSTR RemoteKey)
{
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	pPGPDATA p = (pPGPDATA)cpp_alloc_pdata(ptr);
	ptr->error = ERROR_NONE;

	SAFE_FREE(p->pgpKey);
	p->pgpKey = (PBYTE)_strdup(RemoteKey);

	return 1;
}

int __cdecl pgp_set_keyid(HANDLE context, PVOID RemoteKeyID)
{
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	pPGPDATA p = (pPGPDATA)cpp_alloc_pdata(ptr);
	ptr->error = ERROR_NONE;

	SAFE_FREE(p->pgpKeyID);
	p->pgpKeyID = (PBYTE)malloc(p_pgp_size_keyid());
	memcpy(p->pgpKeyID, RemoteKeyID, p_pgp_size_keyid());

	return 1;
}

int __cdecl pgp_size_keyid()
{
	return p_pgp_size_keyid();
}

PVOID __cdecl pgp_select_keyid(HWND hDlg, LPSTR szKeyID)
{
	return p_pgp_select_keyid(hDlg, szKeyID);
}
