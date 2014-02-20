/*
	TCString.h - TCString class
	Copyright (c) 2005-2008 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include <windows.h>
#include <tchar.h>
#include <crtdbg.h>
#ifdef CHARARRAY_CONVERT
#include "TMyArray.h"
#endif
#include "newpluginapi.h"
#include "m_system.h"
#include "m_database.h"

__inline int My_lstrlen(LPCSTR lpString) {return lstrlenA(lpString);}
__inline int My_lstrlen(LPCWSTR lpString) {return lstrlenW(lpString);}
__inline int My_lstrcmp(LPCSTR lpString1, LPCSTR lpString2) {return lstrcmpA(lpString1, lpString2);}
__inline int My_lstrcmp(LPCWSTR lpString1, LPCWSTR lpString2) {return lstrcmpW(lpString1, lpString2);}
__inline LPCSTR My_strstr(LPCSTR lpString1, LPCSTR lpString2) {return strstr(lpString1, lpString2);}
__inline LPWSTR My_strstr(LPCWSTR lpString1, LPCWSTR lpString2) {return (LPWSTR)wcsstr(lpString1, lpString2);}
__inline LPSTR My_lstrcpy(LPSTR lpString1, LPCSTR lpString2) {return lstrcpyA(lpString1, lpString2);}
__inline LPWSTR My_lstrcpy(LPWSTR lpString1, LPCWSTR lpString2) {return lstrcpyW(lpString1, lpString2);}
__inline LPSTR My_strncpy(LPSTR lpString1, LPCSTR lpString2, int Len) {return strncpy(lpString1, lpString2, Len);}
__inline LPWSTR My_strncpy(LPWSTR lpString1, LPCWSTR lpString2, int Len) {return wcsncpy(lpString1, lpString2, Len);}
__inline LPSTR My_strlwr(LPSTR lpString) {return _strlwr(lpString);}
__inline LPWSTR My_strlwr(LPWSTR lpString) {return _wcslwr(lpString);}

template <class T>
class TString
{
public:
	TString(): pBuf(NULL), nBufSize(0), nAllocSize(0) {}
	TString(const T *pStr): pBuf(NULL), nBufSize(0), nAllocSize(0) {*this = pStr;}
  TString(const TString<T> &Str): pBuf(NULL), nBufSize(0), nAllocSize(0) {*this = Str.pBuf;}
	~TString() {Free();}

	int GetLen() const {return (nBufSize) ? (nBufSize - 1) : 0;};
	int IsEmpty() const {return (!GetLen());};
	T *GetBuffer(int nNewLen = -1);
	void ReleaseBuffer(int nNewLen = -1);
	TString<T>& Cat(const T *pStr);
	TString<T>& Cat(const T c);
	TString<T>& DiffCat(const T *pStart, const T *pEnd);
	TString<T>& Replace(const T *szFind, const T *szReplaceBy);
	TString<T>& Replace(int nIndex, int nCount, const T *szReplaceBy);
	TString<T> Left(int nCount) const;
	TString<T> Right(int nCount) const;
	TString<T> SubStr(int nIndex, int nCount) const;
	TString<T> ToLower() const;
	void Empty();
	void Free();
	T& operator [] (int nIndex) {_ASSERT(nIndex >= 0 && nIndex <= GetLen()); return pBuf[nIndex];}
	operator const T*() const {return pBuf;}
	operator T*() {return pBuf;}
	TString<T>& operator = (const T *pStr);
	TString<T>& operator = (const TString<T> &Str) {return *this = Str.pBuf;}
//	TCString& operator + (const char *pStr)
//		{_ASSERT(pBuf && pStr); TCString Result(*this); return Result.Cat(pStr);}
	friend TString<T> operator + (const TString<T> &Str1, const T *Str2)
		{_ASSERT(Str1.pBuf && Str2); TString<T> Result(Str1); return Result.Cat(Str2);}
/*	friend TCString operator + (const char *Str1, const TCString &Str2)
		{_ASSERT(Str1 && Str2.pBuf); TCString Result(Str1); return Result.Cat(Str2);}*/
	TString<T>& operator += (const T *pStr) {_ASSERT(pBuf && pStr); return this->Cat(pStr);}
	TString<T>& operator += (const T c) {_ASSERT(pBuf); return this->Cat(c);}
	int operator == (const T *pStr) const {return (!pBuf || !pStr) ? (pBuf == pStr) : !My_lstrcmp(pBuf, pStr);}
	int operator != (const T *pStr) const {return (!pBuf || !pStr) ? (pBuf != pStr) : My_lstrcmp(pBuf, pStr);}
	int operator < (const T *pStr) const {_ASSERT(pBuf && pStr); return My_lstrcmp(pBuf, pStr) > 0;}
	int operator > (const T *pStr) const {_ASSERT(pBuf && pStr); return My_lstrcmp(pBuf, pStr) < 0;}
	int operator <= (const T *pStr) const {_ASSERT(pBuf && pStr); return My_lstrcmp(pBuf, pStr) >= 0;}
	int operator >= (const T *pStr) const {_ASSERT(pBuf && pStr); return My_lstrcmp(pBuf, pStr) <= 0;}
