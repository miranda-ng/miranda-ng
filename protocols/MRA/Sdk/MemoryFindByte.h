/*
 * Copyright (c) 2003 Rozhuk Ivan <rozhuk.im@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */



#if !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
#define AFX_MEMORYFINDBYTE__H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



__inline void *MemoryFindByte(size_t dwFrom,const void *pBuff,size_t dwBuffSize,unsigned char chWhatFind)
{
	void *pRet=NULL;

	if (pBuff && dwBuffSize && dwFrom<dwBuffSize)
	{
		pRet=(void*)memchr((const void*)(((size_t)pBuff)+dwFrom),chWhatFind,(dwBuffSize-dwFrom));
	}
return(pRet);
}


#ifndef  _WIN64
__inline void *MemoryFindByteReverse(size_t dwFrom,const void *pBuff,size_t dwBuffSize,unsigned char chWhatFind)
{
	void *pRet=NULL;

	__asm
	{
		push	ebx				// сохраняем регистр
		push	edi				// сохраняем регистр
		push	esi				// сохраняем регистр

		mov		ecx,dwBuffSize
		test	ecx,ecx			//; проверка входного параметра, он !=0
		je		short end_func

		mov		edi,pBuff		//; di = string
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
		mov		pRet,edi		//; ax=pointer to byte
	end_func:

		cld
		pop		esi				// восстанавливаем содержимое регистра
		pop		edi				// восстанавливаем содержимое регистра
		pop		ebx				// восстанавливаем содержимое регистра
	}
return(pRet);
}
#endif


#endif // !defined(AFX_MEMORYFINDBYTE__H__INCLUDED_)
