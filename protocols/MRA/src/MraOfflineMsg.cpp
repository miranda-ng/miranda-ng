#include "Mra.h"
#include "MraOfflineMsg.h"
#include "MraConstans.h"

#define LF			"\n"
#define LFLF		"\n\n"
#define CRLF		"\r\n"
#define CRLFCRLF	"\r\n\r\n"

static DWORD MraOfflineMessageConvertTime(INTERNET_TIME *pitTime)
{
	SYSTEMTIME stTime, stUniversalTime;
	TIME_ZONE_INFORMATION tziTimeZoneMailRu = {0};
	tziTimeZoneMailRu.Bias = pitTime->lTimeZone/100*60;
	tziTimeZoneMailRu.DaylightBias = pitTime->lTimeZone%100;
	SystemTimeToTzSpecificLocalTime(&tziTimeZoneMailRu, &pitTime->stTime, &stUniversalTime);
	SystemTimeToTzSpecificLocalTime(NULL, &stUniversalTime, &stTime);

	return (DWORD)MakeTime32FromLocalSystemTime(&stTime);
}

static DWORD MraOfflineMessageGetHeaderValueLow(LPSTR lpszHeaderLow, size_t dwHeaderSize, LPSTR lpszValueName, size_t dwValueNameSize, LPSTR *plpszValue, size_t *pdwValueSize)
{
	LPSTR lpszValue = (LPSTR)MemoryFind(0, lpszHeaderLow, dwHeaderSize, lpszValueName, dwValueNameSize);
	if (lpszValue) {
		lpszValue += dwValueNameSize;
		LPSTR lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, CRLF, (sizeof(CRLF)-1));
		if (lpszValueEnd == NULL) lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, LF, (sizeof(LF)-1));
		if (lpszValueEnd == NULL) lpszValueEnd = (LPSTR)(lpszHeaderLow+dwHeaderSize);

		lpszValue = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, (lpszValueEnd-lpszHeaderLow), ":", 1);
		if (lpszValue) {
			lpszValue++;
			SkeepSPWSP(lpszValue, lpszValueEnd - lpszValue, plpszValue, pdwValueSize);
			return NO_ERROR;
		}
	}
	return ERROR_NOT_FOUND;
}

static DWORD MraOfflineMessageGetMIMEHeadAndBody(LPCSTR lpszMessage, size_t dwMessageSize, LPSTR *plpszHeader, size_t *pdwHeaderSize, LPSTR *plpszBody, size_t *pdwBodySize)
{
	if (!lpszMessage || !dwMessageSize)
		return ERROR_NOT_FOUND;

	// затычка: майл не придерживается RFC и вместо CRLFCRLF ставит LFLF в MIME частях, иногда ставит
	LPSTR lpszBody = (LPSTR)MemoryFind(0, lpszMessage, dwMessageSize, CRLFCRLF, (sizeof(CRLFCRLF)-1));
	if (lpszBody)
		lpszBody += (sizeof(CRLFCRLF)-1);
	else {
		lpszBody = (LPSTR)MemoryFind(0, lpszMessage, dwMessageSize, LFLF, (sizeof(LFLF)-1));
		if (lpszBody) lpszBody += (sizeof(LFLF)-1);
	}

	if (!lpszBody)
		return ERROR_NOT_FOUND;

	// нашли начало контента миме части
	size_t dwBodySize = (dwMessageSize-(lpszBody-lpszMessage));
	if (plpszHeader)   (*plpszHeader) = (LPSTR)lpszMessage;
	if (pdwHeaderSize) (*pdwHeaderSize) = ((lpszBody-(sizeof(LFLF)-1))-lpszMessage);
	if (plpszBody)	    (*plpszBody) = lpszBody;
	if (pdwBodySize)   (*pdwBodySize) = dwBodySize;
	return NO_ERROR;
}

static DWORD MraOfflineMessageGetHeaderValue(LPSTR lpszHeader, LPSTR lpszHeaderLow, size_t dwHeaderSize, LPSTR lpszValueName, size_t dwValueNameSize, CMStringA &plpszValue)
{
	LPSTR lpszValue = (LPSTR)MemoryFind(0, lpszHeaderLow, dwHeaderSize, lpszValueName, dwValueNameSize);
	if (lpszValue) {
		lpszValue += dwValueNameSize;
		LPSTR lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, CRLF, (sizeof(CRLF)-1));
		if (lpszValueEnd == NULL)
			lpszValueEnd = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, dwHeaderSize, LF, (sizeof(LF)-1));
		if (lpszValueEnd == NULL)
			lpszValueEnd = (LPSTR)(lpszHeaderLow+dwHeaderSize);

		lpszValue = (LPSTR)MemoryFind((lpszValue-lpszHeaderLow), lpszHeaderLow, (lpszValueEnd-lpszHeaderLow), ":", 1);
		if (lpszValue) {
			lpszValue++;
			size_t dwValueSize = lpszValueEnd - lpszValue;
			lpszValue = lpszHeader + (lpszValue-lpszHeaderLow);
			SkeepSPWSP(lpszValue, dwValueSize, &lpszValue, &dwValueSize);
			plpszValue = CMStringA(lpszValue, dwValueSize);
			return NO_ERROR;
		}
	}
	return ERROR_NOT_FOUND;
}

