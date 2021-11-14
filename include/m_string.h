/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

#ifndef M_STRING_H__
#define M_STRING_H__

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
	#include <mbstring.h>
#else
	#include <ctype.h>
#endif // _WINDOWS
#include <wchar.h>

#include <m_core.h>

#ifdef __MINGW32__
#include <limits.h>

__inline size_t strnlen(const char *string, size_t maxlen)
{
	const char *end = (const char *)memchr ((const void *)string, '\0', maxlen);
	return end ? (size_t) (end - string) : maxlen;
}
__inline size_t wcsnlen(const wchar_t *string, size_t maxlen)
{
	const wchar_t *end = wmemchr (string, L'\0', maxlen);
	return end ? (size_t) (end - string) : maxlen;
}

/* FIXME: This may be wrong assumption about Langpack_GetDefaultCodePage */
#define Langpack_GetDefaultCodePage() CP_THREAD_ACP
/* FIXME: This is unsafe */
#define memcpy_s(dest,size,src,count) memcpy(dest,src,count)
/* FIXME: This is quite silly implementation of _mbsstr */
#define _mbsstr(str,search) strstr((const char *)str,(const char *)search)
#define __max(x,y) (((x)<(y))?(y):(x))
#endif /* __MINGW32__ */

/////////////////////////////////////////////////////////////////////////////////////////

struct CMStringData;

MIR_CORE_DLL(CMStringData*) mirstr_allocate(int nChars, int nCharSize);
MIR_CORE_DLL(void)          mirstr_free(CMStringData *pData);
MIR_CORE_DLL(CMStringData*) mirstr_realloc(CMStringData* pData, int nChars, int nCharSize);
MIR_CORE_DLL(CMStringData*) mirstr_getNil();

MIR_CORE_DLL(void) mirstr_lock(CMStringData* pThis);
MIR_CORE_DLL(void) mirstr_release(CMStringData* pThis);
MIR_CORE_DLL(void) mirstr_unlock(CMStringData* pThis);

/////////////////////////////////////////////////////////////////////////////////////////

enum CMStringDataFormat { FORMAT };

struct CMStringData
{
	int nDataLength;   // Length of currently used data in XCHARs (not including terminating null)
	int nAllocLength;  // Length of allocated data in XCHARs (not including terminating null)
	long nRefs;        // Reference count: negative == locked

	__forceinline void* data() { return (this + 1); }
	__forceinline void AddRef() { InterlockedIncrement(&nRefs); }
	__forceinline bool IsLocked() const { return nRefs < 0; }
	__forceinline bool IsShared() const { return nRefs > 1; }

	__forceinline void Lock() { mirstr_lock(this); }
	__forceinline void Release() { mirstr_release(this); }
	__forceinline void Unlock() { mirstr_unlock(this); }
};

template< typename BaseType = char >
class ChTraitsBase
{
public:
	typedef char XCHAR;
	typedef LPSTR PXSTR;
	typedef LPCSTR PCXSTR;
	typedef wchar_t YCHAR;
	typedef LPWSTR PYSTR;
	typedef LPCWSTR PCYSTR;
};

template<>
class ChTraitsBase< wchar_t >
{
public:
	typedef wchar_t XCHAR;
	typedef LPWSTR PXSTR;
	typedef LPCWSTR PCXSTR;
	typedef char YCHAR;
	typedef LPSTR PYSTR;
	typedef LPCSTR PCYSTR;
};

template< typename BaseType >
class CMSimpleStringT
{
public:
	typedef typename ChTraitsBase< BaseType >::XCHAR XCHAR;
	typedef typename ChTraitsBase< BaseType >::PXSTR PXSTR;
	typedef typename ChTraitsBase< BaseType >::PCXSTR PCXSTR;
	typedef typename ChTraitsBase< BaseType >::YCHAR YCHAR;
	typedef typename ChTraitsBase< BaseType >::PYSTR PYSTR;
	typedef typename ChTraitsBase< BaseType >::PCYSTR PCYSTR;

public:
	explicit CMSimpleStringT();

	CMSimpleStringT(const CMSimpleStringT& strSrc);
	CMSimpleStringT(PCXSTR pszSrc);
	CMSimpleStringT(const XCHAR* pchSrc, int nLength);
	~CMSimpleStringT();

	__forceinline operator CMSimpleStringT<BaseType>&()
	{	return *(CMSimpleStringT<BaseType>*)this;
	}

	CMSimpleStringT& operator=(const CMSimpleStringT& strSrc);

	__forceinline CMSimpleStringT& operator=(PCXSTR pszSrc)
	{	SetString(pszSrc);
		return *this;
	}

	__forceinline CMSimpleStringT& operator+=(const CMSimpleStringT& strSrc)
	{	Append(strSrc);
		return *this;
	}

	__forceinline CMSimpleStringT& operator+=(PCXSTR pszSrc)
	{	Append(pszSrc);
		return *this;
	}

	__forceinline CMSimpleStringT& operator+=(char ch)
	{	AppendChar(XCHAR(ch));
		return *this;
	}

