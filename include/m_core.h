/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_CORE_H__
#define M_CORE_H__ 1

#include <stdint.h>

#ifndef M_TYPES_H__
   #include <m_types.h>
#endif

#ifdef MIR_CORE_EXPORTS
	#define MIR_CORE_EXPORT __declspec(dllexport)
#else
	#define MIR_CORE_EXPORT __declspec(dllimport)
#endif

#define MIR_CORE_DLL(T) MIR_CORE_EXPORT T __stdcall
#define MIR_C_CORE_DLL(T) MIR_CORE_EXPORT T __cdecl

#pragma warning(disable:4201 4127 4706)

#if defined(__cplusplus)
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
// command line support

MIR_CORE_DLL(void)    CmdLine_Parse(LPTSTR ptszCmdLine);
MIR_CORE_DLL(LPCTSTR) CmdLine_GetOption(LPCTSTR ptszParameter);

///////////////////////////////////////////////////////////////////////////////
// database functions

typedef uint32_t MCONTACT;
#define INVALID_CONTACT_ID (MCONTACT(-1))

typedef uint32_t MEVENT;



///////////////////////////////////////////////////////////////////////////////
// events, hooks & services

#define MAXMODULELABELLENGTH 64

typedef int (*MIRANDAHOOK)(WPARAM, LPARAM);
typedef int (*MIRANDAHOOKPARAM)(WPARAM, LPARAM, LPARAM);
typedef int (*MIRANDAHOOKOBJ)(void*, WPARAM, LPARAM);
typedef int (*MIRANDAHOOKOBJPARAM)(void*, WPARAM, LPARAM, LPARAM);

typedef INT_PTR (*MIRANDASERVICE)(WPARAM, LPARAM);
typedef INT_PTR (*MIRANDASERVICEPARAM)(WPARAM, LPARAM, LPARAM);
typedef INT_PTR (*MIRANDASERVICEOBJ)(void*, WPARAM, LPARAM);
typedef INT_PTR (*MIRANDASERVICEOBJPARAM)(void*, WPARAM, LPARAM, LPARAM);

#ifdef _WIN64
	#define CALLSERVICE_NOTFOUND      ((INT_PTR)0x8000000000000000)
#else
	#define CALLSERVICE_NOTFOUND      ((int)0x80000000)
#endif

MIR_CORE_DLL(HANDLE)  CreateHookableEvent(const char *name);
MIR_CORE_DLL(int)     DestroyHookableEvent(HANDLE hEvent);
MIR_CORE_DLL(int)     SetHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK pfnHook);
MIR_CORE_DLL(int)     CallPluginEventHook(HINSTANCE hInst, HANDLE hEvent, WPARAM wParam, LPARAM lParam);
MIR_CORE_DLL(int)     NotifyEventHooks(HANDLE hEvent, WPARAM wParam, LPARAM lParam);
MIR_CORE_DLL(int)     NotifyFastHook(HANDLE hEvent, WPARAM wParam, LPARAM lParam);

MIR_CORE_DLL(HANDLE)  HookEvent(const char* name, MIRANDAHOOK hookProc);
MIR_CORE_DLL(HANDLE)  HookEventParam(const char* name, MIRANDAHOOKPARAM hookProc, LPARAM lParam);
MIR_CORE_DLL(HANDLE)  HookEventObj(const char* name, MIRANDAHOOKOBJ hookProc, void* object);
MIR_CORE_DLL(HANDLE)  HookEventObjParam(const char* name, MIRANDAHOOKOBJPARAM hookProc, void* object, LPARAM lParam);
MIR_CORE_DLL(HANDLE)  HookEventMessage(const char* name, HWND hwnd, UINT message);
MIR_CORE_DLL(int)     UnhookEvent(HANDLE hHook);
MIR_CORE_DLL(void)    KillObjectEventHooks(void* pObject);
MIR_CORE_DLL(void)    KillModuleEventHooks(HINSTANCE pModule);

MIR_CORE_DLL(HANDLE)  CreateServiceFunction(const char *name, MIRANDASERVICE serviceProc);
MIR_CORE_DLL(HANDLE)  CreateServiceFunctionParam(const char *name, MIRANDASERVICEPARAM serviceProc, LPARAM lParam);
MIR_CORE_DLL(HANDLE)  CreateServiceFunctionObj(const char *name, MIRANDASERVICEOBJ serviceProc, void* object);
MIR_CORE_DLL(HANDLE)  CreateServiceFunctionObjParam(const char *name, MIRANDASERVICEOBJPARAM serviceProc, void* object, LPARAM lParam);
MIR_CORE_DLL(HANDLE)  CreateProtoServiceFunction(const char *szModule, const char *szService, MIRANDASERVICE serviceProc);
MIR_CORE_DLL(int)     DestroyServiceFunction(HANDLE hService);
MIR_CORE_DLL(int)     ServiceExists(const char *name);

