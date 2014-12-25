/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

extern "C"
{
	MIR_CORE_DLL(int) Langpack_MarkPluginLoaded(PLUGININFOEX* pInfo);
	MIR_CORE_DLL(MUUID*) Langpack_LookupUuid(WPARAM wParam);
};

void UnloadLangPackModule(void);

int  InitialiseModularEngine(void);
void DestroyModularEngine(void);

int  InitPathUtils(void);

void InitProtocols();
void UninitProtocols();

extern HINSTANCE hInst;
extern HWND hAPCWindow;
extern HANDLE hStackMutex, hThreadQueueEmpty;
extern MIDatabase *currDb;

/**** modules.cpp **********************************************************************/

struct THookSubscriber
{
	HINSTANCE hOwner;
	int type;
	union {
		struct {
			union {
				MIRANDAHOOK pfnHook;
				MIRANDAHOOKPARAM pfnHookParam;
				MIRANDAHOOKOBJ pfnHookObj;
				MIRANDAHOOKOBJPARAM pfnHookObjParam;
			};
			void* object;
			LPARAM lParam;
		};
		struct {
			HWND hwnd;
			UINT message;
		};
	};
};

#define HOOK_SECRET_SIGNATURE 0xDEADBABA

struct THook
{
	char name[ MAXMODULELABELLENGTH ];
	int  id;
	int  subscriberCount;
	THookSubscriber* subscriber;
	MIRANDAHOOK pfnHook;
	DWORD secretSignature;
	CRITICAL_SECTION csHook;
};

extern LIST<HINSTANCE__> pluginListAddr;

/**** langpack.cpp *********************************************************************/

char*  LangPackTranslateString(MUUID* pUuid, const char *szEnglish, const int W);
TCHAR* LangPackTranslateStringT(int hLangpack, const TCHAR* tszEnglish);

/**** options.cpp **********************************************************************/

HTREEITEM FindNamedTreeItemAtRoot(HWND hwndTree, const TCHAR *name);

/**** subclass.cpp *********************************************************************/

/**** threads.cpp **********************************************************************/

extern DWORD mir_tls;

/**** utils.cpp ************************************************************************/

void HotkeyToName(TCHAR *buf, int size, BYTE shift, BYTE key);
WORD GetHotkeyValue(INT_PTR idHotkey);

HBITMAP ConvertIconToBitmap(HICON hIcon, HIMAGELIST hIml, int iconId);

class StrConvUT
{
private:
	wchar_t* m_body;

public:
	StrConvUT(const char* pSrc) :
		m_body(mir_a2u(pSrc)) {}

    ~StrConvUT() {  mir_free(m_body); }
	operator const wchar_t* () const { return m_body; }
};

class StrConvAT
{
private:
	char* m_body;

public:
	StrConvAT(const wchar_t* pSrc) :
		m_body(mir_u2a(pSrc)) {}

    ~StrConvAT() {  mir_free(m_body); }
	operator const char*  () const { return m_body; }
	operator const wchar_t* () const { return (wchar_t*)m_body; }  // type cast to fake the interface definition
	operator const LPARAM () const { return (LPARAM)m_body; }
};

#define StrConvT(x) StrConvUT(x)
#define StrConvTu(x) x
#define StrConvA(x) StrConvAT(x)
#define StrConvU(x) x