//	TCString& Format(char *pszFormat, ...);

private:
	void SetBufSize(int nNewBufSize);
	void SetAllocSize(int nNewAllocSize);

	T *pBuf;
	int nBufSize; // current string length + 1 (including 0 at the end)
	int nAllocSize; // allocated memory size
};


typedef TString<TCHAR> TCString;
typedef TString<char> CString;
typedef TString<WCHAR> WCString;

#ifdef CHARARRAY_CONVERT

__inline CHARARRAY WCHAR2ANSI_ARRAY(CHARARRAY &c)
{
	CHARARRAY Result;
	int Len = WideCharToMultiByte(CP_ACP, 0, (WCHAR*)c.GetData(), c.GetSize() / sizeof(WCHAR), NULL, 0, NULL, NULL);
	if (Len)
	{
		Result.SetAtGrow(Len - 1);
		if (!WideCharToMultiByte(CP_ACP, 0, (WCHAR*)c.GetData(), c.GetSize() / sizeof(WCHAR), Result.GetData(), Len, NULL, NULL))
		{
			Result.RemoveAll();
		}
		if (Result.GetSize())
		{
			Result.RemoveElem(Result.GetSize() - 1); // remove the null terminator
		}
	}
	return Result;
}

__inline CHARARRAY ANSI2WCHAR_ARRAY(CHARARRAY &c)
{
	CHARARRAY Result;
	int Len = MultiByteToWideChar(CP_ACP, 0, c.GetData(), c.GetSize(), NULL, 0);
	if (Len)
	{
		Result.SetAtGrow(Len * sizeof(WCHAR) - 1);
		if (!MultiByteToWideChar(CP_ACP, 0, c.GetData(), c.GetSize(), (WCHAR*)Result.GetData(), Len))
		{
			Result.RemoveAll();
		}
		if (Result.GetSize())
		{
			Result.RemoveElem(Result.GetSize() - 1);
			Result.RemoveElem(Result.GetSize() - 1); // remove the null terminator
		}
	}
	return Result;
}


__inline CHARARRAY WCHAR2UTF8(WCString Str)
{
	CHARARRAY Result;
	int Len = WideCharToMultiByte(CP_UTF8, 0, Str, -1, NULL, 0, NULL, NULL);
	if (Len)
	{
		Result.SetAtGrow(Len - 1);
		if (!WideCharToMultiByte(CP_UTF8, 0, Str, -1, Result.GetData(), Len, NULL, NULL))
		{
			Result.RemoveAll();
		}
	}
	return Result;
}


#endif // CHARARRAY_CONVERT


#undef db_get_s
CString db_get_s(MCONTACT hContact, const char *szModule, const char *szSetting, const char *szDefaultValue);
TCString db_get_s(MCONTACT hContact, const char *szModule, const char *szSetting, const TCHAR *szDefaultValue);
int db_get_s(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);
TCString DBGetContactSettingAsString(MCONTACT hContact, const char *szModule, const char *szSetting, const TCHAR *szDefaultValue); // also converts numeric values to a string

// various string helpers. their return values are valid only while the class is visible
class UTF8Encode
{
public:
	UTF8Encode(const char *str) { p = mir_utf8encode(str); }
	UTF8Encode(const wchar_t *str) { p = mir_utf8encodeW(str); }
	~UTF8Encode() { mir_free(p); }
	operator char*() { return p; }

private:
	char *p;
};

class UTF8DecodeA
{
public:
	UTF8DecodeA(const char *str) { p = mir_strdup(str); mir_utf8decode(p, NULL); }
	~UTF8DecodeA() { mir_free(p); }
	operator char*() { return p; }

private:
	char *p;
};

class UTF8DecodeW
{
public:
	UTF8DecodeW(const char *str) { p = mir_utf8decodeW(str); }
	~UTF8DecodeW() { mir_free(p); }
	operator wchar_t*() { return p; }

private:
	wchar_t *p;
};


#define UTF8Decode UTF8DecodeW


/*class mallocStrA
{
public:
	mallocStrA(int n) { p = (char*)malloc((n + 1) * sizeof(char)); }
	mallocStrA(const char *str) { p = str ? strdup(str) : NULL; }
	~mallocStrA() { if (p) free(p); }
	operator char*() { return p; }

private:
	char *p;
};

class mallocStrW
{
public:
	mallocStrW(int n) { p = (wchar_t*)malloc((n + 1) * sizeof(wchar_t)); }
	mallocStrW(const wchar_t *str) { p = str ? _wcsdup(str) : NULL; }
	~mallocStrW() { if (p) free(p); }
	operator wchar_t*() { return p; }

private:
	wchar_t *p;
};


#define mallocStr mallocStrW

*/