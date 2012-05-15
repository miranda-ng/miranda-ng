#include "commonheaders.h"

HINSTANCE g_hInst, g_hIconInst;
MUUID interfaces[] = {MIID_SECUREIM, MIID_LAST};

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
IconExtraColumn g_IEC[1+IEC_CNT*MODE_CNT];

int iBmpDepth;
BOOL bCoreUnicode = false, bMetaContacts = false, bPopupExists = false, bPopupUnicode = false;
BOOL bPGPloaded = false, bPGPkeyrings = false, bUseKeyrings = false, bPGPprivkey = false;
BOOL bGPGloaded = false, bGPGkeyrings = false, bSavePass = false;
BOOL bSFT, bSOM, bASI, bMCD, bSCM, bDGP, bAIP, bNOL, bAAK, bMCM;
BYTE bADV, bPGP, bGPG;
DWORD iCoreVersion = 0;
CRITICAL_SECTION localQueueMutex;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
		MODULENAME" (2in1)",
		__VERSION_DWORD,
		MODULENAME" plugin for Miranda IM ("__DATE__")",
		"Johell, Ghost, Nightwish, __alex, Baloo",
		"Johell@ifrance.com, baloo@bk.ru",
		"© 2003 Johell, © 2005-09 Baloo",
		"http://addons.miranda-im.org/details.php?action=viewfile&id=2445",
		0, 0,
		MIID_SECUREIM
};


LPSTR myDBGetString(HANDLE hContact,const char *szModule,const char *szSetting) {
	char *val=NULL;
	DBVARIANT dbv;
	dbv.type = DBVT_ASCIIZ;
	DBGetContactSetting(hContact,szModule,szSetting,&dbv);
	if( dbv.pszVal && (dbv.type==DBVT_ASCIIZ || dbv.type==DBVT_UTF8 || dbv.type==DBVT_WCHAR) )
		val = mir_strdup(dbv.pszVal);
	DBFreeVariant(&dbv);
	return val;
}


LPSTR myDBGetStringDecode(HANDLE hContact,const char *szModule,const char *szSetting) {
	char *val = myDBGetString(hContact,szModule,szSetting);
	if(!val) return NULL;
	size_t len = strlen(val)+64;
	char *buf = (LPSTR)mir_alloc(len);
	strncpy(buf,val,len); mir_free(val);
	CallService(MS_DB_CRYPT_DECODESTRING,(WPARAM)len,(LPARAM)buf);
	return buf;
}