MIR_CORE_DLL(INT_PTR) CallService(const char *name, WPARAM wParam, LPARAM lParam);
MIR_CORE_DLL(INT_PTR) CallServiceSync(const char *name, WPARAM wParam, LPARAM lParam);

MIR_CORE_DLL(int)     CallFunctionAsync(void (__stdcall *func)(void *), void *arg);
MIR_CORE_DLL(void)    KillModuleServices(HINSTANCE hInst);
MIR_CORE_DLL(void)    KillObjectServices(void* pObject);

#if defined(_STATIC)
__declspec(dllexport) INT_PTR CallContactService(MCONTACT, const char *, WPARAM, LPARAM);
__declspec(dllexport) INT_PTR CallProtoService(LPCSTR szModule, const char *szService, WPARAM wParam, LPARAM lParam);
#else
MIR_C_CORE_DLL(INT_PTR) CallContactService(MCONTACT, const char *, WPARAM, LPARAM);
MIR_C_CORE_DLL(INT_PTR) CallProtoService(LPCSTR szModule, const char *szService, WPARAM wParam, LPARAM lParam);
#endif

///////////////////////////////////////////////////////////////////////////////
// exceptions

typedef DWORD (__cdecl *pfnExceptionFilter)(DWORD code, EXCEPTION_POINTERS* info);

MIR_CORE_DLL(pfnExceptionFilter) GetExceptionFilter(void);
MIR_CORE_DLL(pfnExceptionFilter) SetExceptionFilter(pfnExceptionFilter pMirandaExceptFilter);

///////////////////////////////////////////////////////////////////////////////
// http support

MIR_CORE_DLL(char*) mir_urlEncode(const char *szUrl);

///////////////////////////////////////////////////////////////////////////////
// icons support

extern int hLangpack;

typedef struct tagIconItem
{
	char  *szDescr, *szName;
	int    defIconID, size;
	HANDLE hIcolib;
}
	IconItem;

typedef struct tagIconItemT
{
	TCHAR *tszDescr;
	char  *szName;
	int    defIconID, size;
	HANDLE hIcolib;
}
	IconItemT;

#if defined( __cplusplus )
MIR_CORE_DLL(void) Icon_Register(HINSTANCE hInst, const char* szSection, IconItem* pIcons, size_t iCount, char *prefix = NULL, int = hLangpack);
MIR_CORE_DLL(void) Icon_RegisterT(HINSTANCE hInst, const TCHAR* szSection, IconItemT* pIcons, size_t iCount, char *prefix = NULL, int = hLangpack);
#else
MIR_CORE_DLL(void) Icon_Register(HINSTANCE hInst, const char* szSection, IconItem* pIcons, size_t iCount, char *prefix, int hLangpack);
MIR_CORE_DLL(void) Icon_RegisterT(HINSTANCE hInst, const TCHAR* szSection, IconItemT* pIcons, size_t iCount, char *prefix, int hLangpack);
#endif

///////////////////////////////////////////////////////////////////////////////
// language packs support

#define LANG_UNICODE 0x1000

MIR_CORE_DLL(void)     Langpack_SortDuplicates(void);
MIR_CORE_DLL(int)      Langpack_GetDefaultCodePage( void );
MIR_CORE_DLL(int)      Langpack_GetDefaultLocale(void);
MIR_CORE_DLL(TCHAR*)   Langpack_PcharToTchar(const char* pszStr);

MIR_CORE_DLL(int)      LoadLangPackModule(void);
MIR_CORE_DLL(int)      LoadLangPack(const TCHAR *szLangPack);
MIR_CORE_DLL(void)     ReloadLangpack(TCHAR *pszStr);

MIR_CORE_DLL(char*)    TranslateA_LP(const char* str, int hLang);
MIR_CORE_DLL(wchar_t*) TranslateW_LP(const wchar_t* str, int hLang);
MIR_CORE_DLL(void)     TranslateMenu_LP(HMENU, int hLang);
MIR_CORE_DLL(void)     TranslateDialog_LP(HWND hDlg, int hLang);

#define Translate(s) TranslateA_LP(s, hLangpack)
#define TranslateW(s) TranslateW_LP(s, hLangpack)
#define TranslateMenu(h) TranslateMenu_LP(h,hLangpack)
#define TranslateDialogDefault(h) TranslateDialog_LP(h,hLangpack)

