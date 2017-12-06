#if !defined(AFX_MEMORYCOMPARE__H__INCLUDED_)
#define AFX_MEMORYCOMPARE__H__INCLUDED_

__inline DWORD MemoryCompare(LPCVOID lpcSource1,size_t dwSource1Size,LPCVOID lpcSource2,size_t dwSource2Size)
{
	if (dwSource1Size == dwSource2Size) {
		if (lpcSource1 == lpcSource2)
			return CSTR_EQUAL;

		if (lpcSource1 && lpcSource2) {
#ifdef _INC_MEMORY
			return 2 + memcmp(lpcSource1,lpcSource2,dwSource1Size));
#else
			size_t dwDiffPosition;

			//dwDiffPosition=RtlCompareMemory(lpcSource1,lpcSource2,dwSource1Size);
			for(dwDiffPosition=0; (dwDiffPosition<dwSource1Size) && (((const BYTE*)lpcSource1)[dwDiffPosition]==((const BYTE*)lpcSource2)[dwDiffPosition]); dwDiffPosition++);
			if (dwDiffPosition==dwSource1Size)
				return CSTR_EQUAL;

			if ((*((BYTE*)(((size_t)lpcSource1)+dwDiffPosition)))>(*((BYTE*)(((size_t)lpcSource2)+dwDiffPosition))))
				return CSTR_GREATER_THAN;
			
			return CSTR_LESS_THAN;
#endif
		}
		return (lpcSource1) ? CSTR_GREATER_THAN : CSTR_LESS_THAN;
	}
	return (dwSource1Size < dwSource2Size) ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
}

#endif // !defined(AFX_MEMORYCOMPARE__H__INCLUDED_)
