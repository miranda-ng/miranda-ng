#if !defined(AFX_MEMORYFIND__H__INCLUDED_)
#define AFX_MEMORYFIND__H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

__inline LPVOID MemoryFind(SIZE_T dwFrom,LPCVOID lpcSource,SIZE_T dwSourceSize,LPCVOID lpcWhatFind,SIZE_T dwWhatFindSize)
{
	LPVOID lpRet=NULL;

	__asm
	{
		push	ebx					// сохраняем регистр
		push	edi					// сохраняем регистр
		push	esi					// сохраняем регистр

		mov			ecx,dwSourceSize	//; ecx = Source string Size
		test		ecx,ecx				// is size unknown?
		jz			short end_func

		mov			edx,dwWhatFindSize	//; edx = WhatFind string Size
		test		edx,edx				// is size unknown?
		jz			short end_func

		mov			ebx,dwFrom			// ebx - start pos in Source string
		mov			edi,lpcSource		//; edi = Source string
		mov			esi,lpcWhatFind		//; esi = WhatFind string

		cmp			ebx,ecx				// проверка ecx(=len)=>ulFrom
		jae			short end_func

		add			edi,ebx				// сдвигаем начало на ulFrom(нач смещен)
		sub			ecx,ebx				// уменьшаем длинну SourceSize на ulFrom(нач смещен)

		cmp			ecx,edx				// проверка NEWSourceSize ??? ulWhatFindSize
		je			short begin_memorycompare	// NEWulSourceSize==ulWhatFindSize, Source ??? WhatFind
		jl			short end_func		// NEWulSourceSize<ulWhatFindSize, => Source!=WhatFind

		sub			ecx,edx				// уменьшаем длинну SourceSize на ulWhatFindSize
		inc			ecx

        mov			al,[esi]			//; al=search byte
		dec			edi
		cld								//; сканируя в прямом направлении

	find_loop:
		test		ecx,ecx
		jz			short end_func
		inc			edi
		repne		scasb				//; find that byte
        dec			edi					//; di points to byte which stopped scan

        cmp			[edi],al			//; see if we have a hit
        jne			short end_func		//; yes, point to byte

	begin_memorycompare:
		push		esi
		push		edi
		push		ecx
		mov			ecx,edx				//;	ulWhatFindSize байтов (CX используется в REPE),
		repe		cmpsb				//;	сравниваем их.
		pop			ecx
		pop			edi
		pop			esi
		jne			short find_loop		//; признак ZF = 0, если сравниваемые
										//; строки не совпадают (mismatch) match:
										//; если мы попали сюда, значит, они
										//; совпадают (match)
		mov			lpRet,edi			//; ax=pointer to byte
	end_func:

		pop		esi					// восстанавливаем содержимое регистра
		pop		edi					// восстанавливаем содержимое регистра
		pop		ebx					// восстанавливаем содержимое регистра
	}
return(lpRet);
}


#endif // !defined(AFX_MEMORYFIND__H__INCLUDED_)
