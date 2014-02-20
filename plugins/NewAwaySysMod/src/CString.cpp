/*
	TCString.cpp - TCString class
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

#include "Common.h"

#define STR_GROWBY 64

#define min(a,b) (((a) < (b)) ? (a) : (b))


template <class T>
void TString<T>::Empty()
{
	nBufSize = 1;
	SetAllocSize(STR_GROWBY);
	pBuf[0] = 0;
}


template <class T>
void TString<T>::Free()
{
//	HeapFree(GetProcessHeap(), 0, pBuf);
	free(pBuf);
	pBuf = NULL;
	nBufSize = 0;
	nAllocSize = 0;
}


template <class T>
T *TString<T>::GetBuffer(int nNewLen)
{
	if (nNewLen != -1)
	{
		SetBufSize(nNewLen + 1);
	}
	_ASSERT(pBuf);
	return pBuf;
}


template <class T>
void TString<T>::ReleaseBuffer(int nNewLen)
{
	if (nNewLen == -1)
	{
		nBufSize = My_lstrlen(pBuf) + 1;
	} else
	{
		nBufSize = nNewLen + 1;
		pBuf[nNewLen] = 0;
		_ASSERT(My_lstrlen(pBuf) == nNewLen);
	}
	_ASSERT(nBufSize <= nAllocSize); // prevent buffer overruns
}


template <class T>
void TString<T>::SetAllocSize(int nNewAllocSize)
{
	_ASSERT(nNewAllocSize > 0);
	T *pNewBuf = /*(char *)HeapAlloc(GetProcessHeap(), 0, sizeof(char) * nNewAllocSize);*/
(T *)malloc(sizeof(T) * nNewAllocSize);
	if (pBuf)
	{
		memcpy(pNewBuf, pBuf, sizeof(T) * min(nBufSize, nNewAllocSize));
		//HeapFree(GetProcessHeap(), 0, pBuf);
		free(pBuf);
	}
	pBuf = pNewBuf;
	nAllocSize = nNewAllocSize;
}


template <class T>
void TString<T>::SetBufSize(int nNewBufSize)
{
	_ASSERT(nNewBufSize >= 0);
	if (nNewBufSize < nBufSize)
	{
		_ASSERT(pBuf);
		pBuf[nNewBufSize - 1] = 0;
	}
	if ((unsigned)(nAllocSize - nNewBufSize) / STR_GROWBY)
	{
		SetAllocSize((nNewBufSize + STR_GROWBY - 1) - (nNewBufSize + STR_GROWBY - 1) % STR_GROWBY);
	}
	nBufSize = nNewBufSize;
}


template <class T>
TString<T>& TString<T>::Cat(const T *pStr)
{
	_ASSERT(pBuf && pStr);
	int StrLen = My_lstrlen(pStr);
	SetAllocSize(nBufSize + StrLen);
	My_lstrcpy(GetBuffer() + GetLen(), pStr);
	ReleaseBuffer(nBufSize + StrLen - 1);
	return *this;
}


template <class T>
TString<T>& TString<T>::Cat(const T c)
{
	_ASSERT(pBuf);
	SetAllocSize(nBufSize + 1);
	int CurLen = GetLen();
	T *p = GetBuffer();
	p[CurLen] = c;
	p[CurLen + 1] = '\0';
	ReleaseBuffer(nBufSize);
	return *this;
}


template <class T>
TString<T>& TString<T>::DiffCat(const T *pStart, const T *pEnd)
{
	_ASSERT(pBuf && pStart && pEnd);
	int StrLen = pEnd - pStart;
	SetAllocSize(nBufSize + StrLen);
	My_strncpy(GetBuffer() + GetLen(), pStart, StrLen);
	ReleaseBuffer(nBufSize + StrLen - 1);
	return *this;
}


template <class T>
TString<T>& TString<T>::Replace(const T *szFind, const T *szReplaceBy)
{
	if (!pBuf)
	{
		return *this;
	}
	T *pCurPos = pBuf;
	int FindLen = My_lstrlen(szFind);
	T *p;
	TString<T> Result;
	Result.GetBuffer(1)[0] = '\0';
	Result.ReleaseBuffer(0); // set the string to ""; we can't do it in a usual way (using a constructor or an assignment) because we don't know whether "" needs to be unicode or ansi
	while (p = ( T* )My_strstr(pCurPos, szFind))
	{
		Result.DiffCat(pCurPos, p);
		Result += szReplaceBy;
		pCurPos = p + FindLen;
	}
	Result += pCurPos;
	*this = Result;
	return *this;
}