	__forceinline CMSimpleStringT& operator+=(unsigned char ch)
	{	AppendChar(XCHAR(ch));
		return *this;
	}

	__forceinline CMSimpleStringT& operator+=(wchar_t ch)
	{	AppendChar(XCHAR(ch));
		return *this;
	}

	__forceinline XCHAR operator[](int iChar) const
	{	return m_pszData[iChar];
	}

	__forceinline operator PCXSTR() const
	{	return m_pszData;
	}

	__forceinline PCXSTR c_str() const
	{	return m_pszData;
	}

	__forceinline int GetAllocLength() const
	{	return GetData()->nAllocLength;
	}

	__forceinline XCHAR GetAt(int iChar) const
	{	return m_pszData[iChar];
	}

	__forceinline PXSTR GetBuffer(int nMinBufferLength)
	{	return PrepareWrite(nMinBufferLength);
	}

	__forceinline int GetLength() const
	{	return GetData()->nDataLength;
	}

	__forceinline PCXSTR GetString() const
	{	return m_pszData;
	}

	__forceinline PCXSTR GetTail() const
	{	return m_pszData + GetData()->nDataLength;
	}

	__forceinline bool IsEmpty() const
	{	return GetLength() == 0;
	}

	__forceinline void Preallocate(int nLength)
	{	PrepareWrite(nLength);
	}

	__forceinline void ReleaseBufferSetLength(int nNewLength)
	{	SetLength(nNewLength);
	}

	void   Append(PCXSTR pszSrc);
	void   Append(PCXSTR pszSrc, int nLength);
	void   AppendChar(XCHAR ch);
	void   Append(const CMSimpleStringT& strSrc);

	void   Empty();
	void   FreeExtra();

	PXSTR  GetBuffer();
	PXSTR  GetBufferSetLength(int nLength);

	PXSTR  LockBuffer();
	void   UnlockBuffer();

	void   ReleaseBuffer(int nNewLength = -1);

	void   Truncate(int nNewLength);
	void   SetAt(int iChar, XCHAR ch);
	void   SetString(PCXSTR pszSrc);
	void   SetString(PCXSTR pszSrc, int nLength);

public:
	friend CMSimpleStringT operator+(const CMSimpleStringT& str1, const CMSimpleStringT& str2);
	friend CMSimpleStringT operator+(const CMSimpleStringT& str1, PCXSTR psz2);
	friend CMSimpleStringT operator+(PCXSTR psz1, const CMSimpleStringT& str2);

	static void MIR_SYSCALL CopyChars(XCHAR* pchDest, const XCHAR* pchSrc, int nChars);
	static void MIR_SYSCALL CopyChars(XCHAR* pchDest, size_t nDestLen, const XCHAR* pchSrc, int nChars);
	static void MIR_SYSCALL CopyCharsOverlapped(XCHAR* pchDest, const XCHAR* pchSrc, int nChars);
	static void MIR_SYSCALL CopyCharsOverlapped(XCHAR* pchDest, size_t nDestLen, const XCHAR* pchSrc, int nChars);
	static int  MIR_SYSCALL StringLength(const char* psz);
	static int  MIR_SYSCALL StringLength(const wchar_t* psz);
	static int  MIR_SYSCALL StringLengthN(const char* psz, size_t sizeInXChar);
	static int  MIR_SYSCALL StringLengthN(const wchar_t* psz, size_t sizeInXChar);
	static void MIR_SYSCALL Concatenate(CMSimpleStringT& strResult, PCXSTR psz1, int nLength1, PCXSTR psz2, int nLength2);

	// Implementation
private:
	__forceinline CMStringData* GetData() const
	{	return (reinterpret_cast<CMStringData *>(m_pszData)-1);
	}

	void Attach(CMStringData* pData);
	void Fork(int nLength);
	PXSTR PrepareWrite(int nLength);
	void PrepareWrite2(int nLength);
	void Reallocate(int nLength);
	void SetLength(int nLength);
	static CMStringData* MIR_SYSCALL CloneData(CMStringData* pData);

private:
	PXSTR m_pszData;
};


template< typename _CharType = char >
class ChTraitsCRT : public ChTraitsBase < _CharType >
{
public:
	static char* MIR_SYSCALL CharNext(const char *p)
	{
		#ifdef _MSC_VER
			return reinterpret_cast<char*>(_mbsinc(reinterpret_cast<const unsigned char*>(p)));
		#else
			return reinterpret_cast<char*>(p+1);
		#endif
	}

	static int MIR_SYSCALL IsDigit(char ch)
	{
		#ifdef _MSC_VER
			return _ismbcdigit(ch);
		#else
			return isdigit(ch);
		#endif
	}

	static int MIR_SYSCALL IsSpace(char ch)
	{
		#ifdef _MSC_VER
			return _ismbcspace(ch);
		#else
			return isspace(ch);
		#endif
	}

