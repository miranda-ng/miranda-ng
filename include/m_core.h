/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#ifdef _MSC_VER
	#include <sal.h>
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef M_TYPES_H__
   #include <m_types.h>
#endif

#ifdef MIR_CORE_EXPORTS
    #define MIR_CORE_EXPORT MIR_EXPORT
#else
    #define MIR_CORE_EXPORT MIR_IMPORT
#endif

#define MIR_CORE_DLL(T) MIR_CORE_EXPORT T MIR_SYSCALL
#define MIR_C_CORE_DLL(T) MIR_CORE_EXPORT T MIR_CDECL

#ifdef MIR_APP_EXPORTS
	#define MIR_APP_EXPORT MIR_EXPORT
	typedef struct NetlibUser* HNETLIBUSER;
	typedef struct NetlibConnection* HNETLIBCONN;
	typedef struct NetlibBoundPort* HNETLIBBIND;
#else
	#define MIR_APP_EXPORT MIR_IMPORT
	DECLARE_HANDLE(HNETLIBUSER);
	DECLARE_HANDLE(HNETLIBCONN);
	DECLARE_HANDLE(HNETLIBBIND);
#endif

typedef struct TMO_IntMenuItem* HGENMENU;

class CMPluginBase;
typedef const CMPluginBase* HPLUGIN;

#define MIR_APP_DLL(T) MIR_APP_EXPORT T MIR_SYSCALL

#pragma warning(disable:4201 4127 4312 4706)

#if defined(__cplusplus)
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
// command line support

MIR_CORE_DLL(void)           CmdLine_Parse(const wchar_t *ptszCmdLine);
MIR_CORE_DLL(const wchar_t*) CmdLine_GetOption(const wchar_t *ptszParameter);

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
MIR_CORE_DLL(int)     CallPluginEventHook(HINSTANCE hInst, const char *pszEvent, WPARAM wParam = 0, LPARAM lParam = 0);
MIR_CORE_DLL(int)     CallObjectEventHook(void *pObject, HANDLE hEvent, WPARAM wParam = 0, LPARAM lParam = 0);
MIR_CORE_DLL(int)     NotifyEventHooks(HANDLE hEvent, WPARAM wParam = 0, LPARAM lParam = 0);
MIR_CORE_DLL(int)     NotifyFastHook(HANDLE hEvent, WPARAM wParam = 0, LPARAM lParam = 0);

MIR_CORE_DLL(HANDLE)  HookEvent(const char *name, MIRANDAHOOK hookProc);
MIR_CORE_DLL(HANDLE)  HookEventParam(const char *name, MIRANDAHOOKPARAM hookProc, LPARAM lParam = 0);
MIR_CORE_DLL(HANDLE)  HookEventObj(const char *name, MIRANDAHOOKOBJ hookProc, void* object);
MIR_CORE_DLL(HANDLE)  HookEventObjParam(const char *name, MIRANDAHOOKOBJPARAM hookProc, void* object, LPARAM lParam);
MIR_CORE_DLL(HANDLE)  HookEventMessage(const char *name, HWND hwnd, UINT message);

// executes the event handler if event is missing
MIR_CORE_DLL(HANDLE)  HookTemporaryEvent(const char *name, MIRANDAHOOK hookProc);

MIR_CORE_DLL(int)     UnhookEvent(HANDLE hHook);
MIR_CORE_DLL(void)    KillObjectEventHooks(void* pObject);
MIR_CORE_DLL(void)    KillModuleEventHooks(HINSTANCE pModule);

MIR_CORE_DLL(HANDLE)  CreateServiceFunction(const char *name, MIRANDASERVICE serviceProc);
MIR_CORE_DLL(HANDLE)  CreateServiceFunctionParam(const char *name, MIRANDASERVICEPARAM serviceProc, LPARAM lParam);
MIR_CORE_DLL(HANDLE)  CreateServiceFunctionObj(const char *name, MIRANDASERVICEOBJ serviceProc, void* object);
MIR_CORE_DLL(HANDLE)  CreateServiceFunctionObjParam(const char *name, MIRANDASERVICEOBJPARAM serviceProc, void* object, LPARAM lParam);
MIR_CORE_DLL(HANDLE)  CreateProtoServiceFunction(const char *szModule, const char *szService, MIRANDASERVICE serviceProc);
MIR_CORE_DLL(int)     DestroyServiceFunction(HANDLE hService);
MIR_CORE_DLL(bool)    ServiceExists(const char *name);

