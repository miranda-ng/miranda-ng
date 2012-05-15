#ifndef commonheaders_h__
#define commonheaders_h__

/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

#pragma once
#define MIRANDA_VER 0x0A00

#define _WIN32_WINNT 0x0501

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#ifndef DB_USEHELPERFUNCTIONS		// to supress static inline db helpers
#define DB_NOHELPERFUNCTIONS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS		// to suppress secure warnings in VC2005
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE	// to suppress secure deprecate warnings in VC2005
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if defined (_DEBUG)
#define TRACE(str)  { log0(str); }
#else
  #define TRACE(str)
#endif

#if defined (_DEBUG)
  #define TRACEVAR(str,n) { log1(str,n); }
#else
  #define TRACEVAR(str,n)
#endif

#if defined (_DEBUG)
#define TRACET(str) OutputDebugString(str)
#else
#define TRACET(str)
#endif

#define SERVICE(serviceproc)         static INT_PTR serviceproc(WPARAM wParam,LPARAM lParam)
#define EVENTHOOK(eventhookproc)     static int eventhookproc(WPARAM wParam,LPARAM lParam)
#define CLINTERFACE                  static

#define PLUGININTERFACE extern "C" __declspec( dllexport )
#define UPDATER_PATH "http://miranda-im.org/download/details.php?action=viewfile&id=3684"

#include "m_stdhdr.h"

#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
//#include <process.h>
#include <io.h>
#include <math.h>
#include <string.h>
#include <direct.h>
#include <win2k.h>

#include "modern_global_structure.h"

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_utils.h>

#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_plugins.h>
#include <m_avatars.h>
#include <m_genmenu.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_icolib.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_fontservice.h>
#include <m_file.h>
#include <m_timezones.h>

#include "modern_clc.h"
#include "modern_clist.h"
#include "modern_cluiframes.h"
#include "m_cluiframes.h"
#include "../m_api/m_metacontacts.h"
#include "../m_api/m_skin_eng.h"

#include "modern_rowheight_funcs.h"
#include "modern_cache_funcs.h"
#include "modern_log.h"

#include "richedit.h"
#include "../m_api/m_variables.h"

#include "../m_api/m_smileyadd.h"

#include "../m_api/m_xpTheme.h"
#include "../m_api/m_toolbar.h"

#include "../resource.h"

#include "modern_layered_window_engine.h"

// module name of MetaContacts plugin
extern char *g_szMetaModuleName;

//macros to free data and set it pointer to NULL
#define mir_free_and_nill(x) {mir_free((void*)x); x=NULL;}
// shared vars

#define CLUI_FRAME_AUTOHIDENOTIFY  512
#define CLUI_FRAME_SHOWALWAYS      1024


//#define alloc(n) mir_alloc(n)

#define MAX_REGS(_A_) (sizeof(_A_)/sizeof(_A_[0]))

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

#define MENU_MIRANDAMENU            0xFFFF1234
#define MENU_STATUSMENU             0xFFFF1235
#define MENU_MINIMIZE               0xFFFF1236

#define UM_CREATECLC                (WM_USER+1)
#define UM_SETALLEXTRAICONS         (WM_USER+2)
#define UM_UPDATE                   (WM_USER+50)
#define UM_SYNCCALL                 (WM_USER+654)

// Define constants for CLUI_SizingOnBorder SC_SIZE

#define SCF_NONE    0
#define SCF_LEFT    1
#define SCF_RIGHT   2
#define SCF_TOP     3
#define SCF_BOTTOM  6


extern char* __cdecl strstri( char *a, const char *b);
extern BOOL __cdecl mir_bool_strcmpi(const char *a, const char *b);
extern int __cdecl mir_strcmp (const char *a, const char *b);
extern int __cdecl mir_strlen (const char *a);
extern int __cdecl mir_strcmpi(const char *a, const char *b);
extern int __cdecl mir_tstrcmpi(const TCHAR *a, const TCHAR *b);
extern DWORD exceptFunction(LPEXCEPTION_POINTERS EP);