	static int MIR_SYSCALL StringCompare(const char *pszA, const char *pszB)
	{
		#ifdef _MSC_VER
			return _mbscmp(reinterpret_cast<const unsigned char*>(pszA), reinterpret_cast<const unsigned char*>(pszB));
		#else
			return strcmp(pszA, pszB);
		#endif
	}

	static int MIR_SYSCALL StringCompareIgnore(const char *pszA, const char *pszB)
	{
		#ifdef _MSC_VER
			return _mbsicmp(reinterpret_cast<const unsigned char*>(pszA), reinterpret_cast<const unsigned char*>(pszB));
		#else
			return strcasecmp(pszA, pszB);
		#endif
	}

	static int MIR_SYSCALL StringCollate(const char *pszA, const char *pszB)
	{
		#ifdef _MSC_VER
			return _mbscoll(reinterpret_cast<const unsigned char*>(pszA), reinterpret_cast<const unsigned char*>(pszB));
		#else
			return strcoll(pszA, pszB);
		#endif
	}

	static int MIR_SYSCALL StringCollateIgnore(const char *pszA, const char *pszB)
	{
		#ifdef _MSC_VER
			return _mbsicoll(reinterpret_cast<const unsigned char*>(pszA), reinterpret_cast<const unsigned char*>(pszB));
		#else
			return strcoll(pszA, pszB);
		#endif
	}

	static const char* MIR_SYSCALL StringFindString(const char *pszBlock, const char *pszMatch)
	{
		#ifdef _MSC_VER
			return reinterpret_cast<const char*>(_mbsstr(reinterpret_cast<const unsigned char*>(pszBlock),
				reinterpret_cast<const unsigned char*>(pszMatch)));
		#else
			return strstr(pszBlock, pszMatch);
		#endif
	}

	static char* MIR_SYSCALL StringFindString(char *pszBlock, const char *pszMatch)
	{
		return const_cast<char*>(StringFindString(const_cast<const char*>(pszBlock), pszMatch));
	}

	static const char* MIR_SYSCALL StringFindChar(const char *pszBlock, char chMatch)
	{
		#ifdef _MSC_VER
			return reinterpret_cast<const char*>(_mbschr(reinterpret_cast<const unsigned char*>(pszBlock), (unsigned char)chMatch));
		#else
			return strchr(pszBlock, chMatch);
		#endif
	}

	static const char* MIR_SYSCALL StringFindCharRev(const char *psz, char ch)
	{
		#ifdef _MSC_VER
			return reinterpret_cast<const char*>(_mbsrchr(reinterpret_cast<const unsigned char*>(psz), (unsigned char)ch));
		#else
			return strrchr(psz, ch);
		#endif
	}

	static const char* MIR_SYSCALL StringScanSet(const char *pszBlock, const char *pszMatch)
	{
		#ifdef _MSC_VER
			return reinterpret_cast<const char*>(_mbspbrk(reinterpret_cast<const unsigned char*>(pszBlock),
				reinterpret_cast<const unsigned char*>(pszMatch)));
		#else
			return strpbrk(pszBlock, pszMatch);
		#endif
	}

	static int MIR_SYSCALL StringSpanIncluding(const char *pszBlock, const char *pszSet)
	{
		#ifdef _MSC_VER
			return (int)_mbsspn(reinterpret_cast<const unsigned char*>(pszBlock), reinterpret_cast<const unsigned char*>(pszSet));
		#else
			return (int)strspn(pszBlock, pszSet);
		#endif
	}

	static int MIR_SYSCALL StringSpanExcluding(const char *pszBlock, const char *pszSet)
	{
		#ifdef _MSC_VER
			return (int)_mbscspn(reinterpret_cast<const unsigned char*>(pszBlock), reinterpret_cast<const unsigned char*>(pszSet));
		#else
			return (int)strcspn(pszBlock, pszSet);
		#endif
	}

	static char* MIR_SYSCALL StringUppercase(char *psz)
	{
		#ifdef _MSC_VER
			CharUpperBuffA(psz, (uint32_t)strlen(psz));
		#else
			strupr(psz);
		#endif
		return psz;
	}

	static char* MIR_SYSCALL StringLowercase(char *psz)
	{
		#ifdef _MSC_VER
			CharLowerBuffA(psz, (uint32_t)strlen(psz));
		#else
			strlwr(psz);
		#endif
		return psz;
	}

	static char* MIR_SYSCALL StringUppercase(char *psz, size_t size)
	{
		#ifdef _MSC_VER
			CharUpperBuffA(psz, (uint32_t)size);
		#else
			
		#endif
		return psz;
	}

	static char* MIR_SYSCALL StringLowercase(char *psz, size_t size)
	{
		#ifdef _MSC_VER
			CharLowerBuffA(psz, (uint32_t)size);
		#endif
		return psz;
	}

	static char* MIR_SYSCALL StringReverse(char *psz)
	{
		#ifdef _MSC_VER
			return reinterpret_cast<LPSTR>(_mbsrev(reinterpret_cast<unsigned char*>(psz)));
		#else
			return strrev(psz);
		#endif
	}

