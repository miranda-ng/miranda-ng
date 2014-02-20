/*
	TMyArray.h - TMyArray template
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

#include <stdlib.h>

#define ARRAY_GROWBY 4
// if there is more than ARRAY_FREETHRESHOLD free array elements, then we're reallocating the array
#define ARRAY_FREETHRESHOLD 16

template <class T, class ARG_T = const T&, int GrowBy = ARRAY_GROWBY, int FreeThreshold = ARRAY_FREETHRESHOLD>
class TMyArray
{
public:
	TMyArray();
	TMyArray(const TMyArray<T, ARG_T, GrowBy, FreeThreshold> &A);
	~TMyArray();

	int GetSize() const;
	T* GetData() const;
	int AddElem(ARG_T pElem);
	int Add(const T *pItems, int nItems);
	void InsertElem(ARG_T pElem, int nInsertAt);
	void MoveElem(int nIndex, int nMoveTo);
	T& SetAtGrow(int nIndex);
	int Find(ARG_T pElem); // returns an index of the specified item, or -1 if the array doesn't contain the item
	int BinarySearch(int (*CmpFunc)(ARG_T pItem, LPARAM lParam), LPARAM lParam, int *pIndex = NULL); // returns an index of the specified item, or -1 if the array doesn't contain the item;
	// also it's possible to specify pIndex so that even if the array doesn't contain the item, the function will set *pIndex to a position where the item can be inserted;
	// CmpFunc must return -1, 0 and 1 depending whether pItem is lesser, equal or greater than needed;
	// BinarySearch() requires the array to be sorted in an ascending order
	void RemoveElem(int nIndex, int nItems = 1);
	void RemoveAll();
	const T& operator[](int nIndex) const;
	T& operator[](int nIndex);
	TMyArray<T, ARG_T, GrowBy, FreeThreshold>& operator = (const TMyArray<T, ARG_T, GrowBy, FreeThreshold> &A);
	TMyArray<T, ARG_T, GrowBy, FreeThreshold>& operator += (const TMyArray<T, ARG_T, GrowBy, FreeThreshold> &A);

private:
	int SetAllocNum(int nNewAllocNum);

	T* pData;
	int nElemNum;
	int nAllocNum;
};

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
TMyArray<T, ARG_T, GrowBy, FreeThreshold>::TMyArray()
{
	nElemNum = 0;
	nAllocNum = 0;
	pData = NULL;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
TMyArray<T, ARG_T, GrowBy, FreeThreshold>::TMyArray(const TMyArray<T, ARG_T, GrowBy, FreeThreshold> &A)//: TMyArray<T, ARG_T>()
{
	nElemNum = 0;
	nAllocNum = 0;
	pData = NULL;
	*this = A;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
TMyArray<T, ARG_T, GrowBy, FreeThreshold>::~TMyArray()
{
	RemoveAll();
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline int TMyArray<T, ARG_T, GrowBy, FreeThreshold>::GetSize() const
{
	return nElemNum;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline T* TMyArray<T, ARG_T, GrowBy, FreeThreshold>::GetData() const
{
	return pData;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline int TMyArray<T, ARG_T, GrowBy, FreeThreshold>::SetAllocNum(int nNewAllocNum)
{
	_ASSERT(nNewAllocNum >= nElemNum);
	T*pNewData = (nNewAllocNum) ? (T*)malloc(sizeof(T) * nNewAllocNum) : NULL;
	if (pData)
	{
		if (pNewData)
		{
			memcpy(pNewData, pData, sizeof(T) * nElemNum);
		}
		free(pData);
	}
	pData = pNewData;
	if (!pNewData)
	{
		nAllocNum = 0;
		return -1; // not enough memory?
	} else
	{
		nAllocNum = nNewAllocNum;
		return 0; // everything's ok
	}
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline int TMyArray<T, ARG_T, GrowBy, FreeThreshold>::AddElem(ARG_T pElem)
{
	if (nElemNum >= nAllocNum)
	{ // reallocate memory to fit new element
		SetAllocNum(nAllocNum + GrowBy);
	}
	memset(pData + nElemNum, 0, sizeof(T));
	pData[nElemNum] = pElem;
	return nElemNum++;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline int TMyArray<T, ARG_T, GrowBy, FreeThreshold>::Add(const T *pItems, int nItems)
{
	if (nElemNum + nItems > nAllocNum)
	{ // reallocate memory to fit new items
		SetAllocNum(nAllocNum + nElemNum + nItems - 1 - ((nElemNum + nItems - 1) % GrowBy) + GrowBy);
	}
	memset(pData + nElemNum, 0, sizeof(T) * nItems);
	int I;
	for (I = 0; I < nItems; I++)
	{
    pData[nElemNum++] = pItems[I];
	}
	return nElemNum - nItems; // returns an index of the first item added
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline void TMyArray<T, ARG_T, GrowBy, FreeThreshold>::InsertElem(ARG_T pElem, int nInsertAt)
{
	_ASSERT(nInsertAt >= 0 && nInsertAt <= nElemNum);
	if (nElemNum >= nAllocNum)
	{ // reallocate memory to fit new items
		SetAllocNum(nAllocNum + GrowBy);
	}
	memmove(pData + nInsertAt + 1, pData + nInsertAt, sizeof(T) * (nElemNum - nInsertAt));
	memset(pData + nInsertAt, 0, sizeof(T));
	pData[nInsertAt] = pElem;
	nElemNum++;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline void TMyArray<T, ARG_T, GrowBy, FreeThreshold>::MoveElem(int nIndex, int nMoveTo)
{
	_ASSERT(nIndex >= 0 && nIndex < nElemNum && nMoveTo >= 0 && nMoveTo < nElemNum);
	if (nIndex == nMoveTo)
	{
		return; // nothing to do
	}
	char Elem[sizeof(T)];
	memmove(Elem, pData + nIndex, sizeof(T));
	if (nIndex < nMoveTo)
	{
		memmove(pData + nIndex, pData + nIndex + 1, sizeof(T) * (nMoveTo - nIndex));
	} else
	{
		memmove(pData + nMoveTo + 1, pData + nMoveTo, sizeof(T) * (nIndex - nMoveTo));
	}
	memmove(pData + nMoveTo, Elem, sizeof(T));
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline T& TMyArray<T, ARG_T, GrowBy, FreeThreshold>::SetAtGrow(int nIndex)
{
	_ASSERT(nIndex >= 0);
	if (nIndex < nElemNum)
	{
		return pData[nIndex];
	}
	if (nIndex >= nAllocNum)
	{
		if (SetAllocNum(nIndex - (nIndex % GrowBy) + GrowBy))
		{ // if there was an error
			nElemNum = 0;
			return *pData;
		}
	}
	memset(pData + nElemNum, 0, sizeof(T) * (nIndex - nElemNum + 1));
	nElemNum = nIndex + 1;
	return pData[nIndex];
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline int TMyArray<T, ARG_T, GrowBy, FreeThreshold>::Find(ARG_T pElem)
{
	int I;
	for (I = 0; I < nElemNum; I++)
	{
		if (pData[I] == pElem)
		{
			return I;
		}
	}
	return -1;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline int TMyArray<T, ARG_T, GrowBy, FreeThreshold>::BinarySearch(int (*CmpFunc)(ARG_T pItem, LPARAM lParam), LPARAM lParam, int *pIndex)
{
	int L, R;
	int CmpResult;
	if (!nElemNum)
	{
		if (pIndex)
		{
			*pIndex = -1;
		}
		return -1;
	}
	for (L = 0, R = nElemNum; R - L > 1;)
	{
		int C = (L + R) >> 1; // rounds always to a lesser index if L + R is odd
		CmpResult = CmpFunc(pData[C], lParam); // usually, CmpFunc = pData[C] - lParam; i.e. CmpFunc > 0 when pData[C] is greater than necessary, < 0 when pData[C] is less, and = 0 when pData[C] is the item we search for
		if (CmpResult < 0)
		{
			L = C;
		} else if (CmpResult > 0)
		{
			R = C;
		} else
		{ // CmpResult == 0
			if (pIndex)
			{
				*pIndex = C;
			}
			return C;
		}
	}
	if (pIndex)
	{
		*pIndex = L;
	}
	CmpResult = CmpFunc(pData[L], lParam);
	if (!CmpResult)
	{
		return L;
	}
	if (CmpResult > 0)
	{ // we don't need to check pData[R], as pData[R] > pData[L] > lParam, i.e. there are no suitable item in the array
		return -1;
	}
// CmpResult < 0, we have to check pData[R] too
	if (pIndex)
	{
		*pIndex = R;
	}
	if (R >= nElemNum)
	{
		return -1;
	}
	return CmpFunc(pData[R], lParam) ? -1 : R;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline void TMyArray<T, ARG_T, GrowBy, FreeThreshold>::RemoveElem(int nIndex, int nItems)
{
	_ASSERT(nIndex >= 0 && nIndex + nItems <= nElemNum);
	if (!nItems)
	{
		return;
	}
//	delete pData[nIndex];
//	~pData[nIndex];
	int I;
	for (I = nIndex; I < nIndex + nItems; I++)
	{
		(pData + I)->~T();
	}
	memmove(pData + nIndex, pData + nIndex + nItems, sizeof(T) * (nElemNum - nIndex - nItems));
	nElemNum -= nItems;
	if (nAllocNum - nElemNum >= FreeThreshold)
	{
		SetAllocNum(nAllocNum - FreeThreshold);
	}/* else
	{
		memset(pData + nElemNum, 0, sizeof(T));
	}*/
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline void TMyArray<T, ARG_T, GrowBy, FreeThreshold>::RemoveAll()
{
	int I;
	for (I = 0; I < nElemNum; I++)
	{
		//delete pData[I];
		(pData + I)->~T();
	}
	nElemNum = 0;
	SetAllocNum(0);
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline const T& TMyArray<T, ARG_T, GrowBy, FreeThreshold>::operator[](int nIndex) const
{
	_ASSERT(nIndex >= 0 && nIndex < nElemNum);
	return pData[nIndex];
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline T& TMyArray<T, ARG_T, GrowBy, FreeThreshold>::operator[](int nIndex)
{
	_ASSERT(nIndex >= 0 && nIndex < nElemNum);
	return pData[nIndex];
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline TMyArray<T, ARG_T, GrowBy, FreeThreshold>& TMyArray<T, ARG_T, GrowBy, FreeThreshold>::operator = (const TMyArray<T, ARG_T, GrowBy, FreeThreshold> &A)
{
	RemoveAll();
	int I;
	for (I = 0; I < A.GetSize(); I++)
	{
		AddElem(A[I]);
	}
	return *this;
}

template <class T, class ARG_T, int GrowBy, int FreeThreshold>
__forceinline TMyArray<T, ARG_T, GrowBy, FreeThreshold>& TMyArray<T, ARG_T, GrowBy, FreeThreshold>::operator += (const TMyArray<T, ARG_T, GrowBy, FreeThreshold> &A)
{
	int I;
	for (I = 0; I < A.GetSize(); I++)
	{
		AddElem(A[I]);
	}
	return *this;
}

typedef TMyArray<char, const char&, 1024, 4096> CHARARRAY;
