/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#include <stdio.h>
#include <string.h>
#include <mbstring.h>
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
	explicit CMSimpleStringT()
	{
		CMStringData* pData = mirstr_getNil();
		Attach(pData);
	}

	CMSimpleStringT(const CMSimpleStringT& strSrc)
	{
		CMStringData* pSrcData = strSrc.GetData();
		CMStringData* pNewData = CloneData(pSrcData);
		Attach(pNewData);
	}

	CMSimpleStringT(PCXSTR pszSrc)
	{
		int nLength = StringLength(pszSrc);
		CMStringData* pData = mirstr_allocate(nLength, sizeof(XCHAR));
		if (pData != NULL) {
			Attach(pData);
			SetLength(nLength);
			CopyChars(m_pszData, nLength, pszSrc, nLength);
		}
	}
	CMSimpleStringT(const XCHAR* pchSrc, int nLength)
	{
		CMStringData* pData = mirstr_allocate(nLength, sizeof(XCHAR));
		if (pData != NULL) {
			Attach(pData);
			SetLength(nLength);
			CopyChars(m_pszData, nLength, pchSrc, nLength);
		}
	}
	~CMSimpleStringT()
	{
		CMStringData* pData = GetData();
		pData->Release();
	}

	operator CMSimpleStringT<BaseType>&()
	{
		return *(CMSimpleStringT<BaseType>*)this;
	}

	CMSimpleStringT& operator=(const CMSimpleStringT& strSrc)
	{
		CMStringData* pSrcData = strSrc.GetData();
		CMStringData* pOldData = GetData();
		if (pSrcData != pOldData) {
			if (pOldData->IsLocked())
				SetString(strSrc.GetString(), strSrc.GetLength());
			else {
				CMStringData* pNewData = CloneData(pSrcData);
				pOldData->Release();
				Attach(pNewData);
			}
		}

		return *this;
	}

	CMSimpleStringT& operator=(PCXSTR pszSrc)
	{
		SetString(pszSrc);
		return *this;
	}

	CMSimpleStringT& operator+=(const CMSimpleStringT& strSrc)
	{
		Append(strSrc);
		return *this;
	}

	CMSimpleStringT& operator+=(PCXSTR pszSrc)
	{
		Append(pszSrc);
		return *this;
	}
	CMSimpleStringT& operator+=(char ch)
	{
		AppendChar(XCHAR(ch));
		return *this;
	}
	CMSimpleStringT& operator+=(unsigned char ch)
	{
		AppendChar(XCHAR(ch));
		return *this;
	}
	CMSimpleStringT& operator+=(wchar_t ch)
	{
		AppendChar(XCHAR(ch));
		return *this;
	}

	XCHAR operator[](int iChar) const
	{
		return m_pszData[iChar];
	}

	operator PCXSTR() const
	{
		return m_pszData;
	}

	PCXSTR c_str() const
	{
		return m_pszData;
	}

	void Append(PCXSTR pszSrc)
	{
		Append(pszSrc, StringLength(pszSrc));
	}

	void Append(PCXSTR pszSrc, int nLength)
	{
		// See comment in SetString() about why we do this
		UINT_PTR nOffset = UINT_PTR(pszSrc - GetString());

		int nOldLength = GetLength();
		if (nOldLength < 0) {
			// protects from underflow
			nOldLength = 0;
		}

		//Make sure we don't read pass end of the terminating NULL
		int nSrcLength = StringLength(pszSrc);
		nLength = nLength > nSrcLength ? nSrcLength : nLength;

		int nNewLength = nOldLength + nLength;
		PXSTR pszBuffer = GetBuffer(nNewLength);
		if (nOffset <= UINT_PTR(nOldLength)) {
			pszSrc = pszBuffer + nOffset;
			// No need to call CopyCharsOverlapped, since the destination is
			// beyond the end of the original buffer
		}
		CopyChars(pszBuffer + nOldLength, nLength, pszSrc, nLength);
		ReleaseBufferSetLength(nNewLength);
	}
	void AppendChar(XCHAR ch)
	{
		UINT nOldLength = GetLength();
		int nNewLength = nOldLength + 1;
		PXSTR pszBuffer = GetBuffer(nNewLength);
		pszBuffer[nOldLength] = ch;
		ReleaseBufferSetLength(nNewLength);
	}
	void Append(const CMSimpleStringT& strSrc)
	{
		Append(strSrc.GetString(), strSrc.GetLength());
	}
	void Empty()
	{
		CMStringData* pOldData = GetData();
		if (pOldData->nDataLength == 0)
			return;

		if (pOldData->IsLocked()) {
			// Don't reallocate a locked buffer that's shrinking
			SetLength(0);
		}
		else {
			pOldData->Release();
			CMStringData* pNewData = mirstr_getNil();
			Attach(pNewData);
		}
	}
	void FreeExtra()
	{
		CMStringData* pOldData = GetData();
		int nLength = pOldData->nDataLength;
		if (pOldData->nAllocLength == nLength)
			return;

		if (!pOldData->IsLocked()) { // Don't reallocate a locked buffer that's shrinking
			CMStringData* pNewData = mirstr_allocate(nLength, sizeof(XCHAR));
			if (pNewData == NULL) {
				SetLength(nLength);
				return;
			}

			CopyChars(PXSTR(pNewData->data()), nLength, PCXSTR(pOldData->data()), nLength);

			pOldData->Release();
			Attach(pNewData);
			SetLength(nLength);
		}
	}

	int GetAllocLength() const
	{
		return GetData()->nAllocLength;
	}
	XCHAR GetAt(int iChar) const
	{
		return m_pszData[iChar];
	}
	PXSTR GetBuffer()
	{
		CMStringData* pData = GetData();
		if (pData->IsShared())
			Fork(pData->nDataLength);

		return m_pszData;
	}
	PXSTR GetBuffer(int nMinBufferLength)
	{
		return PrepareWrite(nMinBufferLength);
	}
	PXSTR GetBufferSetLength(int nLength)
	{
		PXSTR pszBuffer = GetBuffer(nLength);
		SetLength(nLength);

		return pszBuffer;
	}
	int GetLength() const
	{
		return GetData()->nDataLength;
	}
	PCXSTR GetString() const
	{
		return m_pszData;
	}
	PCXSTR GetTail() const
	{
		return m_pszData + GetData()->nDataLength;
	}
	bool IsEmpty() const
	{
		return GetLength() == 0;
	}
	PXSTR LockBuffer()
	{
		CMStringData* pData = GetData();
		if (pData->IsShared())
		{
			Fork(pData->nDataLength);
			pData = GetData();  // Do it again, because the fork might have changed it
		}
		pData->Lock();

		return m_pszData;
	}
	void UnlockBuffer()
	{
		CMStringData* pData = GetData();
		pData->Unlock();
	}
	void Preallocate(int nLength)
	{
		PrepareWrite(nLength);
	}
	void ReleaseBuffer(int nNewLength = -1)
	{
		if (nNewLength == -1) {
			int nAlloc = GetData()->nAllocLength;
			nNewLength = StringLengthN(m_pszData, nAlloc);
		}
		SetLength(nNewLength);
	}
	void ReleaseBufferSetLength(int nNewLength)
	{
		SetLength(nNewLength);
	}
	void Truncate(int nNewLength)
	{
		GetBuffer(nNewLength);
		ReleaseBufferSetLength(nNewLength);
	}
	void SetAt(int iChar, XCHAR ch)
	{
		int nLength = GetLength();
		PXSTR pszBuffer = GetBuffer();
		pszBuffer[iChar] = ch;
		ReleaseBufferSetLength(nLength);

	}
	void SetString(PCXSTR pszSrc)
	{
		SetString(pszSrc, StringLength(pszSrc));
	}
	void SetString(PCXSTR pszSrc, int nLength)
	{
		if (nLength == 0)
			Empty();
		else {
			UINT nOldLength = GetLength();
			UINT_PTR nOffset = pszSrc - GetString();

			PXSTR pszBuffer = GetBuffer(nLength);
			if (nOffset <= nOldLength)
				CopyCharsOverlapped(pszBuffer, GetAllocLength(), pszBuffer + nOffset, nLength);
			else
				CopyChars(pszBuffer, GetAllocLength(), pszSrc, nLength);

			ReleaseBufferSetLength(nLength);
		}
	}