int myDBWriteStringEncode(HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {
	int len = (int)strlen(val)+64;
	char *buf = (LPSTR)alloca(len);
	strncpy(buf,val,len);
	CallService(MS_DB_CRYPT_ENCODESTRING,(WPARAM)len,(LPARAM)buf);
	int ret = DBWriteContactSettingString(hContact,szModule,szSetting,buf);
	return ret;
}

/*
int DBWriteString(HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {
	return DBWriteContactSettingString(hContact,szModule,szSetting,val);
}


int DBGetByte(HANDLE hContact,const char *szModule,const char *szSetting,int errorValue) {
	return DBGetContactSettingByte(hContact,szModule,szSetting,errorValue);
}


int DBWriteByte(HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {
	return DBWriteContactSettingByte(hContact,szModule,szSetting,val);
}


int DBGetWord(HANDLE hContact,const char *szModule,const char *szSetting,int errorValue) {
	return DBGetContactSettingWord(hContact,szModule,szSetting,errorValue);
}


int DBWriteWord(HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {
	return DBWriteContactSettingWord(hContact,szModule,szSetting,val);
}
*/

void GetFlags() {
    bSFT = DBGetContactSettingByte(0,szModuleName,"sft",0);
    bSOM = DBGetContactSettingByte(0,szModuleName,"som",0);
    bASI = DBGetContactSettingByte(0,szModuleName,"asi",0);
    bMCD = DBGetContactSettingByte(0,szModuleName,"mcd",0);
    bSCM = DBGetContactSettingByte(0,szModuleName,"scm",0);
    bDGP = DBGetContactSettingByte(0,szModuleName,"dgp",0);
    bAIP = DBGetContactSettingByte(0,szModuleName,"aip",0);
    bADV = DBGetContactSettingByte(0,szModuleName,"adv",0);
    bNOL = DBGetContactSettingByte(0,szModuleName,"nol",0);
    bAAK = DBGetContactSettingByte(0,szModuleName,"aak",0);
    bMCM = DBGetContactSettingByte(0,szModuleName,"mcm",0);
}


void SetFlags() {
    DBWriteContactSettingByte(0,szModuleName,"sft",bSFT);
    DBWriteContactSettingByte(0,szModuleName,"som",bSOM);
    DBWriteContactSettingByte(0,szModuleName,"asi",bASI);
    DBWriteContactSettingByte(0,szModuleName,"mcd",bMCD);
    DBWriteContactSettingByte(0,szModuleName,"scm",bSCM);
    DBWriteContactSettingByte(0,szModuleName,"dgp",bDGP);
    DBWriteContactSettingByte(0,szModuleName,"aip",bAIP);
    DBWriteContactSettingByte(0,szModuleName,"adv",bADV);
    DBWriteContactSettingByte(0,szModuleName,"nol",bNOL);
    DBWriteContactSettingByte(0,szModuleName,"aak",bAAK);
    DBWriteContactSettingByte(0,szModuleName,"mcm",bMCM);
}


/*-----------------------------------------------------*/
/*
LPSTR u2a( LPCWSTR src )
{
	int codepage = ServiceExists(MS_LANGPACK_GETCODEPAGE)?CallService( MS_LANGPACK_GETCODEPAGE, 0, 0 ):CP_ACP;

	int cbLen = WideCharToMultiByte( codepage, 0, src, -1, NULL, 0, NULL, NULL );
	LPSTR result = (LPSTR) mir_alloc( cbLen+1 );
	if ( result == NULL )
		return NULL;

	WideCharToMultiByte( codepage, 0, src, -1, result, cbLen, NULL, NULL );
	result[ cbLen ] = 0;
	return result;
}

LPWSTR a2u( LPCSTR src )
{
	int codepage = ServiceExists(MS_LANGPACK_GETCODEPAGE)?CallService( MS_LANGPACK_GETCODEPAGE, 0, 0 ):CP_ACP;

	int cbLen = MultiByteToWideChar( codepage, 0, src, -1, NULL, 0 );

	LPWSTR result = (LPWSTR) mir_alloc( sizeof(WCHAR)*(cbLen+1));
	if ( result == NULL )
		return NULL;

	MultiByteToWideChar( codepage, 0, src, -1, result, cbLen );
	result[ cbLen ] = 0;

	return result;
}
*/
struct A2U {
	LPSTR a;
	LPSTR u;
};
typedef A2U* pA2U;

pA2U pa2u;
int ca2u=0;

LPSTR TranslateU( LPCSTR lpText ) {
	int i;
	for(i=0;i<ca2u;i++) {
		if( pa2u[i].a == lpText ) {
			return pa2u[i].u;
		}
	}
	ca2u++;
	pa2u = (pA2U) mir_realloc(pa2u,sizeof(A2U)*ca2u);
	pa2u[i].a = (LPSTR) lpText;
	if( bCoreUnicode ) {
		LPWSTR lpwText = mir_a2u(lpText);
		LPWSTR lpwTran = TranslateW(lpwText);
		mir_free(lpwText);
		pa2u[i].u = mir_strdup(exp->utf8encode(lpwTran));
	}
	else {
		LPSTR lpTran = Translate(lpText);
		LPWSTR lpwTran = mir_a2u(lpTran);
		lpTran = exp->utf8encode(lpwTran);
		mir_free(lpwTran);
		pa2u[i].u = mir_strdup(lpTran);
	}
	return pa2u[i].u;
}

int msgbox( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
	if( bCoreUnicode ) {
		LPWSTR lpwText = mir_a2u(lpText);
		LPWSTR lpwCaption = mir_a2u(lpCaption);
		int r = MessageBoxW(hWnd,TranslateW(lpwText),TranslateW(lpwCaption),uType);
		mir_free(lpwCaption);
		mir_free(lpwText);
		return r;
	}
	return MessageBoxA(hWnd,Translate(lpText),Translate(lpCaption),uType);
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

void InitNetlib() {
	NETLIBUSER nl_user;
	memset(&nl_user,0,sizeof(nl_user));
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = (LPSTR)szModuleName;
	nl_user.szDescriptiveName = (LPSTR)szModuleName;
	nl_user.flags = NUF_NOOPTIONS;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);
}

void DeinitNetlib() {
	if(hNetlibUser)
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
