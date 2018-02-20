/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
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
#ifndef M_SYSTEM_CPP_H__
#define M_SYSTEM_CPP_H__ 1

#include <stdlib.h>

#ifndef M_SYSTEM_H__
	#include "m_system.h"
#endif

#if defined(__cplusplus)

///////////////////////////////////////////////////////////////////////////////
// mir_ptr - automatic pointer for buffers, allocated using mir_alloc/mir_calloc

template<class T> class mir_ptr
{
protected:
	T* data;

public:
	__inline explicit mir_ptr() : data(NULL) {}
	__inline explicit mir_ptr(T *_p) : data(_p) {}
	__inline ~mir_ptr() { mir_free(data); }
	__inline T* get() const { return data; }
	__inline T* operator = (T *_p) { if (data) mir_free(data); data = _p; return data; }
	__inline T* operator->() const { return data; }
	__inline operator T*() const { return data; }
	__inline operator INT_PTR() const { return (INT_PTR)data; }
	__inline T* detach() { T *res = data; data = NULL; return res; }
};

typedef mir_ptr<char>  ptrA;
typedef mir_ptr<wchar_t> ptrW;

///////////////////////////////////////////////////////////////////////////////
// mir_cs - simple wrapper for the critical sections

class mir_cs
{
	CRITICAL_SECTION m_cs;

public:
	__inline mir_cs() { ::InitializeCriticalSection(&m_cs); }
	__inline ~mir_cs() { ::DeleteCriticalSection(&m_cs); }

	__inline operator CRITICAL_SECTION&() { return m_cs; }
};

///////////////////////////////////////////////////////////////////////////////
// mir_cslock - simple locker for the critical sections

class mir_cslock
{
	CRITICAL_SECTION &cs;
	__inline mir_cslock& operator = (const mir_cslock&) { return *this; }

public:
	__inline mir_cslock(CRITICAL_SECTION &_cs) : cs(_cs) { ::EnterCriticalSection(&cs); }
	__inline ~mir_cslock() { ::LeaveCriticalSection(&cs); }
};

//////////////////////////////////////////////////////////////////////////////
//pass_ptrA, pass_ptrW and pass_ptrT - automatic pointer for passwords

class pass_ptrA : public mir_ptr<char>
{
public:
	__inline explicit pass_ptrA() : mir_ptr(){}
	__inline explicit pass_ptrA(char *_p) : mir_ptr(_p) {}
	__inline ~pass_ptrA() { zero(); }
	__inline char* operator = (char *_p){ zero(); return mir_ptr::operator=(_p); }
	__inline void zero() {
		if (data) SecureZeroMemory(data, mir_strlen(data));
	}
};

class pass_ptrW : public mir_ptr<wchar_t>
{
public:
	__inline explicit pass_ptrW() : mir_ptr(){}
	__inline explicit pass_ptrW(wchar_t *_p) : mir_ptr(_p) {}
	__inline ~pass_ptrW() { zero(); }
	__inline wchar_t* operator = (wchar_t *_p){ zero(); return mir_ptr::operator=(_p); }
	__inline void zero() {
		if (data) SecureZeroMemory(data, mir_wstrlen(data)*sizeof(wchar_t));
	}
};

///////////////////////////////////////////////////////////////////////////////
// mir_cslockfull - controllable locker for the critical sections

class mir_cslockfull
{
	CRITICAL_SECTION &cs;
	bool bIsLocked;
	__inline mir_cslockfull& operator = (const mir_cslockfull&) { return *this; }

public:
	__inline void lock() { bIsLocked = true; EnterCriticalSection(&cs); }
	__inline void unlock() { bIsLocked = false; LeaveCriticalSection(&cs); }

	__inline mir_cslockfull(CRITICAL_SECTION &_cs) : cs(_cs) { lock(); }
	__inline ~mir_cslockfull() { if (bIsLocked) unlock(); }
};

///////////////////////////////////////////////////////////////////////////////
// basic class for classes that should be cleared inside new()

class MZeroedObject
{
public:
	__inline void* operator new( size_t size )
	{	return calloc( 1, size );
	}
	__inline void operator delete( void* p )
	{	free( p );
	}
};

///////////////////////////////////////////////////////////////////////////////
// general lists' templates

#define	NumericKeySortT -1
#define	HandleKeySortT  -2
#define	PtrKeySortT     -3

