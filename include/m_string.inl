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

#ifndef M_STRING_INL__

template<typename BaseType>
CMSimpleStringT<BaseType>::CMSimpleStringT()
{
	CMStringData* pData = mirstr_getNil();
	Attach(pData);
}

template<typename BaseType>
CMSimpleStringT<BaseType>::CMSimpleStringT(const CMSimpleStringT& strSrc)
{
	CMStringData* pSrcData = strSrc.GetData();
	CMStringData* pNewData = CloneData(pSrcData);
	Attach(pNewData);
}

template<typename BaseType>
CMSimpleStringT<BaseType>::CMSimpleStringT(PCXSTR pszSrc)
{
	int nLength = StringLength(pszSrc);
	CMStringData* pData = mirstr_allocate(nLength, sizeof(XCHAR));
	if (pData != NULL) {
		Attach(pData);
		SetLength(nLength);
		CopyChars(m_pszData, nLength, pszSrc, nLength);
	}
}

template<typename BaseType>
CMSimpleStringT<BaseType>::CMSimpleStringT(const XCHAR* pchSrc, int nLength)
{
	CMStringData* pData = mirstr_allocate(nLength, sizeof(XCHAR));
	if (pData != NULL) {
		Attach(pData);
		SetLength(nLength);
		CopyChars(m_pszData, nLength, pchSrc, nLength);
	}
}

template<typename BaseType>
CMSimpleStringT<BaseType>::~CMSimpleStringT()
{
	CMStringData* pData = GetData();
	pData->Release();
}

template<typename BaseType>
CMSimpleStringT<BaseType>& CMSimpleStringT<BaseType>::operator=(const CMSimpleStringT& strSrc)
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