MIR_CORE_DLL(INT_PTR) CallService(const char *name, WPARAM wParam = 0, LPARAM lParam = 0);
MIR_CORE_DLL(INT_PTR) CallServiceSync(const char *name, WPARAM wParam = 0, LPARAM lParam = 0);

MIR_CORE_DLL(INT_PTR) CallFunctionSync(INT_PTR(MIR_SYSCALL *func)(void *), void *arg);
MIR_CORE_DLL(int)     CallFunctionAsync(void (MIR_SYSCALL *func)(void *), void *arg);
MIR_CORE_DLL(void)    KillModuleServices(HINSTANCE hInst);
MIR_CORE_DLL(void)    KillObjectServices(void* pObject);

MIR_APP_DLL(int)      ProtoServiceExists(const char *szModule, const char *szService);
MIR_APP_DLL(INT_PTR)  CallProtoService(const char *szModule, const char *szService, WPARAM wParam = 0, LPARAM lParam = 0);

///////////////////////////////////////////////////////////////////////////////
// exceptions

typedef uint32_t (MIR_CDECL *pfnExceptionFilter)(uint32_t code, EXCEPTION_POINTERS *info);

MIR_CORE_DLL(pfnExceptionFilter) GetExceptionFilter(void);
MIR_CORE_DLL(pfnExceptionFilter) SetExceptionFilter(pfnExceptionFilter pMirandaExceptFilter);

///////////////////////////////////////////////////////////////////////////////
// icons support

struct IconItem
{
	char  *szDescr, *szName;
	int    defIconID, size;
	HANDLE hIcolib;
};

struct IconItemT
{
	wchar_t *tszDescr;
	char  *szName;
	int    defIconID, size;
	HANDLE hIcolib;
};

MIR_CORE_DLL(void) Icon_Register(HINSTANCE hInst, const char *szSection, IconItem *pIcons, size_t iCount, const char *prefix, HPLUGIN pPlugin);
MIR_CORE_DLL(void) Icon_RegisterT(HINSTANCE hInst, const wchar_t *szSection, IconItemT *pIcons, size_t iCount, const char *prefix, HPLUGIN pPlugin);

///////////////////////////////////////////////////////////////////////////////
// language packs support

MIR_CORE_DLL(unsigned int) mir_hash(const void *key, unsigned int len);

#pragma optimize("gt", on)
__forceinline unsigned int mir_hashstr(const char *key)
{
	if (key == nullptr) return 0;
	else {
		unsigned int len = (unsigned int)strlen((const char*)key);
		return mir_hash(key, len);
}	}

