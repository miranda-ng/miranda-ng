#include "commonheaders.h"

LPCSTR szModuleName = MODULENAME;
LPCSTR szVersionStr = MODULENAME" DLL ("__VERSION_STRING")";
HINSTANCE g_hInst;
PLUGINLINK *pluginLink;
MM_INTERFACE mmi={0};
MUUID interfaces[] = {MIID_CRYPTOPP, MIID_LAST};

HANDLE hPGPPRIV = NULL;
HANDLE hRSA4096 = NULL;

CRITICAL_SECTION localQueueMutex;
CRITICAL_SECTION localContextMutex;

char TEMP[MAX_PATH];
int  TEMP_SIZE = 0;
BOOL isVista = 0;

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
		MODULENAME,
		__VERSION_DWORD,
		MODULENAME" library for SecureIM plugin ("__DATE__")",
		"Baloo",
		"baloo@bk.ru",
		"© 2006-09 Baloo",
		"http://miranda-im.org/download/details.php?action=viewfile&id=2669",
		0, 0
};

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
		MODULENAME,
		__VERSION_DWORD,
		MODULENAME" library for SecureIM plugin ("__DATE__")",
		"Baloo",
		"baloo@bk.ru",
		"© 2006-09 Baloo",
		"http://miranda-im.org/download/details.php?action=viewfile&id=2669",
		0, 0,	
		MIID_CRYPTOPP
};


BOOL ExtractFileFromResource( HANDLE FH, int ResType, int ResId, DWORD* Size )
{
    HRSRC	RH;
    PBYTE	RP;
    DWORD	s,x;

    RH = FindResource( g_hInst, MAKEINTRESOURCE( ResId ), MAKEINTRESOURCE( ResType ) );

    if( RH == NULL ) return FALSE;
    RP = (PBYTE) LoadResource( g_hInst, RH );
    if( RP == NULL ) return FALSE;
    s = SizeofResource( g_hInst, RH );
    if( !WriteFile( FH, RP, s, &x, NULL ) ) return FALSE;
    if( x != s ) return FALSE;
    if( Size ) *Size = s;
    return TRUE;
}


void ExtractFile( char *FileName, int ResType, int ResId )
{
    HANDLE FH;
    FH = CreateFile( FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
    if( FH == INVALID_HANDLE_VALUE ) return;
    if (!ExtractFileFromResource( FH, ResType, ResId, NULL )) MessageBoxA(0,"Can't extract","!!!",MB_OK);
    CloseHandle( FH );
}


int rtrim(LPCSTR str) {
	int len = strlen(str);
	LPSTR ptr = (LPSTR)str+len-1;

	while( len ) {
		char c = *ptr;
		if( c != '\x20' && c != '\x09' && c != '\x0A' && c != '\x0D' ) {
			*(ptr+1) = '\0';
			break;
		}
		len--; ptr--;
	}
	return len;
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
  if(hNetlibUser)
      return CallService(MS_NETLIB_LOG,(WPARAM)hNetlibUser,(LPARAM)szText);
  return 0;
}
#endif


// EOF
