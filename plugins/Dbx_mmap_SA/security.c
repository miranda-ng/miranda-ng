// (C) Artem Shpynov aka FYR and Igonin Vitaliy aka chaos.persei, 2007 - 2008

#include "commonheaders.h"

BOOL bEncoding;
BOOL bEncProcess = 0;

extern HINSTANCE g_hInst;

extern HANDLE hSetPwdMenu = NULL;

char  encryptKey[255];
size_t encryptKeyLength;

int wrongPass = 0;
void* key;

Cryptor* CryptoEngine = NULL;

int ModulesCount = 0;
CryptoModule* Modules[100];


void zero_fill(BYTE * pBuf, size_t bufSize)
{
	size_t i;
	for(i = 0; i < bufSize; i++)
		pBuf[i] = 0;
}

void InitSecurity()
{
	HMODULE hLib;	
	WIN32_FIND_DATAA fd;

	Cryptor* (__stdcall *GetCryptor)();	
	
	HANDLE hFile = FindFirstFileA(".\\plugins\\cryptors\\*.dll", &fd);	

	ModulesCount = 0;
	while (hFile != INVALID_HANDLE_VALUE)
	{	
		char tmp[255];
		strcpy(tmp, ".\\plugins\\cryptors\\");
		strcat(tmp, fd.cFileName);
		
		hLib = LoadLibraryA(tmp);
		if(hLib){
			GetCryptor = (Cryptor* (__stdcall *)()) GetProcAddress(hLib, "GetCryptor");
			if(GetCryptor){
				Modules[ModulesCount] = (CryptoModule*) malloc(sizeof(CryptoModule));
				Modules[ModulesCount]->cryptor = GetCryptor();
				strcpy(Modules[ModulesCount]->dllname, fd.cFileName);
				Modules[ModulesCount]->hLib = hLib;
				
				ModulesCount++;
			}else{
				FreeLibrary(hLib);
			}
		}
		if(ModulesCount >= 100) break;
		if (!FindNextFileA(hFile, &fd)) break;
	}
}

void UnloadSecurity()
{
	int i;

	if(CryptoEngine) CryptoEngine->FreeKey(key);
	
	for(i = 0; i < ModulesCount; i++)
	{
		FreeLibrary(Modules[i]->hLib);
		free(Modules[i]);
	}
}

void EncoderInit()
{
	if (!bEncoding) return;
	
	encryptKey[encryptKeyLength] = 0;
	key = CryptoEngine->GenerateKey(encryptKey);
}

void EncodeCopyMemory(void * dst, void * src, size_t size )
{
	memcpy(dst, src, size);
	
	if (!bEncoding)
		return;
	
	CryptoEngine->EncryptMem(dst, (int)size, key);
}

void DecodeCopyMemory(void * dst, void * src, size_t size )
{
	memcpy(dst, src, size);
	
	if (!bEncoding)
		return;

	CryptoEngine->DecryptMem(dst, (int)size, key);
}

void EncodeDBWrite(DWORD ofs, void * src, size_t size)
{	
	if(bEncoding)
	{
		BYTE * buf;
		
		buf = (BYTE*)GlobalAlloc(GPTR, sizeof(BYTE)*size);
		EncodeCopyMemory(buf, src, size);
		DBWrite(ofs, buf, (int)size);
		GlobalFree(buf);
	}
	else
	{
		DBWrite(ofs, src, (int)size);
	}
}

void DecodeDBWrite(DWORD ofs, void * src, size_t size)
{
	
	if(bEncoding)
	{
		BYTE * buf;

		buf = (BYTE*)GlobalAlloc(GPTR, sizeof(BYTE)*size);
		DecodeCopyMemory(buf, src, size);
		DBWrite(ofs, buf, (int)size);
		GlobalFree(buf);
	}
	else
	{
		DBWrite(ofs, src, (int)size);
	}
}

int bCheckingPass = 0;