#ifdef _UNICODE
	#define TranslateT(s)	 TranslateW_LP(_T(s),hLangpack)
	#define TranslateTS(s)	 TranslateW_LP(s,hLangpack)
	#define TranslateTH(l,s) TranslateW_LP(s,l)
#else
	#define TranslateT(s)	 TranslateA_LP(s,hLangpack)
	#define TranslateTS(s)	 TranslateA_LP(s,hLangpack)
	#define TranslateTH(l,s) TranslateA_LP(s,l)
#endif

MIR_CORE_DLL(unsigned int) mir_hash(const void * key, unsigned int len);

#pragma optimize("gt", on)
__forceinline unsigned int mir_hashstr(const char * key)
{
	if (key == NULL) return 0;
	else {
		unsigned int len = (unsigned int)strlen((const char*)key);
		return mir_hash(key, len);
}	}

__forceinline unsigned int mir_hashstrW(const wchar_t * key)
{
	if (key == NULL) return 0;
	else {
		unsigned int len = (unsigned int)wcslen((const wchar_t*)key);
		return mir_hash(key, len * sizeof(wchar_t));
}	}
#pragma optimize("", on)

#define mir_hashstrT mir_hashstrW

///////////////////////////////////////////////////////////////////////////////
// lists

typedef int (*FSortFunc)(void*, void*);  // sort function prototype

// Assumes first 32 bit value of the data is the numeric key
// and uses it to perform sort/search operations, this results
// in much better performance as no compare function calls needed
// Incredibly useful for Hash Tables
#define NumericKeySort (FSortFunc)(void*) -1
#define HandleKeySort  (FSortFunc)(void*) -2
#define PtrKeySort     (FSortFunc)(void*) -3

typedef struct
{
	void**		items;
	int			realCount;
	int			limit;
	int			increment;

	FSortFunc	sortFunc;
}
	SortedList;

MIR_CORE_DLL(SortedList*) List_Create(int p_limit, int p_increment);
MIR_CORE_DLL(void)        List_Destroy(SortedList* p_list);
MIR_CORE_DLL(void*)       List_Find(SortedList* p_list, void* p_value);
MIR_CORE_DLL(int)         List_GetIndex(SortedList* p_list, void* p_value, int* p_index);
MIR_CORE_DLL(int)         List_IndexOf(SortedList* p_list, void* p_value);
MIR_CORE_DLL(int)         List_Insert(SortedList* p_list, void* p_value, int p_index);
MIR_CORE_DLL(int)         List_InsertPtr(SortedList* list, void* p);
MIR_CORE_DLL(int)         List_Remove(SortedList* p_list, int index);
MIR_CORE_DLL(int)         List_RemovePtr(SortedList* list, void* p);
MIR_CORE_DLL(void)        List_Copy(SortedList* s, SortedList* d, size_t itemSize);
MIR_CORE_DLL(void)        List_ObjCopy(SortedList* s, SortedList* d, size_t itemSize);

///////////////////////////////////////////////////////////////////////////////
// logging functions

MIR_CORE_DLL(HANDLE) mir_createLog(const char *pszName, const TCHAR *ptszDescr, const TCHAR *ptszFile, unsigned options);
MIR_CORE_DLL(void)   mir_closeLog(HANDLE hLogger);

MIR_C_CORE_DLL(int) mir_writeLogA(HANDLE hLogger, const char *format, ...);
MIR_C_CORE_DLL(int) mir_writeLogW(HANDLE hLogger, const wchar_t *format, ...);

MIR_CORE_DLL(int)   mir_writeLogVA(HANDLE hLogger, const char *format, va_list args);
MIR_CORE_DLL(int)   mir_writeLogVW(HANDLE hLogger, const wchar_t *format, va_list args);

///////////////////////////////////////////////////////////////////////////////
// md5 functions

typedef struct mir_md5_state_s {
	UINT32 count[2];  /* message length in bits, lsw first */
	UINT32 abcd[4];    /* digest buffer */
	BYTE   buf[64];    /* accumulate block */
} mir_md5_state_t;

MIR_CORE_DLL(void) mir_md5_init(mir_md5_state_t *pms);
MIR_CORE_DLL(void) mir_md5_append(mir_md5_state_t *pms, const BYTE *data, int nbytes);
MIR_CORE_DLL(void) mir_md5_finish(mir_md5_state_t *pms, BYTE digest[16]);
MIR_CORE_DLL(void) mir_md5_hash(const BYTE *data, int len, BYTE digest[16]);

