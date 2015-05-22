#define MIID_UPDATER	{0x4a47b19b, 0xde5a, 0x4436, { 0xab, 0x4b, 0xe1, 0xf3, 0xa0, 0x22, 0x5d, 0xe7}}

#include <m_database.h>
#include "..\..\..\..\include\m_pluginupdater.h"

#define db_free(A) DBFreeVariant(A)

#define db_get_b(A,B,C,D)  DBGetContactSettingByte(A,B,C,D)
#define db_get_dw(A,B,C,D) DBGetContactSettingDword(A,B,C,D)
#define db_get_s(A,B,C,D)  DBGetContactSettingString(A,B,C,D)
#define db_get_ts(A,B,C,D) DBGetContactSettingTString(A,B,C,D)

#define db_set_b(A,B,C,D)  DBWriteContactSettingByte(A,B,C,D)
#define db_set_dw(A,B,C,D) DBWriteContactSettingDword(A,B,C,D)
#define db_set_s(A,B,C,D)  DBWriteContactSettingString(A,B,C,D)
#define db_set_ts(A,B,C,D) DBWriteContactSettingTString(A,B,C,D)

#define db_get_sa				DBGetStringA
#define db_get_wsa			DBGetStringW
#define db_get_tsa			DBGetStringT

#define PUAddPopupT PUAddPopUpT
#define PUDeletePopup PUDeletePopUp

#define NETLIB_USER_AGENT "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)"

template<class T> class mir_ptr
{
	T *data;

public:
	__inline mir_ptr() : data((T*)mir_calloc(sizeof(T))) {}
	__inline mir_ptr(T *_p) : data(_p) {}
	__inline ~mir_ptr() { mir_free(data); }
	__inline T *operator = (T *_p) { if (data) mir_free(data); data = _p; return data; }
	__inline T *operator->() const { return data; }
	__inline operator T*() const { return data; }
	__inline operator INT_PTR() const { return (INT_PTR)data; }
};

typedef mir_ptr<char>  ptrA;
typedef mir_ptr<TCHAR> ptrT;

struct VARST : public ptrT
{
	__forceinline VARST(const TCHAR *str) :
		ptrT( Utils_ReplaceVarsT(str))
		{}
};

class _A2T
{
	TCHAR *buf;

public:
	__forceinline _A2T(const char *s) : buf(mir_a2t(s)) {}
	__forceinline _A2T(const char *s, int cp) : buf(mir_a2t_cp(s, cp)) {}
	~_A2T() { mir_free(buf); }

	__forceinline operator TCHAR*() const
	{	return buf;
	}
};

class _T2A
{
	char *buf;

public:
	__forceinline _T2A(const TCHAR *s) : buf(mir_t2a(s)) {}
	__forceinline _T2A(const TCHAR *s, int cp) : buf(mir_t2a_cp(s, cp)) {}
	__forceinline ~_T2A() { mir_free(buf); }

	__forceinline operator char*() const
	{	return buf;
	}
};

struct
{
	char *szIconName;
	char *szDescr;
	int   IconID;
}
static iconList[] =
{
	{ "check_update", LPGEN("Check for updates"),           IDI_MENU },
	{ "info",         LPGEN("Plugin info"),                 IDI_INFO },
	{ "plg_list",     LPGEN("Component list"),              IDI_PLGLIST },
	{ "plg_restart",  LPGEN("Restart"),                     IDI_RESTART },
};

__forceinline INT_PTR Options_Open(OPENOPTIONSDIALOG *ood)
{
	return CallService("Opt/OpenOptions", 0, (LPARAM)ood);
}

__forceinline INT_PTR Options_AddPage(WPARAM wParam, OPTIONSDIALOGPAGE *odp)
{
	return CallService("Opt/AddPage", wParam, (LPARAM)odp);
}

char *bin2hex(const void *pData, size_t len, char *dest);
char *rtrim(char *str);
void CreatePathToFileT(TCHAR *ptszPath);
int wildcmpit(const WCHAR *name, const WCHAR *mask);
void InitIcoLib()

#define NEWTSTR_ALLOCA(A) (A == NULL)?NULL:_tcscpy((TCHAR*)alloca((_tcslen(A)+1) *sizeof(TCHAR)), A)

__forceinline HANDLE Skin_AddIcon(SKINICONDESC *si)
{	return (HANDLE)CallService("Skin2/Icons/AddIcon", 0, (LPARAM)si);
}

__forceinline HICON Skin_GetIconByHandle(HANDLE hIcolibIcon, int size=0)
{	return (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, size, (LPARAM)hIcolibIcon);
}

__forceinline HANDLE Skin_GetIconHandle(const char *szIconName)
{	return (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)szIconName);
}

__forceinline HICON Skin_GetIcon(const char *szIconName, int size=0)
{	return (HICON)CallService(MS_SKIN2_GETICON, size, (LPARAM)szIconName);
}

__forceinline void Skin_ReleaseIcon(const char* szIconName)
{	CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)szIconName);
}

__forceinline void Skin_ReleaseIcon(HICON hIcon)
{	CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
}

__forceinline HGENMENU Menu_AddMainMenuItem(CLISTMENUITEM *mi)
{	return (HGENMENU)CallService("CList/AddMainMenuItem", 0, (LPARAM)mi);
}

__forceinline INT_PTR Hotkey_Register(HOTKEYDESC *hk)
{	return CallService("CoreHotkeys/Register", 0, (LPARAM)hk);
}

__forceinline INT_PTR CreateDirectoryTreeT(const TCHAR *ptszPath)
{	return CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)ptszPath);
}

#define _qtoupper(_c) (((_c) >= 'a' && (_c) <= 'z')?((_c)-('a'+'A')):(_c))


