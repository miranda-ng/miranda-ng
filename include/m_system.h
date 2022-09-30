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

#pragma once

#ifndef M_SYSTEM_H__
#define M_SYSTEM_H__ 1

#ifndef MIRANDANAME
	#define MIRANDANAME "Miranda NG"
#endif
#ifndef MIRANDACLASS
	#define MIRANDACLASS	"Miranda"
#endif

// set the default compatibility lever for Miranda 0.4.x
#ifndef MIRANDA_VER
	#define MIRANDA_VER    0x0A00
#endif

#ifdef _MSC_VER
	#pragma warning(disable:4244 4245)
#endif

#define NEWSTR_ALLOCA(A) (A == NULL)?NULL:strcpy((char*)alloca(strlen(A)+1), A)
#define NEWWSTR_ALLOCA(A) ((A==NULL)?NULL:wcscpy((wchar_t*)alloca(sizeof(wchar_t)*(wcslen(A)+1)),A))

#include <m_core.h>

// miranda/system/modulesloaded
// called after all modules have been successfully initialised
// wParam = lParam = 0
// used to resolve double-dependencies in the module load order
#define ME_SYSTEM_MODULESLOADED "Miranda/System/ModulesLoaded"

// miranda/system/shutdown event
// called just before the application terminates
// the database is still guaranteed to be running during this hook.
// wParam = lParam = 0
#define ME_SYSTEM_SHUTDOWN "Miranda/System/Shutdown"

// restarts miranda (0.8+)
// wParam = 0 or 1. 1 - restart with current profile, 0 - restart in default profile or profile manager
// lParam = (wchar_t*)path to a new miranda32.exe binary or NULL to use current
#define MS_SYSTEM_RESTART "Miranda/System/Restart"

// miranda/system/oktoexit event
// called before the app goes into shutdown routine to make sure everyone is
// happy to exit
// wParam = lParam = 0
// return nonzero to stop the exit cycle
#define ME_SYSTEM_OKTOEXIT "Miranda/System/OkToExitEvent"

// gets the version number of Miranda encoded as a uint32_t
// returns the version number, encoded as one version per byte, therefore
// version 1.2.3.10 is 0x0102030a
EXTERN_C MIR_APP_DLL(uint32_t) Miranda_GetVersion(void);

// gets the version number of Miranda encoded as four WORDs   v0.92.2+
// returns the version number, encoded as one version per word, therefore
// version 1.2.3.3210 is 0x0001, 0x0002, 0x0003, 0x0C8a
typedef uint16_t MFileVersion[4];
EXTERN_C MIR_APP_DLL(void) Miranda_GetFileVersion(MFileVersion*);

// gets the version of Miranda encoded as text
// cch is the size of the buffer pointed to by pszVersion, in bytes
// may return a build qualifier, such as "0.1.0.1 alpha"
// returns 0 on success, nonzero on failure
EXTERN_C MIR_APP_DLL(void) Miranda_GetVersionText(char *pDest, size_t cbSize);

// returns a system window that can handle global timers
// a usual practice is to use a unique pointer as a timer id
EXTERN_C MIR_APP_DLL(class CDlgBase *) Miranda_GetSystemWindow();

// Adds an event to the list to be checked in the main message loop
// when a handle gets triggered, an appopriate stub gets called
typedef void (CALLBACK *MWaitableStub)(void);
typedef void (CALLBACK *MWaitableStubEx)(void*);

EXTERN_C MIR_CORE_DLL(void) Miranda_WaitOnHandle(MWaitableStub pFunc, HANDLE hEvent = nullptr);
EXTERN_C MIR_CORE_DLL(void) Miranda_WaitOnHandleEx(MWaitableStubEx pFunc, void *pInfo);

// wParam = 0 (ignored)
// lParam = 0 (ignored)
//
// This hook is fired just before the thread unwind stack is used,
// it allows MT plugins to shutdown threads if they have any special
// processing to do, etc.
#define ME_SYSTEM_PRESHUTDOWN "Miranda/System/PShutdown"