///////////////////////////////////////////////////////////////////////////////
// memory functions

MIR_C_CORE_DLL(void*)  mir_alloc(size_t);
MIR_C_CORE_DLL(void*)  mir_calloc(size_t);
MIR_C_CORE_DLL(void*)  mir_realloc(void* ptr, size_t);
MIR_C_CORE_DLL(void)   mir_free(void* ptr);

MIR_CORE_DLL(size_t)   mir_strlen(const char *p);
MIR_CORE_DLL(size_t)   mir_wstrlen(const wchar_t *p);

MIR_CORE_DLL(char*)    mir_strcpy(char *dest, const char *src);
MIR_CORE_DLL(wchar_t*) mir_wstrcpy(wchar_t *dest, const wchar_t *src);

MIR_CORE_DLL(char*)    mir_strncpy(char *dest, const char *src, size_t len);
MIR_CORE_DLL(wchar_t*) mir_wstrncpy(wchar_t *dest, const wchar_t *src, size_t len);

MIR_CORE_DLL(char*)    mir_strcat(char *dest, const char *src);
MIR_CORE_DLL(wchar_t*) mir_wstrcat(wchar_t *dest, const wchar_t *src);

MIR_CORE_DLL(char*)    mir_strncat(char *dest, const char *src, size_t len);
MIR_CORE_DLL(wchar_t*) mir_wstrncat(wchar_t *dest, const wchar_t *src, size_t len);

MIR_CORE_DLL(int)      mir_strcmp(const char *p1, const char *p2);
MIR_CORE_DLL(int)      mir_wstrcmp(const wchar_t *p1, const wchar_t *p2);

MIR_CORE_DLL(int)      mir_strcmpi(const char *p1, const char *p2);
MIR_CORE_DLL(int)      mir_wstrcmpi(const wchar_t *p1, const wchar_t *p2);

MIR_CORE_DLL(char*)    mir_strdup(const char* str);
MIR_CORE_DLL(wchar_t*) mir_wstrdup(const wchar_t* str);

MIR_CORE_DLL(char*)    mir_strndup(const char* str, size_t len);
MIR_CORE_DLL(wchar_t*) mir_wstrndup(const wchar_t *str, size_t len);

///////////////////////////////////////////////////////////////////////////////
// modules

MIR_CORE_DLL(void) RegisterModule(HINSTANCE hInst);
MIR_CORE_DLL(void) UnregisterModule(HINSTANCE hInst);

MIR_CORE_DLL(HINSTANCE) GetInstByAddress(void* codePtr);

///////////////////////////////////////////////////////////////////////////////
// path utils

MIR_CORE_DLL(void)   CreatePathToFile(char* wszFilePath);
MIR_CORE_DLL(int)    CreateDirectoryTree(const char* szDir);
MIR_CORE_DLL(int)    PathIsAbsolute(const char *pSrc);

MIR_CORE_DLL(void)   CreatePathToFileW(wchar_t* wszFilePath);
MIR_CORE_DLL(int)    CreateDirectoryTreeW(const wchar_t* szDir);
MIR_CORE_DLL(int)    PathIsAbsoluteW(const wchar_t *pSrc);

#if defined( __cplusplus )
	MIR_CORE_DLL(int) PathToAbsolute(const char *pSrc, char *pOut, const char* base=0);
	MIR_CORE_DLL(int) PathToAbsoluteW(const wchar_t *pSrc, wchar_t *pOut, const wchar_t* base = 0);

	MIR_CORE_DLL(int) PathToRelative(const char *pSrc, char *pOut, const char* base = 0);
	MIR_CORE_DLL(int) PathToRelativeW(const wchar_t *pSrc, wchar_t *pOut, const wchar_t* base = 0);
#else
	MIR_CORE_DLL(int) PathToAbsolute(const char *pSrc, char *pOut, const char* base);
	MIR_CORE_DLL(int) PathToAbsoluteW(const wchar_t *pSrc, wchar_t *pOut, const wchar_t* base);

	MIR_CORE_DLL(int) PathToRelative(const char *pSrc, char *pOut, const char* base);
	MIR_CORE_DLL(int) PathToRelativeW(const wchar_t *pSrc, wchar_t *pOut, const wchar_t* base);
#endif

#define CreatePathToFileT CreatePathToFileW
#define CreateDirectoryTreeT CreateDirectoryTreeW
#define PathIsAbsoluteT PathIsAbsoluteW
#define PathToAbsoluteT PathToAbsoluteW
#define PathToRelativeT PathToRelativeW

