/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define NEWSTR_ALLOCA(A) (A==NULL)?NULL:strcpy((char*)alloca(strlen(A)+1),A)
#define NEWTSTR_ALLOCA(A) (A==NULL)?NULL:_tcscpy((TCHAR*)alloca((_tcslen(A)+1)* sizeof(TCHAR)),A)

typedef HMONITOR (WINAPI *pfnMyMonitorFromPoint)(POINT, DWORD);
extern pfnMyMonitorFromPoint MyMonitorFromPoint;

typedef HMONITOR (WINAPI *pfnMyMonitorFromRect)(LPCRECT, DWORD);
extern pfnMyMonitorFromRect MyMonitorFromRect;

typedef HMONITOR(WINAPI *pfnMyMonitorFromWindow) (HWND, DWORD);
extern pfnMyMonitorFromWindow MyMonitorFromWindow;

typedef BOOL(WINAPI *pfnMyGetMonitorInfo) (HMONITOR, LPMONITORINFO);
extern pfnMyGetMonitorInfo MyGetMonitorInfo;

typedef HRESULT (STDAPICALLTYPE *pfnSHAutoComplete)(HWND,DWORD);
extern pfnSHAutoComplete shAutoComplete;

typedef HRESULT (STDAPICALLTYPE *pfnSHGetSpecialFolderPathA)(HWND, LPSTR,  int, BOOL );
typedef HRESULT (STDAPICALLTYPE *pfnSHGetSpecialFolderPathW)(HWND, LPWSTR, int, BOOL );
extern pfnSHGetSpecialFolderPathA shGetSpecialFolderPathA;
extern pfnSHGetSpecialFolderPathW shGetSpecialFolderPathW;

#ifdef _UNICODE
#define shGetSpecialFolderPath shGetSpecialFolderPathW
#else
#define shGetSpecialFolderPath shGetSpecialFolderPathA
#endif

typedef HDESK (WINAPI* pfnOpenInputDesktop)(DWORD, BOOL, DWORD);
extern pfnOpenInputDesktop openInputDesktop;

typedef HDESK (WINAPI* pfnCloseDesktop)(HDESK);
extern pfnCloseDesktop closeDesktop;

typedef BOOL (WINAPI* pfnAnimateWindow)(HWND, DWORD, DWORD);
extern pfnAnimateWindow animateWindow;

typedef BOOL (WINAPI * pfnSetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD);
extern pfnSetLayeredWindowAttributes setLayeredWindowAttributes;

typedef HTHEME  ( STDAPICALLTYPE *pfnOpenThemeData )( HWND, LPCWSTR );
typedef HRESULT ( STDAPICALLTYPE *pfnIsThemeBackgroundPartiallyTransparent )( HTHEME, int, int );
typedef HRESULT ( STDAPICALLTYPE *pfnDrawThemeParentBackground )( HWND, HDC, const RECT * );
typedef HRESULT ( STDAPICALLTYPE *pfnDrawThemeBackground )( HTHEME, HDC, int, int, const RECT *, const RECT * );
typedef HRESULT ( STDAPICALLTYPE *pfnDrawThemeText)( HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT *);
typedef HRESULT ( STDAPICALLTYPE *pfnDrawThemeTextEx)( HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const struct _DTTOPTS * );
typedef HRESULT ( STDAPICALLTYPE *pfnGetThemeBackgroundContentRect)( HTHEME, HDC, int, int, LPCRECT, LPRECT );
typedef HRESULT ( STDAPICALLTYPE *pfnGetThemeFont)( HTHEME, HDC, int, int, int, LOGFONT * );
typedef HRESULT ( STDAPICALLTYPE *pfnCloseThemeData )( HTHEME );
typedef HRESULT ( STDAPICALLTYPE *pfnEnableThemeDialogTexture )( HWND hwnd, DWORD dwFlags );
typedef HRESULT ( STDAPICALLTYPE *pfnSetWindowTheme )( HWND, LPCWSTR, LPCWSTR );
typedef HRESULT ( STDAPICALLTYPE *pfnSetWindowThemeAttribute )( HWND, enum WINDOWTHEMEATTRIBUTETYPE, PVOID, DWORD );
typedef BOOL    ( STDAPICALLTYPE *pfnIsThemeActive )();
typedef HRESULT (STDAPICALLTYPE *pfnBufferedPaintInit)(void);
typedef HRESULT (STDAPICALLTYPE *pfnBufferedPaintUninit)(void);
typedef HANDLE  (STDAPICALLTYPE *pfnBeginBufferedPaint)(HDC, RECT *, BP_BUFFERFORMAT, BP_PAINTPARAMS *, HDC *);
typedef HRESULT (STDAPICALLTYPE *pfnEndBufferedPaint)(HANDLE, BOOL);
typedef HRESULT (STDAPICALLTYPE *pfnGetBufferedPaintBits)(HANDLE, RGBQUAD **, int *);

