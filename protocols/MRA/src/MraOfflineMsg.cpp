#include "Mra.h"
#include "MraOfflineMsg.h"
#include "MraConstans.h"

#define LF			"\n"
#define LFLF		"\n\n"
#define CRLF		"\r\n"
#define CRLFCRLF	"\r\n\r\n"

DWORD MraOfflineMessageGetMIMEHeadAndBody(LPCSTR lpszMessage, size_t dwMessageSize, LPSTR *plpszHeader, size_t *pdwHeaderSize, LPSTR *plpszBody, size_t *pdwBodySize);
DWORD MraOfflineMessageGetNextMIMEPart(LPSTR lpszBody, size_t dwBodySize, LPSTR lpszBoundary, size_t dwBoundarySize, LPSTR *plpszCurMIMEPos, LPSTR *plpszMIMEPart, size_t *pdwMIMEPartSize);
DWORD MraOfflineMessageGetHeaderValue(LPSTR lpszHeader, LPSTR lpszHeaderLow, size_t dwHeaderSize, LPSTR lpszValueName, size_t dwValueNameSize, CMStringA &plpszValue);
DWORD MraOfflineMessageGetHeaderValueLow(LPSTR lpszHeaderLow, size_t dwHeaderSize, LPSTR lpszValueName, size_t dwValueNameSize, LPSTR *plpszValue, size_t *pdwValueSize);
DWORD MraOfflineMessageConvertTime(INTERNET_TIME *pitTime);

