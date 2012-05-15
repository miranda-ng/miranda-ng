#include "Mra.h"
#include "MraOfflineMsg.h"
#include "MraConstans.h"


#define LF			"\n"
#define LFLF		"\n\n"
#define CRLF		"\r\n"
#define CRLFCRLF	"\r\n\r\n"


DWORD MraOfflineMessageGetMIMEHeadAndBody	(LPSTR lpszMessage,SIZE_T dwMessageSize,LPSTR *plpszHeader,SIZE_T *pdwHeaderSize,LPSTR *plpszBody,SIZE_T *pdwBodySize);
DWORD MraOfflineMessageGetNextMIMEPart		(LPSTR lpszBody,SIZE_T dwBodySize,LPSTR lpszBoundary,SIZE_T dwBoundarySize,LPSTR *plpszCurMIMEPos,LPSTR *plpszMIMEPart,SIZE_T *pdwMIMEPartSize);
DWORD MraOfflineMessageGetHeaderValue		(LPSTR lpszHeader,LPSTR lpszHeaderLow,SIZE_T dwHeaderSize,LPSTR lpszValueName,SIZE_T dwValueNameSize,LPSTR *plpszValue,SIZE_T *pdwValueSize);
DWORD MraOfflineMessageGetHeaderValueLow	(LPSTR lpszHeaderLow,SIZE_T dwHeaderSize,LPSTR lpszValueName,SIZE_T dwValueNameSize,LPSTR *plpszValue,SIZE_T *pdwValueSize);
DWORD MraOfflineMessageConvertTime			(INTERNET_TIME *pitTime);