public:
	friend CMSimpleStringT __stdcall operator+(const CMSimpleStringT& str1, const CMSimpleStringT& str2)
	{
		CMSimpleStringT s;
		Concatenate(s, str1, str1.GetLength(), str2, str2.GetLength());
		return s;
	}

	friend CMSimpleStringT __stdcall operator+(const CMSimpleStringT& str1, PCXSTR psz2)
	{
		CMSimpleStringT s;
		Concatenate(s, str1, str1.GetLength(), psz2, StringLength(psz2));
		return s;
	}

	friend CMSimpleStringT __stdcall operator+(PCXSTR psz1, const CMSimpleStringT& str2)
	{
		CMSimpleStringT s;
		Concatenate(s, psz1, StringLength(psz1), str2, str2.GetLength());
		return s;
	}

	static void __stdcall CopyChars(XCHAR* pchDest, const XCHAR* pchSrc, int nChars)
	{
#pragma warning (push)
#pragma warning(disable : 4996)
		memcpy(pchDest, pchSrc, nChars * sizeof(XCHAR));
#pragma warning (pop)
	}
	static void __stdcall CopyChars(XCHAR* pchDest, size_t nDestLen, const XCHAR* pchSrc, int nChars)
	{
		memcpy_s(pchDest, nDestLen * sizeof(XCHAR), pchSrc, nChars * sizeof(XCHAR));
	}

	static void __stdcall CopyCharsOverlapped(XCHAR* pchDest, const XCHAR* pchSrc, int nChars)
	{
#pragma warning (push)
#pragma warning(disable : 4996)
		memmove(pchDest, pchSrc, nChars * sizeof(XCHAR));
#pragma warning (pop)
	}
	static void __stdcall CopyCharsOverlapped(XCHAR* pchDest, size_t nDestLen, const XCHAR* pchSrc, int nChars)
	{
		memmove_s(pchDest, nDestLen * sizeof(XCHAR), pchSrc, nChars * sizeof(XCHAR));
	}
	static int __stdcall StringLength(const char* psz)
	{
		if (psz == NULL)
			return(0);

		return (int(strlen(psz)));
	}
	static int __stdcall StringLength(const wchar_t* psz)
	{
		if (psz == NULL)
			return 0;

		return int(wcslen(psz));
	}
	static int __stdcall StringLengthN(const char* psz, size_t sizeInXChar)
	{
		if (psz == NULL)
			return 0;

		return int(strnlen(psz, sizeInXChar));
	}
	static int __stdcall StringLengthN(const wchar_t* psz, size_t sizeInXChar)
	{
		if (psz == NULL)
			return 0;

		return int(wcsnlen(psz, sizeInXChar));
	}
protected:
	static void __stdcall Concatenate(CMSimpleStringT& strResult, PCXSTR psz1, int nLength1, PCXSTR psz2, int nLength2)
	{
		int nNewLength = nLength1+nLength2;
		PXSTR pszBuffer = strResult.GetBuffer(nNewLength);
		CopyChars(pszBuffer, nLength1, psz1, nLength1);
		CopyChars(pszBuffer + nLength1, nLength2, psz2, nLength2);
		strResult.ReleaseBufferSetLength(nNewLength);
	}
	// Implementation
private:
	void Attach(CMStringData* pData)
	{
		m_pszData = static_cast<PXSTR>(pData->data());
	}
	void Fork(int nLength)
	{
		CMStringData* pOldData = GetData();
		int nOldLength = pOldData->nDataLength;
		CMStringData* pNewData = mirstr_allocate(nLength, sizeof(XCHAR));
		if (pNewData != NULL) {
			int nCharsToCopy = ((nOldLength < nLength) ? nOldLength : nLength)+1;  // Copy '\0'
			CopyChars(PXSTR(pNewData->data()), nCharsToCopy, PCXSTR(pOldData->data()), nCharsToCopy);
			pNewData->nDataLength = nOldLength;
			pOldData->Release();
			Attach(pNewData);
		}
	}
	CMStringData* GetData() const
	{
		return (reinterpret_cast<CMStringData *>(m_pszData) - 1);
	}
	PXSTR PrepareWrite(int nLength)
	{
		CMStringData* pOldData = GetData();
		int nShared = 1 - pOldData->nRefs;  // nShared < 0 means true, >= 0 means false
		int nTooShort = pOldData->nAllocLength-nLength;  // nTooShort < 0 means true, >= 0 means false
		if ((nShared | nTooShort) < 0)  // If either sign bit is set (i.e. either is less than zero), we need to copy data
			PrepareWrite2(nLength);

		return m_pszData;
	}
	void PrepareWrite2(int nLength)
	{
		CMStringData* pOldData = GetData();
		if (pOldData->nDataLength > nLength)
			nLength = pOldData->nDataLength;

		if (pOldData->IsShared())
		{
			Fork(nLength);
		}
		else if (pOldData->nAllocLength < nLength) {
			// Grow exponentially, until we hit 1K.
			int nNewLength = pOldData->nAllocLength;
			if (nNewLength > 1024)
				nNewLength += 1024;
			else
				nNewLength *= 2;

			if (nNewLength < nLength)
				nNewLength = nLength;

			Reallocate(nNewLength);
		}
	}
	void Reallocate(int nLength)
	{
		CMStringData* pOldData = GetData();
		if (pOldData->nAllocLength >= nLength || nLength <= 0)
			return;

		CMStringData* pNewData = mirstr_realloc(pOldData, nLength, sizeof(XCHAR));
		if (pNewData != NULL)
			Attach(pNewData);
	}

	void SetLength(int nLength)
	{
		GetData()->nDataLength = nLength;
		m_pszData[nLength] = 0;
	}

	static CMStringData* __stdcall CloneData(CMStringData* pData)
	{
		CMStringData* pNewData = NULL;

		if (!pData->IsLocked()) {
			pNewData = pData;
			pNewData->AddRef();
		}

		return pNewData;
	}

public :
	//	typedef CStrBufT<BaseType> CStrBuf;
private:
	PXSTR m_pszData;
};


template< typename _CharType = char >
class ChTraitsCRT : public ChTraitsBase< _CharType >
{
public:
	static char* __stdcall CharNext(const char* p)
	{
		return reinterpret_cast< char* >(_mbsinc(reinterpret_cast< const unsigned char* >(p)));
	}

	static int __stdcall IsDigit(char ch)
	{
		return _ismbcdigit(ch);
	}

	static int __stdcall IsSpace(char ch)
	{
		return _ismbcspace(ch);
	}

	static int __stdcall StringCompare(LPCSTR pszA, LPCSTR pszB)
	{
		return _mbscmp(reinterpret_cast< const unsigned char* >(pszA), reinterpret_cast< const unsigned char* >(pszB));
	}

	static int __stdcall StringCompareIgnore(LPCSTR pszA, LPCSTR pszB)
	{
		return _mbsicmp(reinterpret_cast< const unsigned char* >(pszA), reinterpret_cast< const unsigned char* >(pszB));
	}

	static int __stdcall StringCollate(LPCSTR pszA, LPCSTR pszB)
	{
		return _mbscoll(reinterpret_cast< const unsigned char* >(pszA), reinterpret_cast< const unsigned char* >(pszB));
	}

	static int __stdcall StringCollateIgnore(LPCSTR pszA, LPCSTR pszB)
	{
		return _mbsicoll(reinterpret_cast< const unsigned char* >(pszA), reinterpret_cast< const unsigned char* >(pszB));
	}

	static LPCSTR __stdcall StringFindString(LPCSTR pszBlock, LPCSTR pszMatch)
	{
		return reinterpret_cast< LPCSTR >(_mbsstr(reinterpret_cast< const unsigned char* >(pszBlock),
			reinterpret_cast< const unsigned char* >(pszMatch)));
	}

	static LPSTR __stdcall StringFindString(LPSTR pszBlock, LPCSTR pszMatch)
	{
		return const_cast< LPSTR >(StringFindString(const_cast< LPCSTR >(pszBlock), pszMatch));
	}

	static LPCSTR __stdcall StringFindChar(LPCSTR pszBlock, char chMatch)
	{
		return reinterpret_cast< LPCSTR >(_mbschr(reinterpret_cast< const unsigned char* >(pszBlock), (unsigned char)chMatch));
	}

	static LPCSTR __stdcall StringFindCharRev(LPCSTR psz, char ch)
	{
		return reinterpret_cast< LPCSTR >(_mbsrchr(reinterpret_cast< const unsigned char* >(psz), (unsigned char)ch));
	}