extern pfnOpenThemeData openThemeData;
extern pfnIsThemeBackgroundPartiallyTransparent isThemeBackgroundPartiallyTransparent;
extern pfnDrawThemeParentBackground drawThemeParentBackground;
extern pfnDrawThemeBackground drawThemeBackground;
extern pfnDrawThemeText drawThemeText;
extern pfnDrawThemeTextEx drawThemeTextEx;
extern pfnGetThemeBackgroundContentRect getThemeBackgroundContentRect;
extern pfnGetThemeFont getThemeFont;
extern pfnCloseThemeData closeThemeData;
extern pfnEnableThemeDialogTexture enableThemeDialogTexture;
extern pfnSetWindowTheme setWindowTheme;
extern pfnSetWindowThemeAttribute setWindowThemeAttribute;
extern pfnIsThemeActive isThemeActive;
extern pfnBufferedPaintInit bufferedPaintInit;
extern pfnBufferedPaintUninit bufferedPaintUninit;
extern pfnBeginBufferedPaint beginBufferedPaint;
extern pfnEndBufferedPaint endBufferedPaint;
extern pfnGetBufferedPaintBits getBufferedPaintBits;

extern ITaskbarList3 * pTaskbarInterface;

typedef HRESULT ( STDAPICALLTYPE *pfnDwmExtendFrameIntoClientArea )( HWND hwnd, const MARGINS *margins );
typedef HRESULT ( STDAPICALLTYPE *pfnDwmIsCompositionEnabled )( BOOL * );

extern pfnDwmExtendFrameIntoClientArea dwmExtendFrameIntoClientArea;
extern pfnDwmIsCompositionEnabled dwmIsCompositionEnabled;

typedef INT (STDAPICALLTYPE *pfnGetaddrinfo)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA * pHints, PADDRINFOA * ppResult);
typedef INT (STDAPICALLTYPE *pfnFreeaddrinfo)(PADDRINFOA pAddrInfo);

extern pfnGetaddrinfo MyGetaddrinfo;
extern pfnFreeaddrinfo MyFreeaddrinfo;

/**** file.c ***************************************************************************/

void PushFileEvent( HANDLE hContact, HANDLE hdbe, LPARAM lParam );

/**** memory.c *************************************************************************/

#ifdef _STATIC
void*  mir_alloc( size_t );
void*  mir_calloc( size_t );
void*  mir_realloc( void* ptr, size_t );
void   mir_free( void* ptr );
char*  mir_strdup( const char* str );
WCHAR* mir_wstrdup( const WCHAR* str );
char* mir_strndup( const char* str, size_t len );

int    mir_snprintf(char *buffer, size_t count, const char* fmt, ...);
int    mir_sntprintf(TCHAR *buffer, size_t count, const TCHAR* fmt, ...);
int    mir_vsnprintf(char *buffer, size_t count, const char* fmt, va_list va);
int    mir_vsntprintf(TCHAR *buffer, size_t count, const TCHAR* fmt, va_list va);

WCHAR* mir_a2u_cp(const char* src, int codepage);
WCHAR* mir_a2u(const char* src);
char*  mir_u2a_cp(const wchar_t* src, int codepage);
char*  mir_u2a( const wchar_t* src);
#endif

/**** miranda.c ************************************************************************/

extern HINSTANCE hMirandaInst;
extern pfnExceptionFilter pMirandaExceptFilter;

/**** modules.c ************************************************************************/

void KillModuleEventHooks( HINSTANCE );
void KillModuleServices( HINSTANCE );

void KillObjectEventHooks( void* pObject );
void KillObjectServices( void* pObject );
void KillObjectThreads( void* pObject );

/**** utf.c ****************************************************************************/

char* Utf8Decode( char* str, wchar_t** ucs2 );
char* Utf8DecodeCP( char* str, int codepage, wchar_t** ucs2 );

