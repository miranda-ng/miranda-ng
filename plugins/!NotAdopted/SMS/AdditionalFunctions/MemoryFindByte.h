#if !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
#define AFX_MEMORYFINDBYTE__H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



__inline LPVOID MemoryFindByte(SIZE_T dwFrom,LPCVOID lpcSource,SIZE_T dwSourceSize,unsigned char chWhatFind)
{
	LPVOID lpRet=NULL;

	if (lpcSource && dwSourceSize)
	{
		if (dwFrom<dwSourceSize)
		{
			lpRet=memchr((LPCVOID)(((SIZE_T)lpcSource)+dwFrom),chWhatFind,(dwSourceSize-dwFrom));
		}
	}
return(lpRet);
}



__inline LPVOID MemoryFindByteReverse(SIZE_T dwFrom,LPCVOID lpcSource,SIZE_T dwSourceSize,unsigned char chWhatFind)
{
	LPVOID lpRet=NULL;

	__asm
	{
		push	ebx				// сохраняем регистр
		push	edi				// сохраняем регистр
		push	esi				// сохраняем регистр

		mov		ecx,dwSourceSize
		test	ecx,ecx			//; проверка входного параметра, он !=0
		je		short end_func

		mov		edi,lpcSource	//; di = string
		test	edi,edi			//; проверка входного параметра, он !=0
		jz		short end_func

		mov		eax,dwFrom

/////////////////////////////////////////////
		cmp		eax,ecx			//; проверка ecx(=len)=>dwFrom
		jae		short end_func

		std						//; count 'up' on string this time
		sub		ecx,eax			//; уменьшаем длинну на dwFrom(нач смещен)
		add		edi,ecx			//; сдвигаем начало на dwSourceSize(на конец)
		mov		al,chWhatFind	//; al=search byte
 		repne	scasb			//; find that byte
		inc		edi				//; di points to byte which stopped scan
		cmp		[edi],al		//; see if we have a hit
		jne		short end_func	//; yes, point to byte
		mov		lpRet,edi		//; ax=pointer to byte
	end_func:

		cld
		pop		esi				// восстанавливаем содержимое регистра
		pop		edi				// восстанавливаем содержимое регистра
		pop		ebx				// восстанавливаем содержимое регистра
	}
return(lpRet);
}



#endif // !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