extern HANDLE ModernHookEvent( char *EventID, MIRANDAHOOK HookProc );

extern int ModernUnhookEvent(HANDLE hHook);
extern int UnhookAll();

#ifndef MYCMP
#define MYCMP 1
#define strcmp(a,b) mir_strcmp(a,b)
#define strlen(a) mir_strlen(a)
#endif

//  Register of plugin's user
//
//  wParam = (WPARAM)szSetting - string that describes a user
//           format: Category/ModuleName,
//           eg: "Contact list background/CLUI",
//               "Status bar background/StatusBar"
//  lParam = (LPARAM)dwFlags
//
#define MS_BACKGROUNDCONFIG_REGISTER "ModernBkgrCfg/Register"

//
//  Notification about changed background
//  wParam = ModuleName
//  lParam = 0
#define ME_BACKGROUNDCONFIG_CHANGED "ModernBkgrCfg/Changed"



HBITMAP ske_CreateDIB32(int cx, int cy);

extern void InitDisplayNameCache(void);
extern void FreeDisplayNameCache();
extern int CLUI_ShowWindowMod(HWND hwnd, int cmd);

#ifdef UNICODE
	#define GSMDF_TCHAR_MY GSMDF_TCHAR|CNF_UNICODE
#else
	#define GSMDF_TCHAR_MY 0
#endif

#ifndef LWA_COLORKEY
#define LWA_COLORKEY            0x00000001
#endif

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA            0x01
#endif

//#ifdef _DEBUG
//#define DeleteObject(a) DebugDeleteObject(a)
//#endif

#define strsetA(a,b) {if (a) mir_free_and_nill(a); a=mir_strdup(b);}
#define strsetT(a,b) {if (a) mir_free_and_nill(a); a=mir_tstrdup(b);}

extern void TRACE_ERROR();
extern BOOL DebugDeleteObject(HGDIOBJ a);
extern BOOL mod_DeleteDC(HDC hdc);
extern BOOL ske_ResetTextEffect(HDC hdc);
extern BOOL ske_SelectTextEffect(HDC hdc, BYTE EffectID, DWORD FirstColor, DWORD SecondColor);
extern void IvalidateDisplayNameCache(DWORD mode);

typedef BOOL (WINAPI *pfnTryEnterCriticalSection)( LPCRITICAL_SECTION );
extern pfnTryEnterCriticalSection fnTryEnterCriticalSection;

typedef BOOL (WINAPI *pfnGetScrollBarInfo)( HWND, LONG, PSCROLLBARINFO );
extern pfnGetScrollBarInfo fnGetScrollBarInfo;

typedef DWORD (WINAPI *pfnMsgWaitForMultipleObjectsEx)( DWORD, CONST HANDLE*, DWORD, DWORD, DWORD );
extern pfnMsgWaitForMultipleObjectsEx fnMsgWaitForMultipleObjectsEx;

typedef HWND (WINAPI *pfnGetAncestor)( HWND, UINT );
extern pfnGetAncestor fnGetAncestor;
HWND WINAPI MyGetAncestor( HWND, UINT );

typedef BOOL (WINAPI *pfnGetMenuBarInfo)( HWND, LONG, LONG, PMENUBARINFO );
extern pfnGetMenuBarInfo fnGetMenuBarInfo;

void FreeAndNil( void **p );

extern SortedList *clistCache;

HICON LoadSmallIconShared(HINSTANCE hInstance, LPCTSTR lpIconName);
HICON LoadSmallIcon(HINSTANCE hInstance, LPCTSTR lpIconName);
BOOL DestroyIcon_protect(HICON icon);

#ifndef ETDT_ENABLETAB
#define ETDT_DISABLE        0x00000001
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)
#endif