int CheckPassword(WORD checkWord, char * szDBName)
{
	WORD ver;
	int res;

	if(bCheckingPass) return 0;
	bCheckingPass = 1;

	{
		int i;
		int Found = 0;
		for(i = 0; i < ModulesCount; i++) {
			if(dbHeader.cryptorUID == Modules[i]->cryptor->uid){
				CryptoEngine = Modules[i]->cryptor;
				Found = 1;
				break;
			}
		}
		if (!Found){
			MessageBoxA(0, "Sorry, but your database encrypted with unknown module", "Error", MB_OK);
			bCheckingPass = 0;
			return 0;
		}
	}

	while(1){
		res = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_LOGIN), NULL, (DLGPROC)DlgStdInProc, (LPARAM)szDBName);
		if(res == IDCANCEL)
		{
			wrongPass = 0;
			bCheckingPass = 0;
			return 0;
		}
		if(encryptKeyLength < 1) continue;
		EncoderInit();
		DecodeCopyMemory(&ver, &checkWord, sizeof(checkWord));
		if(ver == 0x5195)
		{
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
	WORD uid;
	int i;
	
	if(ModulesCount == 0){
		MessageBox(0, TranslateT("Crypto modules not found"), TranslateT("Error"), MB_OK);
		return 1;
	}

	uid = DBGetContactSettingWord(NULL, "SecureMMAP", "CryptoModule", 0);
	
	if(uid == 0){
		MessageBox(0, TranslateT("Crypto module hasn't been chosen, using first one found"), TranslateT("Notice"), MB_OK);
		DBWriteContactSettingWord(NULL, "SecureMMAP", "CryptoModule", Modules[0]->cryptor->uid);
		CryptoEngine = Modules[0]->cryptor;
	}
	else{
		int Found = 0;
		for(i = 0; i < ModulesCount; i++) {
			if(Modules[i]->cryptor->uid == uid){
				CryptoEngine = Modules[i]->cryptor;
				Found = 1;
				break;
			}
		}
		if (!Found){
			MessageBox(0, TranslateT("Crypto module hasn't been chosen, using first one found"), TranslateT("Notice"), MB_OK);
			DBWriteContactSettingWord(NULL, "SecureMMAP", "CryptoModule", Modules[0]->cryptor->uid);
			CryptoEngine = Modules[0]->cryptor;
		}
	}
	
	return 0;
}

void EncodeAll()
{
	HANDLE hContact;

	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	if(hContact){
		do{
			EncodeContactEvents(hContact);
			EncodeContactSettings(hContact);
		}while(hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0));
	}

	EncodeContactEvents(NULL);	
	EncodeContactSettings(NULL);
}

void DecodeAll()
{
	HANDLE hContact;	
	
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	if(hContact){
		do{
			DecodeContactEvents(hContact);
			DecodeContactSettings(hContact);
		}while(hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0));
	}
	DecodeContactEvents(NULL);
	DecodeContactSettings(NULL);
}

void WritePlainHeader()
{
	DWORD bytesWritten;

	memcpy(dbHeader.signature, &dbSignature, sizeof(dbHeader.signature));
	SetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
	WriteFile(hDbFile,dbHeader.signature,sizeof(dbHeader.signature),&bytesWritten,NULL);
	{
		WORD checkWord;
		checkWord = 0x0700;
		memcpy(&dbHeader.checkWord, &checkWord, sizeof(checkWord));
		WriteFile(hDbFile,&dbHeader.checkWord,sizeof(dbHeader.checkWord),&bytesWritten,NULL);

		dbHeader.cryptorUID = 0x0000; //no encryption
		WriteFile(hDbFile,&dbHeader.cryptorUID,sizeof(dbHeader.cryptorUID),&bytesWritten,NULL);
	}
}