DWORD MraOfflineMessageGet(MRA_LPS *plpsMsg,DWORD *pdwTime,DWORD *pdwFlags,MRA_LPS *plpsEMail,MRA_LPS *plpsText,MRA_LPS *plpsRTFText,MRA_LPS *plpsMultiChatData,LPBYTE *plpbBuff)
{// Сообщение
	DWORD dwRetErrorCode=ERROR_INVALID_HANDLE;


	if (plpsMsg)
	if (plpsMsg->lpszData && plpsMsg->dwSize)
	{
		LPSTR lpszHeader,lpszHeaderLow,lpszBody,lpszContentTypeLow,lpszTemp;
		SIZE_T dwHeaderSize,dwBodySize,dwContentTypeSize,dwTempSize;
		DWORD dwMultichatType;

		#ifdef _DEBUG
			DebugPrintCRLFA(plpsMsg->lpszData);
		#endif

		if (MraOfflineMessageGetMIMEHeadAndBody(plpsMsg->lpszData,plpsMsg->dwSize,&lpszHeader,&dwHeaderSize,&lpszBody,&dwBodySize)==NO_ERROR)
		{
			lpszHeaderLow=(LPSTR)MEMALLOC(dwHeaderSize);
			if (lpszHeaderLow) BuffToLowerCase(lpszHeaderLow,lpszHeader,dwHeaderSize);

			if (pdwTime)
			if (MraOfflineMessageGetHeaderValue(lpszHeader,lpszHeaderLow,dwHeaderSize,"date",4,&lpszTemp,&dwTempSize)==NO_ERROR)
			{
				INTERNET_TIME itTime;
				InternetTimeGetTime(lpszTemp,dwTempSize,&itTime);
				(*pdwTime)=MraOfflineMessageConvertTime(&itTime);
			}else{
				(*pdwTime)=0;
			}

			if (pdwFlags)
			if (MraOfflineMessageGetHeaderValue(lpszHeader,lpszHeaderLow,dwHeaderSize,"x-mrim-flags",12,&lpszTemp,&dwTempSize)==NO_ERROR)
			{
				(*pdwFlags)=StrHexToUNum32(lpszTemp,dwTempSize);
			}else{
				(*pdwFlags)=0;
			}

			if (MraOfflineMessageGetHeaderValue(lpszHeader,lpszHeaderLow,dwHeaderSize,"x-mrim-multichat-type",21,&lpszTemp,&dwTempSize)==NO_ERROR)
			{
				dwMultichatType=StrHexToUNum32(lpszTemp,dwTempSize);
			}else{
				dwMultichatType=0;
			}


			if (plpsEMail)
			if (MraOfflineMessageGetHeaderValue(lpszHeader,lpszHeaderLow,dwHeaderSize,"from",4,&plpsEMail->lpszData,&plpsEMail->dwSize)!=NO_ERROR)
			{
				plpsEMail->lpszData=NULL;
				plpsEMail->dwSize=0;
			}


			if (plpsText)
			{
				plpsText->lpszData=NULL;
				plpsText->dwSize=0;
			}
			if (plpsRTFText)
			{
				plpsRTFText->lpszData=NULL;
				plpsRTFText->dwSize=0;
			}
			if (plpsMultiChatData)
			{
				plpsMultiChatData->lpszData=NULL;
				plpsMultiChatData->dwSize=0;
			}
			if (plpbBuff) (*plpbBuff)=NULL;

			if (plpsText || plpsRTFText)
			if (MraOfflineMessageGetHeaderValueLow(lpszHeaderLow,dwHeaderSize,"content-type",12,&lpszContentTypeLow,&dwContentTypeSize)==NO_ERROR)
			{

				if (MemoryFind(0,lpszContentTypeLow,dwContentTypeSize,"multipart/alternative",21))
				{// Content-Type: multipart/alternative; boundary=1217508709J3777283291217508709T31197726
					LPSTR lpszBoundary,lpszMIMEPart,lpszCurMIMEPos,lpszMIMEHeader,lpszMIMEHeaderLow,lpszMIMEBody,lpszMIMEContentType;
					SIZE_T i,dwBoundarySize,dwMIMEPartSize,dwMIMEHeaderSize,dwMIMEBodySize,dwMIMEContentTypeSize;

					lpszBoundary=(LPSTR)MemoryFind(0,lpszContentTypeLow,dwContentTypeSize,"boundary=",9);
					if (lpszBoundary)
					{
						dwBoundarySize=((dwContentTypeSize-(lpszBoundary-lpszContentTypeLow))-9);
						lpszBoundary=(lpszHeader+((lpszBoundary+9)-lpszHeaderLow));

						i=0;
						lpszCurMIMEPos=lpszBody;
						while(MraOfflineMessageGetNextMIMEPart(lpszBody,dwBodySize,lpszBoundary,dwBoundarySize,&lpszCurMIMEPos,&lpszMIMEPart,&dwMIMEPartSize)==NO_ERROR)
						{
							if (MraOfflineMessageGetMIMEHeadAndBody(lpszMIMEPart,dwMIMEPartSize,&lpszMIMEHeader,&dwMIMEHeaderSize,&lpszMIMEBody,&dwMIMEBodySize)==NO_ERROR)
							{
								lpszMIMEHeaderLow=(LPSTR)MEMALLOC(dwMIMEHeaderSize);
								if (lpszMIMEHeaderLow)
								{
									BuffToLowerCase(lpszMIMEHeaderLow,lpszMIMEHeader,dwMIMEHeaderSize);
									if (MraOfflineMessageGetHeaderValueLow(lpszMIMEHeaderLow,dwMIMEHeaderSize,"content-type",12,&lpszMIMEContentType,&dwMIMEContentTypeSize)==NO_ERROR)
									{
										if (MemoryFind(0,lpszMIMEContentType,dwMIMEContentTypeSize,"text/plain",10))
										{// this is simple text part: text/plain
											if (plpsText)
											{
												if (MemoryFind(0,lpszMIMEContentType,dwMIMEContentTypeSize,"utf-16le",8))
												{// charset=UTF-16LE// предполагаем что оно в base64
													if (plpbBuff)
													{// буффер для декодирования текста можно выделять
														LPWSTR lpwszText;
														SIZE_T dwTextSize;

														lpwszText=(LPWSTR)MEMALLOC(dwMIMEBodySize);
														if (lpwszText)
														{
															BASE64DecodeFormated(lpszMIMEBody,dwMIMEBodySize,lpwszText,dwMIMEBodySize,&dwTextSize);
															plpsText->lpwszData=lpwszText;
															plpsText->dwSize=dwTextSize;
															if (pdwFlags)
															{
																(*pdwFlags)|=MESSAGE_FLAG_v1p16; // set unocode flag if not exist
																(*pdwFlags)&=~MESSAGE_FLAG_CP1251; // reset ansi flag if exist
															}
															(*plpbBuff)=(LPBYTE)lpwszText;
															dwRetErrorCode=NO_ERROR;
														}
													}
												}else
												if (MemoryFind(0,lpszMIMEContentType,dwMIMEContentTypeSize,"cp-1251",7))
												{// charset=CP-1251
													plpsText->lpszData=lpszMIMEBody;
													plpsText->dwSize=dwMIMEBodySize;
													if (pdwFlags)
													{
														(*pdwFlags)&=~MESSAGE_FLAG_v1p16; // reset unocode flag if exist
														(*pdwFlags)|=MESSAGE_FLAG_CP1251; // set ansi flag 
													}
													dwRetErrorCode=NO_ERROR;
												}else{
													DebugBreak();
												}
											}
										}else
										if (MemoryFind(0,lpszMIMEContentType,dwMIMEContentTypeSize,"application/x-mrim-rtf",22))
										{
											if (plpsRTFText)
											{
												plpsRTFText->lpszData=lpszMIMEBody;
												plpsRTFText->dwSize=dwMIMEBodySize;
												if (pdwFlags) (*pdwFlags)|=MESSAGE_FLAG_RTF; // set RTF flag if not exist
												dwRetErrorCode=NO_ERROR;
											}
										}else
										if (MemoryFind(0,lpszMIMEContentType,dwMIMEContentTypeSize,"application/x-mrim+xml",22))
										{
											if (plpsMultiChatData)
											{
												plpsMultiChatData->lpszData=lpszMIMEBody;
												plpsMultiChatData->dwSize=dwMIMEBodySize;
												if (pdwFlags) (*pdwFlags)|=MESSAGE_FLAG_MULTICHAT; // set MESSAGE_FLAG_MULTICHAT flag if not exist
												dwRetErrorCode=NO_ERROR;
											}
										}else{
											DebugBreak();
										}
									}
									MEMFREE(lpszMIMEHeaderLow);
								}
							}
							i++;
						}

						DebugBreakIf((i>3 || i==0));
					}else{// boundary not found
						DebugBreak();
					}
				}else
				if (MemoryFind(0,lpszContentTypeLow,dwContentTypeSize,"text/plain",10))
				{// Content-Type: text/plain; charset=CP-1251
					if (MemoryFind(0,lpszContentTypeLow,dwContentTypeSize,"utf-16le",8))
					{// charset=UTF-16LE// предполагаем что оно в base64
						if (plpbBuff)
						{// буффер для декодирования текста можно выделять
							LPWSTR lpwszText;
							SIZE_T dwTextSize;

							lpwszText=(LPWSTR)MEMALLOC(dwBodySize);
							if (lpwszText)
							{
								BASE64DecodeFormated(lpszBody,dwBodySize,lpwszText,dwBodySize,&dwTextSize);
								plpsText->lpwszData=lpwszText;
								plpsText->dwSize=dwTextSize;
								if (pdwFlags)
								{
									(*pdwFlags)|=MESSAGE_FLAG_v1p16; // set unocode flag if not exist
									(*pdwFlags)&=~MESSAGE_FLAG_CP1251; // reset ansi flag if exist
								}
								(*plpbBuff)=(LPBYTE)lpwszText;
								dwRetErrorCode=NO_ERROR;
							}
						}
					}else
					if (MemoryFind(0,lpszContentTypeLow,dwContentTypeSize,"cp-1251",7))
					{// charset=CP-1251
						plpsText->lpszData=lpszBody;
						plpsText->dwSize=dwBodySize;
						if (pdwFlags)
						{
							(*pdwFlags)&=~MESSAGE_FLAG_v1p16; // reset unocode flag if exist
							(*pdwFlags)|=MESSAGE_FLAG_CP1251; // set ansi flag 
						}
						dwRetErrorCode=NO_ERROR;
					}else{
						DebugBreak();
					}
				}else
				if (MemoryFind(0,lpszContentTypeLow,dwContentTypeSize,"application/x-mrim-auth-req",27))
				{// Content-Type: application/x-mrim-auth-req
					if (plpsText)
					{
						plpsText->lpszData=lpszBody;
						plpsText->dwSize=dwBodySize;
					}
					dwRetErrorCode=NO_ERROR;
				}else{
					DebugBreak();
				}
			}else{
				DebugBreak();
			}

			MEMFREE(lpszHeaderLow);
		}
	}

return(dwRetErrorCode);
}