#define TreeView_InsertItemA(hwnd, lpis) \
	(HTREEITEM)SendMessageA((hwnd), TVM_INSERTITEMA, 0, (LPARAM)(LPTV_INSERTSTRUCTA)(lpis))

#define TreeView_GetItemA(hwnd, pitem) \
	(BOOL)SendMessageA((hwnd), TVM_GETITEMA, 0, (LPARAM)(TV_ITEM *)(pitem))

enum
{
	STATE_DLL_LOADING = 0,
	STATE_CLUI_LOADING,
	STATE_NORMAL,
	STATE_PREPEARETOEXIT,
	STATE_EXITING
};

#define MirandaLoading() ((g_CluiData.bSTATE<STATE_NORMAL))
#define MirandaExiting() ((g_CluiData.bSTATE>STATE_NORMAL))



char * strdupn(const char * src, int len);

#define SKINBUTTONCLASS _T("MirandaSkinButtonClass")

#define SORTBY_NAME	   0
#define SORTBY_STATUS  1
#define SORTBY_LASTMSG 2
#define SORTBY_PROTO   3
#define SORTBY_RATE    4
#define SORTBY_NAME_LOCALE 5
#define SORTBY_NOTHING	10

#define DT_FORCENATIVERENDER   0x10000000

#define _BOOL(a) (a != 0)

/* modern_animated_avatars.c */
int AniAva_InitModule();								   // HAVE TO BE AFTER GDI+ INITIALIZED
int AniAva_UnloadModule();
int AniAva_UpdateOptions();								   //reload options, //hot enable/disable engine

int AniAva_AddAvatar(HANDLE hContact, TCHAR * szFilename, int width, int heigth);  // adds avatars to be displayed
int AniAva_SetAvatarPos(HANDLE hContact, RECT * rc, int overlayIdx, BYTE bAlpha);	   // update avatars pos
int AniAva_InvalidateAvatarPositions(HANDLE hContact);	   // reset positions of avatars to be drawn (still be painted at same place)
int AniAva_RemoveInvalidatedAvatars();					   // all avatars without validated position will be stop painted and probably removed
int AniAva_RemoveAvatar(HANDLE hContact);				   // remove avatar
int AniAva_RedrawAllAvatars(BOOL updateZOrder);			   // request to repaint all
void AniAva_UpdateParent();
int AniAva_RenderAvatar( HANDLE hContact, HDC hdcMem, RECT * rc );


#define CCI_NAME			1
#define CCI_GROUP			(1<<1)
#define CCI_PROTO			(1<<2)
#define CCI_STATUS			(1<<3)
#define CCI_LINES			(1<<4)
#define CCI_HIDDEN			(1<<4)
#define CCI_NOHIDEOFFLINE	(1<<5)
#define CCI_NOPROTO			(1<<6)
#define CCI_HIDESUBCONTACT	(1<<7)
#define CCI_I				(1<<8)
#define CCI_APPARENT		(1<<9)
#define CCI_NOTONLIST		(1<<10)
#define CCI_IDLETS			(1<<11)
#define CCI_CCONTACT		(1<<12)
#define CCI_EXPAND			(1<<13)
#define CCI_UNKNOWN			(1<<14)
#define CCI_TIME			(1<<15)
#define CCI_OTHER			~( CCI_NAME|CCI_GROUP|CCI_PROTO|CCI_STATUS|CCI_LINES|CCI_TIME )
#define CCI_ALL				(0xFFFFFFFF)

void CListSettings_FreeCacheItemData(pdisplayNameCacheEntry pDst);
int CLUI_SyncGetPDNCE(WPARAM wParam, LPARAM lParam);
WORD pdnce___GetStatus(pdisplayNameCacheEntry pdnce);
void pdnce___SetStatus( pdisplayNameCacheEntry pdnce, WORD wStatus );

/* move to list module */
typedef void (*ItemDestuctor)(void*);