void WriteCryptHeader()
{
	DWORD bytesWritten;

	memcpy(dbHeader.signature, &dbSignatureSecured, sizeof(dbHeader.signature));
	SetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
	WriteFile(hDbFile,dbHeader.signature,sizeof(dbHeader.signature),&bytesWritten,NULL);
	{
		WORD checkWord;
		checkWord = 0x5195;
		EncodeCopyMemory(&dbHeader.checkWord, &checkWord, sizeof(checkWord));
		WriteFile(hDbFile,&dbHeader.checkWord,sizeof(dbHeader.checkWord),&bytesWritten,NULL);

		dbHeader.cryptorUID = CryptoEngine->uid;
		WriteFile(hDbFile,&dbHeader.cryptorUID,sizeof(dbHeader.cryptorUID),&bytesWritten,NULL);
	}
}

void EncryptDB()
{
	int action = 0;
	if(bEncProcess) return;		

	if(memcmp(dbHeader.signature, &dbSignatureSecured, sizeof(dbHeader.signature)) == 0){
		MessageBox(0, TranslateT("DB is already secured!"), TranslateT("Error"), MB_OK);
		return;
	}

	if(SelectEncoder()) {		
		return;
	}
	
	bEncProcess = 1;

	action = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_NEWPASS), NULL, (DLGPROC)DlgStdNewPass, (LPARAM)NULL);
	if(action != IDOK || !strlen(encryptKey)) {
		bEncProcess = 0;		
		DBWriteContactSettingByte(NULL, "SecureMMAP", "CryptoModule", 0);		
		return;
	}

	EnterCriticalSection(&csDbAccess);

	bEncoding = 1;
	EncoderInit();

	EncodeAll();

	LeaveCriticalSection(&csDbAccess);		

	WriteCryptHeader();

	xModifyMenu(hSetPwdMenu, 0, LPGENT("Change Password"), 0);

	bEncProcess = 0;
}

void DecryptDB()
{
	char oldKey[255];
	strcpy(oldKey, encryptKey);

	if (!CheckPassword(dbHeader.checkWord, Translate("current database"))){strcpy(encryptKey, oldKey); encryptKeyLength = strlen(oldKey); return;}

	WritePlainHeader();
	
	EnterCriticalSection(&csDbAccess);
	DecodeAll();
	LeaveCriticalSection(&csDbAccess);

	bEncoding = 0;

	zero_fill(encryptKey, sizeof encryptKey);

	xModifyMenu(hSetPwdMenu, 0, LPGENT("Set Password"), 0);

	DBWriteContactSettingWord(NULL, "SecureMMAP", "CryptoModule", 0);

	CryptoEngine->FreeKey(key);

	CryptoEngine = NULL;
}

void RecryptDB()
{
	EnterCriticalSection(&csDbAccess);

	DecodeAll();
	
	CryptoEngine->FreeKey(key);

	SelectEncoder();

	bEncoding = 1;

	EncoderInit();	

	EncodeAll();	

	WriteCryptHeader();

	LeaveCriticalSection(&csDbAccess);
}

void ChangePwd()
{
	char newpass[255] = {0};
	
	int action = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHANGEPASS), NULL, (DLGPROC)DlgChangePass, (LPARAM)newpass);	
	
	if(action == IDCANCEL || (action == IDOK && !strlen(newpass)))
		return;

	EnterCriticalSection(&csDbAccess);

	DecodeAll();

	CryptoEngine->FreeKey(key);

	if(action == IDREMOVE){
		WritePlainHeader();

		bEncoding = 0;
		CryptoEngine = NULL;
		DBWriteContactSettingWord(NULL, "SecureMMAP", "CryptoModule", 0);

		zero_fill(encryptKey, sizeof encryptKey);

		xModifyMenu(hSetPwdMenu, 0, LPGENT("Set Password"), 0);
	}

	if(action == IDOK){
		strcpy(encryptKey, newpass);
		encryptKeyLength = strlen(newpass);		
		
		bEncoding = 1;

		EncoderInit();	

		EncodeAll();		

		WriteCryptHeader();
	}

	zero_fill(newpass, sizeof newpass);

	LeaveCriticalSection(&csDbAccess);
}