	static int MIR_SYSCALL GetFormattedLength(_Printf_format_string_ const char *pszFormat, va_list args)
	{
		#ifdef _MSC_VER
			return _vscprintf(pszFormat, args);
		#else
		#endif
	}

	static int MIR_SYSCALL Format(char *pszBuffer, size_t nlength, _Printf_format_string_ const char *pszFormat, va_list args)
	{
		#ifdef _MSC_VER
			return vsprintf_s(pszBuffer, nlength, pszFormat, args);
		#else
		#endif
	}

	static int MIR_SYSCALL GetBaseTypeLength(const char *pszSrc)
	{
		// Returns required buffer length in XCHARs
		return int(strlen(pszSrc));
	}

	static int MIR_SYSCALL GetBaseTypeLength(const char *pszSrc, int nLength)
	{
		(void)pszSrc;
		// Returns required buffer length in XCHARs
		return nLength;
	}

	static int MIR_SYSCALL GetBaseTypeLength(const wchar_t *pszSource)
	{
		// Returns required buffer length in XCHARs
		#ifdef _MSC_VER
			return ::WideCharToMultiByte(Langpack_GetDefaultCodePage(), 0, pszSource, -1, NULL, 0, NULL, NULL) - 1;
		#else
			return 0;
		#endif
	}

	static int MIR_SYSCALL GetBaseTypeLength(const wchar_t *pszSource, int nLength)
	{
		// Returns required buffer length in XCHARs
		#ifdef _MSC_VER
			return ::WideCharToMultiByte(Langpack_GetDefaultCodePage(), 0, pszSource, nLength, NULL, 0, NULL, NULL);
		#else
			return 0;
		#endif
	}

	static void MIR_SYSCALL ConvertToBaseType(char*pszDest, int nDestLength, const char *pszSrc, int nSrcLength = -1)
	{
		if (nSrcLength == -1) { nSrcLength = 1 + GetBaseTypeLength(pszSrc); }
		// nLen is in XCHARs
		memcpy_s(pszDest, nDestLength*sizeof(char), pszSrc, nSrcLength*sizeof(char));
	}

	static void MIR_SYSCALL ConvertToBaseType(char*pszDest, int nDestLength, const wchar_t *pszSrc, int nSrcLength = -1)
	{
		// nLen is in XCHARs
		#ifdef _MSC_VER
			::WideCharToMultiByte(Langpack_GetDefaultCodePage(), 0, pszSrc, nSrcLength, pszDest, nDestLength, NULL, NULL);
		#endif
	}

	static void ConvertToOem(_CharType* pstrString)
	{
		#ifdef _MSC_VER
			bool fSuccess = ::CharToOemA(pstrString, pstrString);
		#endif // _MSC_VER
	}

	static void ConvertToAnsi(_CharType* pstrString)
	{
        #ifdef _MSC_VER
			bool fSuccess = ::OemToCharA(pstrString, pstrString);
		#endif
	}

	static void ConvertToOem(_CharType* pstrString, size_t size)
	{
		#ifdef _MSC_VER
			uint32_t dwSize = static_cast<uint32_t>(size);
			::CharToOemBuffA(pstrString, pstrString, dwSize);
		#endif
	}

	static void ConvertToAnsi(_CharType* pstrString, size_t size)
	{
		#ifdef _MSC_VER
			uint32_t dwSize = static_cast<uint32_t>(size);
			::OemToCharBuffA(pstrString, pstrString, dwSize);
		#endif
	}

	static void MIR_SYSCALL FloodCharacters(char ch, int nLength, char* pch)
	{
		// nLength is in XCHARs
		memset(pch, ch, nLength);
	}

	static int MIR_SYSCALL SafeStringLen(const char *psz)
	{
		// returns length in bytes
		return (psz != NULL) ? int(strlen(psz)) : 0;
	}

	static int MIR_SYSCALL SafeStringLen(const wchar_t *psz)
	{
		// returns length in wchar_ts
		return (psz != NULL) ? int(wcslen(psz)) : 0;
	}

	static int MIR_SYSCALL GetCharLen(const wchar_t *pch)
	{
		// returns char length
		return 1;
	}

	static int MIR_SYSCALL GetCharLen(const char *pch)
	{
		// returns char length
		#ifdef _MSC_VER
			return int(_mbclen(reinterpret_cast<const unsigned char*>(pch)));
		#else
			return mblen(pch, strlen(pch));
		#endif
	}

	static uint32_t MIR_SYSCALL GetEnvironmentVariable(const char *pszVar, char*pszBuffer, uint32_t dwSize)
	{
		#ifdef _MSC_VER
			return ::GetEnvironmentVariableA(pszVar, pszBuffer, dwSize);
		#endif // _MSC_VER
	}

	static char* MirCopy(const char *pstrString, size_t size)
	{
		return mir_strndup(pstrString, size);
	}
};

