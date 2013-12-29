// (C) Artem Shpynov aka FYR and Igonin Vitaliy aka chaos.persei, 2007 - 2008

#include "commonheaders.h"

int wrongPass = 0;
void* key;

Cryptor* CryptoEngine = NULL;

LIST<CryptoModule> arCryptors(1);

void zero_fill(BYTE * pBuf, size_t bufSize)
{
	size_t i;
	for (i = 0; i < bufSize; i++)
		pBuf[i] = 0;
}

void InitSecurity()
{
	Cryptor* (__stdcall *GetCryptor)();	
	
	TCHAR tszPath[MAX_PATH];
	GetModuleFileName(g_hInst, tszPath, SIZEOF(tszPath));
	TCHAR *p = _tcsrchr(tszPath, '\\')+1; _tcscpy(p, _T("cryptors\\*.dll"));

	WIN32_FIND_DATA fd;
	HANDLE hFile = FindFirstFile(tszPath, &fd);	
	while (hFile != INVALID_HANDLE_VALUE) {	
		mir_sntprintf(p, MAX_PATH - (p-tszPath), _T("cryptors\\%s"), fd.cFileName);
		HMODULE hLib = LoadLibrary(tszPath);
		if (hLib) {
			GetCryptor = (Cryptor* (__stdcall *)()) GetProcAddress(hLib, "GetCryptor");
			if (GetCryptor) {
				CryptoModule* newItem = (CryptoModule*) malloc(sizeof(CryptoModule));
				newItem->cryptor = GetCryptor();
				_tcsncpy(newItem->dllname, fd.cFileName, MAX_PATH);
				newItem->hLib = hLib;
				arCryptors.insert(newItem);
			}
			else FreeLibrary(hLib);
		}
		if (!FindNextFile(hFile, &fd))
			break;
	}
}

void UnloadSecurity()
{
	if (CryptoEngine)
		CryptoEngine->FreeKey(key);

	for (int i = 0; i < arCryptors.getCount(); i++) {
		FreeLibrary(arCryptors[i]->hLib);
		free(arCryptors[i]);
	}
	arCryptors.destroy();
}

void CDbxMmapSA::EncoderInit()
{
	if (!m_bEncoding) return;

	encryptKey[encryptKeyLength] = 0;
	key = CryptoEngine->GenerateKey(encryptKey);
}

void CDbxMmapSA::EncodeCopyMemory(void * dst, void * src, size_t size)
{
	memcpy(dst, src, size);

	if (m_bEncoding)
		CryptoEngine->EncryptMem((BYTE *)dst, (int)size, key);
}

void CDbxMmapSA::DecodeCopyMemory(void * dst, void * src, size_t size)
{
	memcpy(dst, src, size);

	if (m_bEncoding)
		CryptoEngine->DecryptMem((BYTE *)dst, (int)size, key);
}

void CDbxMmapSA::EncodeDBWrite(DWORD ofs, void *src, int size)
{
	if (m_bEncoding) {
		BYTE *buf = (BYTE*)GlobalAlloc(GPTR, sizeof(BYTE)*size);
		EncodeCopyMemory(buf, src, size);
		DBWrite(ofs, buf, (int)size);
		GlobalFree(buf);
	}
	else DBWrite(ofs, src, (int)size);
}

void CDbxMmapSA::DecodeDBWrite(DWORD ofs, void *src, int size)
{
	if (m_bEncoding) {
		BYTE *buf = (BYTE*)GlobalAlloc(GPTR, sizeof(BYTE)*size);
		DecodeCopyMemory(buf, src, size);
		DBWrite(ofs, buf, (int)size);
		GlobalFree(buf);
	}
	else DBWrite(ofs, src, (int)size);
}

int bCheckingPass = 0;

int CDbxMmapSA::CheckPassword(WORD checkWord, TCHAR *szDBName)
{
	if (bCheckingPass)
		return 0;

	bCheckingPass = 1;

	int Found = 0;
	for (int i = 0; i < arCryptors.getCount(); i++) {
		if (HIWORD(m_dbHeader.version) == arCryptors[i]->cryptor->uid) {
			CryptoEngine = arCryptors[i]->cryptor;
			Found = 1;
			break;
		}
	}
	if (!Found) {
		MessageBox(0, TranslateT("Sorry, but your database encrypted with unknown module"), TranslateT("Error"), MB_OK | MB_ICONERROR);
		bCheckingPass = 0;
		return 0;
	}

	while (1) {
		DlgStdInProcParam param = { this, szDBName };
		int res = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_LOGIN), NULL, DlgStdInProc, (LPARAM)&param);
		if (res == IDCANCEL) {
			wrongPass = 0;
			bCheckingPass = 0;
			return 0;
		}
		if (encryptKeyLength < 1)
			continue;

		EncoderInit();

		WORD ver;
		DecodeCopyMemory(&ver, &checkWord, sizeof(checkWord));
		if (ver == 0x5195) {
			wrongPass = 0;
			bCheckingPass = 0;
			return 1;
		}
		wrongPass++;
	}

	bCheckingPass = 0;
}