///////////////////////////////////////////////////////////////////////////////
// print functions

MIR_CORE_DLL(int)    mir_snprintf(char *buffer, size_t count, const char* fmt, ...);
MIR_CORE_DLL(int)    mir_snwprintf(wchar_t *buffer, size_t count, const wchar_t* fmt, ...);
MIR_CORE_DLL(int)    mir_vsnprintf(char *buffer, size_t count, const char* fmt, va_list va);
MIR_CORE_DLL(int)    mir_vsnwprintf(wchar_t *buffer, size_t count, const wchar_t* fmt, va_list va);

///////////////////////////////////////////////////////////////////////////////
// protocol functions

struct PROTO_INTERFACE;

MIR_CORE_DLL(int)     ProtoServiceExists(LPCSTR szModule, const char *szService);
MIR_CORE_DLL(INT_PTR) ProtoBroadcastAck(LPCSTR szModule, MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam);

// Call it in the very beginning of your proto's constructor
MIR_CORE_DLL(void) ProtoConstructor(PROTO_INTERFACE *pThis, const char *pszModuleName, const TCHAR *ptszUserName);

// Call it in the very end of your proto's destructor
MIR_CORE_DLL(void) ProtoDestructor(PROTO_INTERFACE *pThis);

#if defined( __cplusplus )
typedef void (__cdecl PROTO_INTERFACE::*ProtoThreadFunc)(void*);
MIR_CORE_DLL(void)   ProtoForkThread(PROTO_INTERFACE *pThis, ProtoThreadFunc, void *param);
MIR_CORE_DLL(HANDLE) ProtoForkThreadEx(PROTO_INTERFACE *pThis, ProtoThreadFunc, void *param, UINT* threadID);
MIR_CORE_DLL(void)   ProtoWindowAdd(PROTO_INTERFACE *pThis, HWND hwnd);
MIR_CORE_DLL(void)   ProtoWindowRemove(PROTO_INTERFACE *pThis, HWND hwnd);

typedef int (__cdecl PROTO_INTERFACE::*ProtoEventFunc)(WPARAM, LPARAM);
MIR_CORE_DLL(void)   ProtoHookEvent(PROTO_INTERFACE *pThis, const char* szName, ProtoEventFunc pFunc);
MIR_CORE_DLL(HANDLE) ProtoCreateHookableEvent(PROTO_INTERFACE *pThis, const char* szService);

typedef INT_PTR (__cdecl PROTO_INTERFACE::*ProtoServiceFunc)(WPARAM, LPARAM);
MIR_CORE_DLL(void) ProtoCreateService(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFunc);

typedef INT_PTR (__cdecl PROTO_INTERFACE::*ProtoServiceFuncParam)(WPARAM, LPARAM, LPARAM);
MIR_CORE_DLL(void) ProtoCreateServiceParam(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFuncParam, LPARAM);
#endif

MIR_CORE_DLL(void) ProtoLogA(PROTO_INTERFACE *pThis, LPCSTR szFormat, va_list args);
MIR_CORE_DLL(void) ProtoLogW(PROTO_INTERFACE *pThis, LPCWSTR wszFormat, va_list args);

// avatar support functions

// returns image extension by a PA_* constant or empty string for PA_FORMAT_UNKNOWN
MIR_CORE_DLL(const TCHAR*) ProtoGetAvatarExtension(int format);

// detects image format by extension
MIR_CORE_DLL(int) ProtoGetAvatarFormat(const TCHAR *ptszFileName);

// detects image format by its contents
MIR_CORE_DLL(int) ProtoGetAvatarFileFormat(const TCHAR *ptszFileName);

// returns the image format and extension by the first bytes of picture
// ptszExtension might be NULL
#if defined( __cplusplus )
	MIR_CORE_DLL(int) ProtoGetBufferFormat(const void *buf, const TCHAR **ptszExtension = NULL);
#else
	MIR_CORE_DLL(int) ProtoGetBufferFormat(const void *buf, const TCHAR **ptszExtension);
#endif

///////////////////////////////////////////////////////////////////////////////
// sha1 functions

#define MIR_SHA1_HASH_SIZE 20

typedef struct {
  ULONG H[5];
  ULONG W[80];
  int lenW;
  ULONG sizeHi, sizeLo;
} mir_sha1_ctx;