// specialization for wchar_t
template<>
class ChTraitsCRT< wchar_t > : public ChTraitsBase< wchar_t >
{
	static uint32_t MIR_SYSCALL _GetEnvironmentVariableW(const wchar_t *pszName, wchar_t *pszBuffer, uint32_t nSize)
	{
		#ifdef _MSC_VER
			return ::GetEnvironmentVariableW(pszName, pszBuffer, nSize);
		#endif // _MSC_VER
	}

public:
	static wchar_t* MIR_SYSCALL CharNext(const wchar_t *psz)
	{
		return const_cast<wchar_t*>(psz+1);
	}

	static int MIR_SYSCALL IsDigit(wchar_t ch)
	{
		#ifdef _MSC_VER
			return iswdigit(static_cast<unsigned short>(ch));
		#else
			return 0;
		#endif
	}

	static int MIR_SYSCALL IsSpace(wchar_t ch)
	{
		#ifdef _MSC_VER
			return iswspace(static_cast<unsigned short>(ch));
		#else
			return 0;
		#endif
	}

	static int MIR_SYSCALL StringCompare(const wchar_t *pszA, const wchar_t *pszB)
	{
		return wcscmp(pszA, pszB);
	}

	static int MIR_SYSCALL StringCompareIgnore(const wchar_t *pszA, const wchar_t *pszB)
	{
		#ifdef _MSC_VER
			return _wcsicmp(pszA, pszB);
		#else
			return 0;
		#endif
	}

	static int MIR_SYSCALL StringCollate(const wchar_t *pszA, const wchar_t *pszB)
	{
		return wcscoll(pszA, pszB);
	}

	static int MIR_SYSCALL StringCollateIgnore(const wchar_t *pszA, const wchar_t *pszB)
	{
		#ifdef _MSC_VER
			return _wcsicoll(pszA, pszB);
		#else
			return 0;
		#endif
	}

	static const wchar_t* MIR_SYSCALL StringFindString(const wchar_t *pszBlock, const wchar_t *pszMatch)
	{
		return wcsstr(pszBlock, pszMatch);
	}

	static wchar_t* MIR_SYSCALL StringFindString(wchar_t *pszBlock, const wchar_t *pszMatch)
	{
		return const_cast<wchar_t*>(wcsstr(pszBlock, pszMatch));
	}

	static const wchar_t* MIR_SYSCALL StringFindChar(const wchar_t *pszBlock, wchar_t chMatch)
	{
		return wcschr(pszBlock, chMatch);
	}

	static const wchar_t* MIR_SYSCALL StringFindCharRev(const wchar_t *psz, wchar_t ch)
	{
		return wcsrchr(psz, ch);
	}

	static const wchar_t* MIR_SYSCALL StringScanSet(const wchar_t *pszBlock, const wchar_t *pszMatch)
	{
		return wcspbrk(pszBlock, pszMatch);
	}

	static int MIR_SYSCALL StringSpanIncluding(const wchar_t *pszBlock, const wchar_t *pszSet)
	{
		return (int)wcsspn(pszBlock, pszSet);
	}

	static int MIR_SYSCALL StringSpanExcluding(const wchar_t *pszBlock, const wchar_t *pszSet)
	{
		return (int)wcscspn(pszBlock, pszSet);
	}

	static wchar_t* MIR_SYSCALL StringUppercase(wchar_t *psz)
	{
		#ifdef _MSC_VER
			CharUpperBuffW(psz, (uint32_t)wcslen(psz));
		#endif
		return psz;
	}

	static wchar_t* MIR_SYSCALL StringLowercase(wchar_t *psz)
	{
		#ifdef _MSC_VER
			CharLowerBuffW(psz, (uint32_t)wcslen(psz));
		#endif
		return psz;
	}

	static wchar_t* MIR_SYSCALL StringUppercase(wchar_t *psz, size_t len)
	{
		#ifdef _MSC_VER
			CharUpperBuffW(psz, (uint32_t)len);
		#endif
		return psz;
	}

	static wchar_t* MIR_SYSCALL StringLowercase(wchar_t *psz, size_t len)
	{
		#ifdef _MSC_VER
			CharLowerBuffW(psz, (uint32_t)len);
		#endif
		return psz;
	}

	static wchar_t* MIR_SYSCALL StringReverse(wchar_t *psz)
	{
		#ifdef _MSC_VER
			return _wcsrev(psz);
		#else
			return psz;
		#endif
	}

	static int MIR_SYSCALL GetFormattedLength(_Printf_format_string_ const wchar_t *pszFormat, va_list args)
	{
		#ifdef _MSC_VER
			return _vscwprintf(pszFormat, args);
		#else
			return 0;
		#endif
	}

	static int MIR_SYSCALL Format(wchar_t *pszBuffer, _Printf_format_string_ const wchar_t *pszFormat, va_list args)
	{
		#pragma warning(push)
		#pragma warning(disable : 4996)

		#ifdef _MSC_VER
			return vswprintf(pszBuffer, pszFormat, args);
		#else
			return 0;
		#endif
		#pragma warning(pop)
	}