__forceinline unsigned int mir_hashstrW(const wchar_t *key)
{
	if (key == nullptr) return 0;
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

MIR_CORE_DLL(HANDLE) mir_createLog(const char *pszName, const wchar_t *ptszDescr, const wchar_t *ptszFile, unsigned options);
MIR_CORE_DLL(void)   mir_closeLog(HANDLE hLogger);

MIR_C_CORE_DLL(int)  mir_writeLogA(HANDLE hLogger, const char *format, ...);
MIR_C_CORE_DLL(int)  mir_writeLogW(HANDLE hLogger, const wchar_t *format, ...);

MIR_CORE_DLL(int)    mir_writeLogVA(HANDLE hLogger, const char *format, va_list args);
MIR_CORE_DLL(int)    mir_writeLogVW(HANDLE hLogger, const wchar_t *format, va_list args);

///////////////////////////////////////////////////////////////////////////////
// md5 functions

typedef struct mir_md5_state_s {
	uint32_t count[2]; /* message length in bits, lsw first */
	uint32_t abcd[4];  /* digest buffer */
	uint8_t  buf[64];  /* accumulate block */
} mir_md5_state_t;

MIR_CORE_DLL(void) mir_md5_init(mir_md5_state_t *pms);
MIR_CORE_DLL(void) mir_md5_append(mir_md5_state_t *pms, const uint8_t *data, size_t nbytes);
MIR_CORE_DLL(void) mir_md5_finish(mir_md5_state_t *pms, uint8_t digest[16]);
MIR_CORE_DLL(void) mir_md5_hash(const uint8_t *data, size_t len, uint8_t digest[16]);

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
MIR_CORE_DLL(int)      mir_strncmp(const char *p1, const char *p2, size_t n);
MIR_CORE_DLL(int)      mir_wstrcmp(const wchar_t *p1, const wchar_t *p2);
MIR_CORE_DLL(int)      mir_wstrncmp(const wchar_t *p1, const wchar_t *p2, size_t n);

MIR_CORE_DLL(int)      mir_strcmpi(const char *p1, const char *p2);
MIR_CORE_DLL(int)      mir_strncmpi(const char *p1, const char *p2, size_t n);
MIR_CORE_DLL(int)      mir_wstrcmpi(const wchar_t *p1, const wchar_t *p2);
MIR_CORE_DLL(int)      mir_wstrncmpi(const wchar_t *p1, const wchar_t *p2, size_t n);

MIR_CORE_DLL(char*)    mir_strdup(const char* str);
MIR_CORE_DLL(wchar_t*) mir_wstrdup(const wchar_t* str);

MIR_CORE_DLL(char*)    mir_strndup(const char* str, size_t len);
MIR_CORE_DLL(wchar_t*) mir_wstrndup(const wchar_t *str, size_t len);

MIR_CORE_DLL(const wchar_t*) mir_wstrstri(const wchar_t *s1, const wchar_t *s2);

///////////////////////////////////////////////////////////////////////////////
// print functions

MIR_CORE_DLL(int)    mir_snprintf(_Pre_notnull_ _Always_(_Post_z_) char *buffer, size_t count, _Printf_format_string_ const char* fmt, ...);
MIR_CORE_DLL(int)    mir_snwprintf(_Pre_notnull_ _Always_(_Post_z_) wchar_t *buffer, size_t count, _Printf_format_string_ const wchar_t* fmt, ...);
MIR_CORE_DLL(int)    mir_vsnprintf(_Pre_notnull_ _Always_(_Post_z_) char *buffer, size_t count, _Printf_format_string_ const char* fmt, va_list va);
MIR_CORE_DLL(int)    mir_vsnwprintf(_Pre_notnull_ _Always_(_Post_z_) wchar_t *buffer, size_t count, _Printf_format_string_ const wchar_t* fmt, va_list va);

///////////////////////////////////////////////////////////////////////////////
// protocol functions

struct PROTO_INTERFACE;

MIR_APP_DLL(INT_PTR) ProtoBroadcastAck(const char *szModule, MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam = 0);
MIR_APP_DLL(void) ProtoBroadcastAsync(const char *szModule, MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam = 0);

// avatar support functions

// returns image extension by a PA_* constant or empty string for PA_FORMAT_UNKNOWN
MIR_APP_DLL(const wchar_t*) ProtoGetAvatarExtension(int format);

// detects image format by extension
MIR_APP_DLL(int) ProtoGetAvatarFormat(const wchar_t *ptszFileName);

// detects image format by its contents
MIR_APP_DLL(int) ProtoGetAvatarFileFormat(const wchar_t *ptszFileName);

// returns the mime type according to a picture type (PA_*)	passed
MIR_APP_DLL(const char*) ProtoGetAvatarMimeType(int iFileType);

// returns the picture type (PA_*) according to a mime type passed
MIR_APP_DLL(int) ProtoGetAvatarFormatByMimeType(const char *pwszMimeType);

// returns the image format and extension by the first bytes of picture
// ptszExtension might be NULL
#if defined( __cplusplus )
	MIR_APP_DLL(int) ProtoGetBufferFormat(const void *buf, const wchar_t **ptszExtension = nullptr);
#else
	MIR_APP_DLL(int) ProtoGetBufferFormat(const void *buf, const wchar_t **ptszExtension);
#endif

///////////////////////////////////////////////////////////////////////////////
// sha1 functions

#define MIR_SHA1_HASH_SIZE 20
#define MIR_SHA_BLOCKSIZE 64

struct mir_sha1_ctx
{
    uint32_t H[5];
    uint32_t W[80];
    int lenW;
    uint32_t sizeHi, sizeLo;
};

MIR_CORE_DLL(void) mir_sha1_init(mir_sha1_ctx *ctx);
MIR_CORE_DLL(void) mir_sha1_append(mir_sha1_ctx *ctx, const uint8_t *dataIn, size_t len);
MIR_CORE_DLL(void) mir_sha1_finish(mir_sha1_ctx *ctx, uint8_t hashout[MIR_SHA1_HASH_SIZE]);
MIR_CORE_DLL(void) mir_sha1_hash(uint8_t *dataIn, size_t len, uint8_t hashout[MIR_SHA1_HASH_SIZE]);

///////////////////////////////////////////////////////////////////////////////
// sha256 functions

#define MIR_SHA256_HASH_SIZE 32

struct SHA256_CONTEXT
{
	uint32_t  h0, h1, h2, h3, h4, h5, h6, h7;
	uint32_t  nblocks;
	uint8_t buf[MIR_SHA_BLOCKSIZE];
	int  count;
};

MIR_CORE_DLL(void) mir_sha256_init(SHA256_CONTEXT *ctx);
MIR_CORE_DLL(void) mir_sha256_write(SHA256_CONTEXT *ctx, const void *dataIn, size_t len);
MIR_CORE_DLL(void) mir_sha256_final(SHA256_CONTEXT *ctx, uint8_t hashout[MIR_SHA256_HASH_SIZE]);
MIR_CORE_DLL(void) mir_sha256_hash(const void *dataIn, size_t len, uint8_t hashout[MIR_SHA256_HASH_SIZE]);

///////////////////////////////////////////////////////////////////////////////
// strings

MIR_CORE_DLL(void*) mir_base64_decode(const char *input, size_t *outputLen);
MIR_CORE_DLL(char*) mir_base64_encode(const void *input, size_t inputLen);
MIR_CORE_DLL(char*) mir_base64_encodebuf(const void *input, size_t inputLen, char *output, size_t outLen);

__forceinline size_t mir_base64_encode_bufsize(size_t inputLen)
{
	return 4 * ((inputLen + 2) / 3) + 1;
}

MIR_CORE_DLL(char*)  rtrim(char *str);
MIR_CORE_DLL(wchar_t*) rtrimw(wchar_t *str);

MIR_CORE_DLL(char*)  ltrim(char *str);   // returns pointer to the beginning of string
MIR_CORE_DLL(wchar_t*) ltrimw(wchar_t *str);

MIR_CORE_DLL(char*)  ltrimp(char *str);  // returns pointer to the trimmed portion of string
MIR_CORE_DLL(wchar_t*) ltrimpw(wchar_t *str);

MIR_CORE_DLL(char*) strdel(char *str, size_t len);
MIR_CORE_DLL(wchar_t*) strdelw(wchar_t *str, size_t len);

MIR_CORE_DLL(int) wildcmp(const char *name, const char *mask);
MIR_CORE_DLL(int) wildcmpw(const wchar_t *name, const wchar_t *mask);

MIR_CORE_DLL(int) wildcmpi(const char *name, const char *mask);
MIR_CORE_DLL(int) wildcmpiw(const wchar_t *name, const wchar_t *mask);

MIR_CORE_DLL(char*)  bin2hex(const void *pData, size_t len, char *dest);
MIR_CORE_DLL(wchar_t*) bin2hexW(const void *pData, size_t len, wchar_t *dest);

MIR_CORE_DLL(bool) hex2bin(const char *pSrc, void *pData, size_t len);
MIR_CORE_DLL(bool) hex2binW(const wchar_t *pSrc, void *pData, size_t len);

__forceinline char* lrtrim(char *str) { return ltrim(rtrim(str)); };
__forceinline char* lrtrimp(char *str) { return ltrimp(rtrim(str)); };

#if defined( __cplusplus )
	MIR_CORE_DLL(char*) replaceStr(char* &dest, const char *src);
	MIR_CORE_DLL(wchar_t*) replaceStrW(wchar_t* &dest, const wchar_t *src);
#else
	MIR_CORE_DLL(char*) replaceStr(char **dest, const char *src);
	MIR_CORE_DLL(wchar_t*) replaceStrW(wchar_t **dest, const wchar_t *src);
#endif

#ifndef _MSC_VER
	MIR_CORE_DLL(char*) strlwr(char *str);
	MIR_CORE_DLL(char*) strupr(char *str);
	MIR_CORE_DLL(char*) strrev(char *str);
#endif

///////////////////////////////////////////////////////////////////////////////
// text conversion functions

union MAllStrings
{
	char *a; // utf8 or ansi strings
	wchar_t *w; // strings of WCHARs
};

union MAllCStrings
{
	const char *a; // utf8 or ansi strings
	const wchar_t *w; // strings of WCHARs
};

union MAllStringArray
{
	char **a; // array of utf8 or ansi strings
	wchar_t **w; // array of strings of WCHARs
};

union MAllCStringArray
{
	const char **a; // array of utf8 or ansi strings
	const wchar_t **w; // array of strings of WCHARs
};

MIR_CORE_DLL(wchar_t*) mir_a2u_cp(const char* src, int codepage);
MIR_CORE_DLL(wchar_t*) mir_a2u(const char* src);
MIR_CORE_DLL(char*)  mir_u2a_cp(const wchar_t* src, int codepage);
MIR_CORE_DLL(char*)  mir_u2a(const wchar_t* src);

///////////////////////////////////////////////////////////////////////////////
// threads

typedef void (MIR_CDECL *pThreadFunc)(void *param);
typedef unsigned (MIR_SYSCALL *pThreadFuncEx)(void *param);
typedef unsigned (MIR_CDECL *pThreadFuncOwner)(void *owner, void *param);

#if defined( __cplusplus )
	MIR_CORE_DLL(INT_PTR) Thread_Push(HINSTANCE hInst, void* pOwner = nullptr);
#else
	MIR_CORE_DLL(INT_PTR) Thread_Push(HINSTANCE hInst, void* pOwner);
#endif
MIR_CORE_DLL(INT_PTR) Thread_Pop(void);
MIR_CORE_DLL(void)    Thread_Wait(void);

#if defined( __cplusplus )
MIR_CORE_DLL(HANDLE) mir_forkthread(pThreadFunc aFunc, void *arg = nullptr);
MIR_CORE_DLL(HANDLE) mir_forkthreadex(pThreadFuncEx aFunc, void *arg = nullptr, unsigned *pThreadID = nullptr);
MIR_CORE_DLL(HANDLE) mir_forkthreadowner(pThreadFuncOwner aFunc, void *owner, void *arg = nullptr, unsigned *pThreadID = nullptr);
#else
MIR_CORE_DLL(HANDLE) mir_forkthread(pThreadFunc aFunc, void *arg);
MIR_CORE_DLL(HANDLE) mir_forkthreadex(pThreadFuncEx aFunc, void *arg, unsigned *pThreadID);
MIR_CORE_DLL(HANDLE) mir_forkthreadowner(pThreadFuncOwner aFunc, void *owner, void *arg, unsigned *pThreadID);
#endif

MIR_CORE_DLL(void) Thread_SetName(const char *szThreadName);

MIR_CORE_DLL(void) KillObjectThreads(void* pObject);

///////////////////////////////////////////////////////////////////////////////
// utf8 interface

MIR_CORE_DLL(BOOL)  Utf8CheckString(const char* str);
MIR_CORE_DLL(int)   Utf8toUcs2(const char *src, size_t srclen, wchar_t *dst, size_t dstlen); // returns 0 on error

MIR_CORE_DLL(char*) mir_utf8decode(char* str, wchar_t** ucs2);
MIR_CORE_DLL(char*) mir_utf8decodecp(char* str, int codepage, wchar_t** ucs2);
MIR_CORE_DLL(wchar_t*) mir_utf8decodeW(const char* str);

MIR_CORE_DLL(char*) mir_utf8encode(const char* str);
MIR_CORE_DLL(char*) mir_utf8encodecp(const char* src, int codepage);
MIR_CORE_DLL(char*) mir_utf8encodeW(const wchar_t* str);

MIR_CORE_DLL(int)   mir_utf8lenW(const wchar_t *src);

__forceinline char* mir_utf8decodeA(const char* src)
{
    char *tmp = mir_strdup(src);
    mir_utf8decode(tmp, nullptr);
    return tmp;
}

///////////////////////////////////////////////////////////////////////////////
// Window subclassing

#ifdef _MSC_VER

MIR_CORE_DLL(void)    mir_subclassWindow(HWND hWnd, WNDPROC wndProc);
MIR_CORE_DLL(void)    mir_subclassWindowFull(HWND hWnd, WNDPROC wndProc, WNDPROC oldWndProc);
MIR_CORE_DLL(LRESULT) mir_callNextSubclass(HWND hWnd, WNDPROC wndProc, UINT uMsg, WPARAM wParam, LPARAM lParam);
MIR_CORE_DLL(void)    mir_unsubclassWindow(HWND hWnd, WNDPROC wndProc);

MIR_CORE_DLL(void)    KillModuleSubclassing(HMODULE hInst);

///////////////////////////////////////////////////////////////////////////////
// Windows utilities

MIR_CORE_DLL(BOOL) IsWinVerVistaPlus();
MIR_CORE_DLL(BOOL) IsWinVer7Plus();
MIR_CORE_DLL(BOOL) IsWinVer8Plus();
MIR_CORE_DLL(BOOL) IsWinVer81Plus();
MIR_CORE_DLL(BOOL) IsWinVer10Plus();

MIR_CORE_DLL(BOOL) IsFullScreen();
MIR_CORE_DLL(BOOL) IsWorkstationLocked();
MIR_CORE_DLL(BOOL) IsScreenSaverRunning();
MIR_CORE_DLL(BOOL) IsTerminalDisconnected();

#endif // _MSC_VER

// returns OS version in version of Windows NT xx.xx
MIR_CORE_DLL(BOOL) OS_GetShortString(char *buf, size_t bufSize);

// returns full OS version
MIR_CORE_DLL(BOOL) OS_GetDisplayString(char *buf, size_t bufSize);

///////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) UnloadCoreModule(void);

#if defined(__cplusplus)
}