// Сообщение
DWORD MraOfflineMessageGet(CMStringA *plpsMsg, DWORD *pdwTime, DWORD *pdwFlags, CMStringA *plpsEMail, CMStringA *plpsText, CMStringA *plpsRTFText, CMStringA *plpsMultiChatData, LPBYTE *plpbBuff)
{
	DWORD dwRetErrorCode = ERROR_INVALID_HANDLE;

	if (plpsMsg)
	if (!plpsMsg->IsEmpty()) {
		LPSTR lpszHeader, lpszHeaderLow, lpszBody, lpszContentTypeLow;
		size_t dwHeaderSize, dwBodySize, dwContentTypeSize;
		DWORD dwMultichatType;
		CMStringA szTemp;

		#ifdef _DEBUG
			DebugPrintCRLFA(plpsMsg->GetString());
		#endif

		if (MraOfflineMessageGetMIMEHeadAndBody(plpsMsg->GetString(), plpsMsg->GetLength(), &lpszHeader, &dwHeaderSize, &lpszBody, &dwBodySize) == NO_ERROR) {
			lpszHeaderLow = (LPSTR)mir_calloc(dwHeaderSize);
			if (lpszHeaderLow) BuffToLowerCase(lpszHeaderLow, lpszHeader, dwHeaderSize);

			if (pdwTime)
			if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "date", 4, szTemp) == NO_ERROR) {
				INTERNET_TIME itTime;
				InternetTimeGetTime(szTemp, &itTime);
				(*pdwTime) = MraOfflineMessageConvertTime(&itTime);
			}
			else (*pdwTime) = 0;

			if (pdwFlags)
			if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "x-mrim-flags", 12, szTemp) == NO_ERROR)
				*pdwFlags = StrHexToUNum32(szTemp, szTemp.GetLength());
			else
				*pdwFlags = 0;

			if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "x-mrim-multichat-type", 21, szTemp) == NO_ERROR)
				dwMultichatType = StrHexToUNum32(szTemp, szTemp.GetLength());
			else
				dwMultichatType = 0;

			if (plpsEMail)
			if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "from", 4, *plpsEMail) != NO_ERROR)
				plpsEMail->Empty();

			if (plpsText)
				plpsText->Empty();

			if (plpsRTFText)
				plpsRTFText->Empty();

			if (plpsMultiChatData)
				plpsMultiChatData->Empty();

			if (plpbBuff) (*plpbBuff) = NULL;

			if (plpsText || plpsRTFText)
			if (MraOfflineMessageGetHeaderValueLow(lpszHeaderLow, dwHeaderSize, "content-type", 12, &lpszContentTypeLow, &dwContentTypeSize) == NO_ERROR) {
				if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "multipart/alternative", 21)) {
					// Content-Type: multipart/alternative; boundary = 1217508709J3777283291217508709T31197726
					LPSTR lpszBoundary, lpszMIMEPart, lpszCurMIMEPos, lpszMIMEHeader, lpszMIMEHeaderLow, lpszMIMEBody, lpszMIMEContentType;
					size_t i, dwBoundarySize, dwMIMEPartSize, dwMIMEHeaderSize, dwMIMEBodySize, dwMIMEContentTypeSize;

					lpszBoundary = (LPSTR)MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "boundary=", 9);
					if (lpszBoundary) {
						dwBoundarySize = ((dwContentTypeSize-(lpszBoundary-lpszContentTypeLow))-9);
						lpszBoundary = (lpszHeader+((lpszBoundary+9)-lpszHeaderLow));

						i = 0;
						lpszCurMIMEPos = lpszBody;
						while (MraOfflineMessageGetNextMIMEPart(lpszBody, dwBodySize, lpszBoundary, dwBoundarySize, &lpszCurMIMEPos, &lpszMIMEPart, &dwMIMEPartSize) == NO_ERROR) {
							if (MraOfflineMessageGetMIMEHeadAndBody(lpszMIMEPart, dwMIMEPartSize, &lpszMIMEHeader, &dwMIMEHeaderSize, &lpszMIMEBody, &dwMIMEBodySize) == NO_ERROR) {
								lpszMIMEHeaderLow = (LPSTR)mir_calloc(dwMIMEHeaderSize);
								if (lpszMIMEHeaderLow) {
									BuffToLowerCase(lpszMIMEHeaderLow, lpszMIMEHeader, dwMIMEHeaderSize);
									if (MraOfflineMessageGetHeaderValueLow(lpszMIMEHeaderLow, dwMIMEHeaderSize, "content-type", 12, &lpszMIMEContentType, &dwMIMEContentTypeSize) == NO_ERROR) {
										if (MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "text/plain", 10)) {
											// this is simple text part: text/plain
											if (plpsText) {
												if (MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "utf-16le", 8)) {
													// charset = UTF-16LE// предполагаем что оно в base64
													if (plpbBuff) {
														// буффер для декодирования текста можно выделять
														LPS2ANSI(szMime, lpszMIMEBody, dwMIMEBodySize);
														unsigned dwTextSize;
														LPWSTR lpwszText = (LPWSTR)mir_base64_decode(szMime, &dwTextSize);
														if (lpwszText) {															
															*plpsText = CMStringW(lpwszText, dwTextSize);
															if (pdwFlags) {
																(*pdwFlags) |= MESSAGE_FLAG_v1p16; // set unocode flag if not exist
																(*pdwFlags) &= ~MESSAGE_FLAG_CP1251; // reset ansi flag if exist
															}
															*plpbBuff = (LPBYTE)lpwszText;
															dwRetErrorCode = NO_ERROR;
														}
													}
												}
												else if ( MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "cp-1251", 7)) {
													// charset = CP-1251
													*plpsText = CMStringA(lpszMIMEBody, dwMIMEBodySize);
													if (pdwFlags) {
														(*pdwFlags) &= ~MESSAGE_FLAG_v1p16; // reset unocode flag if exist
														(*pdwFlags) |= MESSAGE_FLAG_CP1251; // set ansi flag
													}
													dwRetErrorCode = NO_ERROR;
												}
												else DebugBreak();
											}
										}
										else if (MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "application/x-mrim-rtf", 22)) {
											if (plpsRTFText) {
												*plpsRTFText = CMStringA(lpszMIMEBody, dwMIMEBodySize);
												if (pdwFlags)
													(*pdwFlags) |= MESSAGE_FLAG_RTF; // set RTF flag if not exist
												dwRetErrorCode = NO_ERROR;
											}
										}
										else if (MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "application/x-mrim+xml", 22)) {
											if (plpsMultiChatData) {
												*plpsMultiChatData = CMStringA(lpszMIMEBody, dwMIMEBodySize);
												if (pdwFlags)
													(*pdwFlags) |= MESSAGE_FLAG_MULTICHAT; // set MESSAGE_FLAG_MULTICHAT flag if not exist
												dwRetErrorCode = NO_ERROR;
											}
										}
										else DebugBreak();
									}
									mir_free(lpszMIMEHeaderLow);
								}
							}
							i++;
						}

						DebugBreakIf((i>3 || i == 0));
					}
					else DebugBreak(); // boundary not found
				}
				else if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "text/plain", 10)) {
					// Content-Type: text/plain; charset = CP-1251
					if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "utf-16le", 8)) {
						// charset = UTF-16LE// предполагаем что оно в base64
						if (plpbBuff) {
							// буфер для декодирования текста можно выделять
							LPS2ANSI(szMime, lpszBody, dwBodySize);
							unsigned dwTextSize;
							LPWSTR lpwszText = (LPWSTR)mir_base64_decode(szMime, &dwTextSize);
							if (lpwszText) {
								*plpsText = CMStringW(lpwszText, dwTextSize);
								if (pdwFlags) {
									(*pdwFlags) |= MESSAGE_FLAG_v1p16; // set unocode flag if not exist
									(*pdwFlags) &= ~MESSAGE_FLAG_CP1251; // reset ansi flag if exist
								}
								(*plpbBuff) = (LPBYTE)lpwszText;
								dwRetErrorCode = NO_ERROR;
							}
						}
					}else
					if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "cp-1251", 7))
					{// charset = CP-1251
						*plpsText = CMStringA(lpszBody, dwBodySize);
						if (pdwFlags) {
							(*pdwFlags) &= ~MESSAGE_FLAG_v1p16; // reset unocode flag if exist
							(*pdwFlags) |= MESSAGE_FLAG_CP1251; // set ansi flag
						}
						dwRetErrorCode = NO_ERROR;
					}else {
						DebugBreak();
					}
				}else
				if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "application/x-mrim-auth-req", 27))
				{// Content-Type: application/x-mrim-auth-req
					if (plpsText)
						*plpsText = CMStringA(lpszBody, dwBodySize);
					dwRetErrorCode = NO_ERROR;
				}
				else DebugBreak();
			}
			else DebugBreak();

			mir_free(lpszHeaderLow);
		}
	}