	static int MIR_SYSCALL Format(wchar_t *pszBuffer, size_t nLength, _Printf_format_string_ const wchar_t *pszFormat, va_list args)
	{
		#pragma warning(push)
		#pragma warning(disable : 4996)

		#ifdef _MSC_VER
			return _vsnwprintf(pszBuffer, nLength, pszFormat, args);
		#else
			return 0;
		#endif

		#pragma warning(pop)
	}

	static int MIR_SYSCALL GetBaseTypeLength(const char *pszSrc)
	{
		// Returns required buffer size in wchar_ts
		#ifdef _MSC_VER
			return ::MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, nullptr, 0)-1;
		#else
			return 0;
		#endif
	}

	static int MIR_SYSCALL GetBaseTypeLength(const char *pszSrc, int nLength)
	{
		// Returns required buffer size in wchar_ts
		#ifdef _MSC_VER
			return ::MultiByteToWideChar(CP_ACP, 0, pszSrc, nLength, nullptr, 0);
		#else
			return 0;
		#endif
	}

	static int MIR_SYSCALL GetBaseTypeLength(const wchar_t *pszSrc)
	{
		// Returns required buffer size in wchar_ts
		return (int)wcslen(pszSrc);
	}

	static int MIR_SYSCALL GetBaseTypeLength(const wchar_t *pszSrc, int nLength)
	{
		(void)pszSrc;
		// Returns required buffer size in wchar_ts
		return nLength;
	}

	static void MIR_SYSCALL ConvertToBaseType(wchar_t *pszDest, int nDestLength, const char *pszSrc, int nSrcLength = -1)
	{
		// nLen is in wchar_ts
		#ifdef _MSC_VER
			::MultiByteToWideChar(CP_ACP, 0, pszSrc, nSrcLength, pszDest, nDestLength);
		#endif
	}

	static void MIR_SYSCALL ConvertToBaseType(wchar_t *pszDest, int nDestLength, const wchar_t *pszSrc, int nSrcLength = -1)
	{
		if (nSrcLength == -1) { nSrcLength=1 + GetBaseTypeLength(pszSrc); }
		// nLen is in wchar_ts
		#if _MSC_VER >= 1400
			wmemcpy_s(pszDest, nDestLength, pszSrc, nSrcLength);
		#else
			wmemcpy(pszDest, pszSrc, nDestLength);
		#endif
	}

	static void MIR_SYSCALL FloodCharacters(wchar_t ch, int nLength, wchar_t *psz)
	{
		// nLength is in XCHARs
		for (int i = 0; i < nLength; i++)
		{
			psz[i] = ch;
		}
	}

	static int MIR_SYSCALL SafeStringLen(const char *psz)
	{
		// returns length in bytes
		return (psz != nullptr) ? (int)strlen(psz) : 0;
	}

	static int MIR_SYSCALL SafeStringLen(const wchar_t *psz)
	{
		// returns length in wchar_ts
		return (psz != nullptr) ? (int)wcslen(psz) : 0;
	}

	static int MIR_SYSCALL GetCharLen(const wchar_t* pch)
	{
		(void)pch;
		// returns char length
		return 1;
	}

	static int MIR_SYSCALL GetCharLen(const char* pch)
	{
		// returns char length
		#ifdef _MSC_VER
			return (int)(_mbclen(reinterpret_cast< const unsigned char* >(pch)));
		#else
			return mblen(pch, strlen(pch));
		#endif
	}

	static uint32_t MIR_SYSCALL GetEnvironmentVariable(const wchar_t *pszVar, wchar_t *pszBuffer, uint32_t dwSize)
	{
		return _GetEnvironmentVariableW(pszVar, pszBuffer, dwSize);
	}

	static void MIR_SYSCALL ConvertToOem(wchar_t* /*psz*/)
	{
	}

	static void MIR_SYSCALL ConvertToAnsi(wchar_t* /*psz*/)
	{
	}

	static void MIR_SYSCALL ConvertToOem(wchar_t* /*psz*/, size_t)
	{
	}

	static void MIR_SYSCALL ConvertToAnsi(wchar_t* /*psz*/, size_t)
	{
	}

	static wchar_t* MirCopy(const wchar_t *pstrString, size_t size)
	{
		return mir_wstrndup(pstrString, size);
	}
};

template< typename BaseType, class StringTraits >
class MIR_CORE_EXPORT CMStringT : public CMSimpleStringT< BaseType >
{
public:
	typedef CMSimpleStringT< BaseType> CThisSimpleString;
	typedef typename CThisSimpleString::XCHAR XCHAR;
	typedef typename CThisSimpleString::PXSTR PXSTR;
	typedef typename CThisSimpleString::PCXSTR PCXSTR;
	typedef typename CThisSimpleString::YCHAR YCHAR;
	typedef typename CThisSimpleString::PYSTR PYSTR;
	typedef typename CThisSimpleString::PCYSTR PCYSTR;

public:
	CMStringT();

	// Copy constructor
	CMStringT(const CMStringT& strSrc);