///////////////////////////////////////////////////////////////////////////////
// The UUID structure below is used to for plugin UUID's and module type definitions

struct MUUID
{
	unsigned long a;
	unsigned short b;
	unsigned short c;
	unsigned char d[8];
};

__forceinline bool operator==(const MUUID& p1, const MUUID& p2)
{
	return memcmp(&p1, &p2, sizeof(MUUID)) == 0;
}
__forceinline bool operator!=(const MUUID& p1, const MUUID& p2)
{
	return memcmp(&p1, &p2, sizeof(MUUID)) != 0;
}

///////////////////////////////////////////////////////////////////////////////
// C++ templates

template <typename T>
HANDLE mir_forkThread(void(MIR_CDECL *pFunc)(T* param), T *arg)
{
	return mir_forkthread((pThreadFunc)pFunc, arg);
}

template <size_t _Size>
inline int mir_snprintf(_Pre_notnull_ _Always_(_Post_z_) char(&buffer)[_Size], _In_z_ _Printf_format_string_ const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = mir_vsnprintf(buffer, _Size, fmt, args);
	va_end(args);
	return ret;
}

template <size_t _Size>
inline int mir_snwprintf(_Pre_notnull_ _Always_(_Post_z_) wchar_t(&buffer)[_Size], _In_z_ _Printf_format_string_ const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = mir_vsnwprintf(buffer, _Size, fmt, args);
	va_end(args);
	return ret;
}