	static LPCSTR __stdcall StringScanSet(LPCSTR pszBlock, LPCSTR pszMatch)
	{
		return reinterpret_cast< LPCSTR >(_mbspbrk(reinterpret_cast< const unsigned char* >(pszBlock),
			reinterpret_cast< const unsigned char* >(pszMatch)));
	}

	static int __stdcall StringSpanIncluding(LPCSTR pszBlock, LPCSTR pszSet)
	{
		return (int)_mbsspn(reinterpret_cast< const unsigned char* >(pszBlock), reinterpret_cast< const unsigned char* >(pszSet));
	}

	static int __stdcall StringSpanExcluding(LPCSTR pszBlock, LPCSTR pszSet)
	{
		return (int)_mbscspn(reinterpret_cast< const unsigned char* >(pszBlock), reinterpret_cast< const unsigned char* >(pszSet));
	}

	static LPSTR __stdcall StringUppercase(LPSTR psz)
	{
		CharUpperBuffA(psz, (DWORD)strlen(psz));
		return psz;
	}

	static LPSTR __stdcall StringLowercase(LPSTR psz)
	{
		CharLowerBuffA(psz, (DWORD)strlen(psz));
		return psz;
	}

	static LPSTR __stdcall StringUppercase(LPSTR psz, size_t size)
	{
		CharUpperBuffA(psz, (DWORD)size);
		return psz;
	}

	static LPSTR __stdcall StringLowercase(LPSTR psz, size_t size)
	{
		CharLowerBuffA(psz, (DWORD)size);
		return psz;
	}

	static LPSTR __stdcall StringReverse(LPSTR psz)
	{
		return reinterpret_cast< LPSTR >(_mbsrev(reinterpret_cast< unsigned char* >(psz)));
	}

	static int __stdcall GetFormattedLength(LPCSTR pszFormat, va_list args);

	static int __stdcall Format(LPSTR pszBuffer, size_t nlength, LPCSTR pszFormat, va_list args);

	static int __stdcall GetBaseTypeLength(LPCSTR pszSrc)
	{
		// Returns required buffer length in XCHARs
		return int(strlen(pszSrc));
	}

	static int __stdcall GetBaseTypeLength(LPCSTR pszSrc, int nLength)
	{
		(void)pszSrc;
		// Returns required buffer length in XCHARs
		return nLength;
	}

	static int __stdcall GetBaseTypeLength(LPCWSTR pszSource)
	{
		// Returns required buffer length in XCHARs
		return ::WideCharToMultiByte(Langpack_GetDefaultCodePage(), 0, pszSource, -1, NULL, 0, NULL, NULL)-1;
	}

	static int __stdcall GetBaseTypeLength(LPCWSTR pszSource, int nLength)
	{
		// Returns required buffer length in XCHARs
		return ::WideCharToMultiByte(Langpack_GetDefaultCodePage(), 0, pszSource, nLength, NULL, 0, NULL, NULL);
	}

	static void __stdcall ConvertToBaseType(LPSTR pszDest, int nDestLength, LPCSTR pszSrc, int nSrcLength = -1)
	{
		if (nSrcLength == -1) { nSrcLength=1 + GetBaseTypeLength(pszSrc); }
		// nLen is in XCHARs
		memcpy_s(pszDest, nDestLength*sizeof(char), pszSrc, nSrcLength*sizeof(char));
	}

	static void __stdcall ConvertToBaseType(LPSTR pszDest, int nDestLength, LPCWSTR pszSrc, int nSrcLength = -1)
	{
		// nLen is in XCHARs
		::WideCharToMultiByte(Langpack_GetDefaultCodePage(), 0, pszSrc, nSrcLength, pszDest, nDestLength, NULL, NULL);
	}

	static void ConvertToOem(_CharType* pstrString)
	{
		BOOL fSuccess=::CharToOemA(pstrString, pstrString);
	}

	static void ConvertToAnsi(_CharType* pstrString)
	{
		BOOL fSuccess=::OemToCharA(pstrString, pstrString);
	}

	static void ConvertToOem(_CharType* pstrString, size_t size)
	{
		if (size>UINT_MAX)
		{
			return;
		}
		DWORD dwSize=static_cast<DWORD>(size);
		BOOL fSuccess=::CharToOemBuffA(pstrString, pstrString, dwSize);
	}

	static void ConvertToAnsi(_CharType* pstrString, size_t size)
	{
		if (size>UINT_MAX)
			return;

		DWORD dwSize=static_cast<DWORD>(size);
		BOOL fSuccess=::OemToCharBuffA(pstrString, pstrString, dwSize);
	}

	static void __stdcall FloodCharacters(char ch, int nLength, char* pch)
	{
		// nLength is in XCHARs
		memset(pch, ch, nLength);
	}

	static BSTR __stdcall AllocSysString(const char* pchData, int nDataLength)
	{
		int nLen = ::MultiByteToWideChar(Langpack_GetDefaultCodePage(), 0, pchData, nDataLength, NULL, NULL);
		BSTR bstr = ::SysAllocStringLen(NULL, nLen);
		if (bstr != NULL)
			::MultiByteToWideChar(Langpack_GetDefaultCodePage(), 0, pchData, nDataLength, bstr, nLen);

		return bstr;
	}

	static BOOL __stdcall ReAllocSysString(const char* pchData, BSTR* pbstr, int nDataLength)
	{
		int nLen = ::MultiByteToWideChar(Langpack_GetDefaultCodePage(), 0, pchData, nDataLength, NULL, NULL);
		BOOL bSuccess = ::SysReAllocStringLen(pbstr, NULL, nLen);
		if (bSuccess)
			::MultiByteToWideChar(Langpack_GetDefaultCodePage(), 0, pchData, nDataLength, *pbstr, nLen);

		return bSuccess;
	}

	static int __stdcall SafeStringLen(LPCSTR psz)
	{
		// returns length in bytes
		return (psz != NULL) ? int(strlen(psz)) : 0;
	}

	static int __stdcall SafeStringLen(LPCWSTR psz)
	{
		// returns length in wchar_ts
		return (psz != NULL) ? int(wcslen(psz)) : 0;
	}

	static int __stdcall GetCharLen(const wchar_t* pch)
	{
		// returns char length
		return 1;
	}

	static int __stdcall GetCharLen(const char* pch)
	{
		// returns char length
		return int(_mbclen(reinterpret_cast< const unsigned char* >(pch)));
	}

	static DWORD __stdcall GetEnvironmentVariable(LPCSTR pszVar, LPSTR pszBuffer, DWORD dwSize)
	{
		return ::GetEnvironmentVariableA(pszVar, pszBuffer, dwSize);
	}
};

// specialization for wchar_t
template<>
class ChTraitsCRT< wchar_t > : public ChTraitsBase< wchar_t >
{
	static DWORD __stdcall _GetEnvironmentVariableW(LPCWSTR pszName, LPWSTR pszBuffer, DWORD nSize)
	{
		return ::GetEnvironmentVariableW(pszName, pszBuffer, nSize);
	}

public:
	static LPWSTR __stdcall CharNext(LPCWSTR psz)
	{
		return const_cast< LPWSTR >(psz+1);
	}

	static int __stdcall IsDigit(wchar_t ch)
	{
		return iswdigit(static_cast<unsigned short>(ch));
	}

	static int __stdcall IsSpace(wchar_t ch)
	{
		return iswspace(static_cast<unsigned short>(ch));
	}

	static int __stdcall StringCompare(LPCWSTR pszA, LPCWSTR pszB)
	{
		return wcscmp(pszA, pszB);
	}

	static int __stdcall StringCompareIgnore(LPCWSTR pszA, LPCWSTR pszB)
	{
		return _wcsicmp(pszA, pszB);
	}

	static int __stdcall StringCollate(LPCWSTR pszA, LPCWSTR pszB)
	{
		return wcscoll(pszA, pszB);
	}

	static int __stdcall StringCollateIgnore(LPCWSTR pszA, LPCWSTR pszB)
	{
		return _wcsicoll(pszA, pszB);
	}

	static LPCWSTR __stdcall StringFindString(LPCWSTR pszBlock, LPCWSTR pszMatch)
	{
		return wcsstr(pszBlock, pszMatch);
	}

