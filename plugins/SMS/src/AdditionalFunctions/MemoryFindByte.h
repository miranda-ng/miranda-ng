#if !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
#define AFX_MEMORYFINDBYTE__H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

__inline LPVOID MemoryFindByte(SIZE_T dwFrom,LPCVOID lpcSource,SIZE_T dwSourceSize,unsigned char chWhatFind)
{
	if (lpcSource && dwSourceSize)
		if (dwFrom < dwSourceSize)
			return memchr((BYTE*)lpcSource + dwFrom, chWhatFind, dwSourceSize-dwFrom);

	return NULL;
}

#endif // !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