template <size_t _Size>
inline int mir_vsnprintf(_Pre_notnull_ _Always_(_Post_z_) char(&buffer)[_Size], _In_z_ _Printf_format_string_ const char* fmt, va_list va)
{
	return mir_vsnprintf(buffer, _Size, fmt, va);
}

template <size_t _Size>
inline int mir_vsnwprintf(_Pre_notnull_ _Always_(_Post_z_) wchar_t(&buffer)[_Size], _In_z_ _Printf_format_string_ const wchar_t* fmt, va_list va)
{
	return mir_vsnwprintf(buffer, _Size, fmt, va);
}

MIR_CORE_DLL(char *) mir_base64_encode(const class MBinBuffer &buf);

#endif

#ifdef _MSC_VER
	#ifndef MIR_CORE_EXPORTS
		#pragma comment(lib, "mir_core.lib")
	#endif

	#ifndef MIR_APP_EXPORTS
		#pragma comment(lib, "mir_app.lib")
	#endif
#else
	MIR_CORE_DLL(FILE*) _wfopen(const wchar_t *pwszFileName, const wchar_t *pwszMode);
	MIR_CORE_DLL(int) _wchdir(const wchar_t *pwszPath);

	template <size_t _Size>
	inline wchar_t* wcsncpy_s(wchar_t(&buffer)[_Size], const wchar_t *src, size_t len)
	{
		return wcsncpy(buffer, src, (len == _TRUNCATE) ? _Size : len);
	}

	inline wchar_t* wcsncpy_s(wchar_t *dst, size_t dstLen, const wchar_t *src, size_t len)
	{
		return wcsncpy(dst, src, (len == _TRUNCATE) ? dstLen : len);
	}

	inline wchar_t* wcsncat_s(wchar_t *dst, size_t dstLen, const wchar_t *src, size_t len)
	{
		return wcsncat(dst, src, (len == _TRUNCATE) ? dstLen : len);
	}

	template <size_t _Size>
	inline char* strncpy_s(char(&buffer)[_Size], const char *src, size_t len)
	{
		return strncpy(buffer, src, (len == _TRUNCATE) ? _Size : len);
	}

	inline char* strncpy_s(char *dst, size_t dstLen, const char *src, size_t len)
	{
		return strncpy(dst, src, (len == _TRUNCATE) ? dstLen : len);
	}

	inline char* strncat_s(char *dst, size_t dstLen, const char *src, size_t len)
	{
		return strncat(dst, src, (len == _TRUNCATE) ? dstLen : len);
	}
#endif

#endif // M_CORE_H