void li_ListDestruct(SortedList *pList, ItemDestuctor pItemDestructor);
void li_RemoveDestruct(SortedList *pList, int index, ItemDestuctor pItemDestructor);
void li_RemovePtrDestruct(SortedList *pList, void * ptr, ItemDestuctor pItemDestructor);
void li_SortList(SortedList *pList, FSortFunc pSortFunct);

#define mir_safe_free(a) if(a) mir_free(a)

#ifdef _UNICODE
#define mir_t2a(s) mir_u2a(s)
#define mir_a2t(s) mir_a2u(s)
#define mir_t2u(s) mir_wstrdup(s)
#define mir_u2t(s) mir_wstrdup(s)
#else
#define mir_t2a(s) mir_strdup(s)
#define mir_a2t(s) mir_strdup(s)
#define mir_t2u(s) mir_a2u(s)
#define mir_u2t(s) mir_u2a(s)
#endif


template <class T> class INIT : public T
{
public:
	INIT()
	{
		memset(this, 0, sizeof(T));
		this->cbSize=sizeof(T);
	}
};

#ifdef __cplusplus
const ROWCELL * rowAddCell(ROWCELL* &, int );
void rowDeleteTree(ROWCELL *cell);
BOOL rowParse(ROWCELL* &cell, ROWCELL* parent, char *tbuf, int &hbuf, int &sequence, ROWCELL** RowTabAccess );
void rowSizeWithReposition(ROWCELL* &root, int width);
#endif

//////////////////////////////////////////////////////////////////////////
// Specific class for quick implementation of map<string, *> list
// with some more fast searching it is
// hash_map alternative - for  faked hash search;
// the items are stored no by char* key but both int(hash),char*.
// have items sorted in map firstly via hash, secondly via string
// the method is case insensitive
// To use this simple define like
// 	typedef std::map<HashStringKeyNoCase, _Type > map_Type;
//	map_Type myMap;
// and access it as usual via simpe char* indexing:
//  myList[ "first"  ]=_Type_value;
//  myList[ "second" ]=_Type_value;
//  _Type a = myList[ "second"];

class HashStringKeyNoCase
{
public:

	HashStringKeyNoCase( const char* szKey )
	{
		_strKey=_strdup( szKey );
		_CreateHashKey();
	}

	HashStringKeyNoCase( const HashStringKeyNoCase& hsKey )
	{
		_strKey = _strdup( hsKey._strKey );
		_dwKey  = hsKey._dwKey;
	}

	HashStringKeyNoCase& operator= ( const HashStringKeyNoCase& hsKey )
	{
		_strKey = _strdup( hsKey._strKey );
		_dwKey  = hsKey._dwKey;
	}

#ifdef _UNICODE
	HashStringKeyNoCase( const wchar_t* szKey )
	{
		int codepage=0;
		int cbLen = WideCharToMultiByte( codepage, 0, szKey, -1, NULL, 0, NULL, NULL );
		char* result = ( char* )malloc( cbLen+1 );
		WideCharToMultiByte( codepage, 0, szKey, -1, result, cbLen, NULL, NULL );
		result[ cbLen ] = 0;

		_strKey=result;
		_CreateHashKey();
	}
#endif

	~HashStringKeyNoCase()
	{
		if (_strKey) free (_strKey);
		_strKey = NULL;
		_dwKey=0;
	}

private:
	char*   _strKey;
	DWORD   _dwKey;

	void  _CreateHashKey()
	{
		_strKey=_strupr( _strKey );
		_dwKey = mod_CalcHash( _strKey );
	}

public:
	bool operator< ( const HashStringKeyNoCase& second ) const
	{
		if ( this->_dwKey != second._dwKey )
			return ( this->_dwKey < second._dwKey );
		else
			return ( strcmp( this->_strKey, second._strKey ) < 0 ); // already maked upper so in any case - will be case insensitive
	}

