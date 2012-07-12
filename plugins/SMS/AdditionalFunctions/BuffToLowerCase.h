#if !defined(AFX_BUFFTOLOWERCASE__H__INCLUDED_)
#define AFX_BUFFTOLOWERCASE__H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



__inline DWORD BuffToLowerCase(LPCVOID lpcOutBuff,LPCVOID lpcBuff,SIZE_T dwLen)
{
	DWORD dwRetErrorCode=NO_ERROR;

	__asm
	{
		mov		ecx,dwLen
		test	ecx,ecx
		jz		short end_func

		push	ebx					// сохраняем регистр
		push	edi					// сохраняем регистр
		push	esi					// сохраняем регистр
		mov		esi,lpcBuff
		mov		edi,lpcOutBuff
		mov		bl,'A'
		mov		bh,'Z'
		mov		ah,32
		cld

	lowcaseloop:
		lodsb
		cmp		al,bl
		jl		short savebyte
		cmp		al,bh
		jg		short savebyte
		or		al,ah

	savebyte:
		stosb

		dec		ecx
		jnz		short lowcaseloop

		pop		esi					// восстанавливаем содержимое регистра
		pop		edi					// восстанавливаем содержимое регистра
		pop		ebx					// восстанавливаем содержимое регистра
	end_func:
	}

return(dwRetErrorCode);
}




#endif // !defined(AFX_BUFFTOLOWERCASE__H__INCLUDED_)