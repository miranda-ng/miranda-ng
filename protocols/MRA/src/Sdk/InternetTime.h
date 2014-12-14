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



#if !defined(AFX_INTERNET_TIME_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_INTERNET_TIME_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#pragma once

#include "Sdk/StrToNum.h"
#include "Sdk/MemoryFind.h"
#include "Sdk/MemoryFindByte.h"

typedef struct
{
	LONG		lTimeZone;
	SYSTEMTIME	stTime;
} INTERNET_TIME;


static LPCSTR lpcszenmMonthEnum[13]=	{"---","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static LPCSTR lpcszenmDayOfWeakEnum[7]=	{"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

#define CRLF "\r\n"

__inline BOOL SkeepSPWSP(LPCSTR lpszBuff,size_t dwBuffSize,LPSTR *plpszRetBuff,size_t *pdwRetBuffSize)
{
	BOOL bRet=TRUE;

	if (lpszBuff && dwBuffSize && (plpszRetBuff || pdwRetBuffSize))
	{
		while ((*lpszBuff)<33 && dwBuffSize)
		{
			dwBuffSize--;
			lpszBuff++;
		}

		if (plpszRetBuff)	(*plpszRetBuff)=(LPSTR)lpszBuff;
		if (pdwRetBuffSize)	(*pdwRetBuffSize)=dwBuffSize;
	}else{
		bRet=FALSE;
	}
return(bRet);
}


__inline BOOL WSP2SP(LPCSTR lpszBuff,size_t dwBuffSize,LPSTR lpszRetBuff,size_t *pdwRetBuffSize)
{//	WSP->SP
	BOOL bRet=TRUE;

	if (lpszBuff && dwBuffSize && lpszRetBuff)
	{
		LPSTR lpszCurReadPos,pCRLF,lpszCurWritePos;
		size_t dwToCopy,dwRetBuffSize;

		pCRLF=(LPSTR)lpszBuff;
		lpszCurReadPos=(LPSTR)lpszBuff;
		lpszCurWritePos=lpszRetBuff;
		dwRetBuffSize=0;

		while(pCRLF)
		{
			pCRLF=(LPSTR)MemoryFind((pCRLF-lpszBuff),lpszBuff,(dwBuffSize-1),CRLF,2);
			if (pCRLF)
			{
				pCRLF+=2;
				if ((*pCRLF)==9 || (*pCRLF)==32)// LWS: <US-ASCII HT, horizontal-tab (9)> || <US-ASCII SP, space (32)>
				{
					dwToCopy=((pCRLF-2)-lpszCurReadPos);
					pCRLF++;

					memmove((LPVOID)lpszCurWritePos,(CONST VOID*)lpszCurReadPos,dwToCopy);
					dwRetBuffSize+=(dwToCopy+1);
					lpszCurWritePos+=dwToCopy;
					lpszCurWritePos[0]=32;
					lpszCurWritePos++;
					lpszCurReadPos=pCRLF;
				}
			}else{
				dwToCopy=((lpszBuff+dwBuffSize)-lpszCurReadPos);
				dwRetBuffSize+=dwToCopy;
				memmove((LPVOID)lpszCurWritePos,(CONST VOID*)lpszCurReadPos,dwToCopy);
			}
		}

		if (pdwRetBuffSize) (*pdwRetBuffSize)=dwRetBuffSize;
	}else{
		bRet=FALSE;
	}
return(bRet);
}

__inline void InternetTimeGetCurrentTime(INTERNET_TIME *pitTime)
{
	TIME_ZONE_INFORMATION tzi={0};
	GetTimeZoneInformation(&tzi);
	pitTime->lTimeZone=tzi.Bias;
	GetSystemTime(&pitTime->stTime);
}

//	Переводит время из MAILTIME в строковое
__inline CMStringA InternetTimeGetString(INTERNET_TIME *pitTime)
{
	char lpszBuff[100];
	LPSTR lpszCurPos = lpszBuff;
	size_t dwTimeLen = 0, dwtm;

	// day of weak// date of mounth// mounth name// year// hours // minutes// seconds
	dwtm = wsprintfA(lpszCurPos, "%s, %02lu %s %04lu %02lu:%02lu:%02lu ",
		lpcszenmDayOfWeakEnum[pitTime->stTime.wDayOfWeek],
		pitTime->stTime.wDay,
		lpcszenmMonthEnum[pitTime->stTime.wMonth],
		pitTime->stTime.wYear,
		pitTime->stTime.wHour,
		pitTime->stTime.wMinute,
		pitTime->stTime.wSecond);
	lpszCurPos += dwtm;
	dwTimeLen += dwtm;

	// time zone
	if (pitTime->lTimeZone) {
		if (pitTime->lTimeZone < 0) { // нужно добавить плюсик, минус добавляется автоматом
			(*((BYTE*)lpszCurPos)) = '+';
			lpszCurPos ++;
			dwTimeLen ++;
		}

		dwtm = wsprintfA(lpszCurPos, "%04ld", -(((pitTime->lTimeZone / 60) * 100) + pitTime->lTimeZone % 60));
		lpszCurPos += dwtm;
		dwTimeLen += dwtm;
	} else {
		dwtm = wsprintfA(lpszCurPos, "GMT");
		lpszCurPos += dwtm;
		dwTimeLen += dwtm;
	}

	return lpszBuff;
}

//	Переводит время из строкового в INTERNET_TIME
__inline DWORD InternetTimeGetTime(const CMStringA &lpszTime, INTERNET_TIME &pitTime)
{
	if (lpszTime.IsEmpty())
		return ERROR_INVALID_HANDLE;
	
	// = Thu, 21 May 1998 05:33:29 -0700 =
	char sztmBuff[4096];
	LPSTR lpszCurPos=sztmBuff,lpszTemp;
	size_t i,dwCurSize=4096,dwTemp;

	memset(&pitTime, 0, sizeof(INTERNET_TIME));
	WSP2SP(lpszTime, lpszTime.GetLength(), lpszCurPos, &dwCurSize);

	if (dwCurSize > 3) { // день недели
		if (lpszCurPos[3] == ',') {
			for (i=0; i < 8; i++) {
				if ( !_memicmp(lpcszenmDayOfWeakEnum[i], lpszCurPos, 3)) {
					pitTime.stTime.wDayOfWeek=(unsigned short)i;
					break;
				}
			}

			lpszCurPos+=4;
			dwCurSize-=4;
		}

		if (dwCurSize>2) { // день месяца
			SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
			if ((lpszTemp = (LPSTR)MemoryFindByte(0, lpszCurPos, dwCurSize,' '))) {
				dwTemp=(lpszTemp-lpszCurPos);
				pitTime.stTime.wDay=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

				lpszCurPos=(lpszTemp+1);
				dwCurSize-=(dwTemp+1);

				if (dwCurSize > 3) { // месяц
					SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);

					for (i=1; i < 14; i++)
						if ( !_memicmp(lpcszenmMonthEnum[i], lpszCurPos, 3)) {
							pitTime.stTime.wMonth=(unsigned short)i;
							break;
						}

					lpszCurPos += 3;
					dwCurSize -= 3;

					if (dwCurSize > 3) { // год
						SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
						if ((lpszTemp = (LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,' '))) {
							dwTemp=(lpszTemp-lpszCurPos);
							pitTime.stTime.wYear=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

							lpszCurPos=(lpszTemp+1);
							dwCurSize-=(dwTemp+1);

							if (dwCurSize > 2) { // часы
								SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
								if ((lpszTemp = (LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,':'))) {
									dwTemp=(lpszTemp-lpszCurPos);
									pitTime.stTime.wHour=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

									lpszCurPos=(lpszTemp+1);
									dwCurSize-=(dwTemp+1);

									if (dwCurSize > 2) { // минуты
										SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
										if ((lpszTemp=(LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,':')))
										{
											dwTemp=(lpszTemp-lpszCurPos);
											pitTime.stTime.wMinute=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

											lpszCurPos = (lpszTemp+1);
											dwCurSize -= (dwTemp+1);

											if (dwCurSize > 2) { // секунды, они есть
												if ((lpszTemp = (LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,' '))) {
													dwTemp=(lpszTemp-lpszCurPos);
													pitTime.stTime.wSecond=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

													lpszCurPos=(lpszTemp+1);
													dwCurSize-=(dwTemp+1);
												}
											}
											else {// зоны нет
												if (dwCurSize) {
													pitTime.stTime.wSecond=(unsigned short)StrToUNum32(lpszCurPos,dwCurSize);
													lpszCurPos+=dwCurSize;
													dwCurSize=0;
												}
											}
										}
										else {
											if ((lpszTemp = (LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,' '))) {
												dwTemp = (lpszTemp-lpszCurPos);
												pitTime.stTime.wMinute = (unsigned short)StrToUNum32(lpszCurPos,dwTemp);

												lpszCurPos = (lpszTemp+1);
												dwCurSize -= (dwTemp+1);
											}
										}

										if (dwCurSize) { // часовой пояс
											SkeepSPWSP(lpszCurPos, dwCurSize, &lpszCurPos, &dwCurSize);
											pitTime.lTimeZone = (LONG)StrToNum(lpszCurPos, dwCurSize);
											if (pitTime.lTimeZone > 1300 || pitTime.lTimeZone < -1200)
												pitTime.lTimeZone = 2400;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NO_ERROR;
}



#endif // !defined(AFX_INTERNET_TIME_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