MIR_CORE_DLL(void) mir_sha1_init(mir_sha1_ctx *ctx);
MIR_CORE_DLL(void) mir_sha1_append(mir_sha1_ctx *ctx, const BYTE *dataIn, int len);
MIR_CORE_DLL(void) mir_sha1_finish(mir_sha1_ctx *ctx, BYTE hashout[MIR_SHA1_HASH_SIZE]);
MIR_CORE_DLL(void) mir_sha1_hash(BYTE *dataIn, int len, BYTE hashout[MIR_SHA1_HASH_SIZE]);

MIR_CORE_DLL(void) mir_hmac_sha1(BYTE hashout[MIR_SHA1_HASH_SIZE], const BYTE *key, size_t keylen, const BYTE *text, size_t textlen);

///////////////////////////////////////////////////////////////////////////////
// strings

MIR_CORE_DLL(void*) mir_base64_decode(const char *input, unsigned *outputLen);
MIR_CORE_DLL(char*) mir_base64_encode(const BYTE *input, unsigned inputLen);
MIR_CORE_DLL(char*) mir_base64_encodebuf(const BYTE *input, unsigned inputLen, char *output, unsigned outLen);

__forceinline unsigned mir_base64_encode_bufsize(unsigned inputLen)
{
	return 4 * ((inputLen + 2) / 3) + 1;
}

MIR_CORE_DLL(char*)  rtrim(char *str);
MIR_CORE_DLL(wchar_t*) rtrimw(wchar_t *str);

MIR_CORE_DLL(char*)  ltrim(char *str);   // returns pointer to the beginning of string
MIR_CORE_DLL(wchar_t*) ltrimw(wchar_t *str);

MIR_CORE_DLL(char*)  ltrimp(char *str);  // returns pointer to the trimmed portion of string
MIR_CORE_DLL(wchar_t*) ltrimpw(wchar_t *str);

MIR_CORE_DLL(int) wildcmp(const char *name, const char *mask);
MIR_CORE_DLL(int) wildcmpw(const wchar_t *name, const wchar_t *mask);

MIR_CORE_DLL(int) wildcmpi(const char *name, const char *mask);
MIR_CORE_DLL(int) wildcmpiw(const wchar_t *name, const wchar_t *mask);

MIR_CORE_DLL(char*)  bin2hex(const void *pData, size_t len, char *dest);
MIR_CORE_DLL(wchar_t*) bin2hexW(const void *pData, size_t len, wchar_t *dest);

__forceinline char* lrtrim(char *str) { return ltrim(rtrim(str)); };
__forceinline char* lrtrimp(char *str) { return ltrimp(rtrim(str)); };

#if defined( __cplusplus )
	MIR_CORE_DLL(char*) replaceStr(char* &dest, const char *src);
	MIR_CORE_DLL(wchar_t*) replaceStrW(wchar_t* &dest, const wchar_t *src);
#else
	MIR_CORE_DLL(char*) replaceStr(char **dest, const char *src);
	MIR_CORE_DLL(wchar_t*) replaceStrW(wchar_t **dest, const wchar_t *src);
#endif

///////////////////////////////////////////////////////////////////////////////
// text conversion functions

typedef union {
	char *a; // utf8 or ansi strings
	TCHAR *t; // strings of TCHARs
	wchar_t *w; // strings of WCHARs
} MAllStrings;

typedef union {
	char **a; // array of utf8 or ansi strings
	TCHAR **t; // array of strings of TCHARs
	wchar_t **w; // array of strings of WCHARs
} MAllStringArray;

#ifdef _UNICODE
	#define mir_t2a(s) mir_u2a(s)
	#define mir_a2t(s) mir_a2u(s)
	#define mir_t2u(s) mir_wstrdup(s)
	#define mir_u2t(s) mir_wstrdup(s)

	#define mir_t2a_cp(s,c) mir_u2a_cp(s,c)
	#define mir_a2t_cp(s,c) mir_a2u_cp(s,c)
	#define mir_t2u_cp(s,c) mir_wstrdup(s)
	#define mir_u2t_cp(s,c) mir_wstrdup(s)

	#define mir_tstrlen  mir_wstrlen
	#define mir_tstrcpy  mir_wstrcpy
	#define mir_tstrncpy mir_wstrncpy
	#define mir_tstrcat  mir_wstrcat
	#define mir_tstrncat mir_wstrncat
	#define mir_tstrcmp  mir_wstrcmp
	#define mir_tstrcmpi mir_wstrcmpi
	#define mir_tstrdup  mir_wstrdup
	#define mir_tstrndup mir_wstrndup

	#define replaceStrT replaceStrW
	#define bin2hexT    bin2hexW

	#define rtrimt  rtrimw
	#define ltrimt  ltrimw
	#define ltrimpt ltrimpw

	#define wildcmpt  wildcmpw
	#define wildcmpit wildcmpiw

	#define mir_sntprintf  mir_snwprintf
	#define mir_vsntprintf mir_vsnwprintf

	#define mir_writeLogT  mir_writeLogW
	#define mir_writeLogVT mir_writeLogVW