//	Returns true when Miranda has got WM_QUIT and is in the process of shutting down
EXTERN_C MIR_CORE_DLL(bool) Miranda_IsTerminated(void);

// Enables termination flag
EXTERN_C MIR_CORE_DLL(void) Miranda_SetTerminated(void);

// Check if everyone is happy to exit
// if everyone acknowleges OK to exit then returns true, otherwise false
EXTERN_C MIR_APP_DLL(bool) Miranda_OkToExit(void);

// Used by contact lists inside CloseAction
// Waits for a permission to exit and destroys contact list
EXTERN_C MIR_APP_DLL(void) Miranda_Close(void);

// returns the last window tick where a monitored event was seen, currently WM_CHAR/WM_MOUSEMOVE
EXTERN_C MIR_CORE_DLL(uint32_t) Miranda_GetIdle(void);

///////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)

#ifndef M_STRING_H__
	#include <m_string.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// general lists' templates

struct MIR_CORE_EXPORT MNonCopyable
{
	__inline MNonCopyable() {}

	MNonCopyable(const MNonCopyable &) = delete;
	MNonCopyable &operator=(const MNonCopyable &) = delete;
};

///////////////////////////////////////////////////////////////////////////////
// mir_ptr - automatic pointer for buffers, allocated using mir_alloc/mir_calloc

template<class T> class mir_ptr
{
protected:
	T *data;

public:
	__inline explicit mir_ptr() : data(nullptr) {}
	__inline explicit mir_ptr(T *_p) : data(_p) {}
	__inline ~mir_ptr() { mir_free(data); }
	__inline T *get() const { return data; }
	__inline T *operator=(T *_p) { if (data) mir_free(data); data = _p; return data; }
	__inline T *operator->() const { return data; }
	__inline operator T *() const { return data; }
	__inline operator INT_PTR() const { return (INT_PTR)data; }
	__inline T *detach() { T *res = data; data = nullptr; return res; }
};

typedef mir_ptr<char>  ptrA;
typedef mir_ptr<wchar_t> ptrW;

///////////////////////////////////////////////////////////////////////////////
// mir_cs - simple wrapper for the critical sections

class MIR_CORE_EXPORT mir_cs : public MNonCopyable
{
    #ifdef _MSC_VER
	CRITICAL_SECTION m_cs;
	#endif

public:
	mir_cs();
	~mir_cs();

	void Lock();
	void Unlock();
};

///////////////////////////////////////////////////////////////////////////////
// mir_cslock - simple locker for the critical sections

class mir_cslock : public MNonCopyable
{
	mir_cs &cs;

public:
	__inline mir_cslock(mir_cs &_cs) : cs(_cs) { cs.Lock(); }
	__inline ~mir_cslock() { cs.Unlock(); }
};

class mir_cslockfull : public MNonCopyable
{
	mir_cs &cs;
	bool bIsLocked = false;

public:
	__inline void lock() { bIsLocked = true; cs.Lock(); }
	__inline void unlock() { bIsLocked = false; cs.Unlock(); }

	__inline mir_cslockfull(mir_cs &_cs) : cs(_cs) { lock(); }
	__inline ~mir_cslockfull() { if (bIsLocked) unlock(); }
};

//////////////////////////////////////////////////////////////////////////////
//pass_ptrA, pass_ptrW and pass_ptrT - automatic pointer for passwords

class pass_ptrA : public mir_ptr<char>
{
public:
	__inline explicit pass_ptrA() : mir_ptr() {}
	__inline explicit pass_ptrA(char *_p) : mir_ptr(_p) {}
	__inline ~pass_ptrA() { zero(); }
	__inline char *operator=(char *_p) { zero(); return mir_ptr::operator=(_p); }
	__inline void zero()
	{
		if (data) SecureZeroMemory(data, mir_strlen(data));
	}
};