	CMStringT(const XCHAR* pszSrc);
	CMStringT(CMStringDataFormat, _Printf_format_string_ const XCHAR* pszFormat, ...);

	CMStringT(const YCHAR* pszSrc);
	CMStringT(const unsigned char* pszSrc);

	CMStringT(char ch, int nLength = 1);
	CMStringT(wchar_t ch, int nLength = 1);

	CMStringT(const XCHAR* pch, int nLength);
	CMStringT(const YCHAR* pch, int nLength);

	// Destructor
	~CMStringT();

	// Assignment operators
	CMStringT& operator=(const CMStringT& strSrc);
	CMStringT& operator=(PCXSTR pszSrc);
	CMStringT& operator=(PCYSTR pszSrc);
	CMStringT& operator=(const unsigned char* pszSrc);
	CMStringT& operator=(char ch);
	CMStringT& operator=(wchar_t ch);

	CMStringT& operator+=(const CMStringT& str);
	CMStringT& operator+=(const CThisSimpleString& str);
	CMStringT& operator+=(PCXSTR pszSrc);
	CMStringT& operator+=(PCYSTR pszSrc);
	CMStringT& operator+=(char ch);
	CMStringT& operator+=(unsigned char ch);
	CMStringT& operator+=(wchar_t ch);

	// Comparison

	int Compare(PCXSTR psz) const;
	int CompareNoCase(PCXSTR psz) const;
	int Collate(PCXSTR psz) const;
	int CollateNoCase(PCXSTR psz) const;

	// Advanced manipulation

	// Delete 'nCount' characters, starting at index 'iIndex'
	int Delete(int iIndex, int nCount = 1);

	// Insert character 'ch' before index 'iIndex'
	int Insert(int iIndex, XCHAR ch);

	// Insert string 'psz' before index 'iIndex'
	int Insert(int iIndex, PCXSTR psz);

	// Replace all occurrences of character 'chOld' with character 'chNew'
	int Replace(XCHAR chOld, XCHAR chNew);

	// Replace all occurrences of string 'pszOld' with string 'pszNew'
	int Replace(PCXSTR pszOld, PCXSTR pszNew);

	// Remove all occurrences of character 'chRemove'
	int Remove(XCHAR chRemove);

	CMStringT Tokenize(PCXSTR pszTokens, int& iStart) const;

	// find routines

	// Find the first occurrence of character 'ch', starting at index 'iStart'
	int Find(XCHAR ch, int iStart = 0) const;

	// look for a specific sub-string

	// Find the first occurrence of string 'pszSub', starting at index 'iStart'
	int Find(PCXSTR pszSub, int iStart = 0) const;

	// Find the first occurrence of any of the characters in string 'pszCharSet'
	int FindOneOf(PCXSTR pszCharSet) const;

	// Find the last occurrence of character 'ch'
	int ReverseFind(XCHAR ch) const;

	// manipulation

	// Convert the string to uppercase
	CMStringT& MakeUpper();

	// Convert the string to lowercase
	CMStringT& MakeLower();

	// Reverse the string
	CMStringT& MakeReverse();

	// trimming

	// Remove all trailing whitespace
	CMStringT& TrimRight();

	// Remove all leading whitespace
	CMStringT& TrimLeft();

	// Remove all leading and trailing whitespace
	CMStringT& Trim();

	// Remove all leading and trailing occurrences of character 'chTarget'
	CMStringT& Trim(XCHAR chTarget);

	// Remove all leading and trailing occurrences of any of the characters in the string 'pszTargets'
	CMStringT& Trim(PCXSTR pszTargets);

	// trimming anything (either side)

	// Remove all trailing occurrences of character 'chTarget'
	CMStringT& TrimRight(XCHAR chTarget);

	// Remove all trailing occurrences of any of the characters in string 'pszTargets'
	CMStringT& TrimRight(PCXSTR pszTargets);

	// Remove all leading occurrences of character 'chTarget'
	CMStringT& TrimLeft(XCHAR chTarget);

	// Remove all leading occurrences of any of the characters in string 'pszTargets'
	CMStringT& TrimLeft(PCXSTR pszTargets);

	// Convert the string to the OEM character set
	void AnsiToOem();

	// Convert the string to the ANSI character set
	void OemToAnsi();

	// Very simple sub-string extraction

	// Return the substring starting at index 'iFirst'
	CMStringT Mid(int iFirst) const;

	// Return the substring starting at index 'iFirst', with length 'nCount'
	CMStringT Mid(int iFirst, int nCount) const;

	// Return the substring consisting of the rightmost 'nCount' characters
	CMStringT Right(int nCount) const;

	// Return the substring consisting of the leftmost 'nCount' characters
	CMStringT Left(int nCount) const;

	// Return the substring consisting of the leftmost characters in the set 'pszCharSet'
	CMStringT SpanIncluding(PCXSTR pszCharSet) const;

	// Return the substring consisting of the leftmost characters not in the set 'pszCharSet'
	CMStringT SpanExcluding(PCXSTR pszCharSet) const;

