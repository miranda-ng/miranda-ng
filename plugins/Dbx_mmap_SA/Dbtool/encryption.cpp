// (C) Artem Shpynov aka FYR and Igonin Vitaliy aka chaos.persei, 2007 - 2008

#include "dbtool.h"

typedef struct{
	void* (__stdcall *GenerateKey)(char* pwd);
	void (__stdcall *FreeKey)(void* key);
	void (__stdcall *EncryptMem)(BYTE* data, int size, void* key);
	void (__stdcall *DecryptMem)(BYTE* data, int size, void* key);

    char* Name;
    char* Info;
    char* Author;
    char* Site;
    char* Email;

	DWORD Version;

	WORD uid;
} Cryptor;

typedef struct{
	char dllname[255];
	HMODULE hLib;
	Cryptor* cryptor;
} CryptoModule;

BOOL CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

	{	
		TCHAR szMirandaDir[MAX_PATH];
		szMirandaDir[ 0 ] = 0;
		TCHAR *str2;
		GetModuleFileName(NULL,szMirandaDir,SIZEOF(szMirandaDir));
		str2 = _tcsrchr(szMirandaDir,'\\');
		if( str2 != NULL )
			*str2=0;
		_tchdir(szMirandaDir);
	}
	
	HANDLE hFile = FindFirstFileA(".\\plugins\\cryptors\\*.dll", &fd);	

	AddToStatus(STATUS_MESSAGE,TranslateT("Scanning cryptors directory"));
	
	ModulesCount = 0;
	while (hFile != INVALID_HANDLE_VALUE)
	{	
		char tmp[MAX_PATH], buf[255];
		strcpy(tmp, ".\\plugins\\cryptors\\");
		strcat(tmp, fd.cFileName);
		
		hLib = LoadLibraryA(tmp);
		if(hLib){
			GetCryptor = (Cryptor* (__stdcall *)()) GetProcAddress(hLib, "GetCryptor");
			if(GetCryptor){
				TCHAR Name[100], Version[100], DllName[100];
				

				Modules[ModulesCount] = (CryptoModule*) malloc(sizeof(CryptoModule));
				Modules[ModulesCount]->cryptor = GetCryptor();
				strcpy(Modules[ModulesCount]->dllname, fd.cFileName);
				Modules[ModulesCount]->hLib = hLib;
				
				_snprintf(buf,SIZEOF(buf),"%d.%d.%d.%d", HIBYTE(HIWORD(Modules[ModulesCount]->cryptor->Version)), LOBYTE(HIWORD(Modules[ModulesCount]->cryptor->Version)), HIBYTE(LOWORD(Modules[ModulesCount]->cryptor->Version)), LOBYTE(LOWORD(Modules[ModulesCount]->cryptor->Version)));
#ifdef _UNICODE
				mbstowcs(Name, Modules[ModulesCount]->cryptor->Name, 100);
				mbstowcs(Version, buf, 100);
				mbstowcs(DllName, Modules[ModulesCount]->dllname, 100);
#else
				strcpy(Name, Modules[ModulesCount]->cryptor->Name);
				strcpy(Version, buf);
				strcpy(DllName, Modules[ModulesCount]->dllname);
#endif
				AddToStatus(STATUS_MESSAGE,TranslateT("Cryptor loaded: %s [%s] (%s)"), Name, Version, DllName);
				
				ModulesCount++;
			}else{
				FreeLibrary(hLib);
			}
		}
		if(ModulesCount >= 100) break;
		if(!FindNextFileA(hFile, &fd)) break;
	}

	AddToStatus(STATUS_MESSAGE,TranslateT("%d crypto modules loaded"), ModulesCount);
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
	encryptKey[encryptKeyLength] = 0;
	key = CryptoEngine->GenerateKey(encryptKey);
}

void EncodeCopyMemory(BYTE * dst, void * src, size_t size )
{
	memcpy(dst, src, size);
	CryptoEngine->EncryptMem(dst, (int)size, key);
}

void DecodeCopyMemory(BYTE * dst, void * src, size_t size )
{
	memcpy(dst, src, size);
	CryptoEngine->DecryptMem(dst, (int)size, key);
}