static DWORD MraOfflineMessageGetNextMIMEPart(LPSTR lpszBody, size_t dwBodySize, LPSTR lpszBoundary, size_t dwBoundarySize, LPSTR &plpszCurMIMEPos, LPSTR *plpszMIMEPart, size_t *pdwMIMEPartSize)
{
	if (!lpszBody || !dwBodySize)
		return ERROR_INVALID_HANDLE;

	LPSTR lpszCurMIMEPos = plpszCurMIMEPos;

	LPSTR lpszMIMEPart = (LPSTR)MemoryFind((lpszCurMIMEPos-lpszBody), lpszBody, dwBodySize, lpszBoundary, dwBoundarySize);
	if (lpszMIMEPart) {// первая миме часть
		lpszMIMEPart += dwBoundarySize;

		// затычка: майл не придерживается RFC и вместо CRLF ставит LF в MIME частях, иногда ставит
		if ((*((WORD*)lpszMIMEPart)) == (*((WORD*)CRLF)))
			lpszMIMEPart += (sizeof(CRLF)-1);
		else if ((*((BYTE*)lpszMIMEPart)) == (*((BYTE*)LF)))
			lpszMIMEPart += (sizeof(LF)-1);
		else if ((*((WORD*)lpszMIMEPart)) == '--')
			lpszMIMEPart = NULL;
		else
			_CrtDbgBreak();

		if (lpszMIMEPart == NULL)
			return ERROR_NO_MORE_ITEMS;
		LPSTR lpszTemp = (LPSTR)MemoryFind((lpszMIMEPart-lpszBody), lpszBody, dwBodySize, lpszBoundary, dwBoundarySize);
		if (lpszTemp) {// нашли конец миме части с текстом
			size_t dwMIMEPartSize = (lpszTemp-lpszMIMEPart);// 4 = CRLF"--"Boundary / 3 = LF"--"Boundary
			// затычка: майл не придерживается RFC и вместо CRLF ставит LF в MIME частях, иногда ставит
			if ((*((WORD*)(lpszTemp-4))) == (*((WORD*)CRLF)))
				dwMIMEPartSize -= 4;
			else if ((*((BYTE*)(lpszTemp-3))) == (*((BYTE*)LF)))
				dwMIMEPartSize -= 3;
			else
				_CrtDbgBreak();

			if (plpszMIMEPart) (*plpszMIMEPart) = lpszMIMEPart;
			if (pdwMIMEPartSize) (*pdwMIMEPartSize) = dwMIMEPartSize;
			plpszCurMIMEPos = lpszTemp;
			return NO_ERROR;
		}
	}

	return ERROR_NOT_FOUND;
}

// Сообщение
static DWORD PlainText2message(const CMStringA &szContentType, const CMStringA &szBody, CMStringA &plpsText, DWORD *pdwFlags)
{
	// Content-Type: text/plain; charset = CP-1251
	if ( strstr(szContentType, "utf-16le")) {
		// charset = UTF-16LE// предполагаем что оно в base64
		unsigned dwTextSize;
		ptrA lpszText((LPSTR)mir_base64_decode(szBody, &dwTextSize));
		if (lpszText) {
			plpsText = CMStringA(lpszText, dwTextSize);
			if (pdwFlags) {
				(*pdwFlags) |= MESSAGE_FLAG_v1p16; // set unicode flag if not exist
				(*pdwFlags) &= ~MESSAGE_FLAG_CP1251; // reset ansi flag if exist
			}
			return NO_ERROR;
		}
	}
	else if ( strstr(szContentType, "cp-1251")) { // charset = CP-1251
		plpsText = szBody;
		if (pdwFlags) {
			(*pdwFlags) &= ~MESSAGE_FLAG_v1p16; // reset unicode flag if exist
			(*pdwFlags) |= MESSAGE_FLAG_CP1251; // set ansi flag
		}
		return NO_ERROR;
	}
	else _CrtDbgBreak();

	return ERROR_INVALID_HANDLE;
}