	// Format data using format string 'pszFormat'
	PCXSTR Format(PCXSTR _Printf_format_string_ pszFormat, ...);
	PCXSTR FormatV(PCXSTR _Printf_format_string_ pszFormat, va_list args);

	// Append formatted data using format string 'pszFormat'
	PCXSTR AppendFormat(PCXSTR _Printf_format_string_ pszFormat, ...);
	void   AppendFormatV(PCXSTR _Printf_format_string_ pszFormat, va_list args);

	// return a copy of string to be freed by mir_free()
	PXSTR Detach() const;

	// Set the string to the value of environment variable 'pszVar'
	BOOL GetEnvironmentVariable(PCXSTR pszVar);

	friend bool __forceinline operator==(const CMStringT& str1, const CMStringT& str2) { return str1.Compare(str2) == 0; }
	friend bool __forceinline operator==(const CMStringT& str1, PCXSTR psz2) { return str1.Compare(psz2) == 0; }
	friend bool __forceinline operator==(PCXSTR psz1, const CMStringT& str2) { return str2.Compare(psz1) == 0; }
	friend bool __forceinline operator==(const CMStringT& str1, PCYSTR psz2) { return str1 == CMStringT(psz2); }
	friend bool __forceinline operator==(PCYSTR psz1, const CMStringT& str2) { return CMStringT(psz1) == str2; }

	friend bool __forceinline operator!=(const CMStringT& str1, const CMStringT& str2) { return str1.Compare(str2) != 0; }
	friend bool __forceinline operator!=(const CMStringT& str1, PCXSTR psz2) { return str1.Compare(psz2) != 0; }
	friend bool __forceinline operator!=(PCXSTR psz1, const CMStringT& str2) { return str2.Compare(psz1) != 0; }
	friend bool __forceinline operator!=(const CMStringT& str1, PCYSTR psz2) { return str1 != CMStringT(psz2); }
	friend bool __forceinline operator!=(PCYSTR psz1, const CMStringT& str2) { return CMStringT(psz1) != str2; }

	friend bool __forceinline operator<(const CMStringT& str1, const CMStringT& str2) { return str1.Compare(str2) < 0; }
	friend bool __forceinline operator<(const CMStringT& str1, PCXSTR psz2) { return str1.Compare(psz2) < 0; }
	friend bool __forceinline operator<(PCXSTR psz1, const CMStringT& str2) { return str2.Compare(psz1) > 0; }

	friend bool __forceinline operator>(const CMStringT& str1, const CMStringT& str2) { return str1.Compare(str2) > 0; }
	friend bool __forceinline operator>(const CMStringT& str1, PCXSTR psz2) { return str1.Compare(psz2) > 0; }
	friend bool __forceinline operator>(PCXSTR psz1, const CMStringT& str2) { return str2.Compare(psz1) < 0; }

	friend bool __forceinline operator<=(const CMStringT& str1, const CMStringT& str2) { return str1.Compare(str2) <= 0; }
	friend bool __forceinline operator<=(const CMStringT& str1, PCXSTR psz2) { return str1.Compare(psz2) <= 0; }
	friend bool __forceinline operator<=(PCXSTR psz1, const CMStringT& str2) { return str2.Compare(psz1) >= 0; }

	friend bool __forceinline operator>=(const CMStringT& str1, const CMStringT& str2) { return str1.Compare(str2) >= 0; }
	friend bool __forceinline operator>=(const CMStringT& str1, PCXSTR psz2) { return str1.Compare(psz2) >= 0; }
	friend bool __forceinline operator>=(PCXSTR psz1, const CMStringT& str2) { return str2.Compare(psz1) <= 0; }

	friend bool __forceinline operator==(XCHAR ch1, const CMStringT& str2) { return (str2.GetLength() == 1) && (str2[0] == ch1); }
	friend bool __forceinline operator==(const CMStringT& str1, XCHAR ch2) { return (str1.GetLength() == 1) && (str1[0] == ch2); }

	friend bool __forceinline operator!=(XCHAR ch1, const CMStringT& str2) { return (str2.GetLength() != 1) || (str2[0] != ch1); }
	friend bool __forceinline operator!=(const CMStringT& str1, XCHAR ch2) { return (str1.GetLength() != 1) || (str1[0] != ch2); }
};

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, const CMStringT<BaseType, StringTraits>& str2);

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, typename CMStringT<BaseType, StringTraits>::PCXSTR psz2);

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(typename CMStringT<BaseType, StringTraits>::PCXSTR psz1, const CMStringT<BaseType, StringTraits>& str2);

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, wchar_t ch2);

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, char ch2);

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator + (wchar_t ch1, const CMStringT<BaseType, StringTraits>& str2);

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(char ch1, const CMStringT<BaseType, StringTraits>& str2);

typedef CMStringT< wchar_t, ChTraitsCRT< wchar_t > > CMStringW;
typedef CMStringT< char, ChTraitsCRT< char > > CMStringA;

#endif // M_STRING_H__