#else
	#define mir_t2a(s) mir_strdup(s)
	#define mir_a2t(s) mir_strdup(s)
	#define mir_t2u(s) mir_a2u(s)
	#define mir_u2t(s) mir_u2a(s)

	#define mir_t2a_cp(s,c) mir_strdup(s)
	#define mir_a2t_cp(s,c) mir_strdup(s)
	#define mir_t2u_cp(s,c) mir_a2u_cp(s,c)
	#define mir_u2t_cp(s,c) mir_u2a_cp(s,c)

	#define mir_tstrlen  mir_strlen
	#define mir_tstrcpy  mir_strcpy
	#define mir_tstrncpy mir_strncpy
	#define mir_tstrcat  mir_strcat
	#define mir_tstrncat mir_strncat
	#define mir_tstrcmp  mir_strcmp
	#define mir_tstrcmpi mir_strcmpi
	#define mir_tstrdup  mir_strdup
	#define mir_tstrndup mir_strndup

	#define replaceStrT replaceStr
	#define bin2hexT    bin2hex

	#define rtrimt rtrim
	#define ltrimt ltrim
	#define ltrimpt ltrimp

	#define wildcmpt wildcmp
	#define wildcmpit wildcmpi

	#define mir_sntprintf  mir_snprintf
	#define mir_vsntprintf mir_vsnprintf

	#define mir_writeLogT  mir_writeLogA
	#define mir_writeLogVT mir_writeLogVA
#endif

MIR_CORE_DLL(wchar_t*) mir_a2u_cp(const char* src, int codepage);
MIR_CORE_DLL(wchar_t*) mir_a2u(const char* src);
MIR_CORE_DLL(char*)  mir_u2a_cp(const wchar_t* src, int codepage);
MIR_CORE_DLL(char*)  mir_u2a(const wchar_t* src);

#if defined(__cplusplus)

class _A2T
{
	TCHAR* buf;

public:
	__forceinline _A2T(const char* s) : buf(mir_a2t(s)) {}
	__forceinline _A2T(const char* s, int cp) : buf(mir_a2t_cp(s, cp)) {}
	~_A2T() { mir_free(buf); }

	__forceinline operator LPARAM() const { return (LPARAM)buf; }
	__forceinline operator TCHAR*() const { return buf; }
};

class _T2A
{
	char* buf;

public:
	__forceinline _T2A(const TCHAR* s) : buf(mir_t2a(s)) {}
	__forceinline _T2A(const TCHAR* s, int cp) : buf(mir_t2a_cp(s, cp)) {}
	__forceinline ~_T2A() { mir_free(buf); }

	__forceinline operator LPARAM() const { return (LPARAM)buf; }
	__forceinline operator char*() const {	return buf; }
};

#endif

///////////////////////////////////////////////////////////////////////////////
// threads

typedef void (__cdecl *pThreadFunc)(void*);
typedef unsigned (__stdcall *pThreadFuncEx)(void*);
typedef unsigned (__cdecl *pThreadFuncOwner)(void *owner, void* param);

#if defined( __cplusplus )
	MIR_CORE_DLL(INT_PTR) Thread_Push(HINSTANCE hInst, void* pOwner=NULL);
#else
	MIR_CORE_DLL(INT_PTR) Thread_Push(HINSTANCE hInst, void* pOwner);
#endif
MIR_CORE_DLL(INT_PTR) Thread_Pop(void);
MIR_CORE_DLL(void)    Thread_Wait(void);

MIR_CORE_DLL(UINT_PTR) forkthread(pThreadFunc, unsigned long stacksize, void *arg);
MIR_CORE_DLL(UINT_PTR) forkthreadex(void *sec, unsigned stacksize, pThreadFuncEx, void* owner, void *arg, unsigned *thraddr);

__forceinline HANDLE mir_forkthread(pThreadFunc aFunc, void* arg)
{	return (HANDLE)forkthread(aFunc, 0, arg);
}

__forceinline HANDLE mir_forkthreadex(pThreadFuncEx aFunc, void* arg, unsigned* pThreadID)
{	return (HANDLE)forkthreadex(NULL, 0, aFunc, NULL, arg, pThreadID);
}

