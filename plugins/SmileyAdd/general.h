/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GENERAL_
#define _GENERAL_

#define _HAS_EXCEPTIONS 0
#define _SECURE_SCL 0
#define _SECURE_SCL_THROWS 0
#define __STDC_WANT_SECURE_LIB__ 0
#define _STRALIGN_USE_SECURE_CRT 0
#define _NO_EXCEPTIONS

#include <m_stdhdr.h>

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#include <windows.h>
#include <gdiplus.h>

#include <commctrl.h>
#include <richedit.h>

#include "bkstring.h"

#undef _MT

#pragma warning( push )
#pragma warning( disable : 4530 )
#pragma warning( disable : 4275 )
#pragma warning( disable : 4390 )

#if defined _UNICODE || defined UNICODE

#include <wcpattern.h>
#include <wcmatcher.h>

typedef WCPattern _TPattern;
typedef WCMatcher _TMatcher;
#define createTMatcher createWCMatcher

#else

#include <pattern.h>
#include <matcher.h>

typedef Pattern _TPattern;
typedef Matcher _TMatcher;
#define createTMatcher createMatcher

#endif

#pragma warning( pop )

#define _MT

#include "resource.h"

#pragma warning( push )
#pragma warning( disable : 4100 )
#define MIRANDA_VER 0x0A00
#define NETLIB_NOLOGGING

#include <win2k.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_button.h>
#include <m_message.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_netlib.h>
#include <m_icolib.h>
#include <m_genmenu.h>
#include <m_imgsrvc.h>
#pragma warning( pop )

//globals, defined int main.cpp
extern HINSTANCE g_hInst;
extern char* metaProtoName;

//some system and NT stuff...
#ifndef OPENFILENAME_SIZE_VERSION_400
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif


#define IDC_SMLBUTTON        (WM_USER + 33)

///////////////////////////////////////////////////
//
//defines from miranda im sources (undocumented!)
//
#define MI_IDC_LOG           1001  //rich edit
#define MI_IDC_MESSAGE       1002  //edit control
#define MI_IDC_QUOTE         1034  //button control
#define MI_IDC_NAME          1009  //text control
#define MI_IDC_ADD           1070  //Add button.

#define DM_REMAKELOG         (WM_USER + 11)
#define DM_OPTIONSAPPLIED    (WM_USER + 14)
#define DM_APPENDTOLOG       (WM_USER + 17)

#define WM_REMAKERICH        (WM_USER + 0x3457)

/////////////////////////////////////////////////////

#define MAX_SMILEY_LENGTH    96

class A2W_SM
{
public:
	wchar_t* m_psz;

	A2W_SM(const char* psz, unsigned nCodePage = CP_ACP)
	{
		const int nLength = MultiByteToWideChar(nCodePage, 0, psz, -1, NULL, 0);
		m_psz = new wchar_t[nLength];
		MultiByteToWideChar(nCodePage, 0, psz, -1, m_psz, nLength);
	}
	~A2W_SM() { delete [] m_psz; }
	operator wchar_t*() const { return m_psz; }
};


class W2A_SM
{
public:
	char* m_psz;

	W2A_SM(const wchar_t* psz, unsigned nCodePage = CP_ACP)
	{
		const int nLength = WideCharToMultiByte(nCodePage, 0, psz, -1, NULL, 0, NULL, NULL);
		m_psz = new char[nLength];
		WideCharToMultiByte(nCodePage, 0, psz, -1, m_psz, nLength, NULL, NULL);
	}
	~W2A_SM() { delete [] m_psz; }
	operator char*() const { return m_psz; }
};

#if defined _UNICODE || defined UNICODE
#define T2A_SM     (char*)W2A_SM
#define T2W_SM(p1) (wchar_t*)p1
#define A2T_SM     (wchar_t*)A2W_SM
#define W2T_SM(p1) (TCHAR*)p1
#else
#define T2A_SM(p1) (char*)p1
#define T2W_SM     A2W_SM
#define A2T_SM(p1) (TCHAR*)p1
#define W2T_SM     W2A_SM
#endif


template<class T> struct SMOBJLIST : public OBJLIST<T>
{
	SMOBJLIST() : OBJLIST<T>(5) {};

	SMOBJLIST<T>& operator = (const SMOBJLIST<T>& lst)
	{ 
		OBJLIST<T>::destroy();
		return operator += (lst); 
	}

	SMOBJLIST<T>& operator += (const SMOBJLIST<T>& lst)
	{ 		
		for (int i=0; i<lst.getCount(); ++i)
		{
			T *p = new T(lst[i]); 
			insert(p);
		}
		return *this; 
	}

	void splice(SMOBJLIST<T>& lst)
	{
		for (int i=0; i<lst.getCount(); ++i)
			insert(&lst[i]);
		lst.LIST<T>::destroy();
	}
};

inline unsigned short GetWinVer(void)
{
	unsigned short ver = LOWORD(GetVersion());
	ver = (ver & 0xFF) << 8 | (ver >> 8);
	return ver;
}


// init functions
void InstallDialogBoxHook(void);
void RemoveDialogBoxHook(void);
int UpdateSrmmDlg(WPARAM wParam, LPARAM lParam);
bool IsOldSrmm(void);

//functions for general use (defined in general.cpp)
int CalculateTextHeight(HDC hdc, CHARFORMAT2* chf);
const TCHAR* GetImageExt(bkstring &fname);

HANDLE DecodeMetaContact(HANDLE hContact);
bool IsSmileyProto(char* proto);

HICON ImageList_GetIconFixed (HIMAGELIST himl, INT i, UINT fStyle);

void pathToRelative(const bkstring& pSrc, bkstring& pOut);
void pathToAbsolute(const bkstring& pSrc, bkstring& pOut);

bool InitGdiPlus(void);
void DestroyGdiPlus(void);

void ReportError(const TCHAR* errmsg);
HICON GetDefaultIcon(bool copy = true);

void CloseRichCallback(HWND hwnd, bool force);
void CloseRichOwnerCallback(HWND hwnd, bool force);
bool SetRichCallback(HWND hwnd, HANDLE hContact, bool subany, bool subnew);
void SetRichOwnerCallback(HWND hwnd, HWND hwndInput, HWND hwndLog);
void ProcessAllInputAreas(bool restoreText);
void RichEditData_Destroy(void);

void CloseSmileys(void);
void DestroySmileyBase(void);
void DestroyAniSmileys(void);

void UrlDecode(char* str);

#endif