class pass_ptrW : public mir_ptr<wchar_t>
{
public:
	__inline explicit pass_ptrW() : mir_ptr() {}
	__inline explicit pass_ptrW(wchar_t *_p) : mir_ptr(_p) {}
	__inline ~pass_ptrW() { zero(); }
	__inline wchar_t *operator=(wchar_t *_p) { zero(); return mir_ptr::operator=(_p); }
	__inline void zero()
	{
		if (data) SecureZeroMemory(data, mir_wstrlen(data) * sizeof(wchar_t));
	}
};

///////////////////////////////////////////////////////////////////////////////
// basic class for classes that should be cleared inside new()

class MZeroedObject
{
public:
	__inline void *operator new(size_t size)
	{
		return calloc(1, size);
	}

	__inline void operator delete(void *p)
	{
		free(p);
	}
};

///////////////////////////////////////////////////////////////////////////////
// general lists' templates

#define	NumericKeySortT -1
#define	HandleKeySortT  -2
#define	PtrKeySortT     -3

template<class T> struct LIST
{
	typedef int (*FTSortFunc)(const T *p1, const T *p2);

	__inline LIST(int aincr, FTSortFunc afunc = nullptr)
	{
		memset(this, 0, sizeof(*this));
		increment = aincr;
		sortFunc = afunc;
	}

	__inline LIST(int aincr, INT_PTR id)
	{
		memset(this, 0, sizeof(*this));
		increment = aincr;
		sortFunc = FTSortFunc(id);
	}

	__inline LIST(const LIST &x)
	{
		items = nullptr;
		List_Copy((SortedList *)&x, (SortedList *)this, sizeof(T));
	}

	__inline LIST &operator = (const LIST &x)
	{
		destroy();
		List_Copy((SortedList *)&x, (SortedList *)this, sizeof(T));
		return *this;
	}

	__inline ~LIST()
	{
		destroy();
	}

	__inline T *operator[](int idx) const { return (idx >= 0 && idx < count) ? items[idx] : nullptr; }
	__inline int getCount(void)      const { return count; }
	__inline T **getArray(void)      const { return items; }

	__inline int getIndex(T *p) const
	{
		int idx;
		return (!List_GetIndex((SortedList *)this, p, &idx)) ? -1 : idx;
	}

	class reverse_iterator
	{
		int index;
		T **base;

	public:
		reverse_iterator(const LIST &_lst) :
			index(_lst.getCount() - 1),
			base(_lst.getArray())
		{
		}

		class iterator
		{
			T **ptr;

		public:
			iterator(T **_p) : ptr(_p) {}
			iterator operator++() { --ptr; return *this; }
			bool operator!=(const iterator &p) { return ptr != p.ptr; }
			operator T **() const { return ptr; }
		};

		__inline iterator begin() const { return iterator(base + index); }
		__inline iterator end() const { return iterator(base - 1); }
		__inline int indexOf(T **p) const { return int(p - base); }
	};

	__inline void destroy(void) { List_Destroy((SortedList *)this); }
	__inline T*   find(T *p) const { return (T *)List_Find((SortedList *)this, p); }
	__inline int  indexOf(T *p) const { return List_IndexOf((SortedList *)this, p); }
	__inline int  insert(T *p, int idx) { return List_Insert((SortedList *)this, p, idx); }
	__inline int  remove(int idx) { return List_Remove((SortedList *)this, idx); }

	__inline int  insert(T *p) { return List_InsertPtr((SortedList *)this, p); }
	__inline int  remove(T *p) { return List_RemovePtr((SortedList *)this, p); }

	__inline int  indexOf(T **p) const { return int(p - items); }

	__inline T* removeItem(T **p)
	{
		T *savePtr = *p;
		List_Remove((SortedList *)this, int(p - items));
		return savePtr;
	}

	__inline void put(int idx, T *p) { items[idx] = p; }

	__inline T **begin() const { return items; }
	__inline T **end() const { return items + count; }

	__inline reverse_iterator rev_iter() const { return reverse_iterator(*this); }

protected:
	T **items;
	int        count, limit, increment;
	FTSortFunc sortFunc;
};