template <class T>
TString<T>& TString<T>::Replace(int nIndex, int nCount, const T *szReplaceBy)
{
	if (!pBuf || !szReplaceBy || nIndex < 0 || nCount < 0)
	{
		return *this;
	}

	TString<T> Result;
	Result.GetBuffer(1)[0] = '\0';
	Result.ReleaseBuffer(0); // set the string to ""; we can't do it in a usual way (using a constructor or an assignment) because we don't know whether "" needs to be unicode or ansi
	if (nIndex > GetLen())
	{
		nIndex = GetLen();
	}
	if (nIndex + nCount > GetLen())
	{
		nCount = GetLen() - nIndex;
	}
	Result.DiffCat(pBuf, pBuf + nIndex);
	Result += szReplaceBy;
	Result += pBuf + nIndex + nCount;
	*this = Result;
	return *this;
}


template <class T>
TString<T> TString<T>::Left(int nCount) const
{
	_ASSERT(nCount >= 0);
	TString<T> Result(*this);
	Result.SetBufSize(nCount + 1);
	return Result;
}


template <class T>
TString<T> TString<T>::Right(int nCount) const
{
	_ASSERT(nCount >= 0);
	if (nCount < GetLen())
	{
		return &pBuf[GetLen() - nCount];
	} else
	{
		return *this;
	}
}


template <class T>
TString<T> TString<T>::SubStr(int nIndex, int nCount) const
{
	_ASSERT(nIndex >= 0 && nCount >= 0);
	TString<T> Result;
	if (nIndex < GetLen())
	{
		My_strncpy(Result.GetBuffer(nCount), &pBuf[nIndex], nCount);
		Result.ReleaseBuffer();
	} else
	{
		Result.GetBuffer(1)[0] = '\0';
		Result.ReleaseBuffer(0);
	}
	return Result;
}


template <class T>
TString<T> TString<T>::ToLower() const
{
	TString<T> Result(*this);
	if (!pBuf)
	{
		return Result; // return NULL string
	}
	My_strlwr((T*)Result);
	return Result;
}


template <class T>
TString<T>& TString<T>::operator = (const T *pStr)
{
	if (pStr)
	{
		int StrLen = My_lstrlen(pStr);
		SetBufSize(StrLen + 1);
		My_lstrcpy(GetBuffer(), pStr);
		ReleaseBuffer(StrLen);
	} else
	{
		Free();
	}
	return *this;
}

template class TString<TCHAR>;
template class TString<char>;
template class TString<WCHAR>;

CString db_get_s(MCONTACT hContact, const char *szModule, const char *szSetting, const char *szDefaultValue)
{
	ptrA p( db_get_sa(hContact, szModule, szSetting));
	return CString(p == NULL ? szDefaultValue : p);
}

TCString db_get_s(MCONTACT hContact, const char *szModule, const char *szSetting, const TCHAR *szDefaultValue)
{
	ptrT p( db_get_tsa(hContact, szModule, szSetting));
	return TCString(p == NULL ? szDefaultValue : p);
}

TCString DBGetContactSettingAsString(MCONTACT hContact, const char *szModule, const char *szSetting, const TCHAR *szDefaultValue)
{ // also converts numeric values to a string
	DBVARIANT dbv = {0};
	int iRes = db_get_ws(hContact, szModule, szSetting, &dbv);

	TCString Result;
	if (!iRes && (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR))
	{
		Result = dbv.ptszVal;
	}
	else if (dbv.type == DBVT_BYTE || dbv.type == DBVT_WORD || dbv.type == DBVT_DWORD)
	{
		long value = (dbv.type == DBVT_DWORD) ? dbv.dVal : (dbv.type == DBVT_WORD ? dbv.wVal : dbv.bVal);
		_ultot(value, Result.GetBuffer(64), 10);
		Result.ReleaseBuffer();
	}
	else Result = szDefaultValue;

	if (!iRes)
		db_free(&dbv);

	return Result;
}
