/*
 * Copyright (c) 2005 Rozhuk Ivan <rozhuk.im@gmail.com>
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



#if !defined(AFX_STRTONUM__H__INCLUDED_)
#define AFX_STRTONUM__H__INCLUDED_

#pragma once

__inline DWORD StrToUNum32(LPCSTR lpcszString,size_t dwStringLen)
{
	DWORD dwNum=0;
	BYTE bCurentFigure;


	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			dwNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			dwNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	return(dwNum);
}
							  
__inline SSIZE_T StrToNum(LPCSTR lpcszString,size_t dwStringLen)
{
	SSIZE_T lNum=0,lSingn=1;
	BYTE bCurentFigure;

	while(dwStringLen && ((bCurentFigure=((*lpcszString)-48))>9))
	{
		if (bCurentFigure=='-') lSingn=-1;
		if (bCurentFigure=='+') lSingn=1;

		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}

	while(dwStringLen)
	{
		if ((bCurentFigure=((*lpcszString)-48))<10)
		{
			lNum*=10;// сдвигаем предыдущее число на один разряд чтоб добавить в младший разряд новую цифру
			lNum+=bCurentFigure;// добавляем цифру в младший разряд
		}
		lpcszString++;// перемещаем указатель на следующую позицию
		dwStringLen--;// уменьшаем длинну
	}
	lNum*=lSingn;

	return(lNum);
}


#endif // !defined(AFX_STRTONUM__H__INCLUDED_)