return(dwRetErrorCode);
}


DWORD MraOfflineMessageGetMIMEHeadAndBody(LPCSTR lpszMessage, size_t dwMessageSize, LPSTR *plpszHeader, size_t *pdwHeaderSize, LPSTR *plpszBody, size_t *pdwBodySize)
{
	DWORD dwRetErrorCode = ERROR_NOT_FOUND;

	if (lpszMessage && dwMessageSize)
	{
		LPSTR lpszBody;
		size_t dwBodySize;

		// затычка: майл не придерживается RFC и вместо CRLFCRLF ставит LFLF в MIME частях, иногда ставит
		lpszBody = (LPSTR)MemoryFind(0, lpszMessage, dwMessageSize, CRLFCRLF, (sizeof(CRLFCRLF)-1));
		if (lpszBody)
		{
			lpszBody += (sizeof(CRLFCRLF)-1);
		}else {
			lpszBody = (LPSTR)MemoryFind(0, lpszMessage, dwMessageSize, LFLF, (sizeof(LFLF)-1));
			if (lpszBody) lpszBody += (sizeof(LFLF)-1);
		}

		if (lpszBody)
		{// нашли начало контента миме части
			dwBodySize = (dwMessageSize-(lpszBody-lpszMessage));

			if (plpszHeader)   (*plpszHeader) = (LPSTR)lpszMessage;
			if (pdwHeaderSize) (*pdwHeaderSize) = ((lpszBody-(sizeof(LFLF)-1))-lpszMessage);
			if (plpszBody)	    (*plpszBody) = lpszBody;
			if (pdwBodySize)   (*pdwBodySize) = dwBodySize;

			dwRetErrorCode = NO_ERROR;
		}
	}else {
		dwRetErrorCode = ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraOfflineMessageGetNextMIMEPart(LPSTR lpszBody, size_t dwBodySize, LPSTR lpszBoundary, size_t dwBoundarySize, LPSTR *plpszCurMIMEPos, LPSTR *plpszMIMEPart, size_t *pdwMIMEPartSize)
{
	DWORD dwRetErrorCode = ERROR_NOT_FOUND;

	if (lpszBody && dwBodySize)
	{
		LPSTR lpszMIMEPart, lpszCurMIMEPos, lpszTemp;
		size_t dwMIMEPartSize;

		if (plpszCurMIMEPos) lpszCurMIMEPos = (*plpszCurMIMEPos);
		lpszMIMEPart = (LPSTR)MemoryFind((lpszCurMIMEPos-lpszBody), lpszBody, dwBodySize, lpszBoundary, dwBoundarySize);
		if (lpszMIMEPart)
		{// первая миме часть
			lpszMIMEPart += dwBoundarySize;

			// затычка: майл не придерживается RFC и вместо CRLF ставит LF в MIME частях, иногда ставит
			if ((*((WORD*)lpszMIMEPart)) == (*((WORD*)CRLF)))
			{
				lpszMIMEPart += (sizeof(CRLF)-1);
			}else
			if ((*((BYTE*)lpszMIMEPart)) == (*((BYTE*)LF)))
			{
				lpszMIMEPart += (sizeof(LF)-1);
			}else
			if ((*((WORD*)lpszMIMEPart)) == '--')
			{
				lpszMIMEPart = NULL;
			}else {
				DebugBreak();
			}

			if (lpszMIMEPart)
			{
				lpszTemp = (LPSTR)MemoryFind((lpszMIMEPart-lpszBody), lpszBody, dwBodySize, lpszBoundary, dwBoundarySize);
				if (lpszTemp)
				{// нашли конец миме части с текстом
					dwMIMEPartSize = (lpszTemp-lpszMIMEPart);// 4 = CRLF"--"Boundary / 3 = LF"--"Boundary
					// затычка: майл не придерживается RFC и вместо CRLF ставит LF в MIME частях, иногда ставит
					if ((*((WORD*)(lpszTemp-4))) == (*((WORD*)CRLF)))
					{
						dwMIMEPartSize-=4;
					}else
					if ((*((BYTE*)(lpszTemp-3))) == (*((BYTE*)LF)))
					{
						dwMIMEPartSize-=3;
					}else {
						DebugBreak();
					}

					if (plpszMIMEPart) (*plpszMIMEPart) = lpszMIMEPart;
					if (pdwMIMEPartSize) (*pdwMIMEPartSize) = dwMIMEPartSize;
					if (plpszCurMIMEPos) (*plpszCurMIMEPos) = lpszTemp;
					dwRetErrorCode = NO_ERROR;
				}
			}else {
				dwRetErrorCode = ERROR_NO_MORE_ITEMS;
			}
		}
	}else {
		dwRetErrorCode = ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraOfflineMessageGetHeaderValue(LPSTR lpszHeader, LPSTR lpszHeaderLow, size_t dwHeaderSize, LPSTR lpszValueName, size_t dwValueNameSize, CMStringA &plpszValue)
{
	LPSTR lpszValue = (LPSTR)MemoryFind(0, lpszHeaderLow, dwHeaderSize, lpszValueName, dwValueNameSize);
	if (lpszValue)
	{
		lpszValue += dwValueNameSize;
		LPSTR lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, CRLF, (sizeof(CRLF)-1));
		if (lpszValueEnd == NULL) lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, LF, (sizeof(LF)-1));
		if (lpszValueEnd == NULL) lpszValueEnd = (LPSTR)(lpszHeaderLow+dwHeaderSize);

		lpszValue = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, (lpszValueEnd-lpszHeaderLow), ":", 1);
		if (lpszValue) {
			lpszValue++;
			plpszValue = CMStringA(lpszHeader+(lpszValue-lpszHeaderLow), (lpszValueEnd-lpszValue));
			return NO_ERROR;
		}
	}
	return ERROR_NOT_FOUND;
}


DWORD MraOfflineMessageGetHeaderValueLow(LPSTR lpszHeaderLow, size_t dwHeaderSize, LPSTR lpszValueName, size_t dwValueNameSize, LPSTR *plpszValue, size_t *pdwValueSize)
{
	DWORD dwRetErrorCode = ERROR_NOT_FOUND;
	LPSTR lpszValue, lpszValueEnd;
	size_t dwValueSize;

	lpszValue = (LPSTR)MemoryFind(0, lpszHeaderLow, dwHeaderSize, lpszValueName, dwValueNameSize);
	if (lpszValue)
	{
		lpszValue += dwValueNameSize;
		lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, CRLF, (sizeof(CRLF)-1));
		if (lpszValueEnd == NULL) lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, LF, (sizeof(LF)-1));
		if (lpszValueEnd == NULL) lpszValueEnd = (LPSTR)(lpszHeaderLow+dwHeaderSize);

		lpszValue = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, (lpszValueEnd-lpszHeaderLow), ":", 1);
		if (lpszValue)
		{
			lpszValue++;
			dwValueSize = (lpszValueEnd-lpszValue);
			SkeepSPWSP(lpszValue, dwValueSize, plpszValue, pdwValueSize);
			dwRetErrorCode = NO_ERROR;
		}
	}
