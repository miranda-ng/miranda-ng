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

#if !defined(AFX__SHA1_H__INCLUDED_)
#define AFX__SHA1_H__INCLUDED_

#pragma once

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//////////////////////////////RFC 2104//////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

__inline DWORD BSWAP(DWORD dwIn)
{
	return((((dwIn<<8) & 0x00ff0000) | (dwIn<<24) | ((dwIn>>8) & 0x0000ff00) | (dwIn>>24)));
}

__inline void CopyMemoryReverseDWORD(LPCVOID lpcDestination,LPCVOID lpcSource,size_t dwSize)
{
#ifdef  _WIN64
	BYTE *pDestination=(BYTE*)lpcDestination,*pSource=(BYTE*)lpcSource;

	//for(size_t i=0;i<dwSize;i++) pDestination[i]=pSource[(i&~0x00000003)+(3-(i&0x00000003))];
	for(size_t i=0;i<dwSize;i+=4) (*((DWORD*)(pDestination+i)))=BSWAP((*((DWORD*)(pSource+i))));

#else
	__asm{
		push	edi					// сохраняем регистр
		push	esi					// сохраняем регистр
	
		mov		ecx,dwSize			// ecx = длинна входного буффера
		mov     edi,lpcDestination	// edi = адрес выходного буффера
		mov     esi,lpcSource		// esi = указатель на входной буффер
		cld

	read_loop:
		lodsd						// читаем 4 байта
		bswap   eax
		stosd
		sub     ecx,4
		jg		short read_loop		// если длинна 3 и более байт, то продолжаем дальше
		
		pop		esi					// восстанавливаем содержимое регистра
		pop		edi					// восстанавливаем содержимое регистра
	}
#endif
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

__inline void SHA1GetDigest(LPVOID lpBuff,size_t dwBuffSize,BYTE *digest)
{
	mir_sha1_ctx sha;

	mir_sha1_init(&sha);
	mir_sha1_append(&sha,(BYTE*)lpBuff,(int)dwBuffSize);
	mir_sha1_finish(&sha,digest);
}

#endif //AFX__SHA1_H__INCLUDED_