int SelectEncoder()
{
	if (arCryptors.getCount() == 0) {
		MessageBox(0, TranslateT("Crypto modules not found"), TranslateT("Error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	WORD uid = db_get_w(NULL, "SecureMMAP", "CryptoModule", 0);
	if (uid != 0) {
		for (int i = 0; i < arCryptors.getCount(); i++) {
			if (arCryptors[i]->cryptor->uid == uid) {
				CryptoEngine = arCryptors[i]->cryptor;
				return 0;
			}
		}
	}

	db_set_w(NULL, "SecureMMAP", "CryptoModule", arCryptors[0]->cryptor->uid);
	CryptoEngine = arCryptors[0]->cryptor;
	return 0;
}

void CDbxMmapSA::EncodeAll()
{
	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		EncodeContactEvents(hContact);
		EncodeContactSettings(hContact);
	}

	EncodeContactEvents(NULL);
	EncodeContactSettings(NULL);
}

void CDbxMmapSA::DecodeAll()
{
	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		DecodeContactEvents(hContact);
		DecodeContactSettings(hContact);
	}
	DecodeContactEvents(NULL);
	DecodeContactSettings(NULL);
}

void CDbxMmapSA::WritePlainHeader()
{
	DWORD bytesWritten;

	memcpy(m_dbHeader.signature, &dbSignatureNonSecured, sizeof(m_dbHeader.signature));
	SetFilePointer(m_hDbFile, 0, NULL, FILE_BEGIN);
	WriteFile(m_hDbFile, m_dbHeader.signature, sizeof(m_dbHeader.signature), &bytesWritten, NULL);

	m_dbHeader.version = MAKELONG(0x0700, 0x0000); //no encryption
	WriteFile(m_hDbFile, &m_dbHeader.version, sizeof(m_dbHeader.version), &bytesWritten, NULL);
}

void CDbxMmapSA::WriteCryptHeader()
{
	DWORD bytesWritten;

	memcpy(m_dbHeader.signature, &dbSignatureSecured, sizeof(m_dbHeader.signature));
	SetFilePointer(m_hDbFile, 0, NULL, FILE_BEGIN);
	WriteFile(m_hDbFile, m_dbHeader.signature, sizeof(m_dbHeader.signature), &bytesWritten, NULL);

	WORD checkWord = 0x5195, cryptWord;
	EncodeCopyMemory(&cryptWord, &checkWord, sizeof(checkWord));
	m_dbHeader.version = MAKELONG(cryptWord, CryptoEngine->uid);
	WriteFile(m_hDbFile, &m_dbHeader.version, sizeof(m_dbHeader.version), &bytesWritten, NULL);
}

void CDbxMmapSA::EncryptDB()
{
	int action = 0;
	if (bEncProcess)
		return;

	if (memcmp(m_dbHeader.signature, &dbSignatureSecured, sizeof(m_dbHeader.signature)) == 0) {
		MessageBox(0, TranslateT("DB is already secured!"), TranslateT("Error"), MB_OK | MB_ICONERROR);
		return;
	}

	if (SelectEncoder())
		return;

	bEncProcess = 1;

	action = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_NEWPASS), NULL, DlgStdNewPass, (LPARAM)this);
	if (action != IDOK || !strlen(encryptKey)) {
		bEncProcess = 0;
		db_set_b(NULL, "SecureMMAP", "CryptoModule", 0);
		return;
	}

	EnterCriticalSection(&m_csDbAccess);

	m_bEncoding = 1;
	EncoderInit();

	EncodeAll();

	LeaveCriticalSection(&m_csDbAccess);

	WriteCryptHeader();

	xModifyMenu(hSetPwdMenu, 0, LPGENT("Change Password"), 0);

	bEncProcess = 0;
}

void CDbxMmapSA::DecryptDB()
{
	char oldKey[255];
	strcpy(oldKey, encryptKey);

	if (!CheckPassword(LOWORD(m_dbHeader.version), TranslateT("current database"))) {
		strcpy(encryptKey, oldKey);
		encryptKeyLength = strlen(oldKey);
		return;
	}

	WritePlainHeader();

	EnterCriticalSection(&m_csDbAccess);
	DecodeAll();
	LeaveCriticalSection(&m_csDbAccess);

	m_bEncoding = 0;

	zero_fill((BYTE *)encryptKey, sizeof encryptKey);

	xModifyMenu(hSetPwdMenu, 0, LPGENT("Set Password"), 0);

	db_set_w(NULL, "SecureMMAP", "CryptoModule", 0);

	CryptoEngine->FreeKey(key);

	CryptoEngine = NULL;
}

void CDbxMmapSA::RecryptDB()
{
	EnterCriticalSection(&m_csDbAccess);

	DecodeAll();

	CryptoEngine->FreeKey(key);

	SelectEncoder();

	m_bEncoding = 1;

	EncoderInit();

	EncodeAll();

	WriteCryptHeader();

	LeaveCriticalSection(&m_csDbAccess);
}

void CDbxMmapSA::ChangePwd()
{
	char newpass[255] = { 0 };

	DlgChangePassParam param = { this, newpass };
	int action = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHANGEPASS), NULL, DlgChangePass, (LPARAM)&param);

	if (action == IDCANCEL || (action == IDOK && !strlen(newpass)))
		return;

	EnterCriticalSection(&m_csDbAccess);

	DecodeAll();

	CryptoEngine->FreeKey(key);

	if (action == IDREMOVE) {
		WritePlainHeader();

		m_bEncoding = 0;
		CryptoEngine = NULL;
		db_set_w(NULL, "SecureMMAP", "CryptoModule", 0);

		zero_fill((BYTE *)encryptKey, sizeof encryptKey);

		xModifyMenu(hSetPwdMenu, 0, LPGENT("Set Password"), 0);
	}

	if (action == IDOK) {
		strcpy(encryptKey, newpass);
		encryptKeyLength = strlen(newpass);

		m_bEncoding = 1;

		EncoderInit();

		EncodeAll();

		WriteCryptHeader();
	}

	zero_fill((BYTE *)newpass, sizeof newpass);

	LeaveCriticalSection(&m_csDbAccess);
}