return(dwRetErrorCode);
}


DWORD MraOfflineMessageConvertTime(INTERNET_TIME *pitTime)
{
	SYSTEMTIME stTime, stUniversalTime;
	TIME_ZONE_INFORMATION tziTimeZoneMailRu = {0}, tziTimeZoneLocal;

	GetTimeZoneInformation(&tziTimeZoneLocal);
	if (GetTimeZoneInformation(&tziTimeZoneMailRu) == TIME_ZONE_ID_DAYLIGHT) tziTimeZoneMailRu.DaylightBias *= 2;
	tziTimeZoneMailRu.Bias = MAILRU_SERVER_TIME_ZONE;

	//TzSpecificLocalTimeToSystemTime(&tziTimeZoneMailRu, &pitTime->stTime, &stUniversalTime);
	{// for win 2000 compatible
		tziTimeZoneMailRu.Bias = -tziTimeZoneMailRu.Bias;
		tziTimeZoneMailRu.DaylightBias = -tziTimeZoneMailRu.DaylightBias;
		SystemTimeToTzSpecificLocalTime(&tziTimeZoneMailRu, &pitTime->stTime, &stUniversalTime);
	}//*/
	SystemTimeToTzSpecificLocalTime(&tziTimeZoneLocal, &stUniversalTime, &stTime);

return((DWORD)MakeTime32FromLocalSystemTime(&stTime));
}