	struct HashKeyLess
	{
		bool operator() ( const HashStringKeyNoCase& first, const HashStringKeyNoCase& second ) const
		{	return ( first < second ); }
	};
};

#ifdef _UNICODE
#define ModernGetStringT ModernGetStringW
#else
#define ModernGetStringT ModernGetStringA
#endif

char *	   ModernGetStringA			( HANDLE hContact, const char *szModule, const char *szSetting );
wchar_t *  ModernGetStringW			( HANDLE hContact, const char *szModule, const char *szSetting );

WORD	ModernGetSettingRangedWord  ( HANDLE hContact, const char *szModule, const char *szSetting, WORD errorValue, WORD minValue, WORD maxValue);


#define ModernGetSetting(a,b,c,d)             ModernGetSetting_Helper(a,b,c,d,__FILE__,__LINE__)
#define ModernGetSettingByte(a,b,c,d)         ModernGetSettingByte_Helper(a,b,c,d,__FILE__,__LINE__)
#define ModernGetSettingWord(a,b,c,d)         ModernGetSettingWord_Helper(a,b,c,d,__FILE__,__LINE__)
#define ModernGetSettingDword(a,b,c,d)        ModernGetSettingDword_Helper(a,b,c,d,__FILE__,__LINE__)
#define ModernGetSettingString(a,b,c,d)       ModernGetSettingString_Helper(a,b,c,d,__FILE__,__LINE__,DBVT_ASCIIZ)
#define ModernGetSettingWString(a,b,c,d)      ModernGetSettingString_Helper(a,b,c,d,__FILE__,__LINE__,DBVT_WCHAR)
#define ModernGetSettingUTF8String(a,b,c,d)   ModernGetSettingString_Helper(a,b,c,d,__FILE__,__LINE__,DBVT_UTF8)

int  __cdecl ModernGetSettingByte_Helper   ( HANDLE hContact, const char *szModule, const char *szSetting, int errorValue, const char *szFile, const int nLine);
int  __cdecl ModernGetSettingWord_Helper   ( HANDLE hContact, const char *szModule, const char *szSetting, int errorValue, const char *szFile, const int nLine);
int  __cdecl ModernGetSettingDword_Helper  ( HANDLE hContact, const char *szModule, const char *szSetting, int errorValue, const char *szFile, const int nLine);
int  __cdecl ModernGetSettingString_Helper ( HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv, const char *szFile, const int nLine, const int nType);
int  __cdecl ModernGetSetting_Helper       ( HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv, const char *szFile, const int nLine);

int __cdecl ModernWriteSettingByte       ( HANDLE hContact, const char *szModule, const char *szSetting, BYTE  val  );
int __cdecl ModernWriteSettingWord       ( HANDLE hContact, const char *szModule, const char *szSetting, WORD  val  );
int __cdecl ModernWriteSettingDword      ( HANDLE hContact, const char *szModule, const char *szSetting, DWORD val  );
int __cdecl ModernWriteSettingString     ( HANDLE hContact, const char *szModule, const char *szSetting, const char *val  );


int __cdecl ModernDeleteSetting     ( HANDLE hContact, const char *szModule, const char *szSetting);
int __cdecl ModernDBFreeVariant     ( DBVARIANT *dbv );


#ifdef _UNICODE
	int __cdecl ModernWriteSettingWString    ( HANDLE hContact, const char *szModule, const char *szSetting, const WCHAR *val );
	#define ModernWriteSettingTString(a,b,c,d) ModernWriteSettingWString( a,b,c,d )
    #define ModernGetSettingTString(a,b,c,d)   ModernGetSettingWString(a,b,c,d)
#else
    #define ModernWriteSettingTString(a,b,c,d) ModernWriteSettingString( a,b,c,d )
    #define ModernGetSettingTString(a,b,c,d)   ModernGetSettingString(a,b,c,d)
#endif //_UNICODE


#define EXTRACOLUMNCOUNT 10


#endif // commonheaders_h__