__forceinline HANDLE mir_forkthreadowner(pThreadFuncOwner aFunc, void* owner, void* arg, unsigned* pThreadID)
{	return (HANDLE)forkthreadex(NULL, 0, (pThreadFuncEx)aFunc, owner, arg, pThreadID);
}

MIR_CORE_DLL(void) Thread_SetName(const char *szThreadName);

MIR_CORE_DLL(void) KillObjectThreads(void* pObject);

///////////////////////////////////////////////////////////////////////////////
// utf8 interface

MIR_CORE_DLL(char*) Utf8Decode(char* str, wchar_t** ucs2);
MIR_CORE_DLL(char*) Utf8DecodeCP(char* str, int codepage, wchar_t** ucs2);

MIR_CORE_DLL(wchar_t*) Utf8DecodeW(const char* str);

MIR_CORE_DLL(char*) Utf8Encode(const char* str);
MIR_CORE_DLL(char*) Utf8EncodeCP(const char* src, int codepage);

MIR_CORE_DLL(char*) Utf8EncodeW(const wchar_t* str);
MIR_CORE_DLL(int)   Ucs2toUtf8Len(const wchar_t *src);

MIR_CORE_DLL(BOOL)  Utf8CheckString(const char* str);

#define Utf8DecodeT Utf8DecodeW
#define Utf8EncodeT Utf8EncodeW

#define mir_utf8decode(A, B)      Utf8Decode(A, B)
#define mir_utf8decodecp(A, B, C) Utf8DecodeCP(A, B, C)
#define mir_utf8decodeW(A)   	    Utf8DecodeW(A)
#define mir_utf8encode(A)         Utf8Encode(A)
#define mir_utf8encodecp(A, B)    Utf8EncodeCP(A, B)
#define mir_utf8encodeW(A)        Utf8EncodeW(A)
#define mir_utf8lenW(A)           Ucs2toUtf8Len(A)

__forceinline char* mir_utf8decodeA(const char* src)
{
    char* tmp = mir_strdup(src);
    mir_utf8decode(tmp, NULL);
    return tmp;
}

#if defined(_UNICODE)
	#define mir_utf8decodeT mir_utf8decodeW
	#define mir_utf8encodeT mir_utf8encodeW
#else
	#define mir_utf8decodeT mir_utf8decodeA
	#define mir_utf8encodeT mir_utf8encode
#endif

class T2Utf
{
	char* m_str;

public:
	__forceinline T2Utf(const TCHAR *str) :
		m_str(mir_utf8encodeT(str))
	{}

	__forceinline ~T2Utf()
	{	mir_free(m_str);
	}

	__forceinline char* detach()
	{	char *res = m_str; m_str = NULL;
		return res;
	}

	__forceinline char& operator[](int idx) const { return m_str[idx]; }
	__forceinline operator char*() const {	return m_str; }
	__forceinline operator unsigned char*() const {	return (unsigned char*)m_str; }
	__forceinline operator LPARAM() const { return (LPARAM)m_str; }
	#ifdef _XSTRING_
		std::string str() const { return std::string(m_str); }
	#endif
};

///////////////////////////////////////////////////////////////////////////////
// Window subclassing

MIR_CORE_DLL(void)    mir_subclassWindow(HWND hWnd, WNDPROC wndProc);
MIR_CORE_DLL(void)    mir_subclassWindowFull(HWND hWnd, WNDPROC wndProc, WNDPROC oldWndProc);
MIR_CORE_DLL(LRESULT) mir_callNextSubclass(HWND hWnd, WNDPROC wndProc, UINT uMsg, WPARAM wParam, LPARAM lParam);
MIR_CORE_DLL(void)    mir_unsubclassWindow(HWND hWnd, WNDPROC wndProc);

MIR_CORE_DLL(void)    KillModuleSubclassing(HMODULE hInst);

///////////////////////////////////////////////////////////////////////////////
// Windows utilities

MIR_CORE_DLL(BOOL)    IsWinVerVistaPlus();
MIR_CORE_DLL(BOOL)    IsWinVer7Plus();

MIR_CORE_DLL(BOOL)    IsFullScreen();
MIR_CORE_DLL(BOOL)    IsWorkstationLocked();
MIR_CORE_DLL(BOOL)    IsScreenSaverRunning();

///////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) UnloadCoreModule(void);

#if defined(__cplusplus)
}
#endif

#ifndef MIR_CORE_EXPORTS
	#if !defined( _WIN64 )
		#pragma comment(lib, "mir_core.lib")
	#else
		#pragma comment(lib, "mir_core64.lib")
	#endif
#endif

#endif // M_CORE_H