DWORD MraOfflineMessageGetMIMEHeadAndBody(LPSTR lpszMessage,SIZE_T dwMessageSize,LPSTR *plpszHeader,SIZE_T *pdwHeaderSize,LPSTR *plpszBody,SIZE_T *pdwBodySize)
{
	DWORD dwRetErrorCode=ERROR_NOT_FOUND;

	if (lpszMessage && dwMessageSize)
	{
		LPSTR lpszBody;
		SIZE_T dwBodySize;
		
		// затычка: майл не придерживается RFC и вместо CRLFCRLF ставит LFLF в MIME частях, иногда ставит
		lpszBody=(LPSTR)MemoryFind(0,lpszMessage,dwMessageSize,CRLFCRLF,(sizeof(CRLFCRLF)-1));
		if (lpszBody)
		{
			lpszBody+=(sizeof(CRLFCRLF)-1);
		}else{
			lpszBody=(LPSTR)MemoryFind(0,lpszMessage,dwMessageSize,LFLF,(sizeof(LFLF)-1));
			if (lpszBody) lpszBody+=(sizeof(LFLF)-1);
		}

		if (lpszBody)
		{// нашли начало контента миме части
			dwBodySize=(dwMessageSize-(lpszBody-lpszMessage));

			if (plpszHeader)	(*plpszHeader)=lpszMessage;
			if (pdwHeaderSize)	(*pdwHeaderSize)=((lpszBody-(sizeof(LFLF)-1))-lpszMessage);
			if (plpszBody)		(*plpszBody)=lpszBody;
			if (pdwBodySize)	(*pdwBodySize)=dwBodySize;

			dwRetErrorCode=NO_ERROR;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraOfflineMessageGetNextMIMEPart(LPSTR lpszBody,SIZE_T dwBodySize,LPSTR lpszBoundary,SIZE_T dwBoundarySize,LPSTR *plpszCurMIMEPos,LPSTR *plpszMIMEPart,SIZE_T *pdwMIMEPartSize)
{
	DWORD dwRetErrorCode=ERROR_NOT_FOUND;

	if (lpszBody && dwBodySize)
	{
		LPSTR lpszMIMEPart,lpszCurMIMEPos,lpszTemp;
		SIZE_T dwMIMEPartSize;

		if (plpszCurMIMEPos) lpszCurMIMEPos=(*plpszCurMIMEPos);
		lpszMIMEPart=(LPSTR)MemoryFind((lpszCurMIMEPos-lpszBody),lpszBody,dwBodySize,lpszBoundary,dwBoundarySize);
		if (lpszMIMEPart)
		{// первая миме часть
			lpszMIMEPart+=dwBoundarySize;

			// затычка: майл не придерживается RFC и вместо CRLF ставит LF в MIME частях, иногда ставит
			if ((*((WORD*)lpszMIMEPart))==(*((WORD*)CRLF)))
			{
				lpszMIMEPart+=(sizeof(CRLF)-1);
			}else
			if ((*((BYTE*)lpszMIMEPart))==(*((BYTE*)LF)))
			{
				lpszMIMEPart+=(sizeof(LF)-1);
			}else
			if ((*((WORD*)lpszMIMEPart))=='--')
			{
				lpszMIMEPart=NULL;
			}else{
				DebugBreak();
			}

			if (lpszMIMEPart)
			{
				lpszTemp=(LPSTR)MemoryFind((lpszMIMEPart-lpszBody),lpszBody,dwBodySize,lpszBoundary,dwBoundarySize);
				if (lpszTemp)
				{// нашли конец миме части с текстом
					dwMIMEPartSize=(lpszTemp-lpszMIMEPart);// 4=CRLF"--"Boundary / 3=LF"--"Boundary
					// затычка: майл не придерживается RFC и вместо CRLF ставит LF в MIME частях, иногда ставит
					if ((*((WORD*)(lpszTemp-4)))==(*((WORD*)CRLF)))
					{
						dwMIMEPartSize-=4;
					}else
					if ((*((BYTE*)(lpszTemp-3)))==(*((BYTE*)LF)))
					{
						dwMIMEPartSize-=3;
					}else{
						DebugBreak();
					}

					if (plpszMIMEPart) (*plpszMIMEPart)=lpszMIMEPart;
					if (pdwMIMEPartSize) (*pdwMIMEPartSize)=dwMIMEPartSize;
					if (plpszCurMIMEPos) (*plpszCurMIMEPos)=lpszTemp;
					dwRetErrorCode=NO_ERROR;
				}
			}else{
				dwRetErrorCode=ERROR_NO_MORE_ITEMS;
			}
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraOfflineMessageGetHeaderValue(LPSTR lpszHeader,LPSTR lpszHeaderLow,SIZE_T dwHeaderSize,LPSTR lpszValueName,SIZE_T dwValueNameSize,LPSTR *plpszValue,SIZE_T *pdwValueSize)
{
	DWORD dwRetErrorCode=ERROR_NOT_FOUND;
	LPSTR lpszValue,lpszValueEnd;
	SIZE_T dwValueSize;

	lpszValue=(LPSTR)MemoryFind(0,lpszHeaderLow,dwHeaderSize,lpszValueName,dwValueNameSize);
	if (lpszValue)
	{
		lpszValue+=dwValueNameSize;
		lpszValueEnd=(LPSTR)MemoryFind((lpszValue-lpszHeaderLow),lpszHeaderLow,dwHeaderSize,CRLF,(sizeof(CRLF)-1));
		if (lpszValueEnd==NULL) lpszValueEnd=(LPSTR)MemoryFind((lpszValue-lpszHeaderLow),lpszHeaderLow,dwHeaderSize,LF,(sizeof(LF)-1));
		if (lpszValueEnd==NULL) lpszValueEnd=(LPSTR)(lpszHeaderLow+dwHeaderSize);

		lpszValue=(LPSTR)MemoryFind((lpszValue-lpszHeaderLow),lpszHeaderLow,(lpszValueEnd-lpszHeaderLow),":",1);
		if (lpszValue)
		{
			lpszValue++;
			dwValueSize=(lpszValueEnd-lpszValue);
			SkeepSPWSP((lpszHeader+(lpszValue-lpszHeaderLow)),dwValueSize,plpszValue,pdwValueSize);
			dwRetErrorCode=NO_ERROR;
		}
	}
return(dwRetErrorCode);
}


DWORD MraOfflineMessageGetHeaderValueLow(LPSTR lpszHeaderLow,SIZE_T dwHeaderSize,LPSTR lpszValueName,SIZE_T dwValueNameSize,LPSTR *plpszValue,SIZE_T *pdwValueSize)
{
	DWORD dwRetErrorCode=ERROR_NOT_FOUND;
	LPSTR lpszValue,lpszValueEnd;
	SIZE_T dwValueSize;

	lpszValue=(LPSTR)MemoryFind(0,lpszHeaderLow,dwHeaderSize,lpszValueName,dwValueNameSize);
	if (lpszValue)
	{
		lpszValue+=dwValueNameSize;
		lpszValueEnd=(LPSTR)MemoryFind((lpszValue-lpszHeaderLow),lpszHeaderLow,dwHeaderSize,CRLF,(sizeof(CRLF)-1));
		if (lpszValueEnd==NULL) lpszValueEnd=(LPSTR)MemoryFind((lpszValue-lpszHeaderLow),lpszHeaderLow,dwHeaderSize,LF,(sizeof(LF)-1));
		if (lpszValueEnd==NULL) lpszValueEnd=(LPSTR)(lpszHeaderLow+dwHeaderSize);

		lpszValue=(LPSTR)MemoryFind((lpszValue-lpszHeaderLow),lpszHeaderLow,(lpszValueEnd-lpszHeaderLow),":",1);
		if (lpszValue)
		{
			lpszValue++;
			dwValueSize=(lpszValueEnd-lpszValue);
			SkeepSPWSP(lpszValue,dwValueSize,plpszValue,pdwValueSize);
			dwRetErrorCode=NO_ERROR;
		}
	}
return(dwRetErrorCode);
}


DWORD MraOfflineMessageConvertTime(INTERNET_TIME *pitTime)
{
	SYSTEMTIME stTime,stUniversalTime;
	TIME_ZONE_INFORMATION tziTimeZoneMailRu={0},tziTimeZoneLocal;

	GetTimeZoneInformation(&tziTimeZoneLocal);
	if (GetTimeZoneInformation(&tziTimeZoneMailRu)==TIME_ZONE_ID_DAYLIGHT) tziTimeZoneMailRu.DaylightBias*=2;
	tziTimeZoneMailRu.Bias=MAILRU_SERVER_TIME_ZONE;

	//TzSpecificLocalTimeToSystemTime(&tziTimeZoneMailRu,&pitTime->stTime,&stUniversalTime);
	{// for win 2000 compatible
		tziTimeZoneMailRu.Bias=-tziTimeZoneMailRu.Bias;
		tziTimeZoneMailRu.DaylightBias=-tziTimeZoneMailRu.DaylightBias;
		SystemTimeToTzSpecificLocalTime(&tziTimeZoneMailRu,&pitTime->stTime,&stUniversalTime);
	}//*/
	SystemTimeToTzSpecificLocalTime(&tziTimeZoneLocal,&stUniversalTime,&stTime);

return((DWORD)MakeTime32FromLocalSystemTime(&stTime));
}
