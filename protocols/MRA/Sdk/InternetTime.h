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


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <StrToNum.h>
#include <MemoryFind.h>
#include <MemoryFindByte.h>



typedef struct
{
	LONG		lTimeZone;
	SYSTEMTIME	stTime;
} INTERNET_TIME;


static LPCSTR lpcszenmMonthEnum[13]=	{"---","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static LPCSTR lpcszenmDayOfWeakEnum[7]=	{"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

#define CRLF "\r\n"
#define LF "\n"
//WSP++
#define LWSHT "\r\n\t"
#define LWSSP "\r\n "
//WSP--

//FWS++
#define FWSHT LWSHT
#define FWSSP LWSSP
//FWS--

#define SEPARATOR "\r\n\r\n"


//DWORD	InternetTimeGetTime	(LPCSTR lpszTime,SIZE_T dwTimeSize,INTERNET_TIME *pitTime);
//BOOL	SkeepSPWSP			(LPCSTR lpszBuff,SIZE_T dwBuffSize,LPSTR *plpszRetBuff,SIZE_T *pdwRetBuffSize);
//BOOL	WSP2SP				(LPCSTR lpszBuff,SIZE_T dwBuffSize,LPSTR lpszRetBuff,SIZE_T *pdwRetBuffSize);





__inline BOOL SkeepSPWSP(LPCSTR lpszBuff,SIZE_T dwBuffSize,LPSTR *plpszRetBuff,SIZE_T *pdwRetBuffSize)
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


__inline BOOL WSP2SP(LPCSTR lpszBuff,SIZE_T dwBuffSize,LPSTR lpszRetBuff,SIZE_T *pdwRetBuffSize)
{//	WSP->SP
	BOOL bRet=TRUE;

	if (lpszBuff && dwBuffSize && lpszRetBuff)
	{
		LPSTR lpszCurReadPos,pCRLF,lpszCurWritePos;
		SIZE_T dwToCopy,dwRetBuffSize;

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



__inline BOOL HT2SP(LPCSTR lpszBuff,SIZE_T dwBuffSize,LPSTR lpszRetBuff,SIZE_T *pdwRetBuffSize)
{//	HT->SP
	BOOL bRet=TRUE;

	if (lpszBuff && dwBuffSize && lpszRetBuff)
	{
		LPSTR lpszCurReadPos,pHT,lpszCurWritePos;
		SIZE_T dwToCopy,dwRetBuffSize;

		pHT=(LPSTR)lpszBuff;
		lpszCurReadPos=(LPSTR)lpszBuff;
		lpszCurWritePos=lpszRetBuff;
		dwRetBuffSize=0;

		while(pHT)
		{
			pHT=(LPSTR)MemoryFind((pHT-lpszBuff),lpszBuff,dwBuffSize,"\t",1);
			if (pHT)
			{
				dwToCopy=(pHT-lpszCurReadPos);
				pHT++;

				dwRetBuffSize+=(dwToCopy+1);
				memmove((LPVOID)lpszCurWritePos,(CONST VOID*)lpszCurReadPos,dwToCopy);
				lpszCurWritePos+=dwToCopy;
				lpszCurWritePos[0]=32;
				lpszCurWritePos++;
				lpszCurReadPos=pHT;
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




__inline BOOL CleanUnneededSP(LPCSTR lpszBuff,SIZE_T dwBuffSize,LPSTR lpszRetBuff,SIZE_T *pdwRetBuffSize)
{//	nSP->SP, SPCRLF->CRLF, CRLFSP->CRLF
	BOOL bRet=TRUE;

	if (lpszBuff && dwBuffSize && lpszRetBuff)
	{
		LPSTR lpszCurReadPos,pSP,pSPStart,pSPEnd,lpszCurWritePos,pEnd;
		SIZE_T dwToCopy,dwRetBuffSize;

		pSP=(LPSTR)lpszBuff;
		lpszCurReadPos=(LPSTR)lpszBuff;
		lpszCurWritePos=lpszRetBuff;
		pEnd=((LPSTR)lpszBuff+dwBuffSize);
		dwRetBuffSize=0;

		while(pSP)
		{
			pSP=(LPSTR)MemoryFind((pSP-lpszBuff),lpszBuff,dwBuffSize," ",1);
			if (pSP)
			{
				dwToCopy=(pSP-lpszCurReadPos);
				pSPStart=pSP;
				pSPEnd=pSP;
				while((*pSPEnd)==32 && pSPEnd<pEnd) pSPEnd++;

				// check SP on line start
				if ((pSPStart-1)>lpszBuff)
				{
					if ((*((WORD*)(pSPStart-2)))!=(*((WORD*)CRLF))) dwToCopy++;
				}else{// buff start
					if (pSPStart>lpszBuff) dwToCopy++;
				}

				// check SP on line end
				if ((pSPEnd+1)<=pEnd)
				{
					if ((*((WORD*)pSPEnd))!=(*((WORD*)CRLF))) dwToCopy++;
				}else{// buff start
					if (pSPEnd>lpszBuff) dwToCopy++;
				}

				memmove((LPVOID)lpszCurWritePos,(CONST VOID*)lpszCurReadPos,dwToCopy);
				lpszCurWritePos+=dwToCopy;
				dwRetBuffSize+=dwToCopy;
				lpszCurReadPos=pSPEnd;
				pSP=pSPEnd;
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




__inline SIZE_T CopyText(LPVOID lpOutBuff,LPCVOID lpcBuff,SIZE_T dwLen)
{
	SIZE_T dwRet=0;

	if (lpOutBuff && lpcBuff && dwLen)
	{
		BYTE bt;
		LPBYTE lpbtIn=(LPBYTE)lpcBuff,lpbtOut=(LPBYTE)lpOutBuff;

		for(SIZE_T i=dwLen;i;i--)
		{
			bt=(*(lpbtIn++));
			if (bt<127 && (bt>31 || bt==9 || bt==10 || bt==13))
			{
				(*(lpbtOut++))=bt;
				dwRet++;
			}
		}
	}
return(dwRet);
}


__inline void InternetTimeGetCurrentTime(INTERNET_TIME *pitTime)
{
	TIME_ZONE_INFORMATION tzi={0};
	GetTimeZoneInformation(&tzi);
	pitTime->lTimeZone=tzi.Bias;
	GetSystemTime(&pitTime->stTime);
}


__inline DWORD InternetTimeGetString(INTERNET_TIME *pitTime,LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet)
{//	Переводит время из MAILTIME в строковое
	DWORD dwRet=NO_ERROR;

	if (dwBuffSize>31)
	{
		LPSTR lpszCurPos=lpszBuff;
		SIZE_T dwTimeLen=0,dwtm;

		// day of weak// date of mounth// mounth name// year// hours // minutes// seconds
		dwtm=wsprintfA(lpszCurPos,"%s, %02lu %s %04lu %02lu:%02lu:%02lu ",lpcszenmDayOfWeakEnum[pitTime->stTime.wDayOfWeek],pitTime->stTime.wDay,lpcszenmMonthEnum[pitTime->stTime.wMonth],pitTime->stTime.wYear,pitTime->stTime.wHour,pitTime->stTime.wMinute,pitTime->stTime.wSecond);
		lpszCurPos+=dwtm;
		dwTimeLen+=dwtm;

		// time zone
		if (pitTime->lTimeZone)
		{
			if (pitTime->lTimeZone < 0)
			{// нужно добавить плюсик, минус добавляется автоматом
				(*((BYTE*)lpszCurPos))='+';
				lpszCurPos++;
				dwTimeLen++;
			}

			dwtm=wsprintfA(lpszCurPos,"%04ld",-(((pitTime->lTimeZone/60)*100)+pitTime->lTimeZone%60));
			lpszCurPos+=dwtm;
			dwTimeLen+=dwtm;
		}else{
			dwtm=wsprintfA(lpszCurPos,"GMT");
			lpszCurPos+=dwtm;
			dwTimeLen+=dwtm;
		}

		if (pdwBuffSizeRet) (*pdwBuffSizeRet)=dwTimeLen;
	}else{// переданный буффер слишком мал
		if (pdwBuffSizeRet) (*pdwBuffSizeRet)=32;
		dwRet=ERROR_INSUFFICIENT_BUFFER;
	}
return(dwRet);
}



__inline DWORD InternetTimeGetTime(LPCSTR lpszTime,SIZE_T dwTimeSize,INTERNET_TIME *pitTime)
{//	Переводит время из строкового в INTERNET_TIME
	DWORD dwRet=NO_ERROR;

	if (lpszTime && dwTimeSize && dwTimeSize<4097 && pitTime)
	{// = Thu, 21 May 1998 05:33:29 -0700 =
		char sztmBuff[4096];
		LPSTR lpszCurPos=sztmBuff,lpszTemp;
		SIZE_T i,dwCurSize=4096,dwTemp;

		memset(pitTime, 0, sizeof(INTERNET_TIME));
		WSP2SP((LPSTR)lpszTime,dwTimeSize,lpszCurPos,&dwCurSize);

		if (dwCurSize>3)
		{// день недели
			if (lpszCurPos[3]==',')
			{
				for (i=0;i<8;i++)
				{
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpcszenmDayOfWeakEnum[i],3,lpszCurPos,3)==CSTR_EQUAL) 
					{
						pitTime->stTime.wDayOfWeek=(unsigned short)i;
						break;
					}
				}

				lpszCurPos+=4;
				dwCurSize-=4;
			}

			if (dwCurSize>2)
			{// день месяца
				SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
				if ((lpszTemp=(LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,' ')))
				{
					dwTemp=(lpszTemp-lpszCurPos);
					pitTime->stTime.wDay=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

					lpszCurPos=(lpszTemp+1);
					dwCurSize-=(dwTemp+1);

					if (dwCurSize>3)
					{// месяц
						SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);

						for (i=1;i<14;i++)
						{
							if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpcszenmMonthEnum[i],3,lpszCurPos,3)==CSTR_EQUAL)
							{
								pitTime->stTime.wMonth=(unsigned short)i;
								break;
							}
						}

						lpszCurPos+=3;
						dwCurSize-=3;


						if (dwCurSize>3)
						{// год
							SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
							if ((lpszTemp=(LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,' ')))
							{
								dwTemp=(lpszTemp-lpszCurPos);
								pitTime->stTime.wYear=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

								lpszCurPos=(lpszTemp+1);
								dwCurSize-=(dwTemp+1);

								if (dwCurSize>2)
								{ // часы
									SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
									if ((lpszTemp=(LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,':')))
									{
										dwTemp=(lpszTemp-lpszCurPos);
										pitTime->stTime.wHour=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

										lpszCurPos=(lpszTemp+1);
										dwCurSize-=(dwTemp+1);

										if (dwCurSize>2)
										{// минуты
											SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
											if ((lpszTemp=(LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,':')))
											{
												dwTemp=(lpszTemp-lpszCurPos);
												pitTime->stTime.wMinute=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

												lpszCurPos=(lpszTemp+1);
												dwCurSize-=(dwTemp+1);

												if (dwCurSize>2)
												{// секунды, они есть
													if ((lpszTemp=(LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,' ')))
													{
														dwTemp=(lpszTemp-lpszCurPos);
														pitTime->stTime.wSecond=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

														lpszCurPos=(lpszTemp+1);
														dwCurSize-=(dwTemp+1);
													}
												}else{// зоны нет
													if (dwCurSize)
													{
														pitTime->stTime.wSecond=(unsigned short)StrToUNum32(lpszCurPos,dwCurSize);
														lpszCurPos+=dwCurSize;
														dwCurSize=0;
													}
												}
											}else{
												if ((lpszTemp=(LPSTR)MemoryFindByte(0,lpszCurPos,dwCurSize,' ')))
												{
													dwTemp=(lpszTemp-lpszCurPos);
													pitTime->stTime.wMinute=(unsigned short)StrToUNum32(lpszCurPos,dwTemp);

													lpszCurPos=(lpszTemp+1);
													dwCurSize-=(dwTemp+1);
												}
											}

											if (dwCurSize)
											{// часовой пояс
												SkeepSPWSP(lpszCurPos,dwCurSize,&lpszCurPos,&dwCurSize);
												pitTime->lTimeZone=(LONG)StrToNum(lpszCurPos,dwCurSize);
												if (pitTime->lTimeZone>1300 || pitTime->lTimeZone<-1200) pitTime->lTimeZone=2400;
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
	}else{
		dwRet=ERROR_INVALID_HANDLE;
	}
return(dwRet);
}



#endif // !defined(AFX_INTERNET_TIME_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
