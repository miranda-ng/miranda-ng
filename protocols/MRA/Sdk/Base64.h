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


#if !defined(AFX_BASE64__H__INCLUDED_)
#define AFX_BASE64__H__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



//typedef unsigned char BYTE;
//
//      BASE64 coding:
//      214             46              138
//      11010100        00101110        10001010
//            !             !             !
//      ---------->>> convert 3 8bit to 4 6bit
//      110101  000010  111010  001010
//      53      2       58      10
//      this numbers is offset in array coding below...
//
						   //01234567890123456789012345
static BYTE *pbtCodingTableBASE64=(BYTE*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";                   //52...63
static BYTE btDeCodingTableBASE64[256]={64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64};




__inline void BASE64CopyUnSafe(LPCVOID lpcSource,LPCVOID lpcDestination,SIZE_T dwSize,SIZE_T *pdwCopyedSize)
{// копируем только б64 символы
	LPBYTE lpbSource,lpbDestination;

	lpbSource=(LPBYTE)lpcSource;
	lpbDestination=(LPBYTE)lpcDestination;
	while(dwSize--)
	{
		if ((*lpbSource)>32 && (*lpbSource)<128) (*lpbDestination++)=(*lpbSource);
		lpbSource++;
	}
	if (pdwCopyedSize) (*pdwCopyedSize)=((SIZE_T)lpbDestination-(SIZE_T)lpcDestination);
}



__inline DWORD BASE64EncodeUnSafe(LPCVOID lpcSource,SIZE_T dwSourceSize,LPCVOID lpcDestination,SIZE_T dwDestinationSize,SIZE_T *pdwEncodedSize)
{// BASE64 кодирование
	DWORD dwRetErrorCode;
	SIZE_T dwEncodedSize=((dwSourceSize*4+11)/12*4+1);//(((dwSourceSize+2)/3)*4);
		
	if ((dwDestinationSize<dwEncodedSize))
	{// выходной буффер слишком мал
		dwRetErrorCode=ERROR_INSUFFICIENT_BUFFER;
	}else{// размер выходного буффера достаточен
		dwEncodedSize=0;
		if(lpcSource && lpcDestination && dwSourceSize)
		{
#ifdef  _WIN64
			LPBYTE lpbtSource=(LPBYTE)lpcSource,lpbtDestination=(LPBYTE)lpcDestination;
			SIZE_T i;

			for (i=0;i<dwSourceSize;i+=3)
			{
				*(lpbtDestination++)=pbtCodingTableBASE64[(*lpbtSource)>>2];									// c1
				*(lpbtDestination++)=pbtCodingTableBASE64[(((*lpbtSource)<<4)&060) | ((lpbtSource[1]>>4)&017)];	// c2
				*(lpbtDestination++)=pbtCodingTableBASE64[((lpbtSource[1]<<2)&074) | ((lpbtSource[2]>>6)&03)];	// c3
				*(lpbtDestination++)=pbtCodingTableBASE64[lpbtSource[2] & 077];									// c4
				lpbtSource+=3;
			}

			// If dwSourceSize was not a multiple of 3, then we have encoded too many characters.  Adjust appropriately.
			if(i==(dwSourceSize+1))
			{// There were only 2 bytes in that last group
				lpbtDestination[-1]='=';
			}else
			if(i==(dwSourceSize+2))
			{// There was only 1 byte in that last group
				lpbtDestination[-1]='=';
				lpbtDestination[-2]='=';
			}

			(*lpbtDestination)=0;
			dwEncodedSize=(lpbtDestination-((LPBYTE)lpcDestination));

#else
		__asm{
			push	ebx					// сохраняем регистр
			push	edi					// сохраняем регистр
			push	esi					// сохраняем регистр
		
			mov		ebx,pbtCodingTableBASE64// ebx = адрес таблицы перекодировки
			mov		ecx,dwSourceSize	// ecx = длинна входного буффера
			mov     edi,lpcDestination	// edi = адрес выходного буффера
			mov     esi,lpcSource		// esi = указатель на входной буффер
			cld
			jmp		short read_loop_cmp

		//////////Code function///////////////////////////////////////////
		// функция кодировки
		// eax - входящий буффер, используется только 3 байта //in buff, 3 byte used
		// eax - выходящий буффер, используется 4 байта //out buff, 4 byte used
		code_func:
			// третья версия функции разложения,
			// меняем местами два крайних байта
			bswap   eax
			rol     eax,6
			shl     al,2
			ror     eax,10
			shr     ax,2
			shr     al,2
			xlat
			rol     eax,8
			xlat
			rol     eax,8
			xlat
			rol     eax,8
			xlat
			rol     eax,8
			bswap   eax// 188-235					
			ret
		//////////////////////////////////////////////////////////////////

		/////////////Read & converting cycle//////////////////////////////
		read_loop:
			lodsd						// читаем 4 байта
			dec     esi					// используем только 3
			and		eax,0x00FFFFFF
		//====================================================
			// третья версия функции разложения,
			// меняем местами два крайних байта
			bswap   eax
			rol     eax,6
			shl     al,2
			ror     eax,10
			shr     ax,2
			shr     al,2
			xlat
			rol     eax,8
			xlat
			rol     eax,8
			xlat
			rol     eax,8
			xlat
			rol     eax,8
			bswap   eax
		//====================================================
			stosd
			sub     ecx,3
		
		read_loop_cmp:
			cmp		ecx,3				// проверяем, чтобы длинна была как минимум 4 байта
			jg		short read_loop		// если длинна 3 и более байт, то продолжаем дальше
			
		/////////////////////////////////////////////////////////////////
			xor		eax,eax				// обнуляем регистр
			cmp		ecx,3				// сравниваем длинну с 3
			je		short tree_byte_tail// если длинна 3 байта, то переходим к функции обрабатывающей остаток такой длинны
			cmp		ecx,2				// сравниваем длинну с 2
			je		short two_byte_tail	// если длинна 2 байта, то переходим к функции обрабатывающей остаток такой длинны
											// иначе, длинна остатка равна 1
		//////////tail 1 byte////////////////////////////////////////////
			mov		al,byte ptr [esi]	// читаем 1 байт
			call	code_func			// преобразуем
			and		eax,0x0000FFFF		// обнуляем последние два байта
			or		eax,0x3D3D0000		// записываем в последние два байта 61("=")
			jmp		short end_tail_handler	//

		//////////tail 2 byte////////////////////////////////////////////
		two_byte_tail:
			mov		ax,word ptr [esi]	// читаем 2 байта
			call	code_func			// преобразуем
			and		eax,0x00FFFFFF		// обнуляем последний байт
			or		eax,0x3D000000		// записываем в последний байт 61("=")
			jmp		short end_tail_handler	//
			
		//////////tail 3 byte////////////////////////////////////////////
		tree_byte_tail:
			lodsw
			ror		eax,16
			mov		al,byte ptr [esi]	// читаем 1 байт
			rol		eax,16
			call	code_func			// преобразуем

		end_tail_handler:
			stosd						// записываем 4 байта, уже закодированных

			sub		edi,lpcDestination	// вычисляем колличество байт, записанных в выходной буффер
			mov		dwEncodedSize,edi	// записываем колличество байт в возвращаемую переменную
			pop		esi					// восстанавливаем содержимое регистра
			pop		edi					// восстанавливаем содержимое регистра
			pop		ebx					// восстанавливаем содержимое регистра
			}
#endif
			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=ERROR_INVALID_HANDLE;
		}
	}
	if (pdwEncodedSize) (*pdwEncodedSize)=dwEncodedSize;

return(dwRetErrorCode);
}


__inline DWORD BASE64Encode(LPCVOID lpcSource,SIZE_T dwSourceSize,LPCVOID lpcDestination,SIZE_T dwDestinationSize,SIZE_T *pdwEncodedSize)
{// BASE64 кодирование
	DWORD dwRetErrorCode;

	__try
	{
		dwRetErrorCode=BASE64EncodeUnSafe(lpcSource,dwSourceSize,lpcDestination,dwDestinationSize,pdwEncodedSize);
	}__except(EXCEPTION_EXECUTE_HANDLER){
		dwRetErrorCode=ERROR_INVALID_ADDRESS;
	}
return(dwRetErrorCode);
}



__inline DWORD BASE64DecodeUnSafe(LPCVOID lpcSource,SIZE_T dwSourceSize,LPCVOID lpcDestination,SIZE_T dwDestinationSize,SIZE_T *pdwDecodedSize)
{// BASE64 декодирование
	DWORD dwRetErrorCode;
	SIZE_T dwDecodedSize=((dwSourceSize>>2)*3);// ((dwSourceSize/4)*3);

	if ((dwDestinationSize<dwDecodedSize))
	{// выходной буффер слишком мал
		dwRetErrorCode=ERROR_INSUFFICIENT_BUFFER;
	}else{// размер выходного буффера достаточен
		dwDecodedSize=0;
		if(lpcSource && lpcDestination)
		{// буффера в порядке
			if (dwSourceSize>3)
			{
#ifdef  _WIN64
				LPBYTE lpbtSource=(LPBYTE)lpcSource,lpbtDestination=(LPBYTE)lpcDestination;

				for(SIZE_T i=0;i<dwSourceSize;i+=4)
				{
					*(lpbtDestination++)=(unsigned char) (btDeCodingTableBASE64[(*lpbtSource)] << 2 | btDeCodingTableBASE64[lpbtSource[1]] >> 4);
					*(lpbtDestination++)=(unsigned char) (btDeCodingTableBASE64[lpbtSource[1]] << 4 | btDeCodingTableBASE64[lpbtSource[2]] >> 2);
					*(lpbtDestination++)=(unsigned char) (btDeCodingTableBASE64[lpbtSource[2]] << 6 | btDeCodingTableBASE64[lpbtSource[3]]);
					lpbtSource+=4;
				}

				dwDecodedSize=(lpbtDestination-((LPBYTE)lpcDestination));
				if((*((BYTE*)lpcSource+(dwSourceSize-1)))=='=') dwDecodedSize--;
				if((*((BYTE*)lpcSource+(dwSourceSize-2)))=='=') dwDecodedSize--;


#else
			__asm{
				push	ebx					// сохраняем регистр
				push	edi					// сохраняем регистр
				push	esi					// сохраняем регистр
				
				mov		ebx,offset btDeCodingTableBASE64// ebx = адрес таблицы перекодировки
				mov		ecx,dwSourceSize	// ecx = длинна входного буффера
				mov     edi,lpcDestination	// edi = адрес выходного буффера
				mov     esi,lpcSource		// esi = указатель на входной буффер
				cld

			read_loop:
				lodsd						// читаем 4 байта
			//===============bit conversion====================================
			// eax - входящий буффер, используется только 4 байта //in buff, 4 byte used
			// eax - выходящий буффер, используется только 3 байта //out buff, 3 byte used
				// третья версия функции сложения бит
				bswap   eax

				ror     eax,8
				xlat

				ror     eax,8
				xlat

				ror     eax,8
				xlat

				ror     eax,8
				xlat

				shl     al,2
				shl     ax,2
				rol     eax,10
				shr     al,2
				ror     eax,6
				bswap   eax
				mov		edx,eax
												//234-250
			//=============================================================== 
				stosd
				dec		edi
				sub     ecx,4
				cmp		ecx,3
				jg		short read_loop

				sub		edi,lpcDestination	// вычисляем колличество байт, записанных в выходной буффер
				mov		dwDecodedSize,edi	// записываем колличество байт в возвращаемую переменную
				pop		esi					// восстанавливаем содержимое регистра
				pop		edi					// восстанавливаем содержимое регистра
				pop		ebx					// восстанавливаем содержимое регистра
				}

				if((*((BYTE*)lpcSource+(dwSourceSize-1)))=='=') dwDecodedSize--;
				if((*((BYTE*)lpcSource+(dwSourceSize-2)))=='=') dwDecodedSize--;
#endif
				dwRetErrorCode=NO_ERROR;
			}else{// во входном буффере слишком мало данных
				dwRetErrorCode=ERROR_INSUFFICIENT_BUFFER;
			}
		}else{
			dwRetErrorCode=ERROR_INVALID_HANDLE;
		}
	}

	if (pdwDecodedSize) (*pdwDecodedSize)=dwDecodedSize;
return(dwRetErrorCode);
}


__inline DWORD BASE64Decode(LPCVOID lpcSource,SIZE_T dwSourceSize,LPCVOID lpcDestination,SIZE_T dwDestinationSize,SIZE_T *pdwDecodedSize)
{// BASE64 декодирование
	DWORD dwRetErrorCode;

	__try
	{
		dwRetErrorCode=BASE64DecodeUnSafe(lpcSource,dwSourceSize,lpcDestination,dwDestinationSize,pdwDecodedSize);
	}__except(EXCEPTION_EXECUTE_HANDLER){
		dwRetErrorCode=ERROR_INVALID_ADDRESS;
	}
return(dwRetErrorCode);
}


__inline DWORD BASE64DecodeFormated(LPCVOID lpcSource,SIZE_T dwSourceSize,LPCVOID lpcDestination,SIZE_T dwDestinationSize,SIZE_T *pdwDecodedSize)
{// BASE64 декодирование и игнорирование форматирования
	DWORD dwRetErrorCode;

	if (dwSourceSize<=dwDestinationSize)
	{
		BASE64CopyUnSafe(lpcSource,lpcDestination,dwSourceSize,&dwSourceSize);
		dwRetErrorCode=BASE64DecodeUnSafe(lpcDestination,dwSourceSize,lpcDestination,dwDestinationSize,pdwDecodedSize);
	}else{// во входном буффере слишком мало данных
		dwRetErrorCode=ERROR_INSUFFICIENT_BUFFER;
	}

return(dwRetErrorCode);
}



#endif // !defined(AFX_BASE64__H__INCLUDED_)