template<class T> struct LIST
{
	typedef int (*FTSortFunc)(const T* p1, const T* p2);

	__inline LIST(int aincr, FTSortFunc afunc = NULL)
	{	memset(this, 0, sizeof(*this));
		increment = aincr;
		sortFunc = afunc;
	}

	__inline LIST(int aincr, INT_PTR id)
	{	memset(this, 0, sizeof(*this));
		increment = aincr;
		sortFunc = FTSortFunc(id);
	}

	__inline LIST(const LIST& x)
	{
		items = NULL;
		List_Copy((SortedList*)&x, (SortedList*)this, sizeof(T));
	}

	__inline LIST& operator = (const LIST& x)
	{
		destroy();
		List_Copy((SortedList*)&x, (SortedList*)this, sizeof(T));
		return *this;
	}

	__inline ~LIST()
	{
		destroy();
	}

	__inline T*  operator[](int idx) const { return (idx >= 0 && idx < count) ? items[idx] : NULL; }
	__inline int getCount(void)      const { return count; }
	__inline T** getArray(void)      const { return items; }

	__inline int getIndex(T *p) const
	{	int idx;
		return (!List_GetIndex((SortedList*)this, p, &idx)) ? -1 : idx;
	}

	__inline void destroy(void)         { List_Destroy((SortedList*)this); }
	__inline T*   find(T *p) const      { return (T*)List_Find((SortedList*)this, p); }
	__inline int  indexOf(T *p) const   { return List_IndexOf((SortedList*)this, p); }
	__inline int  insert(T *p, int idx) { return List_Insert((SortedList*)this, p, idx); }
	__inline int  remove(int idx)       { return List_Remove((SortedList*)this, idx); }

	__inline int  insert(T *p)          { return List_InsertPtr((SortedList*)this, p); }
	__inline int  remove(T *p)          { return List_RemovePtr((SortedList*)this, p); }

	__inline void put(int idx, T *p)    { items[idx] = p; }

	// stl-like functions
	__inline int size(void) const        { return count; }
	__inline T** data(void)              { return items; }
	__inline const T**  data(void) const { return items; }

	// iterator
	struct iterator
	{
		iterator(T** ptr, int idx = 0)
			: _items(ptr), _idx(idx) { }

		bool operator!=(const iterator& other) const { return _idx != other._idx; }
		T& operator*() const { return *_items[_idx]; }
		const iterator& operator++() { ++_idx; return *this; }

	private:
		T** _items;
		int _idx;
	};
	
	__inline iterator begin(void)   const { return iterator(items, 0); }
	__inline iterator end(void)     const { return iterator(items, count); }

	__inline T** begin() const { return items; }
	__inline T** end() const { return items + count; }

protected:
	T**        items;
	int        count, limit, increment;
	FTSortFunc sortFunc;
};

template<class T> struct OBJLIST : public LIST<T>
{
	typedef int (*FTSortFunc)(const T* p1, const T* p2);

	__inline OBJLIST(int aincr, FTSortFunc afunc = NULL) :
		LIST<T>(aincr, afunc)
		{}

	__inline OBJLIST(int aincr, INT_PTR id) :
		LIST<T>(aincr, (FTSortFunc) id)
		{}

	__inline OBJLIST(const OBJLIST& x) :
		LIST<T>(x.increment, x.sortFunc)
		{	this->items = NULL;
			List_ObjCopy((SortedList*)&x, (SortedList*)this, sizeof(T));
		}

	__inline OBJLIST& operator = (const OBJLIST& x)
		{	destroy();
			List_ObjCopy((SortedList*)&x, (SortedList*)this, sizeof(T));
			return *this;
		}

	~OBJLIST()
	{
		destroy();
	}

	__inline void destroy(void)
	{
		for (int i=0; i < this->count; i++)
			delete this->items[i];

		List_Destroy((SortedList*)this);
	}

	__inline int remove(int idx) {
		delete this->items[idx];
		return List_Remove((SortedList*)this, idx);
	}

	__inline int remove(T *p)
	{
		int i = getIndex( p );
		if ( i != -1 ) {
			remove(i);
			return 1;
		}
		return 0;
	}

	__inline T& operator[](int idx) const { return *this->items[idx]; }
};

#define __A2W(s) L ## s
#define _A2W(s) __A2W(s)

class _A2T : public ptrW
{
public:
	__inline _A2T(const char* s) : ptrW(mir_a2u(s)) {}
	__inline _A2T(const char* s, int cp) : ptrW(mir_a2u_cp(s, cp)) {}
};

class _T2A : public ptrA
{
public:
	__forceinline _T2A(const wchar_t* s) : ptrA(mir_u2a(s)) {}
	__forceinline _T2A(const wchar_t* s, int cp) : ptrA(mir_u2a_cp(s, cp)) {}
};

class T2Utf : public ptrA
{
public:
	__forceinline T2Utf(const wchar_t *str) : ptrA(mir_utf8encodeW(str)) {}
	__forceinline operator BYTE* () const { return (BYTE*)data; }
	#ifdef _XSTRING_
		std::string str() const { return std::string(data); }
	#endif
};

///////////////////////////////////////////////////////////////////////////////
// basic class for classes that should be cleared inside new()

class MIR_CORE_EXPORT MBinBuffer
{
	char *m_buf;
	size_t m_len;

public:
	MBinBuffer();
	~MBinBuffer();

	__forceinline char*  data() const { return m_buf; }
	__forceinline bool   isEmpty() const { return m_len == 0; }
	__forceinline size_t length() const { return m_len; }

	// adds a buffer to the end
	void append(void *pBuf, size_t bufLen);

	// adds a buffer to the beginning
	void appendBefore(void *pBuf, size_t bufLen);

	// drops a part of buffer
	void remove(size_t sz);
};

///////////////////////////////////////////////////////////////////////////////
// parameter classes for XML, JSON & HTTP requests

struct PARAM
{
	const char *szName;
	__forceinline PARAM(const char *_name) : szName(_name)
	{}
};

struct BOOL_PARAM : public PARAM
{
	bool bValue;
	__forceinline BOOL_PARAM(const char *_name, bool _value) :
		PARAM(_name), bValue(_value)
	{}
};

struct INT_PARAM : public PARAM
{
	int32_t iValue;
	__forceinline INT_PARAM(const char *_name, int32_t _value) :
		PARAM(_name), iValue(_value)
	{}
};

struct INT64_PARAM : public PARAM
{
	int64_t iValue;
	__forceinline INT64_PARAM(const char *_name, int64_t _value) :
		PARAM(_name), iValue(_value)
	{}
};

struct CHAR_PARAM : public PARAM
{
	const char *szValue;
	__forceinline CHAR_PARAM(const char *_name, const char *_value) :
		PARAM(_name), szValue(_value)
	{}
};

struct WCHAR_PARAM : public PARAM
{
	const wchar_t *wszValue;
	__forceinline WCHAR_PARAM(const char *_name, const wchar_t *_value) :
		PARAM(_name), wszValue(_value)
	{}
};

#endif

#endif // M_SYSTEM_CPP_H
