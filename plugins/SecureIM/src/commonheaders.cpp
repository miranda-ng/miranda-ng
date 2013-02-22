#include "commonheaders.h"

HINSTANCE g_hInst, g_hIconInst;

LPCSTR szModuleName = MODULENAME;
LPCSTR szVersionStr = MODULENAME" DLL ("__VERSION_STRING")";
char TEMP[MAX_PATH];
int  TEMP_SIZE = 0;

HANDLE g_hEvent[2], g_hMenu[15], g_hCLIcon=0, g_hFolders=0;
HANDLE *g_hService=NULL;
HANDLE *g_hHook=NULL;
int iService=0;
int iHook=0;

HICON g_hICO[ICO_CNT], g_hPOP[POP_CNT], g_hIEC[1+IEC_CNT*MODE_CNT] = {0};
HANDLE g_IEC[1+IEC_CNT*MODE_CNT];

int iBmpDepth;
BOOL bMetaContacts = false, bPopupExists = false;
BOOL bPGPloaded = false, bPGPkeyrings = false, bUseKeyrings = false, bPGPprivkey = false;
BOOL bGPGloaded = false, bGPGkeyrings = false, bSavePass = false;
BOOL bSFT, bSOM, bASI, bMCD, bSCM, bDGP, bAIP, bNOL, bAAK, bMCM;
BYTE bPGP, bGPG;
DWORD iCoreVersion = 0;
CRITICAL_SECTION localQueueMutex;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
		MODULENAME,
		__VERSION_DWORD,
		MODULENAME" plugin for Miranda NG.",
		"Johell, Ghost, Nightwish, __alex, Baloo",
		"Johell@ifrance.com, baloo@bk.ru",
		"© 2003 Johell, © 2005-09 Baloo",
		"http://miranda-ng.org/",
		UNICODE_AWARE,
		//1B2A39E5-E2F6-494D-958D-1808FD110DD5
		{0x1B2A39E5, 0xE2F6, 0x494D, {0x95, 0x8D, 0x18, 0x08, 0xFD, 0x11, 0x0D, 0xD5}}
};

LPSTR myDBGetString(HANDLE hContact,const char *szModule,const char *szSetting)
{
	char *val=NULL;
	DBVARIANT dbv;
	dbv.type = DBVT_ASCIIZ;
	DBGetContactSetting(hContact,szModule,szSetting,&dbv);
	if ( dbv.pszVal && (dbv.type==DBVT_ASCIIZ || dbv.type==DBVT_UTF8 || dbv.type==DBVT_WCHAR))
		val = mir_strdup(dbv.pszVal);
	DBFreeVariant(&dbv);
	return val;
}


LPSTR myDBGetStringDecode(HANDLE hContact,const char *szModule,const char *szSetting)
{
	char *val = myDBGetString(hContact,szModule,szSetting);
	if (!val) return NULL;
	size_t len = strlen(val)+64;
	char *buf = (LPSTR)mir_alloc(len);
	strncpy(buf,val,len); mir_free(val);
	CallService(MS_DB_CRYPT_DECODESTRING,(WPARAM)len,(LPARAM)buf);
	return buf;
}


int myDBWriteStringEncode(HANDLE hContact,const char *szModule,const char *szSetting,const char *val)
{
	int len = (int)strlen(val)+64;
	char *buf = (LPSTR)alloca(len);
	strncpy(buf,val,len);
	CallService(MS_DB_CRYPT_ENCODESTRING,(WPARAM)len,(LPARAM)buf);
	int ret = DBWriteContactSettingString(hContact,szModule,szSetting,buf);
	return ret;
}

void GetFlags()
{
	bSFT = db_get_b(0, szModuleName, "sft", 0);
	bSOM = db_get_b(0, szModuleName, "som", 0);
	bASI = db_get_b(0, szModuleName, "asi", 0);
	bMCD = db_get_b(0, szModuleName, "mcd", 0);
	bSCM = db_get_b(0, szModuleName, "scm", 0);
	bDGP = db_get_b(0, szModuleName, "dgp", 0);
	bAIP = db_get_b(0, szModuleName, "aip", 0);
	bNOL = db_get_b(0, szModuleName, "nol", 0);
	bAAK = db_get_b(0, szModuleName, "aak", 0);
	bMCM = db_get_b(0, szModuleName, "mcm", 0);
}

void SetFlags()
{
	db_set_b(0, szModuleName, "sft", bSFT);
	db_set_b(0, szModuleName, "som", bSOM);
	db_set_b(0, szModuleName, "asi", bASI);
	db_set_b(0, szModuleName, "mcd", bMCD);
	db_set_b(0, szModuleName, "scm", bSCM);
	db_set_b(0, szModuleName, "dgp", bDGP);
	db_set_b(0, szModuleName, "aip", bAIP);
	db_set_b(0, szModuleName, "nol", bNOL);
	db_set_b(0, szModuleName, "aak", bAAK);
	db_set_b(0, szModuleName, "mcm", bMCM);
}

struct A2U {
	LPSTR a;
	LPSTR u;
};
typedef A2U* pA2U;

pA2U pa2u;
int ca2u=0;

LPSTR TranslateU( LPCSTR lpText )
{
	int i;
	for(i=0;i<ca2u;i++) {
		if ( pa2u[i].a == lpText ) {
			return pa2u[i].u;
		}
	}
	ca2u++;
	pa2u = (pA2U) mir_realloc(pa2u,sizeof(A2U)*ca2u);
	pa2u[i].a = (LPSTR) lpText;
	LPWSTR lpwText = mir_a2u(lpText);
	LPWSTR lpwTran = TranslateW(lpwText);
	mir_free(lpwText);
	pa2u[i].u = mir_strdup(exp->utf8encode(lpwTran));
	return pa2u[i].u;
}

int msgbox( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	LPWSTR lpwText = mir_a2u(lpText);
	LPWSTR lpwCaption = mir_a2u(lpCaption);
	int r = MessageBoxW(hWnd,TranslateW(lpwText),TranslateW(lpwCaption),uType);
	mir_free(lpwCaption);
	mir_free(lpwText);
	return r;
}

void CopyToClipboard(HWND hwnd,LPSTR msg)
{
	HGLOBAL hglbCopy;
	LPSTR lpstrCopy;

	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, lstrlenA(msg)+1); 
	lpstrCopy = (LPSTR)GlobalLock(hglbCopy); 
	lstrcpyA(lpstrCopy, msg); 
	GlobalUnlock(hglbCopy); 

	OpenClipboard(NULL);
	EmptyClipboard(); 
	SetClipboardData(CF_TEXT, hglbCopy); 
	CloseClipboard(); 
}

#if defined(_DEBUG) || defined(NETLIB_LOG)
HANDLE hNetlibUser;

void InitNetlib()
{
	NETLIBUSER nl_user = { sizeof(nl_user) };
	nl_user.szSettingsModule = (LPSTR)szModuleName;
	nl_user.szDescriptiveName = (LPSTR)szModuleName;
	nl_user.flags = NUF_NOOPTIONS;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);
}

void DeinitNetlib()
{
	if (hNetlibUser)
		CallService(MS_NETLIB_CLOSEHANDLE, (WPARAM)hNetlibUser, 0);
}

int Sent_NetLog(const char *fmt,...)
{
	va_list va;
	char szText[1024];

	va_start(va,fmt);
	mir_vsnprintf(szText,sizeof(szText),fmt,va);
	va_end(va);
	return CallService(MS_NETLIB_LOG,(WPARAM)hNetlibUser,(LPARAM)szText);
}
#endif


// EOF
