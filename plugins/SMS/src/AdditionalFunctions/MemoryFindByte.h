#if !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
#define AFX_MEMORYFINDBYTE__H__INCLUDED_

__inline LPVOID MemoryFindByte(size_t dwFrom,LPCVOID lpcSource,size_t dwSourceSize,unsigned char chWhatFind)
{
	if (lpcSource && dwSourceSize)
		if (dwFrom < dwSourceSize)
			return memchr((BYTE*)lpcSource + dwFrom, chWhatFind, dwSourceSize-dwFrom);

	return NULL;
}

#endif // !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