	static LPWSTR __stdcall StringFindString(LPWSTR pszBlock, LPCWSTR pszMatch)
	{
		return const_cast< LPWSTR >(StringFindString(const_cast< LPCWSTR >(pszBlock), pszMatch));
	}

	static LPCWSTR __stdcall StringFindChar(LPCWSTR pszBlock, wchar_t chMatch)
	{
		return wcschr(pszBlock, chMatch);
	}

	static LPCWSTR __stdcall StringFindCharRev(LPCWSTR psz, wchar_t ch)
	{
		return wcsrchr(psz, ch);
	}

	static LPCWSTR __stdcall StringScanSet(LPCWSTR pszBlock, LPCWSTR pszMatch)
	{
		return wcspbrk(pszBlock, pszMatch);
	}

	static int __stdcall StringSpanIncluding(LPCWSTR pszBlock, LPCWSTR pszSet)
	{
		return (int)wcsspn(pszBlock, pszSet);
	}

	static int __stdcall StringSpanExcluding(LPCWSTR pszBlock, LPCWSTR pszSet)
	{
		return (int)wcscspn(pszBlock, pszSet);
	}

	static LPWSTR __stdcall StringUppercase(LPWSTR psz)
	{
		CharUpperBuffW(psz, (DWORD)wcslen(psz));
		return psz;
	}

	static LPWSTR __stdcall StringLowercase(LPWSTR psz)
	{
		CharLowerBuffW(psz, (DWORD)wcslen(psz));
		return psz;
	}

	static LPWSTR __stdcall StringUppercase(LPWSTR psz, size_t len)
	{
		CharUpperBuffW(psz, (DWORD)len);
		return psz;
	}

	static LPWSTR __stdcall StringLowercase(LPWSTR psz, size_t len)
	{
		CharLowerBuffW(psz, (DWORD)len);
		return psz;
	}

	static LPWSTR __stdcall StringReverse(LPWSTR psz)
	{
		return _wcsrev(psz);
	}

	static int __stdcall GetFormattedLength(LPCWSTR pszFormat, va_list args);

	static int __stdcall Format(LPWSTR pszBuffer, LPCWSTR pszFormat, va_list args)
	{
#pragma warning (push)
#pragma warning(disable : 4996)
		return vswprintf(pszBuffer, pszFormat, args); //!!!!!!!!!
#pragma warning (pop)
	}

	static int __stdcall Format(LPWSTR pszBuffer, size_t nLength, LPCWSTR pszFormat, va_list args);

	static int __stdcall GetBaseTypeLength(LPCSTR pszSrc)
	{
		// Returns required buffer size in wchar_ts
		return ::MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0)-1;
	}

	static int __stdcall GetBaseTypeLength(LPCSTR pszSrc, int nLength)
	{
		// Returns required buffer size in wchar_ts
		return ::MultiByteToWideChar(CP_ACP, 0, pszSrc, nLength, NULL, 0);
	}

	static int __stdcall GetBaseTypeLength(LPCWSTR pszSrc)
	{
		// Returns required buffer size in wchar_ts
		return (int)wcslen(pszSrc);
	}

	static int __stdcall GetBaseTypeLength(LPCWSTR pszSrc, int nLength)
	{
		(void)pszSrc;
		// Returns required buffer size in wchar_ts
		return nLength;
	}

	static void __stdcall ConvertToBaseType(LPWSTR pszDest, int nDestLength, LPCSTR pszSrc, int nSrcLength = -1)
	{
		// nLen is in wchar_ts
		::MultiByteToWideChar(CP_ACP, 0, pszSrc, nSrcLength, pszDest, nDestLength);
	}

	static void __stdcall ConvertToBaseType(LPWSTR pszDest, int nDestLength, LPCWSTR pszSrc, int nSrcLength = -1)
	{
		if (nSrcLength == -1) { nSrcLength=1 + GetBaseTypeLength(pszSrc); }
		// nLen is in wchar_ts
		#if _MSC_VER >= 1400
			wmemcpy_s(pszDest, nDestLength, pszSrc, nSrcLength);
		#else
			wmemcpy(pszDest, pszSrc, nDestLength);
		#endif
	}

	static void __stdcall FloodCharacters(wchar_t ch, int nLength, LPWSTR psz)
	{
		// nLength is in XCHARs
		for (int i = 0; i < nLength; i++)
		{
			psz[i] = ch;
		}
	}

	static BSTR __stdcall AllocSysString(const wchar_t* pchData, int nDataLength)
	{
		return ::SysAllocStringLen(pchData, nDataLength);
	}

	static BOOL __stdcall ReAllocSysString(const wchar_t* pchData, BSTR* pbstr, int nDataLength)
	{
		return ::SysReAllocStringLen(pbstr, pchData, nDataLength);
	}

	static int __stdcall SafeStringLen(LPCSTR psz)
	{
		// returns length in bytes
		return (psz != NULL) ? (int)strlen(psz) : 0;
	}

	static int __stdcall SafeStringLen(LPCWSTR psz)
	{
		// returns length in wchar_ts
		return (psz != NULL) ? (int)wcslen(psz) : 0;
	}

	static int __stdcall GetCharLen(const wchar_t* pch)
	{
		(void)pch;
		// returns char length
		return 1;
	}

	static int __stdcall GetCharLen(const char* pch)
	{
		// returns char length
		return (int)(_mbclen(reinterpret_cast< const unsigned char* >(pch)));
	}

	static DWORD __stdcall GetEnvironmentVariable(LPCWSTR pszVar, LPWSTR pszBuffer, DWORD dwSize)
	{
		return _GetEnvironmentVariableW(pszVar, pszBuffer, dwSize);
	}

	static void __stdcall ConvertToOem(LPWSTR /*psz*/)
	{
	}

	static void __stdcall ConvertToAnsi(LPWSTR /*psz*/)
	{
	}

	static void __stdcall ConvertToOem(LPWSTR /*psz*/, size_t)
	{
	}

	static void __stdcall ConvertToAnsi(LPWSTR /*psz*/, size_t)
	{
	}
};

