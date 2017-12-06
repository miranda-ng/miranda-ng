#define MIID_UPDATER	{0x4a47b19b, 0xde5a, 0x4436, { 0xab, 0x4b, 0xe1, 0xf3, 0xa0, 0x22, 0x5d, 0xe7}}

#include <m_database.h>

#define MS_PU_SHOWLIST "PluginUpdater/ShowList"
#define MS_PU_CHECKUPDATES "PluginUpdater/CheckUpdates"

#define db_free(A) DBFreeVariant(A)

#define db_get_b(A,B,C,D)  DBGetContactSettingByte(A,B,C,D)
#define db_get_dw(A,B,C,D) DBGetContactSettingDword(A,B,C,D)
#define db_get_s(A,B,C,D)  DBGetContactSettingString(A,B,C,D)
#define db_get_ts(A,B,C,D) DBGetContactSettingTString(A,B,C,D)

#define db_set_b(A,B,C,D)  DBWriteContactSettingByte(A,B,C,D)
#define db_set_dw(A,B,C,D) DBWriteContactSettingDword(A,B,C,D)
#define db_set_s(A,B,C,D)  DBWriteContactSettingString(A,B,C,D)
#define db_set_ts(A,B,C,D) DBWriteContactSettingTString(A,B,C,D)
#define db_set_ws(A,B,C,D) DBWriteContactSettingWString(A,B,C,D)

#define db_get_sa				DBGetStringA
#define db_get_wsa			DBGetStringW
#define db_get_tsa			DBGetStringT

#define PUAddPopupT PUAddPopUpT
#define PUDeletePopup PUDeletePopUp

#define NETLIB_USER_AGENT "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)"

typedef HANDLE MCONTACT;
typedef HANDLE HNETLIBUSER;
typedef HANDLE HNETLIBCONN;

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
typedef mir_ptr<WCHAR> ptrW;

struct VARSW : public ptrT
{
	__forceinline VARSW(const WCHAR *str) :
		ptrW( Utils_ReplaceVarsT(str))
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
	{ "plg_list",     LPGEN("Component list"),              IDI_PLGLIST }
};

__forceinline void Thread_SetName(const char*) {}
__forceinline void Skin_PlaySound(const char*) {}

__forceinline void Window_FreeIcon_IcoLib(HWND) {}

__forceinline int Clist_TrayNotifyW(const char *szProto, const wchar_t *wszInfoTitle, const wchar_t *wszInfo, DWORD dwInfoFlags, UINT uTimeout)
{
	return 1;
}

__forceinline HNETLIBUSER Netlib_RegisterUser(NETLIBUSER *nlu)
{
	return (HNETLIBUSER)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)nlu);
}

__forceinline NETLIBHTTPREQUEST* Netlib_HttpTransaction(HNETLIBUSER nlu, NETLIBHTTPREQUEST *nlhr)
{
	return (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)nlu, (LPARAM)nlhr);
}

__forceinline int Netlib_FreeHttpRequest(NETLIBHTTPREQUEST *nlhr)
{
	return CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhr);
}

char *bin2hex(const void *pData, size_t len, char *dest);
char *rtrim(char *str);
void CreatePathToFileW(WCHAR *ptszPath);
int wildcmpiw(const WCHAR *name, const WCHAR *mask);
void InitIcoLib();
wchar_t* strdelw(wchar_t *str, size_t len);

int mir_vsnwprintf(wchar_t *buffer, size_t count, const wchar_t* fmt, va_list va);
int mir_snwprintf(WCHAR *buffer, size_t count, const WCHAR* fmt, ...);
int mir_wstrcmpi(const wchar_t *p1, const wchar_t *p2);

template <size_t _Size>
inline int mir_snwprintf(wchar_t(&buffer)[_Size], const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = mir_vsnwprintf(buffer, _Size, fmt, args);
	va_end(args);
	return ret;
}

#define NEWTSTR_ALLOCA(A) (A == NULL)?NULL:_tcscpy((TCHAR*)alloca((_tcslen(A)+1) *sizeof(TCHAR)), A)

__forceinline INT_PTR Options_Open(OPENOPTIONSDIALOG *ood) {
	return CallService(MS_OPT_OPENOPTIONS, 0, (LPARAM)ood);
}

__forceinline INT_PTR Options_AddPage(WPARAM wParam, OPTIONSDIALOGPAGE *odp) {
	return CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)odp);
}

__forceinline HANDLE IcoLib_AddIcon(SKINICONDESC *si) {
	return (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)si);
}

__forceinline HICON IcoLib_GetIconByHandle(HANDLE hIcolibIcon, int size=0) {
	return (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, size, (LPARAM)hIcolibIcon);
}

__forceinline HANDLE IcoLib_GetIconHandle(const char *szIconName) {
	return (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)szIconName);
}

__forceinline HICON IcoLib_GetIcon(const char *szIconName, int size=0) {
	return (HICON)CallService(MS_SKIN2_GETICON, size, (LPARAM)szIconName);
}

__forceinline void IcoLib_ReleaseIcon(const char* szIconName) {
	CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)szIconName);
}

__forceinline void IcoLib_ReleaseIcon(HICON hIcon) {
	CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
}

__forceinline HGENMENU Menu_AddMainMenuItem(CLISTMENUITEM *mi) {
	return (HGENMENU)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)mi);
}

__forceinline INT_PTR Hotkey_Register(HOTKEYDESC *hk) {
	return CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)hk);
}

__forceinline INT_PTR CreateDirectoryTreeW(const WCHAR *ptszPath) {
	return CallService(MS_UTILS_CREATEDIRTREEW, 0, (LPARAM)ptszPath);
}

__forceinline WCHAR* Utils_ReplaceVarsW(const WCHAR *szData, MCONTACT hContact, REPLACEVARSARRAY *variables) {
	REPLACEVARSDATA vars;
	vars.cbSize = sizeof(vars);
	vars.dwFlags = RVF_TCHAR;
	vars.hContact = hContact;
	vars.variables = variables;
	return (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)szData, (LPARAM)&vars);
}

void strdelt(TCHAR *parBuffer, int len)
{
	TCHAR* p;
	for (p = parBuffer + len; *p != 0; p++)
		p[-len] = *p;

	p[-len] = '\0';
}

#define _qtoupper(_c) (((_c) >= 'a' && (_c) <= 'z')?((_c)-('a'+'A')):(_c))