void EncodeMemory(BYTE * mem, size_t size)
{
	CryptoEngine->EncryptMem(mem, (int)size, key);
}

void DecodeMemory(BYTE * mem, size_t size)
{
	CryptoEngine->DecryptMem(mem, (int)size, key);
}

int bCheckingPass = 0;

int CheckPassword(WORD checkWord, WORD cryptorUID, char * szDBName)
{
	WORD ver;
	int res;

	if(bCheckingPass) return 0;
	bCheckingPass = 1;

	{
		int i;
		int Found = 0;
		for(i = 0; i < ModulesCount; i++){
			if(cryptorUID == Modules[i]->cryptor->uid){
				CryptoEngine = Modules[i]->cryptor;
				Found = 1;
				break;
			}
		}
		if(!Found){
			AddToStatus(STATUS_FATAL, TranslateT("Sorry, but your database encrypted with unknown module"), MB_OK);
			bCheckingPass = 0;
			return 0;
		}
	}

	{
		wchar_t Name[100], Author[100];

		mbstowcs(Name, CryptoEngine->Name, 100);
		mbstowcs(Author, CryptoEngine->Author, 100);

		AddToStatus(STATUS_MESSAGE, TranslateT("Database encrypted with %s by %s"), Name, Author);
	}

	while(1){
		res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LOGIN), NULL, (DLGPROC)DlgStdInProc, (LPARAM)szDBName);
		if(res == IDCANCEL)
		{
			wrongPass = 0;
			bCheckingPass = 0;
			return 0;
		}
		if(encryptKeyLength < 1) continue;
		EncoderInit();
		DecodeCopyMemory((BYTE*)&ver, &checkWord, sizeof(checkWord));
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


UINT oldLangID = 0;
void LanguageChanged(HWND hDlg)
{
	UINT LangID = (UINT)GetKeyboardLayout(0);
	char Lang[3] = {0};
	if (LangID != oldLangID)
	{
		oldLangID = LangID;
		GetLocaleInfoA(MAKELCID((LangID & 0xffffffff),  SORT_DEFAULT),  LOCALE_SABBREVLANGNAME, Lang, 2);
		SetDlgItemTextA(hDlg, IDC_LANG, Lang);
	}
}

BOOL CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hwndCtrl;
//			if(pluginLink && ServiceExists(MS_LANGPACK_TRANSLATEDIALOG))
			TranslateDialog(hDlg);

			if(lParam && !wrongPass) SetDlgItemTextA(hDlg, IDC_DBNAME, (LPCSTR)lParam);
			if(wrongPass) 
			{
				if (wrongPass > 2)
				{
					hwndCtrl = GetDlgItem(hDlg, IDC_USERPASS);
					EnableWindow(hwndCtrl, FALSE);
					hwndCtrl = GetDlgItem(hDlg, IDOK);
					EnableWindow(hwndCtrl, FALSE);
					
					SetDlgItemText(hDlg, IDC_LOGININFO, TranslateT("Too many errors!"));

				}
				else
				{
					SetDlgItemText(hDlg, IDC_LOGININFO, TranslateT("Password is not correct!"));
				}
			}
			oldLangID = 0;
			SetTimer(hDlg,1,200,NULL);
			LanguageChanged(hDlg);
			return TRUE;
		}

	case WM_CTLCOLORSTATIC:
		{
			if ((HWND)lParam == GetDlgItem(hDlg, IDC_LANG))
			{
				SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetSysColorBrush(COLOR_HIGHLIGHT);
			}

			return FALSE;
		}

	case WM_COMMAND:
		{
			UINT uid = LOWORD(wParam);

			if(uid == IDOK){
				if(!GetWindowLongPtr(hDlg,GWLP_USERDATA))
				{
					encryptKeyLength = GetDlgItemTextA(hDlg, IDC_USERPASS, encryptKey, 254);
					EndDialog(hDlg,IDOK);
				}else{
					
				}
			}else if(uid == IDCANCEL){
				EndDialog(hDlg,IDCANCEL);
			}
		}
	case WM_TIMER:
		{
			LanguageChanged(hDlg);
			return FALSE;
		}
	case WM_DESTROY:
		{
			KillTimer(hDlg, 1);
			return FALSE;
		}
	}

	return FALSE;
}