template<class T> struct OBJLIST : public LIST<T>
{
	typedef int (*FTSortFunc)(const T *p1, const T *p2);

	__inline OBJLIST(int aincr, FTSortFunc afunc = nullptr) :
		LIST<T>(aincr, afunc)
	{
	}

	__inline OBJLIST(int aincr, INT_PTR id) :
		LIST<T>(aincr, (FTSortFunc)id)
	{
	}

	__inline OBJLIST(const OBJLIST &x) :
		LIST<T>(x.increment, x.sortFunc)
	{
		this->items = nullptr;
		List_ObjCopy((SortedList *)&x, (SortedList *)this, sizeof(T));
	}

	__inline OBJLIST &operator = (const OBJLIST &x)
	{
		destroy();
		List_ObjCopy((SortedList *)&x, (SortedList *)this, sizeof(T));
		return *this;
	}

	~OBJLIST()
	{
		destroy();
	}

	__inline void destroy(void)
	{
		for (int i = 0; i < this->count; i++)
			delete this->items[i];

		List_Destroy((SortedList *)this);
	}

	__inline int remove(int idx)
	{
		delete this->items[idx];
		return List_Remove((SortedList *)this, idx);
	}

	__inline int remove(T *p)
	{
		int i = this->getIndex(p);
		if (i != -1) {
			remove(i);
			return 1;
		}
		return 0;
	}

	__inline T &operator[](int idx) const { return *this->items[idx]; }
};

#define __A2W(s) L ## s
#define _A2W(s) __A2W(s)

class _A2T : public ptrW
{
public:
	__inline _A2T(const char *s) : ptrW(mir_a2u(s)) {}
	__inline _A2T(const char *s, int cp) : ptrW(mir_a2u_cp(s, cp)) {}
};

class _T2A : public ptrA
{
public:
	__forceinline _T2A(const wchar_t *s) : ptrA(mir_u2a(s)) {}
	__forceinline _T2A(const wchar_t *s, int cp) : ptrA(mir_u2a_cp(s, cp)) {}
};

class T2Utf : public ptrA
{
public:
	__forceinline T2Utf(const wchar_t *str) : ptrA(mir_utf8encodeW(str)) {}
	__forceinline operator uint8_t*() const { return (uint8_t*)data; }
#ifdef _XSTRING_
	std::string str() const { return std::string(data); }
#endif
};

class Utf2T : public ptrW
{
public:
	__forceinline Utf2T(const char *str) : ptrW(mir_utf8decodeW(str)) {}
	__forceinline operator wchar_t *() const { return data; }
#ifdef _XSTRING_
	std::wstring str() const { return std::wstring(data); }
#endif
};

///////////////////////////////////////////////////////////////////////////////
// basic class for classes that should be cleared inside new()

class MIR_CORE_EXPORT MBinBuffer
{
	char *m_buf = nullptr;

public:
	MBinBuffer();
	MBinBuffer(size_t preAlloc);
	MBinBuffer(const MBinBuffer &orig);
	~MBinBuffer();
	MBinBuffer& operator=(MBinBuffer &&) noexcept;

	__forceinline char* data() const { return m_buf; }
	__forceinline bool isEmpty() const { return m_buf == nullptr; }
	size_t length() const;

	// adds a buffer to the end
	void append(const void *pBuf, size_t bufLen);

	// adds a buffer to the beginning
	void appendBefore(const void *pBuf, size_t bufLen);

	// replaces buffer contents
	void assign(const void *pBuf, size_t bufLen);

	// drops a part of buffer
	void remove(size_t sz);
};

///////////////////////////////////////////////////////////////////////////////
// thread handle controller

class MThreadLock
{
	HANDLE &m_pHandle;

public:
	__forceinline MThreadLock(HANDLE &pHandle) :
		m_pHandle(pHandle)
	{
	}