template< typename BaseType, class StringTraits >
class CMStringT : public CMSimpleStringT< BaseType >
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
	CMStringT() : CThisSimpleString()
	  {
	  }

	static void __stdcall Construct(CMStringT* pString)
	{
		new(pString) CMStringT;
	}

	// Copy constructor
	CMStringT(const CMStringT& strSrc) :
		CThisSimpleString(strSrc)
	{
	}

	CMStringT(const XCHAR* pszSrc) :
		CThisSimpleString()
	{
		*this = pszSrc;
	}
	
	CMStringT(CMStringDataFormat, const XCHAR* pszFormat, ...) :
		CThisSimpleString()
	{
		va_list args;
		va_start(args, pszFormat);
		FormatV(pszFormat, args);
	}
	
	CMStringT(const YCHAR* pszSrc) :
		CThisSimpleString()
	{
		*this = pszSrc;
	}


	CMStringT(const unsigned char* pszSrc) :
		CThisSimpleString()
	{
		*this = reinterpret_cast<const char*>(pszSrc);
	}

	CMStringT(char ch, int nLength = 1) :
		CThisSimpleString()
	{
		if (nLength > 0) {
			PXSTR pszBuffer = this->GetBuffer(nLength);
			StringTraits::FloodCharacters(XCHAR(ch), nLength, pszBuffer);
			this->ReleaseBufferSetLength(nLength);
		}
	}

	CMStringT(wchar_t ch, int nLength = 1) :
		CThisSimpleString()
	{
		if (nLength > 0) {
			//Convert ch to the BaseType
			wchar_t pszCh[2] = { ch, 0 };
			int nBaseTypeCharLen = 1;

			if (ch != L'\0')
				nBaseTypeCharLen = StringTraits::GetBaseTypeLength(pszCh);

			XCHAR *buffBaseTypeChar = new XCHAR[nBaseTypeCharLen + 1];
			StringTraits::ConvertToBaseType(buffBaseTypeChar, nBaseTypeCharLen + 1, pszCh, 1);
			//allocate enough characters in String and flood (replicate) with the (converted character)*nLength
			PXSTR pszBuffer = this->GetBuffer(nLength*nBaseTypeCharLen);
			if (nBaseTypeCharLen == 1)  //Optimization for a common case - wide char translates to 1 ansi/wide char.
				StringTraits::FloodCharacters(buffBaseTypeChar[0], nLength, pszBuffer);
			else {
				XCHAR* p = pszBuffer;
				for (int i = 0; i < nLength; i++) {
					for (int j = 0; j < nBaseTypeCharLen; ++j) {
						*p = buffBaseTypeChar[j];
						++p;
					}
				}
			}
			this->ReleaseBufferSetLength(nLength*nBaseTypeCharLen);
			delete[] buffBaseTypeChar;
		}
	}

	CMStringT(const XCHAR* pch, int nLength) :
		CThisSimpleString(pch, nLength)
	{}

	CMStringT(const YCHAR* pch, int nLength) :
		CThisSimpleString()
	{
		if (nLength > 0) {
			int nDestLength = StringTraits::GetBaseTypeLength(pch, nLength);
			PXSTR pszBuffer = this->GetBuffer(nDestLength);
			StringTraits::ConvertToBaseType(pszBuffer, nDestLength, pch, nLength);
			this->ReleaseBufferSetLength(nDestLength);
		}
	}

	// Destructor
	~CMStringT()
	{}

	// Assignment operators
	CMStringT& operator=(const CMStringT& strSrc)
	{
		CThisSimpleString::operator=(strSrc);
		return *this;
	}

	CMStringT& operator=(PCXSTR pszSrc)
	{
		CThisSimpleString::operator=(pszSrc);
		return *this;
	}

	CMStringT& operator=(PCYSTR pszSrc)
	{
		// nDestLength is in XCHARs
		int nDestLength = (pszSrc != NULL) ? StringTraits::GetBaseTypeLength(pszSrc) : 0;
		if (nDestLength > 0) {
			PXSTR pszBuffer = this->GetBuffer(nDestLength);
			StringTraits::ConvertToBaseType(pszBuffer, nDestLength, pszSrc);
			this->ReleaseBufferSetLength(nDestLength);
		}
		else this->Empty();

		return *this;
	}

	CMStringT& operator=(const unsigned char* pszSrc)
	{
		return operator=(reinterpret_cast<const char*>(pszSrc));
	}

	CMStringT& operator=(char ch)
	{
		char ach[2] = { ch, 0 };
		return operator=(ach);
	}

	CMStringT& operator=(wchar_t ch)
	{
		wchar_t ach[2] = { ch, 0 };
		return operator=(ach);
	}

	//	  CMStringT& operator=(const VARIANT& var);

	CMStringT& operator+=(const CMStringT& str)
	{
		CThisSimpleString::operator+=(str);
		return *this;
	}

	CMStringT& operator+=(const CThisSimpleString& str)
	{
		CThisSimpleString::operator+=(str);
		return *this;
	}

	CMStringT& operator+=(PCXSTR pszSrc)
	{
		CThisSimpleString::operator+=(pszSrc);
		return *this;
	}

	CMStringT& operator+=(PCYSTR pszSrc)
	{
		CMStringT str(pszSrc);
		return operator+=(str);
	}

	CMStringT& operator+=(char ch)
	{
		CThisSimpleString::operator+=(ch);
		return *this;
	}

	CMStringT& operator+=(unsigned char ch)
	{
		CThisSimpleString::operator+=(ch);
		return *this;
	}

	CMStringT& operator+=(wchar_t ch)
	{
		CThisSimpleString::operator+=(ch);
		return *this;
	}

	// Comparison

	int Compare(PCXSTR psz) const
	{
		return StringTraits::StringCompare(this->GetString(), psz);
	}

	int CompareNoCase(PCXSTR psz) const
	{
		return StringTraits::StringCompareIgnore(this->GetString(), psz);
	}

	int Collate(PCXSTR psz) const
	{
		return StringTraits::StringCollate(this->GetString(), psz);
	}

	int CollateNoCase(PCXSTR psz) const
	{
		return StringTraits::StringCollateIgnore(this->GetString(), psz);
	}

	// Advanced manipulation

	// Delete 'nCount' characters, starting at index 'iIndex'
	int Delete(int iIndex, int nCount = 1)
	{
		if (iIndex < 0)
			iIndex = 0;

		if (nCount < 0)
			nCount = 0;

		int nLength = this->GetLength();
		if (nCount + iIndex > nLength)
			nCount = nLength-iIndex;

		if (nCount > 0) {
			int nNewLength = nLength-nCount;
			int nXCHARsToCopy = nLength-(iIndex+nCount)+1;
			PXSTR pszBuffer = this->GetBuffer();
#if _MSC_VER >= 1400
			memmove_s(pszBuffer+iIndex, nXCHARsToCopy*sizeof(XCHAR), pszBuffer+iIndex+nCount, nXCHARsToCopy*sizeof(XCHAR));
#else
			memmove(pszBuffer+iIndex, pszBuffer+iIndex+nCount, nXCHARsToCopy*sizeof(XCHAR));
#endif
			this->ReleaseBufferSetLength(nNewLength);
		}

		return this->GetLength();
	}

	// Insert character 'ch' before index 'iIndex'
	int Insert(int iIndex, XCHAR ch)
	{
		if (iIndex < 0)
			iIndex = 0;

		if (iIndex > this->GetLength())
			iIndex = this->GetLength();

		int nNewLength = this->GetLength()+1;

		PXSTR pszBuffer = this->GetBuffer(nNewLength);

		// move existing bytes down
#if _MSC_VER >= 1400
		memmove_s(pszBuffer+iIndex+1, (nNewLength-iIndex)*sizeof(XCHAR), pszBuffer+iIndex, (nNewLength-iIndex)*sizeof(XCHAR));
#else
		memmove(pszBuffer+iIndex+1, pszBuffer+iIndex, (nNewLength-iIndex)*sizeof(XCHAR));
#endif
		pszBuffer[iIndex] = ch;

		this->ReleaseBufferSetLength(nNewLength);
		return nNewLength;
	}

	// Insert string 'psz' before index 'iIndex'
	int Insert(int iIndex, PCXSTR psz)
	{
		if (iIndex < 0)
			iIndex = 0;

		if (iIndex > this->GetLength())
			iIndex = this->GetLength();

		// nInsertLength and nNewLength are in XCHARs
		int nInsertLength = StringTraits::SafeStringLen(psz);
		int nNewLength = this->GetLength();
		if (nInsertLength > 0) {
			nNewLength += nInsertLength;

			PXSTR pszBuffer = this->GetBuffer(nNewLength);
			// move existing bytes down
#if _MSC_VER >= 1400
			memmove_s(pszBuffer+iIndex+nInsertLength, (nNewLength-iIndex-nInsertLength+1)*sizeof(XCHAR), pszBuffer+iIndex, (nNewLength-iIndex-nInsertLength+1)*sizeof(XCHAR));
			memcpy_s(pszBuffer+iIndex, nInsertLength*sizeof(XCHAR), psz, nInsertLength*sizeof(XCHAR));
#else
			memmove(pszBuffer+iIndex+nInsertLength, pszBuffer+iIndex, (nNewLength-iIndex-nInsertLength+1)*sizeof(XCHAR));
			memcpy(pszBuffer+iIndex, psz, nInsertLength*sizeof(XCHAR));
#endif
			this->ReleaseBufferSetLength(nNewLength);
		}

		return nNewLength;
	}

	// Replace all occurrences of character 'chOld' with character 'chNew'
	int Replace(XCHAR chOld, XCHAR chNew)
	{
		int nCount = 0;

		// short-circuit the nop case
		if (chOld != chNew) {
			// otherwise modify each character that matches in the string
			bool bCopied = false;
			PXSTR pszBuffer = const_cast< PXSTR >(this->GetString());  // We don't actually write to pszBuffer until we've called GetBuffer().

			int nLength = this->GetLength();
			int iChar = 0;
			while (iChar < nLength) {
				// replace instances of the specified character only
				if (pszBuffer[iChar] == chOld) {
					if (!bCopied) {
						bCopied = true;
						pszBuffer = this->GetBuffer(nLength);
					}
					pszBuffer[iChar] = chNew;
					nCount++;
				}
				iChar = int(StringTraits::CharNext(pszBuffer + iChar) - pszBuffer);
			}

			if (bCopied)
				this->ReleaseBufferSetLength(nLength);
		}

		return nCount;
	}

	// Replace all occurrences of string 'pszOld' with string 'pszNew'
	int Replace(PCXSTR pszOld, PCXSTR pszNew)
	{
		// can't have empty or NULL lpszOld

		// nSourceLen is in XCHARs
		int nSourceLen = StringTraits::SafeStringLen(pszOld);
		if (nSourceLen == 0)
			return 0;
		// nReplacementLen is in XCHARs
		int nReplacementLen = StringTraits::SafeStringLen(pszNew);

		// loop once to figure out the size of the result string
		int nCount = 0;
		{
			PCXSTR pszStart = this->GetString();
			PCXSTR pszEnd = pszStart + this->GetLength();
			while (pszStart < pszEnd) {
				PCXSTR pszTarget;
				while ((pszTarget = StringTraits::StringFindString(pszStart, pszOld)) != NULL) {
					nCount++;
					pszStart = pszTarget + nSourceLen;
				}
				pszStart += StringTraits::SafeStringLen(pszStart) + 1;
			}
		}

		// if any changes were made, make them
		if (nCount > 0) {
			// if the buffer is too small, just
			// allocate a new buffer (slow but sure)
			int nOldLength = this->GetLength();
			int nNewLength = nOldLength+(nReplacementLen-nSourceLen)*nCount;

			PXSTR pszBuffer = this->GetBuffer(__max(nNewLength, nOldLength));

			PXSTR pszStart = pszBuffer;
			PXSTR pszEnd = pszStart+nOldLength;

			// loop again to actually do the work
			while (pszStart < pszEnd) {
				PXSTR pszTarget;
				while ((pszTarget = StringTraits::StringFindString(pszStart, pszOld)) != NULL) {
					int nBalance = nOldLength - int(pszTarget - pszBuffer + nSourceLen);
					memmove_s(pszTarget + nReplacementLen, nBalance*sizeof(XCHAR),
						pszTarget + nSourceLen, nBalance*sizeof(XCHAR));
					memcpy_s(pszTarget, nReplacementLen*sizeof(XCHAR),
						pszNew, nReplacementLen*sizeof(XCHAR));
					pszStart = pszTarget + nReplacementLen;
					pszTarget[nReplacementLen + nBalance] = 0;
					nOldLength += (nReplacementLen - nSourceLen);
				}
				pszStart += StringTraits::SafeStringLen(pszStart) + 1;
			}
			this->ReleaseBufferSetLength(nNewLength);
		}

		return nCount;
	}

	// Remove all occurrences of character 'chRemove'
	int Remove(XCHAR chRemove)
	{
		int nLength = this->GetLength();
		PXSTR pszBuffer = this->GetBuffer(nLength);

		PXSTR pszSource = pszBuffer;
		PXSTR pszDest = pszBuffer;
		PXSTR pszEnd = pszBuffer+nLength;

		while (pszSource < pszEnd) {
			PXSTR pszNewSource = StringTraits::CharNext(pszSource);
			if (*pszSource != chRemove) {
				// Copy the source to the destination.  Remember to copy all bytes of an MBCS character
				// Copy the source to the destination.  Remember to copy all bytes of an MBCS character
				size_t NewSourceGap = (pszNewSource - pszSource);
				PXSTR pszNewDest = pszDest + NewSourceGap;
				for (size_t i = 0; pszDest != pszNewDest && i < NewSourceGap; i++) {
					*pszDest = *pszSource;
					pszSource++;
					pszDest++;
				}
			}
			pszSource = pszNewSource;
		}
		*pszDest = 0;
		int nCount = int(pszSource - pszDest);
		this->ReleaseBufferSetLength(nLength - nCount);

		return nCount;
	}

	CMStringT Tokenize(PCXSTR pszTokens, int& iStart) const
	{
		if ((pszTokens == NULL) || (*pszTokens == (XCHAR)0)) {
			if (iStart < this->GetLength())
				return CMStringT(this->GetString() + iStart);
		}
		else {
			PCXSTR pszPlace = this->GetString() + iStart;
			PCXSTR pszEnd = this->GetString() + this->GetLength();
			if (pszPlace < pszEnd) {
				int nIncluding = StringTraits::StringSpanIncluding(pszPlace, pszTokens);

				if ((pszPlace + nIncluding) < pszEnd) {
					pszPlace += nIncluding;
					int nExcluding = StringTraits::StringSpanExcluding(pszPlace, pszTokens);

					int iFrom = iStart + nIncluding;
					int nUntil = nExcluding;
					iStart = iFrom + nUntil + 1;

					return Mid(iFrom, nUntil);
				}
			}
		}

		// return empty string, done tokenizing
		iStart = -1;

		return CMStringT();
	}

	// find routines

	// Find the first occurrence of character 'ch', starting at index 'iStart'
	int Find(XCHAR ch, int iStart = 0) const
	{
		// nLength is in XCHARs
		int nLength = this->GetLength();
		if (iStart < 0 || iStart >= nLength)
			return -1;

		// find first single character
		PCXSTR psz = StringTraits::StringFindChar(this->GetString() + iStart, ch);

		// return -1 if not found and index otherwise
		return (psz == NULL) ? -1 : int(psz - this->GetString());
	}

	// look for a specific sub-string

	// Find the first occurrence of string 'pszSub', starting at index 'iStart'
	int Find(PCXSTR pszSub, int iStart = 0) const
	{
		// iStart is in XCHARs
		if (pszSub == NULL)
			return -1;

		// nLength is in XCHARs
		int nLength = this->GetLength();
		if (iStart < 0 || iStart > nLength)
			return -1;

		// find first matching substring
		PCXSTR psz = StringTraits::StringFindString(this->GetString() + iStart, pszSub);

		// return -1 for not found, distance from beginning otherwise
		return (psz == NULL) ? -1 : int(psz - this->GetString());
	}

	// Find the first occurrence of any of the characters in string 'pszCharSet'
	int FindOneOf(PCXSTR pszCharSet) const
	{
		PCXSTR psz = StringTraits::StringScanSet(this->GetString(), pszCharSet);
		return (psz == NULL) ? -1 : int(psz - this->GetString());
	}

	// Find the last occurrence of character 'ch'
	int ReverseFind(XCHAR ch) const
	{
		// find last single character
		PCXSTR psz = StringTraits::StringFindCharRev(this->GetString(), ch);

		// return -1 if not found, distance from beginning otherwise
		return (psz == NULL) ? -1 : int(psz - this->GetString());
	}

	// manipulation

	// Convert the string to uppercase
	CMStringT& MakeUpper()
	{
		int nLength = this->GetLength();
		PXSTR pszBuffer = this->GetBuffer(nLength);
		StringTraits::StringUppercase(pszBuffer, nLength + 1);
		this->ReleaseBufferSetLength(nLength);

		return *this;
	}

	// Convert the string to lowercase
	CMStringT& MakeLower()
	{
		int nLength = this->GetLength();
		PXSTR pszBuffer = this->GetBuffer(nLength);
		StringTraits::StringLowercase(pszBuffer, nLength + 1);
		this->ReleaseBufferSetLength(nLength);

		return *this;
	}

	// Reverse the string
	CMStringT& MakeReverse()
	{
		int nLength = this->GetLength();
		PXSTR pszBuffer = this->GetBuffer(nLength);
		StringTraits::StringReverse(pszBuffer);
		this->ReleaseBufferSetLength(nLength);

		return *this;
	}

	// trimming

	// Remove all trailing whitespace
	CMStringT& TrimRight()
	{
		// find beginning of trailing spaces by starting
		// at beginning (DBCS aware)

		PCXSTR psz = this->GetString();
		PCXSTR pszLast = NULL;

		while (*psz != 0) {
			if (StringTraits::IsSpace(*psz)) {
				if (pszLast == NULL)
					pszLast = psz;
			}
			else pszLast = NULL;

			psz = StringTraits::CharNext(psz);
		}

		if (pszLast != NULL) {
			// truncate at trailing space start
			int iLast = int(pszLast - this->GetString());

			this->Truncate(iLast);
		}

		return *this;
	}

	// Remove all leading whitespace
	CMStringT& TrimLeft()
	{
		// find first non-space character

		PCXSTR psz = this->GetString();
		while (StringTraits::IsSpace(*psz))
			psz = StringTraits::CharNext(psz);

		if (psz != this->GetString()) {
			// fix up data and length
			int iFirst = int(psz - this->GetString());
			PXSTR pszBuffer = this->GetBuffer(this->GetLength());
			psz = pszBuffer + iFirst;
			int nDataLength = this->GetLength() - iFirst;
			memmove_s(pszBuffer, (this->GetLength() + 1)*sizeof(XCHAR),
				psz, (nDataLength + 1)*sizeof(XCHAR));
			this->ReleaseBufferSetLength(nDataLength);
		}

		return *this;
	}

	// Remove all leading and trailing whitespace
	CMStringT& Trim()
	{
		return TrimRight().TrimLeft();
	}

	// Remove all leading and trailing occurrences of character 'chTarget'
	CMStringT& Trim(XCHAR chTarget)
	{
		return TrimRight(chTarget).TrimLeft(chTarget);
	}

	// Remove all leading and trailing occurrences of any of the characters in the string 'pszTargets'
	CMStringT& Trim(PCXSTR pszTargets)
	{
		return TrimRight(pszTargets).TrimLeft(pszTargets);
	}

	// trimming anything (either side)

	// Remove all trailing occurrences of character 'chTarget'
	CMStringT& TrimRight(XCHAR chTarget)
	{
		// find beginning of trailing matches
		// by starting at beginning (DBCS aware)

		PCXSTR psz = this->GetString();
		PCXSTR pszLast = NULL;

		while (*psz != 0) {
			if (*psz == chTarget) {
				if (pszLast == NULL)
					pszLast = psz;
			}
			else pszLast = NULL;

			psz = StringTraits::CharNext(psz);
		}

		if (pszLast != NULL) {
			// truncate at left-most matching character
			int iLast = int(pszLast - this->GetString());
			this->Truncate(iLast);
		}

		return *this;
	}

	// Remove all trailing occurrences of any of the characters in string 'pszTargets'
	CMStringT& TrimRight(PCXSTR pszTargets)
	{
		// if we're not trimming anything, we're not doing any work
		if ((pszTargets == NULL) || (*pszTargets == 0)) {
			return *this;
		}

		// find beginning of trailing matches
		// by starting at beginning (DBCS aware)

		PCXSTR psz = this->GetString();
		PCXSTR pszLast = NULL;

		while (*psz != 0) {
			if (StringTraits::StringFindChar(pszTargets, *psz) != NULL) {
				if (pszLast == NULL) {
					pszLast = psz;
				}
			}
			else {
				pszLast = NULL;
			}
			psz = StringTraits::CharNext(psz);
		}

		if (pszLast != NULL) {
			// truncate at left-most matching character
			int iLast = int(pszLast - this->GetString());
			this->Truncate(iLast);
		}

		return *this;
	}

	// Remove all leading occurrences of character 'chTarget'
	CMStringT& TrimLeft(XCHAR chTarget)
	{
		// find first non-matching character
		PCXSTR psz = this->GetString();

		while (chTarget == *psz) {
			psz = StringTraits::CharNext(psz);
		}

		if (psz != this->GetString()) {
			// fix up data and length
			int iFirst = int(psz - this->GetString());
			PXSTR pszBuffer = this->GetBuffer(this->GetLength());
			psz = pszBuffer + iFirst;
			int nDataLength = this->GetLength() - iFirst;
			memmove_s(pszBuffer, (this->GetLength() + 1)*sizeof(XCHAR),
				psz, (nDataLength + 1)*sizeof(XCHAR));
			this->ReleaseBufferSetLength(nDataLength);
		}

		return *this;
	}

	// Remove all leading occurrences of any of the characters in string 'pszTargets'
	CMStringT& TrimLeft(PCXSTR pszTargets)
	{
		// if we're not trimming anything, we're not doing any work
		if ((pszTargets == NULL) || (*pszTargets == 0)) {
			return *this;
		}

		PCXSTR psz = this->GetString();
		while ((*psz != 0) && (StringTraits::StringFindChar(pszTargets, *psz) != NULL)) {
			psz = StringTraits::CharNext(psz);
		}

		if (psz != this->GetString()) {
			// fix up data and length
			int iFirst = int(psz - this->GetString());
			PXSTR pszBuffer = this->GetBuffer(this->GetLength());
			psz = pszBuffer + iFirst;
			int nDataLength = this->GetLength() - iFirst;
			memmove_s(pszBuffer, (this->GetLength() + 1)*sizeof(XCHAR),
				psz, (nDataLength + 1)*sizeof(XCHAR));
			this->ReleaseBufferSetLength(nDataLength);
		}

		return *this;
	}

	// Convert the string to the OEM character set
	void AnsiToOem()
	{
		int nLength = this->GetLength();
		PXSTR pszBuffer = this->GetBuffer(nLength);
		StringTraits::ConvertToOem(pszBuffer, nLength + 1);
		this->ReleaseBufferSetLength(nLength);
	}

	// Convert the string to the ANSI character set
	void OemToAnsi()
	{
		int nLength = this->GetLength();
		PXSTR pszBuffer = this->GetBuffer(nLength);
		StringTraits::ConvertToAnsi(pszBuffer, nLength + 1);
		this->ReleaseBufferSetLength(nLength);
	}

	// Very simple sub-string extraction

	// Return the substring starting at index 'iFirst'
	CMStringT Mid(int iFirst) const
	{
		return Mid(iFirst, this->GetLength() - iFirst);
	}

	// Return the substring starting at index 'iFirst', with length 'nCount'
	CMStringT Mid(int iFirst, int nCount) const
	{
		// nCount is in XCHARs

		// out-of-bounds requests return sensible things
		if (iFirst < 0)
			iFirst = 0;
		if (nCount < 0)
			nCount = 0;

		if ((iFirst + nCount) > this->GetLength())
			nCount = this->GetLength() - iFirst;

		if (iFirst > this->GetLength())
			nCount = 0;

		// optimize case of returning entire string
		if ((iFirst == 0) && ((iFirst + nCount) == this->GetLength()))
			return *this;

		return CMStringT(this->GetString() + iFirst, nCount);
	}

	// Return the substring consisting of the rightmost 'nCount' characters
	CMStringT Right(int nCount) const
	{
		// nCount is in XCHARs
		if (nCount < 0)
			nCount = 0;

		int nLength = this->GetLength();
		if (nCount >= nLength) {
			return *this;
		}

		return CMStringT(this->GetString() + nLength - nCount, nCount);
	}

	// Return the substring consisting of the leftmost 'nCount' characters
	CMStringT Left(int nCount) const
	{
		// nCount is in XCHARs
		if (nCount < 0)
			nCount = 0;

		int nLength = this->GetLength();
		if (nCount >= nLength)
			return *this;

		return CMStringT(this->GetString(), nCount);
	}

	// Return the substring consisting of the leftmost characters in the set 'pszCharSet'
	CMStringT SpanIncluding(PCXSTR pszCharSet) const
	{
		return Left(StringTraits::StringSpanIncluding(this->GetString(), pszCharSet));
	}

	// Return the substring consisting of the leftmost characters not in the set 'pszCharSet'
	CMStringT SpanExcluding(PCXSTR pszCharSet) const
	{
		return Left(StringTraits::StringSpanExcluding(this->GetString(), pszCharSet));
	}

	// Format data using format string 'pszFormat'
	PCXSTR Format(PCXSTR pszFormat, ...)
	{
		va_list argList;
		va_start(argList, pszFormat);
		FormatV(pszFormat, argList);
		va_end(argList);
		return GetString();
	}

	// Append formatted data using format string 'pszFormat'
	PCXSTR AppendFormat(PCXSTR pszFormat, ...)
	{
		va_list argList;
		va_start(argList, pszFormat);
		AppendFormatV(pszFormat, argList);
		va_end(argList);
		return GetString();
	}

	void AppendFormatV(PCXSTR pszFormat, va_list args)
	{
		int nCurrentLength = this->GetLength();
		int nAppendLength = StringTraits::GetFormattedLength(pszFormat, args);
		PXSTR pszBuffer = this->GetBuffer(nCurrentLength + nAppendLength);
		StringTraits::Format(pszBuffer + nCurrentLength, nAppendLength + 1, pszFormat, args);
		this->ReleaseBufferSetLength(nCurrentLength + nAppendLength);
	}

	PCXSTR FormatV(PCXSTR pszFormat, va_list args)
	{
		int nLength = StringTraits::GetFormattedLength(pszFormat, args);
		PXSTR pszBuffer = this->GetBuffer(nLength);
		StringTraits::Format(pszBuffer, nLength + 1, pszFormat, args);
		this->ReleaseBufferSetLength(nLength);
		return GetString();
	}

	// OLE BSTR support

	// allocate a BSTR containing a copy of the string
	BSTR AllocSysString() const
	{
		BSTR bstrResult = StringTraits::AllocSysString(this->GetString(), this->GetLength());
		return bstrResult;
	}

	BSTR SetSysString(BSTR* pbstr) const
	{
		StringTraits::ReAllocSysString(this->GetString(), pbstr, this->GetLength());
		return *pbstr;
	}

	// Set the string to the value of environment variable 'pszVar'
	BOOL GetEnvironmentVariable(PCXSTR pszVar)
	{
		ULONG nLength = StringTraits::GetEnvironmentVariable(pszVar, NULL, 0);
		BOOL bRetVal = FALSE;

		if (nLength == 0)
			this->Empty();
		else {
			PXSTR pszBuffer = this->GetBuffer(nLength);
			StringTraits::GetEnvironmentVariable(pszVar, pszBuffer, nLength);
			this->ReleaseBuffer();
			bRetVal = TRUE;
		}

		return bRetVal;
	}

	// Load the string from resource 'nID'
	BOOL LoadString(UINT nID)
	{
		HINSTANCE hInst = StringTraits::FindStringResourceInstance(nID);
		if (hInst == NULL)
			return FALSE;

		return LoadString(hInst, nID);
	}

	friend CMStringT __stdcall operator+(const CMStringT& str1, const CMStringT& str2)
	{
		CMStringT strResult;

		Concatenate(strResult, str1, str1.GetLength(), str2, str2.GetLength());

		return strResult;
	}

	friend CMStringT __stdcall operator+(const CMStringT& str1, PCXSTR psz2)
	{
		CMStringT strResult;

		Concatenate(strResult, str1, str1.GetLength(), psz2, StringLength(psz2));

		return strResult;
	}

	friend CMStringT __stdcall operator+(PCXSTR psz1, const CMStringT& str2)
	{
		CMStringT strResult;

		Concatenate(strResult, psz1, StringLength(psz1), str2, str2.GetLength());

		return strResult;
	}

	friend CMStringT __stdcall operator+(const CMStringT& str1, wchar_t ch2)
	{
		CMStringT strResult;
		XCHAR chTemp = XCHAR(ch2);

		Concatenate(strResult, str1, str1.GetLength(), &chTemp, 1);

		return strResult;
	}

	friend CMStringT __stdcall operator+(const CMStringT& str1, char ch2)
	{
		CMStringT strResult;
		XCHAR chTemp = XCHAR(ch2);

		Concatenate(strResult, str1, str1.GetLength(), &chTemp, 1);

		return strResult;
	}

	friend CMStringT __stdcall operator+(wchar_t ch1, const CMStringT& str2)
	{
		CMStringT strResult;
		XCHAR chTemp = XCHAR(ch1);

		Concatenate(strResult, &chTemp, 1, str2, str2.GetLength());

		return strResult;
	}

	friend CMStringT __stdcall operator+(char ch1, const CMStringT& str2)
	{
		CMStringT strResult;
		XCHAR chTemp = XCHAR(ch1);

		Concatenate(strResult, &chTemp, 1, str2, str2.GetLength());

		return strResult;
	}

	friend bool __stdcall operator==(const CMStringT& str1, const CMStringT& str2)
	{
		return str1.Compare(str2) == 0;
	}

	friend bool __stdcall operator==(const CMStringT& str1, PCXSTR psz2)
	{
		return str1.Compare(psz2) == 0;
	}

	friend bool __stdcall operator==(PCXSTR psz1, const CMStringT& str2)
	{
		return str2.Compare(psz1) == 0;
	}

	friend bool __stdcall operator==(const CMStringT& str1, PCYSTR psz2)
	{
		CMStringT str2(psz2);

		return str1 == str2;
	}

	friend bool __stdcall operator==(PCYSTR psz1, const CMStringT& str2)
	{
		CMStringT str1(psz1);

		return str1 == str2;
	}

	friend bool __stdcall operator!=(const CMStringT& str1, const CMStringT& str2)
	{
		return str1.Compare(str2) != 0;
	}

	friend bool __stdcall operator!=(const CMStringT& str1, PCXSTR psz2)
	{
		return str1.Compare(psz2) != 0;
	}

	friend bool __stdcall operator!=(PCXSTR psz1, const CMStringT& str2)
	{
		return str2.Compare(psz1) != 0;
	}

	friend bool __stdcall operator!=(const CMStringT& str1, PCYSTR psz2)
	{
		CMStringT str2(psz2);

		return str1 != str2;
	}

	friend bool __stdcall operator!=(PCYSTR psz1, const CMStringT& str2)
	{
		CMStringT str1(psz1);

		return str1 != str2;
	}

	friend bool __stdcall operator<(const CMStringT& str1, const CMStringT& str2)
	{
		return str1.Compare(str2) < 0;
	}

	friend bool __stdcall operator<(const CMStringT& str1, PCXSTR psz2)
	{
		return str1.Compare(psz2) < 0;
	}

	friend bool __stdcall operator<(PCXSTR psz1, const CMStringT& str2)
	{
		return str2.Compare(psz1) > 0;
	}

	friend bool __stdcall operator>(const CMStringT& str1, const CMStringT& str2)
	{
		return str1.Compare(str2) > 0;
	}

	friend bool __stdcall operator>(const CMStringT& str1, PCXSTR psz2)
	{
		return str1.Compare(psz2) > 0;
	}

	friend bool __stdcall operator>(PCXSTR psz1, const CMStringT& str2)
	{
		return str2.Compare(psz1) < 0;
	}

	friend bool __stdcall operator<=(const CMStringT& str1, const CMStringT& str2)
	{
		return str1.Compare(str2) <= 0;
	}

	friend bool __stdcall operator<=(const CMStringT& str1, PCXSTR psz2)
	{
		return str1.Compare(psz2) <= 0;
	}

	friend bool __stdcall operator<=(PCXSTR psz1, const CMStringT& str2)
	{
		return str2.Compare(psz1) >= 0;
	}

	friend bool __stdcall operator>=(const CMStringT& str1, const CMStringT& str2)
	{
		return str1.Compare(str2) >= 0;
	}

	friend bool __stdcall operator>=(const CMStringT& str1, PCXSTR psz2)
	{
		return str1.Compare(psz2) >= 0;
	}

	friend bool __stdcall operator>=(PCXSTR psz1, const CMStringT& str2)
	{
		return str2.Compare(psz1) <= 0;
	}

	friend bool __stdcall operator==(XCHAR ch1, const CMStringT& str2)
	{
		return (str2.GetLength() == 1) && (str2[0] == ch1);
	}

	friend bool __stdcall operator==(const CMStringT& str1, XCHAR ch2)
	{
		return (str1.GetLength() == 1) && (str1[0] == ch2);
	}

	friend bool __stdcall operator!=(XCHAR ch1, const CMStringT& str2)
	{
		return (str2.GetLength() != 1) || (str2[0] != ch1);
	}

	friend bool __stdcall operator!=(const CMStringT& str1, XCHAR ch2)
	{
		return (str1.GetLength() != 1) || (str1[0] != ch2);
	}
};