wchar_t* Utf8DecodeUcs2( const char* str );

__forceinline char* Utf8DecodeA(const char* src)
{
    char* tmp = mir_strdup(src);
    Utf8Decode(tmp, NULL);
    return tmp;
}


char* Utf8Encode( const char* str );
char* Utf8EncodeCP( const char* src, int codepage );

char* Utf8EncodeUcs2( const wchar_t* str );

int   Ucs2toUtf8Len(const wchar_t *src);

#if defined( _UNICODE )
	#define Utf8DecodeT Utf8DecodeUcs2
	#define Utf8EncodeT Utf8EncodeUcs2
#else
	#define Utf8DecodeT Utf8DecodeA
	#define Utf8EncodeT Utf8Encode
#endif

/**** langpack.c ***********************************************************************/

int    LangPackGetDefaultCodePage();
int    LangPackGetDefaultLocale();
TCHAR* LangPackPcharToTchar( const char* pszStr );
char*  LangPackTranslateString(struct LangPackMuuid* pUuid, const char *szEnglish, const int W);

unsigned int __fastcall hash(const void * key, unsigned int len);

#pragma optimize( "gt", on )
__inline unsigned int hashstr(const char * key)
{
	if (key == NULL) return 0;
	const unsigned int len = (unsigned int)strlen((const char*)key);
	return hash(key, len);
}

__inline unsigned int hashstr(const wchar_t * key)
{
	if (key == NULL) return 0;
	const unsigned int len = (unsigned int)wcslen((const wchar_t*)key);
	return hash(key, len * sizeof(wchar_t));
}
#pragma optimize( "", on )

/**** path.c ***************************************************************************/

int pathToAbsolute(const char *pSrc, char *pOut, char* base);
void CreatePathToFile( char* wszFilePath );
int CreateDirectoryTree(const char *szDir);
#if defined( _UNICODE )
	void CreatePathToFileW( WCHAR* wszFilePath );
	int CreateDirectoryTreeW(const WCHAR *szDir);
	int pathToAbsoluteW(const TCHAR *pSrc, TCHAR *pOut, TCHAR* base);
	#define pathToAbsoluteT pathToAbsoluteW
	#define CreatePathToFileT CreatePathToFileW
	#define CreateDirectoryTreeT CreateDirectoryTreeW
#else
	#define pathToAbsoluteT pathToAbsolute
	#define CreatePathToFileT CreatePathToFile
	#define CreateDirectoryTreeT CreateDirectoryTree
#endif

/**** skin2icons.c *********************************************************************/

HANDLE IcoLib_AddNewIcon( SKINICONDESC* sid );
HICON  IcoLib_GetIcon( const char* pszIconName, bool big );
HICON  IcoLib_GetIconByHandle( HANDLE hItem, bool big );
HANDLE IcoLib_IsManaged( HICON hIcon );
int    IcoLib_ReleaseIcon( HICON hIcon, char* szIconName, bool big );

/**** skinicons.c **********************************************************************/

HICON LoadSkinProtoIcon( const char* szProto, int status, bool big = false );
HICON LoadSkinIcon( int idx, bool big = false );
HANDLE GetSkinIconHandle( int idx );

HICON LoadIconEx(HINSTANCE hInstance, LPCTSTR lpIconName, BOOL bShared);
int ImageList_AddIcon_NotShared(HIMAGELIST hIml, LPCTSTR szResource);
int ImageList_ReplaceIcon_NotShared(HIMAGELIST hIml, int iIndex, HINSTANCE hInstance, LPCTSTR szResource);

int ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId);
int ImageList_AddIcon_ProtoIconLibLoaded(HIMAGELIST hIml, const char* szProto, int iconId);
int ImageList_ReplaceIcon_IconLibLoaded(HIMAGELIST hIml, int nIndex, HICON hIcon);

void Button_SetIcon_IcoLib(HWND hDlg, int itemId, int iconId, const char* tooltip);
void Button_FreeIcon_IcoLib(HWND hDlg, int itemId);

void Window_SetIcon_IcoLib(HWND hWnd, int iconId);
void Window_SetProtoIcon_IcoLib(HWND hWnd, const char* szProto, int iconId);
void Window_FreeIcon_IcoLib(HWND hWnd);