DWORD MraOfflineMessageGet(const CMStringA &plpsMsg, DWORD &pdwTime, DWORD &pdwFlags, CMStringA &plpsEMail, CMStringA &plpsText, CMStringA &plpsRTFText, CMStringA &plpsMultiChatData)
{
	if (plpsMsg.IsEmpty())
		return ERROR_INVALID_HANDLE;

	LPSTR lpszHeader, lpszBody, lpszContentTypeLow;
	size_t dwHeaderSize, dwBodySize, dwContentTypeSize;
	CMStringA szTemp;

	if (MraOfflineMessageGetMIMEHeadAndBody(plpsMsg.GetString(), plpsMsg.GetLength(), &lpszHeader, &dwHeaderSize, &lpszBody, &dwBodySize) != NO_ERROR)
		return ERROR_INVALID_HANDLE;

	ptrA lpszHeaderLow((LPSTR)mir_calloc(dwHeaderSize));
	if (lpszHeaderLow == NULL)
		return ERROR_OUTOFMEMORY;
	BuffToLowerCase(lpszHeaderLow, lpszHeader, dwHeaderSize);

	if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "date", 4, szTemp) == NO_ERROR) {
		INTERNET_TIME itTime;
		InternetTimeGetTime(szTemp, itTime);
		pdwTime = MraOfflineMessageConvertTime(&itTime);
	}
	else pdwTime = 0;

	if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "x-mrim-flags", 12, szTemp) == NO_ERROR)
		pdwFlags = StrHexToUNum32(szTemp, szTemp.GetLength());
	else
		pdwFlags = 0;

	DWORD dwMultichatType;
	if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "x-mrim-multichat-type", 21, szTemp) == NO_ERROR)
		dwMultichatType = StrHexToUNum32(szTemp, szTemp.GetLength());
	else
		dwMultichatType = 0;

	if (MraOfflineMessageGetHeaderValue(lpszHeader, lpszHeaderLow, dwHeaderSize, "from", 4, plpsEMail) != NO_ERROR)
		plpsEMail.Empty();

	plpsText.Empty();
	plpsRTFText.Empty();
	plpsMultiChatData.Empty();

	if (MraOfflineMessageGetHeaderValueLow(lpszHeaderLow, dwHeaderSize, "content-type", 12, &lpszContentTypeLow, &dwContentTypeSize) != NO_ERROR)
		return ERROR_NOT_FOUND;

	if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "multipart/alternative", 21)) {
		// Content-Type: multipart/alternative; boundary = 1217508709J3777283291217508709T31197726
		LPSTR lpszBoundary = (LPSTR)MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "boundary=", 9);
		if (lpszBoundary) {
			size_t dwBoundarySize = ((dwContentTypeSize - (lpszBoundary - lpszContentTypeLow)) - 9);
			lpszBoundary = lpszHeader + (lpszBoundary + 9 - (LPSTR)lpszHeaderLow);

			int i = 0;
			size_t dwMIMEPartSize, dwMIMEHeaderSize, dwMIMEBodySize, dwMIMEContentTypeSize;
			LPSTR lpszCurMIMEPos = lpszBody, lpszMIMEPart, lpszMIMEHeader, lpszMIMEBody;
			while (MraOfflineMessageGetNextMIMEPart(lpszBody, dwBodySize, lpszBoundary, dwBoundarySize, lpszCurMIMEPos, &lpszMIMEPart, &dwMIMEPartSize) == NO_ERROR) {
				if (MraOfflineMessageGetMIMEHeadAndBody(lpszMIMEPart, dwMIMEPartSize, &lpszMIMEHeader, &dwMIMEHeaderSize, &lpszMIMEBody, &dwMIMEBodySize) == NO_ERROR) {
					ptrA lpszMIMEHeaderLow((LPSTR)mir_calloc(dwMIMEHeaderSize));
					if (lpszMIMEHeaderLow == NULL)
						return ERROR_OUTOFMEMORY;

					BuffToLowerCase(lpszMIMEHeaderLow, lpszMIMEHeader, dwMIMEHeaderSize);

					LPSTR lpszMIMEContentType;
					if (MraOfflineMessageGetHeaderValueLow(lpszMIMEHeaderLow, dwMIMEHeaderSize, "content-type", 12, &lpszMIMEContentType, &dwMIMEContentTypeSize) == NO_ERROR) {
						if (MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "text/plain", 10)) {
							// this is simple text part: text/plain
							return PlainText2message(CMStringA(lpszMIMEContentType, dwMIMEContentTypeSize), CMStringA(lpszMIMEBody, dwMIMEBodySize), plpsText, &pdwFlags);
						}
						if (MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "application/x-mrim-rtf", 22)) {
							plpsRTFText = CMStringA(lpszMIMEBody, dwMIMEBodySize);
							pdwFlags |= MESSAGE_FLAG_RTF; // set RTF flag if not exist
							return NO_ERROR;
						}
						if (MemoryFind(0, lpszMIMEContentType, dwMIMEContentTypeSize, "application/x-mrim+xml", 22)) {
							plpsMultiChatData = CMStringA(lpszMIMEBody, dwMIMEBodySize);
							pdwFlags |= MESSAGE_FLAG_MULTICHAT; // set MESSAGE_FLAG_MULTICHAT flag if not exist
							return NO_ERROR;
						}
						_CrtDbgBreak();
					}
				}
				i++;
			}

			_ASSERTE((i > 3 || i == 0));
		}
		return ERROR_NOT_FOUND;
	}

	if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "text/plain", 10))
		return PlainText2message(CMStringA(lpszContentTypeLow, dwContentTypeSize), CMStringA(lpszBody, dwBodySize), plpsText, &pdwFlags);

	if (MemoryFind(0, lpszContentTypeLow, dwContentTypeSize, "application/x-mrim-auth-req", 27)) { // Content-Type: application/x-mrim-auth-req
		plpsText = CMStringA(lpszBody, dwBodySize);
		return NO_ERROR;
	}

	return ERROR_INVALID_HANDLE;
}