	__forceinline ~MThreadLock()
	{
		m_pHandle = nullptr;
	}
};

///////////////////////////////////////////////////////////////////////////////
// parameter classes for XML, JSON & HTTP requests

struct PARAM
{
	const char *szName;
	__forceinline PARAM(const char *_name) : szName(_name)
	{
	}
};

struct BOOL_PARAM : public PARAM
{
	bool bValue;
	__forceinline BOOL_PARAM(const char *_name, bool _value) :
		PARAM(_name), bValue(_value)
	{
	}
};

struct INT_PARAM : public PARAM
{
	int32_t iValue;
	__forceinline INT_PARAM(const char *_name, int32_t _value) :
		PARAM(_name), iValue(_value)
	{
	}
};

struct INT64_PARAM : public PARAM
{
	int64_t iValue;
	__forceinline INT64_PARAM(const char *_name, int64_t _value) :
		PARAM(_name), iValue(_value)
	{
	}
};

struct SINT64_PARAM : public PARAM
{
	int64_t iValue;
	__forceinline SINT64_PARAM(const char *_name, int64_t _value) :
		PARAM(_name), iValue(_value)
	{
	}
};

struct CHAR_PARAM : public PARAM
{
	const char *szValue;
	__forceinline CHAR_PARAM(const char *_name, const char *_value) :
		PARAM(_name), szValue(_value)
	{
	}
};

struct WCHAR_PARAM : public PARAM
{
	const wchar_t *wszValue;
	__forceinline WCHAR_PARAM(const char *_name, const wchar_t *_value) :
		PARAM(_name), wszValue(_value)
	{
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Callbacks

class CCallbackImp
{
	struct CDummy
	{
		int foo;
	};

	typedef void (CDummy:: *TFnCallback)(void *argument);

	CDummy *m_object;
	TFnCallback m_func;

protected:
	template<typename TClass, typename TArgument>
	__inline CCallbackImp(TClass *object, void (TClass:: *func)(TArgument *argument)) :
		m_object((CDummy *)object),
		m_func((TFnCallback)func)
	{
	}

	__inline void Invoke(void *argument) const { if (m_func && m_object) (m_object->*m_func)(argument); }

public:
	__inline CCallbackImp() : m_object(nullptr), m_func(nullptr) {}

	__inline CCallbackImp(const CCallbackImp &other) : m_object(other.m_object), m_func(other.m_func) {}
	__inline CCallbackImp &operator=(const CCallbackImp &other) { m_object = other.m_object; m_func = other.m_func; return *this; }

	__inline bool operator==(const CCallbackImp &other) const { return (m_object == other.m_object) && (m_func == other.m_func); }
	__inline bool operator!=(const CCallbackImp &other) const { return (m_object != other.m_object) || (m_func != other.m_func); }

	__inline operator bool() const { return m_object && m_func; }
};

template<typename TArgument>
struct CCallback : public CCallbackImp
{
	typedef CCallbackImp CSuper;

public:
	__inline CCallback() {}

	template<typename TClass>
	__inline CCallback(TClass *object, void (TClass:: *func)(TArgument *argument)) : CCallbackImp(object, func) {}

	__inline CCallback &operator=(const CCallbackImp &x) { CSuper::operator =(x); return *this; }

	__inline void operator()(TArgument *argument) const { Invoke((void *)argument); }
};

template<typename TClass, typename TArgument>
__inline CCallback<TArgument> Callback(TClass *object, void (TClass:: *func)(TArgument *argument))
{
	return CCallback<TArgument>(object, func);
}

///////////////////////////////////////////////////////////////////////////////
// http support

// works inline, in the same buffer, thus destroying its contents
// returns the address of buffer passed

MIR_CORE_DLL(char *) mir_urlDecode(char *szUrl);

MIR_CORE_DLL(CMStringA) mir_urlEncode(const char *szUrl);

#endif // __cpluscplus

#endif // M_SYSTEM_H