#define IconLib_ReleaseIcon(hIcon, szName) IcoLib_ReleaseIcon(hIcon, szName, false);
#define Safe_DestroyIcon(hIcon) if (hIcon) DestroyIcon(hIcon)

/**** clistmenus.c **********************************************************************/

extern HANDLE hMainMenuObject, hContactMenuObject, hStatusMenuObject;
extern HANDLE hPreBuildMainMenuEvent, hPreBuildContactMenuEvent;

extern const int statusModeList[ MAX_STATUS_COUNT ];
extern const int skinIconStatusList[ MAX_STATUS_COUNT ];
extern const int skinIconStatusFlags[ MAX_STATUS_COUNT ];

int TryProcessDoubleClick( HANDLE hContact );

/**** protocols.c ***********************************************************************/

#define OFFSET_PROTOPOS 200
#define OFFSET_VISIBLE  400
#define OFFSET_ENABLED  600
#define OFFSET_NAME     800

extern LIST<PROTOACCOUNT> accounts;

PROTOACCOUNT* __fastcall Proto_GetAccount( const char* accName );
PROTOACCOUNT* __fastcall Proto_GetAccount( HANDLE hContact );
PROTOCOLDESCRIPTOR* __fastcall Proto_IsProtocolLoaded( const char* szProtoName );

bool __fastcall Proto_IsAccountEnabled( PROTOACCOUNT* pa );
bool __fastcall Proto_IsAccountLocked( PROTOACCOUNT* pa );

PROTO_INTERFACE* AddDefaultAccount( const char* szProtoName );
int  FreeDefaultAccount( PROTO_INTERFACE* ppi );

BOOL ActivateAccount( PROTOACCOUNT* pa );
void EraseAccount( const char* pszProtoName );
void DeactivateAccount( PROTOACCOUNT* pa, bool bIsDynamic, bool bErase );
void UnloadAccount( PROTOACCOUNT* pa, bool bIsDynamic, bool bErase );
void OpenAccountOptions( PROTOACCOUNT* pa );

void LoadDbAccounts( void );
void WriteDbAccounts( void );

INT_PTR CallProtoServiceInt( HANDLE hContact, const char* szModule, const char* szService, WPARAM, LPARAM );
INT_PTR CallContactService( HANDLE hContact, const char *szProtoService, WPARAM, LPARAM );

__inline static INT_PTR CallProtoService( const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam )
{
	return CallProtoServiceInt( NULL, szModule, szService, wParam, lParam );
}

/**** utils.c **************************************************************************/

#if defined( _UNICODE )
	char*  __fastcall rtrim(char* str);
#endif
TCHAR* __fastcall rtrim(TCHAR* str);
char*  __fastcall ltrim(char* str);
char* __fastcall ltrimp(char* str);
__inline char* lrtrim(char* str) { return ltrim(rtrim(str)); };
__inline char* lrtrimp(char* str) { return ltrimp(rtrim(str)); };

bool __fastcall wildcmp(char * name, char * mask);

void HotkeyToName(TCHAR *buf, int size, BYTE shift, BYTE key);
WORD GetHotkeyValue( INT_PTR idHotkey );

HBITMAP ConvertIconToBitmap(HICON hIcon, HIMAGELIST hIml, int iconId);

class StrConvUT
{
private:
	wchar_t* m_body;

public:
	StrConvUT( const char* pSrc ) :
		m_body( mir_a2u( pSrc )) {}

    ~StrConvUT() {  mir_free( m_body ); }
	operator const wchar_t* () const { return m_body; }
};

class StrConvAT
{
private:
	char* m_body;

public:
	StrConvAT( const wchar_t* pSrc ) :
		m_body( mir_u2a( pSrc )) {}

    ~StrConvAT() {  mir_free( m_body ); }
	operator const char*  () const { return m_body; }
	operator const wchar_t* () const { return ( wchar_t* )m_body; }  // type cast to fake the interface definition
	operator const LPARAM () const { return ( LPARAM )m_body; }
};

#ifdef _UNICODE

#define StrConvT( x ) StrConvUT( x )
#define StrConvTu( x ) x
#define StrConvA( x ) StrConvAT( x )
#define StrConvU( x ) x

#else

#define StrConvT( x ) x
#define StrConvTu( x ) StrConvAT( x )
#define StrConvA( x ) x
#define StrConvU( x ) StrConvUT( x )

#endif