template<typename BaseType>
void CMSimpleStringT<BaseType>::Append(PCXSTR pszSrc)
{
	Append(pszSrc, StringLength(pszSrc));
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::Append(PCXSTR pszSrc, int nLength)
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

template<typename BaseType>
void CMSimpleStringT<BaseType>::AppendChar(XCHAR ch)
{
	UINT nOldLength = GetLength();
	int nNewLength = nOldLength + 1;
	PXSTR pszBuffer = GetBuffer(nNewLength);
	pszBuffer[nOldLength] = ch;
	ReleaseBufferSetLength(nNewLength);
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::Append(const CMSimpleStringT<BaseType>& strSrc)
{
	Append(strSrc.GetString(), strSrc.GetLength());
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::Empty()
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

template<typename BaseType>
void CMSimpleStringT<BaseType>::FreeExtra()
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

template<typename BaseType>
typename CMSimpleStringT<BaseType>::PXSTR CMSimpleStringT<BaseType>::GetBuffer()
{
	CMStringData* pData = GetData();
	if (pData->IsShared())
		Fork(pData->nDataLength);

	return m_pszData;
}

template<typename BaseType>
typename CMSimpleStringT<BaseType>::PXSTR CMSimpleStringT<BaseType>::GetBufferSetLength(int nLength)
{
	PXSTR pszBuffer = GetBuffer(nLength);
	SetLength(nLength);

	return pszBuffer;
}

template<typename BaseType>
typename CMSimpleStringT<BaseType>::PXSTR CMSimpleStringT<BaseType>::LockBuffer()
{
	CMStringData* pData = GetData();
	if (pData->IsShared()) {
		Fork(pData->nDataLength);
		pData = GetData();  // Do it again, because the fork might have changed it
	}
	pData->Lock();

	return m_pszData;
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::UnlockBuffer()
{
	CMStringData* pData = GetData();
	pData->Unlock();
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::ReleaseBuffer(int nNewLength = -1)
{
	if (nNewLength == -1) {
		int nAlloc = GetData()->nAllocLength;
		nNewLength = StringLengthN(m_pszData, nAlloc);
	}
	SetLength(nNewLength);
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::Truncate(int nNewLength)
{
	GetBuffer(nNewLength);
	ReleaseBufferSetLength(nNewLength);
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::SetAt(int iChar, XCHAR ch)
{
	int nLength = GetLength();
	PXSTR pszBuffer = GetBuffer();
	pszBuffer[iChar] = ch;
	ReleaseBufferSetLength(nLength);

}

template<typename BaseType>
void CMSimpleStringT<BaseType>::SetString(PCXSTR pszSrc)
{
	SetString(pszSrc, StringLength(pszSrc));
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::SetString(PCXSTR pszSrc, int nLength)
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

template<typename BaseType>
typename CMSimpleStringT<BaseType> operator+(const CMSimpleStringT<BaseType>& str1, const CMSimpleStringT<BaseType>& str2)
{
	CMSimpleStringT s;
	Concatenate(s, str1, str1.GetLength(), str2, str2.GetLength());
	return s;
}

template<typename BaseType>
typename CMSimpleStringT<BaseType> operator+(const CMSimpleStringT<BaseType>& str1, typename CMSimpleStringT<BaseType>::PCXSTR psz2)
{
	CMSimpleStringT s;
	Concatenate(s, str1, str1.GetLength(), psz2, StringLength(psz2));
	return s;
}

template<typename BaseType>
CMSimpleStringT<BaseType> operator+(typename CMSimpleStringT<BaseType>::PCXSTR psz1, const CMSimpleStringT<BaseType>& str2)
{
	CMSimpleStringT s;
	Concatenate(s, psz1, StringLength(psz1), str2, str2.GetLength());
	return s;
}

template<typename BaseType>
void __stdcall CMSimpleStringT<BaseType>::CopyChars(XCHAR* pchDest, const XCHAR* pchSrc, int nChars)
{
	#pragma warning (push)
	#pragma warning(disable : 4996)
	memcpy(pchDest, pchSrc, nChars * sizeof(XCHAR));
	#pragma warning (pop)
}

template<typename BaseType>
void __stdcall CMSimpleStringT<BaseType>::CopyChars(XCHAR* pchDest, size_t nDestLen, const XCHAR* pchSrc, int nChars)
{
	memcpy_s(pchDest, nDestLen * sizeof(XCHAR), pchSrc, nChars * sizeof(XCHAR));
}

template<typename BaseType>
void __stdcall CMSimpleStringT<BaseType>::CopyCharsOverlapped(XCHAR* pchDest, const XCHAR* pchSrc, int nChars)
{
	#pragma warning (push)
	#pragma warning(disable : 4996)
	memmove(pchDest, pchSrc, nChars * sizeof(XCHAR));
	#pragma warning (pop)
}

template<typename BaseType>
void __stdcall CMSimpleStringT<BaseType>::CopyCharsOverlapped(XCHAR* pchDest, size_t nDestLen, const XCHAR* pchSrc, int nChars)
{
	memmove_s(pchDest, nDestLen * sizeof(XCHAR), pchSrc, nChars * sizeof(XCHAR));
}

template<typename BaseType>
int __stdcall CMSimpleStringT<BaseType>::StringLength(const char* psz)
{
	if (psz == NULL)
		return(0);

	return (int(strlen(psz)));
}

template<typename BaseType>
int __stdcall CMSimpleStringT<BaseType>::StringLength(const wchar_t* psz)
{
	if (psz == NULL)
		return 0;

	return int(wcslen(psz));
}

template<typename BaseType>
int __stdcall CMSimpleStringT<BaseType>::StringLengthN(const char* psz, size_t sizeInXChar)
{
	if (psz == NULL)
		return 0;

	return int(strnlen(psz, sizeInXChar));
}

template<typename BaseType>
int __stdcall CMSimpleStringT<BaseType>::StringLengthN(const wchar_t* psz, size_t sizeInXChar)
{
	if (psz == NULL)
		return 0;

	return int(wcsnlen(psz, sizeInXChar));
}

template<typename BaseType>
void __stdcall CMSimpleStringT<BaseType>::Concatenate(CMSimpleStringT<BaseType>& strResult, PCXSTR psz1, int nLength1, PCXSTR psz2, int nLength2)
{
	int nNewLength = nLength1 + nLength2;
	PXSTR pszBuffer = strResult.GetBuffer(nNewLength);
	CopyChars(pszBuffer, nLength1, psz1, nLength1);
	CopyChars(pszBuffer + nLength1, nLength2, psz2, nLength2);
	strResult.ReleaseBufferSetLength(nNewLength);
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::Attach(CMStringData* pData)
{
	m_pszData = static_cast<PXSTR>(pData->data());
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::Fork(int nLength)
{
	CMStringData* pOldData = GetData();
	int nOldLength = pOldData->nDataLength;
	CMStringData* pNewData = mirstr_allocate(nLength, sizeof(XCHAR));
	if (pNewData != NULL) {
		int nCharsToCopy = ((nOldLength < nLength) ? nOldLength : nLength) + 1;  // Copy '\0'
		CopyChars(PXSTR(pNewData->data()), nCharsToCopy, PCXSTR(pOldData->data()), nCharsToCopy);
		pNewData->nDataLength = nOldLength;
		pOldData->Release();
		Attach(pNewData);
	}
}

template<typename BaseType>
typename CMSimpleStringT<BaseType>::PXSTR CMSimpleStringT<BaseType>::PrepareWrite(int nLength)
{
	CMStringData* pOldData = GetData();
	int nShared = 1 - pOldData->nRefs;  // nShared < 0 means true, >= 0 means false
	int nTooShort = pOldData->nAllocLength - nLength;  // nTooShort < 0 means true, >= 0 means false
	if ((nShared | nTooShort) < 0)  // If either sign bit is set (i.e. either is less than zero), we need to copy data
		PrepareWrite2(nLength);

	return m_pszData;
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::PrepareWrite2(int nLength)
{
	CMStringData* pOldData = GetData();
	if (pOldData->nDataLength > nLength)
		nLength = pOldData->nDataLength;

	if (pOldData->IsShared()) {
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

template<typename BaseType>
void CMSimpleStringT<BaseType>::Reallocate(int nLength)
{
	CMStringData* pOldData = GetData();
	if (pOldData->nAllocLength >= nLength || nLength <= 0)
		return;

	CMStringData* pNewData = mirstr_realloc(pOldData, nLength, sizeof(XCHAR));
	if (pNewData != NULL)
		Attach(pNewData);
}

template<typename BaseType>
void CMSimpleStringT<BaseType>::SetLength(int nLength)
{
	GetData()->nDataLength = nLength;
	m_pszData[nLength] = 0;
}

template<typename BaseType>
CMStringData* __stdcall CMSimpleStringT<BaseType>::CloneData(CMStringData* pData)
{
	CMStringData* pNewData = NULL;

	if (!pData->IsLocked()) {
		pNewData = pData;
		pNewData->AddRef();
	}

	return pNewData;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT() : 
	CThisSimpleString()
{
}

// Copy constructor
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(const CMStringT<BaseType, StringTraits>& strSrc) :
	CThisSimpleString(strSrc)
{
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(const XCHAR* pszSrc) :
	CThisSimpleString()
{
	*this = pszSrc;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(CMStringDataFormat, const XCHAR* pszFormat, ...) :
	CThisSimpleString()
{
	va_list args;
	va_start(args, pszFormat);
	FormatV(pszFormat, args);
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(const YCHAR* pszSrc) :
	CThisSimpleString()
{
	*this = pszSrc;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(const unsigned char* pszSrc) :
	CThisSimpleString()
{
	*this = reinterpret_cast<const char*>(pszSrc);
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(char ch, int nLength = 1) :
	CThisSimpleString()
{
	if (nLength > 0) {
		PXSTR pszBuffer = this->GetBuffer(nLength);
		StringTraits::FloodCharacters(XCHAR(ch), nLength, pszBuffer);
		this->ReleaseBufferSetLength(nLength);
	}
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(wchar_t ch, int nLength = 1) :
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

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(const XCHAR* pch, int nLength) :
	CThisSimpleString(pch, nLength)
{
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::CMStringT(const YCHAR* pch, int nLength) :
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>::~CMStringT()
{
}

// Assignment operators
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator=(const CMStringT& strSrc)
{
	CThisSimpleString::operator=(strSrc);
	return *this;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator=(PCXSTR pszSrc)
{
	CThisSimpleString::operator=(pszSrc);
	return *this;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator=(PCYSTR pszSrc)
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

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator=(const unsigned char* pszSrc)
{
	return operator=(reinterpret_cast<const char*>(pszSrc));
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator=(char ch)
{
	char ach[2] = { ch, 0 };
	return operator=(ach);
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator=(wchar_t ch)
{
	wchar_t ach[2] = { ch, 0 };
	return operator=(ach);
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator+=(const CMStringT& str)
{
	CThisSimpleString::operator+=(str);
	return *this;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator+=(const CThisSimpleString& str)
{
	CThisSimpleString::operator+=(str);
	return *this;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator+=(PCXSTR pszSrc)
{
	CThisSimpleString::operator+=(pszSrc);
	return *this;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator+=(PCYSTR pszSrc)
{
	CMStringT str(pszSrc);
	return operator+=(str);
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator+=(char ch)
{
	CThisSimpleString::operator+=(ch);
	return *this;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator+=(unsigned char ch)
{
	CThisSimpleString::operator+=(ch);
	return *this;
}

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::operator+=(wchar_t ch)
{
	CThisSimpleString::operator+=(ch);
	return *this;
}

// Comparison

template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Compare(PCXSTR psz) const
{
	return StringTraits::StringCompare(this->GetString(), psz);
}

template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::CompareNoCase(PCXSTR psz) const
{
	return StringTraits::StringCompareIgnore(this->GetString(), psz);
}

template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Collate(PCXSTR psz) const
{
	return StringTraits::StringCollate(this->GetString(), psz);
}

template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::CollateNoCase(PCXSTR psz) const
{
	return StringTraits::StringCollateIgnore(this->GetString(), psz);
}

// Advanced manipulation

// Delete 'nCount' characters, starting at index 'iIndex'
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Delete(int iIndex, int nCount = 1)
{
	if (iIndex < 0)
		iIndex = 0;

	if (nCount < 0)
		nCount = 0;

	int nLength = this->GetLength();
	if (nCount + iIndex > nLength)
		nCount = nLength - iIndex;

	if (nCount > 0) {
		int nNewLength = nLength - nCount;
		int nXCHARsToCopy = nLength - (iIndex + nCount) + 1;
		PXSTR pszBuffer = this->GetBuffer();
#if _MSC_VER >= 1400
		memmove_s(pszBuffer + iIndex, nXCHARsToCopy*sizeof(XCHAR), pszBuffer + iIndex + nCount, nXCHARsToCopy*sizeof(XCHAR));
#else
		memmove(pszBuffer+iIndex, pszBuffer+iIndex+nCount, nXCHARsToCopy*sizeof(XCHAR));
#endif
		this->ReleaseBufferSetLength(nNewLength);
	}

	return this->GetLength();
}

// Insert character 'ch' before index 'iIndex'
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Insert(int iIndex, XCHAR ch)
{
	if (iIndex < 0)
		iIndex = 0;

	if (iIndex > this->GetLength())
		iIndex = this->GetLength();

	int nNewLength = this->GetLength() + 1;

	PXSTR pszBuffer = this->GetBuffer(nNewLength);

	// move existing bytes down
#if _MSC_VER >= 1400
	memmove_s(pszBuffer + iIndex + 1, (nNewLength - iIndex)*sizeof(XCHAR), pszBuffer + iIndex, (nNewLength - iIndex)*sizeof(XCHAR));
#else
	memmove(pszBuffer+iIndex+1, pszBuffer+iIndex, (nNewLength-iIndex)*sizeof(XCHAR));
#endif
	pszBuffer[iIndex] = ch;

	this->ReleaseBufferSetLength(nNewLength);
	return nNewLength;
}

// Insert string 'psz' before index 'iIndex'
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Insert(int iIndex, PCXSTR psz)
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
		memmove_s(pszBuffer + iIndex + nInsertLength, (nNewLength - iIndex - nInsertLength + 1)*sizeof(XCHAR), pszBuffer + iIndex, (nNewLength - iIndex - nInsertLength + 1)*sizeof(XCHAR));
		memcpy_s(pszBuffer + iIndex, nInsertLength*sizeof(XCHAR), psz, nInsertLength*sizeof(XCHAR));
#else
		memmove(pszBuffer+iIndex+nInsertLength, pszBuffer+iIndex, (nNewLength-iIndex-nInsertLength+1)*sizeof(XCHAR));
		memcpy(pszBuffer+iIndex, psz, nInsertLength*sizeof(XCHAR));
#endif
		this->ReleaseBufferSetLength(nNewLength);
	}

	return nNewLength;
}

// Replace all occurrences of character 'chOld' with character 'chNew'
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Replace(XCHAR chOld, XCHAR chNew)
{
	int nCount = 0;

	// short-circuit the nop case
	if (chOld != chNew) {
		// otherwise modify each character that matches in the string
		bool bCopied = false;
		PXSTR pszBuffer = const_cast<PXSTR>(this->GetString());  // We don't actually write to pszBuffer until we've called GetBuffer().

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
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Replace(PCXSTR pszOld, PCXSTR pszNew)
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
		int nNewLength = nOldLength + (nReplacementLen - nSourceLen)*nCount;

		PXSTR pszBuffer = this->GetBuffer(__max(nNewLength, nOldLength));

		PXSTR pszStart = pszBuffer;
		PXSTR pszEnd = pszStart + nOldLength;

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
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Remove(XCHAR chRemove)
{
	int nLength = this->GetLength();
	PXSTR pszBuffer = this->GetBuffer(nLength);

	PXSTR pszSource = pszBuffer;
	PXSTR pszDest = pszBuffer;
	PXSTR pszEnd = pszBuffer + nLength;

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

template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits> CMStringT<BaseType, StringTraits>::Tokenize(PCXSTR pszTokens, int& iStart) const
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
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Find(XCHAR ch, int iStart = 0) const
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
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::Find(PCXSTR pszSub, int iStart = 0) const
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
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::FindOneOf(PCXSTR pszCharSet) const
{
	PCXSTR psz = StringTraits::StringScanSet(this->GetString(), pszCharSet);
	return (psz == NULL) ? -1 : int(psz - this->GetString());
}

// Find the last occurrence of character 'ch'
template< typename BaseType, class StringTraits >
int CMStringT<BaseType, StringTraits>::ReverseFind(XCHAR ch) const
{
	// find last single character
	PCXSTR psz = StringTraits::StringFindCharRev(this->GetString(), ch);

	// return -1 if not found, distance from beginning otherwise
	return (psz == NULL) ? -1 : int(psz - this->GetString());
}

// manipulation

// Convert the string to uppercase
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::MakeUpper()
{
	int nLength = this->GetLength();
	PXSTR pszBuffer = this->GetBuffer(nLength);
	StringTraits::StringUppercase(pszBuffer, nLength + 1);
	this->ReleaseBufferSetLength(nLength);

	return *this;
}

// Convert the string to lowercase
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::MakeLower()
{
	int nLength = this->GetLength();
	PXSTR pszBuffer = this->GetBuffer(nLength);
	StringTraits::StringLowercase(pszBuffer, nLength + 1);
	this->ReleaseBufferSetLength(nLength);

	return *this;
}

// Reverse the string
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::MakeReverse()
{
	int nLength = this->GetLength();
	PXSTR pszBuffer = this->GetBuffer(nLength);
	StringTraits::StringReverse(pszBuffer);
	this->ReleaseBufferSetLength(nLength);

	return *this;
}

// trimming

// Remove all trailing whitespace
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::TrimRight()
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::TrimLeft()
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::Trim()
{
	return TrimRight().TrimLeft();
}

// Remove all leading and trailing occurrences of character 'chTarget'
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::Trim(XCHAR chTarget)
{
	return TrimRight(chTarget).TrimLeft(chTarget);
}

// Remove all leading and trailing occurrences of any of the characters in the string 'pszTargets'
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::Trim(PCXSTR pszTargets)
{
	return TrimRight(pszTargets).TrimLeft(pszTargets);
}

// trimming anything (either side)

// Remove all trailing occurrences of character 'chTarget'
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::TrimRight(XCHAR chTarget)
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::TrimRight(PCXSTR pszTargets)
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::TrimLeft(XCHAR chTarget)
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits>& CMStringT<BaseType, StringTraits>::TrimLeft(PCXSTR pszTargets)
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
template< typename BaseType, class StringTraits >
void CMStringT<BaseType, StringTraits>::AnsiToOem()
{
	int nLength = this->GetLength();
	PXSTR pszBuffer = this->GetBuffer(nLength);
	StringTraits::ConvertToOem(pszBuffer, nLength + 1);
	this->ReleaseBufferSetLength(nLength);
}

// Convert the string to the ANSI character set
template< typename BaseType, class StringTraits >
void CMStringT<BaseType, StringTraits>::OemToAnsi()
{
	int nLength = this->GetLength();
	PXSTR pszBuffer = this->GetBuffer(nLength);
	StringTraits::ConvertToAnsi(pszBuffer, nLength + 1);
	this->ReleaseBufferSetLength(nLength);
}

// Very simple sub-string extraction

// Return the substring starting at index 'iFirst'
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits> CMStringT<BaseType, StringTraits>::Mid(int iFirst) const
{
	return Mid(iFirst, this->GetLength() - iFirst);
}

// Return the substring starting at index 'iFirst', with length 'nCount'
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits> CMStringT<BaseType, StringTraits>::Mid(int iFirst, int nCount) const
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits> CMStringT<BaseType, StringTraits>::Right(int nCount) const
{
	// nCount is in XCHARs
	if (nCount < 0)
		nCount = 0;

	int nLength = this->GetLength();
	if (nCount >= nLength)
		return *this;

	return CMStringT(this->GetString() + nLength - nCount, nCount);
}

// Return the substring consisting of the leftmost 'nCount' characters
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits> CMStringT<BaseType, StringTraits>::Left(int nCount) const
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
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits> CMStringT<BaseType, StringTraits>::SpanIncluding(PCXSTR pszCharSet) const
{
	return Left(StringTraits::StringSpanIncluding(this->GetString(), pszCharSet));
}

// Return the substring consisting of the leftmost characters not in the set 'pszCharSet'
template< typename BaseType, class StringTraits >
CMStringT<BaseType, StringTraits> CMStringT<BaseType, StringTraits>::SpanExcluding(PCXSTR pszCharSet) const
{
	return Left(StringTraits::StringSpanExcluding(this->GetString(), pszCharSet));
}

// Format data using format string 'pszFormat'
template< typename BaseType, class StringTraits >
typename CMStringT<BaseType, StringTraits>::PCXSTR CMStringT<BaseType, StringTraits>::Format(PCXSTR pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);
	FormatV(pszFormat, argList);
	va_end(argList);
	return GetString();
}

// Append formatted data using format string 'pszFormat'
template< typename BaseType, class StringTraits >
typename CMStringT<BaseType, StringTraits>::PCXSTR CMStringT<BaseType, StringTraits>::AppendFormat(PCXSTR pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);
	AppendFormatV(pszFormat, argList);
	va_end(argList);
	return GetString();
}

template< typename BaseType, class StringTraits >
void CMStringT<BaseType, StringTraits>::AppendFormatV(PCXSTR pszFormat, va_list args)
{
	int nCurrentLength = this->GetLength();
	int nAppendLength = StringTraits::GetFormattedLength(pszFormat, args);
	PXSTR pszBuffer = this->GetBuffer(nCurrentLength + nAppendLength);
	StringTraits::Format(pszBuffer + nCurrentLength, nAppendLength + 1, pszFormat, args);
	this->ReleaseBufferSetLength(nCurrentLength + nAppendLength);
}

template< typename BaseType, class StringTraits >
typename CMStringT<BaseType, StringTraits>::PCXSTR CMStringT<BaseType, StringTraits>::FormatV(PCXSTR pszFormat, va_list args)
{
	int nLength = StringTraits::GetFormattedLength(pszFormat, args);
	PXSTR pszBuffer = this->GetBuffer(nLength);
	StringTraits::Format(pszBuffer, nLength + 1, pszFormat, args);
	this->ReleaseBufferSetLength(nLength);
	return GetString();
}

// Set the string to the value of environment variable 'pszVar'
template< typename BaseType, class StringTraits >
BOOL CMStringT<BaseType, StringTraits>::GetEnvironmentVariable(PCXSTR pszVar)
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

// Set the string to the value of environment variable 'pszVar'
template< typename BaseType, class StringTraits >
typename CMStringT<BaseType, StringTraits>::PXSTR CMStringT<BaseType, StringTraits>::Detach() const
{
	return StringTraits::MirCopy(CMStringT<BaseType, StringTraits>::GetString(), GetLength());
}

/////////////////////////////////////////////////////////////////////////////////////////

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, const CMStringT<BaseType, StringTraits>& str2)
{
	CMStringT<BaseType, StringTraits> strResult;
	CMStringT<BaseType, StringTraits>::Concatenate(strResult, str1, str1.GetLength(), str2, str2.GetLength());
	return strResult;
}

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, typename CMStringT<BaseType, StringTraits>::PCXSTR psz2)
{
	CMStringT<BaseType, StringTraits> strResult;
	CMStringT<BaseType, StringTraits>::Concatenate(strResult, str1, str1.GetLength(), psz2, CMStringT<BaseType, StringTraits>::StringLength(psz2));
	return strResult;
}

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(typename CMStringT<BaseType, StringTraits>::PCXSTR psz1, const CMStringT<BaseType, StringTraits>& str2)
{
	CMStringT<BaseType, StringTraits> strResult;
	CMStringT<BaseType, StringTraits>::Concatenate(strResult, psz1, CMStringT<BaseType, StringTraits>::StringLength(psz1), str2, str2.GetLength());
	return strResult;
}

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, wchar_t ch2)
{
	CMStringT<BaseType, StringTraits> strResult;
	typename CMStringT<BaseType, StringTraits>::XCHAR chTemp = typename CMStringT<BaseType, StringTraits>::XCHAR(ch2);
	CMStringT<BaseType, StringTraits>::Concatenate(strResult, str1, str1.GetLength(), &chTemp, 1);
	return strResult;
}

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(const CMStringT<BaseType, StringTraits>& str1, char ch2)
{
	CMStringT<BaseType, StringTraits> strResult;
	typename CMStringT<BaseType, StringTraits>::XCHAR chTemp = typename CMStringT<BaseType, StringTraits>::XCHAR(ch2);
	CMStringT<BaseType, StringTraits>::Concatenate(strResult, str1, str1.GetLength(), &chTemp, 1);
	return strResult;
}

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(wchar_t ch1, const CMStringT<BaseType, StringTraits>& str2)
{
	CMStringT<BaseType, StringTraits> strResult;
	typename CMStringT<BaseType, StringTraits>::XCHAR chTemp = typename CMStringT<BaseType, StringTraits>::XCHAR(ch1);
	CMStringT<BaseType, StringTraits>::Concatenate(strResult, &chTemp, 1, str2, str2.GetLength());
	return strResult;
}

template< typename BaseType, class StringTraits >
MIR_CORE_EXPORT CMStringT<BaseType, StringTraits> CALLBACK operator+(char ch1, const CMStringT<BaseType, StringTraits>& str2)
{
	CMStringT<BaseType, StringTraits> strResult;
	typename CMStringT<BaseType, StringTraits>::XCHAR chTemp = typename CMStringT<BaseType, StringTraits>::XCHAR(ch1);
	CMStringT<BaseType, StringTraits>::Concatenate(strResult, &chTemp, 1, str2, str2.GetLength());
	return strResult;
}

#endif // M_STRING_INL__