typedef CMStringT< wchar_t, ChTraitsCRT< wchar_t > > CMStringW;
typedef CMStringT< char, ChTraitsCRT< char > > CMStringA;
typedef CMStringT< TCHAR, ChTraitsCRT< TCHAR > > CMString;

/////////////////////////////////////////////////////////////////////////////////////////
// ChTraitsCRT<wchar_t>

__forceinline int ChTraitsCRT<wchar_t>::GetFormattedLength(LPCWSTR pszFormat, va_list args)
{
	return _vscwprintf(pszFormat, args);
}

__forceinline int ChTraitsCRT<wchar_t>::Format(LPWSTR pszBuffer, size_t nLength, LPCWSTR pszFormat, va_list args)
{
	return _vsnwprintf(pszBuffer, nLength, pszFormat, args);
}

/////////////////////////////////////////////////////////////////////////////////////////
// ChTraitsCRT<char>

__forceinline int ChTraitsCRT<char>::GetFormattedLength(LPCSTR pszFormat, va_list args)
{
	return _vscprintf(pszFormat, args);
}

__forceinline int ChTraitsCRT<char>::Format(LPSTR pszBuffer, size_t nlength, LPCSTR pszFormat, va_list args)
{
	return vsprintf_s(pszBuffer, nlength, pszFormat, args);
}

#endif // M_STRING_H__
