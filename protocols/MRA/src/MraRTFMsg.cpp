#include "Mra.h"
#include "MraRTFMsg.h"


#define COLORTABLE_COUNT	8
#define RTF_COLORTBLCOLOURS "\\red255\\green0\\blue0;\\red0\\green255\\blue0;\\red0\\green0\\blue255;\\red255\\green0\\blue255;\\red255\\green255\\blue0;\\red0\\green255\\blue255;\\red0\\green0\\blue0;\\red255\\green255\\blue255;"
static const LPSTR lpszColours[COLORTABLE_COUNT] =
{
	"red",
	"green",
	"blue",
	"magenta",
	"yellow",
	"cyan",
	"black",
	"white"
};

#define CRLF "\r\n"
#define PAR "\\par "


#define BB_COLOR_TAG	"[color="
#define SYMBOLS_COUNT	19
static const LPSTR lpszSimbols[SYMBOLS_COUNT] =
{
	"\r\n",
	"\\",
	"{",
	"}",
	"[b]",
	"[/b]",
	"[u]",
	"[/u]",
	"[i]",
	"[/i]",
	"[/color]",
	"[color = red]",
	"[color = green]",
	"[color = blue]",
	"[color = magenta]",
	"[color = yellow]",
	"[color = cyan]",
	"[color = black]",
	"[color = white]",
};

static const size_t dwcSimbolsCount[SYMBOLS_COUNT] =
{
	2,
	1,
	1,
	1,
	3,
	4,
	3,
	4,
	3,
	4,
	8,
	11,
	13,
	12,
	15,
	14,
	12,
	13,
	13,
};

static const LPSTR lpszRTFTags[SYMBOLS_COUNT] =
{
	"\\par",
	"\\\\",
	"\\{",
	"\\}",
	"{\\b ",
	"}",
	"{\\ul ",
	"}",
	"{\\i ",
	"}",
	"}",
	"{\\cf2 ",
	"{\\cf3 ",
	"{\\cf4 ",
	"{\\cf5 ",
	"{\\cf6 ",
	"{\\cf7 ",
	"{\\cf8 ",
	"{\\cf9 ",
};

static const size_t dwcRTFTagsCount[SYMBOLS_COUNT] =
{
	4, 2, 2, 2, 4, 1, 5, 1, 4, 1, 1, 6, 6, 6, 6, 6, 6, 6, 6
};

DWORD MraTextToRTFData(LPSTR lpszMessage, size_t dwMessageSize, LPSTR lpszMessageConverted, size_t dwMessageConvertedBuffSize, size_t *pdwMessageConvertedSize);

bool MraIsMessageFlashAnimation(const CMStringW &lpwszMessage)
{
	int iStart = lpwszMessage.Find(_T("<SMILE>id = flas"));
	if (iStart != -1)
		if (lpwszMessage.Find(_T("'</SMILE>"), iStart) != -1)
			return true;

	return false;
}

DWORD MraTextToRTFData(LPSTR lpszMessage, size_t dwMessageSize, LPSTR lpszMessageConverted, size_t dwMessageConvertedBuffSize, size_t *pdwMessageConvertedSize)
{
	if (lpszMessage && dwMessageSize && lpszMessageConverted && dwMessageConvertedBuffSize) {
		BYTE tm, bCurByte;
		LPSTR lpszMessageConvertedCur, lpszMessageConvertedMax;
		size_t i;

		lpszMessageConvertedCur = lpszMessageConverted;
		lpszMessageConvertedMax = (lpszMessageConverted+dwMessageConvertedBuffSize);
		for (i = 0; i < dwMessageSize && lpszMessageConvertedMax-lpszMessageConvertedCur > 3; i++) {
			*((WORD*)lpszMessageConvertedCur) = *((WORD*)"\\'");
			bCurByte = (*((BYTE*)lpszMessage+i));
			tm = (bCurByte>>4)&0xf;
			lpszMessageConvertedCur[2] = (tm>9)? ('a'+tm-10):('0'+tm);

			tm = bCurByte&0xf;
			lpszMessageConvertedCur[3] = (tm>9)? ('a'+tm-10):('0'+tm);
			lpszMessageConvertedCur += 4;
			(*((BYTE*)lpszMessageConvertedCur)) = 0;
		}
		if (pdwMessageConvertedSize)
			*pdwMessageConvertedSize = lpszMessageConvertedCur - lpszMessageConverted;
		return NO_ERROR;
	}

	if (pdwMessageConvertedSize)
		*pdwMessageConvertedSize = 0;
	return ERROR_INVALID_HANDLE;
}

DWORD MraSymbolsToRTFTags(LPSTR lpszMessage, size_t dwMessageSize, LPSTR lpszMessageConverted, size_t dwMessageConvertedBuffSize, size_t *pdwMessageConvertedSize)
{
	DWORD dwRetErrorCode = NO_ERROR;
	LPSTR lpszFounded[SYMBOLS_COUNT], lpszMessageConvertedCur, lpszMessageCur, lpszMessageCurPrev, lpszMessageConvertedMax;
	size_t i, dwFirstFoundIndex = 0, dwFoundCount = 0, dwMemPartToCopy;

	lpszMessageCurPrev = lpszMessage;
	lpszMessageConvertedCur = lpszMessageConverted;
	lpszMessageConvertedMax = (lpszMessageConverted+dwMessageConvertedBuffSize);
	for (i = 0; i < SYMBOLS_COUNT; i++) { // loking for first time
		lpszFounded[i] = (LPSTR)MemoryFind((lpszMessageCurPrev-lpszMessage), lpszMessage, dwMessageSize, lpszSimbols[i], dwcSimbolsCount[i]);
		if (lpszFounded[i]) dwFoundCount++;
	}

	while (dwFoundCount) {
		for (i = 0;i<SYMBOLS_COUNT;i++)
			if (lpszFounded[i] && (lpszFounded[i]<lpszFounded[dwFirstFoundIndex] || lpszFounded[dwFirstFoundIndex] == NULL))
				dwFirstFoundIndex = i;

		if (lpszFounded[dwFirstFoundIndex]) { // found
			dwMemPartToCopy = (lpszFounded[dwFirstFoundIndex]-lpszMessageCurPrev);
			if (lpszMessageConvertedMax > (lpszMessageConvertedCur+(dwMemPartToCopy+dwcRTFTagsCount[dwFirstFoundIndex]))) {
				MraTextToRTFData(lpszMessageCurPrev, dwMemPartToCopy, lpszMessageConvertedCur, (lpszMessageConvertedMax-lpszMessageConvertedCur), &i);lpszMessageConvertedCur += i;
				memcpy(lpszMessageConvertedCur, lpszRTFTags[dwFirstFoundIndex], dwcRTFTagsCount[dwFirstFoundIndex]);lpszMessageConvertedCur += dwcRTFTagsCount[dwFirstFoundIndex];
				lpszMessageCurPrev = (lpszFounded[dwFirstFoundIndex]+dwcSimbolsCount[dwFirstFoundIndex]);

				for (i = 0;i<SYMBOLS_COUNT;i++) { // looking for the next time
					if (lpszFounded[i] && lpszFounded[i] < lpszMessageCurPrev) {
						dwFoundCount--;// вычитаем тут, чтобы учесть схожие смайлы: "):-(" и ":-("
						lpszFounded[i] = (LPSTR)MemoryFind((lpszMessageCurPrev-lpszMessage), lpszMessage, dwMessageSize, lpszSimbols[i], dwcSimbolsCount[i]);
						if (lpszFounded[i]) dwFoundCount++;
					}
				}
			}
			else {
				dwRetErrorCode = ERROR_BUFFER_OVERFLOW;
				_CrtDbgBreak();
				break;
			}
		}
	}

	lpszMessageCur = (lpszMessage+dwMessageSize);
	MraTextToRTFData(lpszMessageCurPrev, (lpszMessageCur-lpszMessageCurPrev), lpszMessageConvertedCur, (lpszMessageConvertedMax-lpszMessageConvertedCur), &i);lpszMessageConvertedCur += i;
	*((WORD*)lpszMessageConvertedCur) = 0;

	if (pdwMessageConvertedSize)
		*pdwMessageConvertedSize = lpszMessageConvertedCur - lpszMessageConverted;

	return(dwRetErrorCode);
}

DWORD CMraProto::MraConvertToRTFW(const CMStringW &wszMessage, CMStringA &szMessageRTF)
{
	if (wszMessage.IsEmpty())
		return ERROR_INVALID_HANDLE;

	ptrA lpszMessage( mir_u2a(wszMessage));
	if (!lpszMessage)
		return GetLastError();

	LPCSTR lpszBase = szMessageRTF;
	LPSTR lpszMessageRTFCur = (LPSTR)lpszBase;
	size_t dwtm;
	DWORD dwRTFFontColour, dwFontSize;
	LOGFONT lf = {0};

	dwRTFFontColour = getDword("RTFFontColour", MRA_DEFAULT_RTF_FONT_COLOUR);
	if ( !mraGetContactSettingBlob(NULL, "RTFFont", &lf, sizeof(LOGFONT), NULL)) {
		HDC hDC = GetDC(NULL);// kegl
		lf.lfCharSet = MRA_DEFAULT_RTF_FONT_CHARSET;
		lf.lfHeight = -MulDiv(MRA_DEFAULT_RTF_FONT_SIZE, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		mir_tstrncpy(lf.lfFaceName, MRA_DEFAULT_RTF_FONT_NAME, LF_FACESIZE);
		ReleaseDC(NULL, hDC);
	}
	dwFontSize = ((-lf.lfHeight)+(((-lf.lfHeight)+4)/8));

	lpszMessageRTFCur += mir_snprintf(lpszMessageRTFCur, (szMessageRTF.GetLength()-(lpszMessageRTFCur-lpszBase)), "{\\rtf1\\ansi\\ansicpg1251\\deff0\\deflang1049{\\fonttbl{\\f0\\fnil\\fcharset%lu %s;}}\r\n", lf.lfCharSet, lf.lfFaceName);

	if (MemoryFind(0, lpszMessage, wszMessage.GetLength(), BB_COLOR_TAG, (sizeof(BB_COLOR_TAG)-1)))
		lpszMessageRTFCur += mir_snprintf(lpszMessageRTFCur, (szMessageRTF.GetLength()-(lpszMessageRTFCur-lpszBase)), "{\\colortbl;\\red%lu\\green%lu\\blue%lu;%s}\r\n", (*((RGBTRIPLE*)&dwRTFFontColour)).rgbtBlue, (*((RGBTRIPLE*)&dwRTFFontColour)).rgbtGreen, (*((RGBTRIPLE*)&dwRTFFontColour)).rgbtRed, RTF_COLORTBLCOLOURS);
	else
		lpszMessageRTFCur += mir_snprintf(lpszMessageRTFCur, (szMessageRTF.GetLength()-(lpszMessageRTFCur-lpszBase)), "{\\colortbl;\\red%lu\\green%lu\\blue%lu;}\r\n", (*((RGBTRIPLE*)&dwRTFFontColour)).rgbtBlue, (*((RGBTRIPLE*)&dwRTFFontColour)).rgbtGreen, (*((RGBTRIPLE*)&dwRTFFontColour)).rgbtRed);

	LPSTR	lpszNotfink = "",
			lpszBold = ((lf.lfWeight == FW_BOLD)? "\\b1":lpszNotfink),
			lpszItalic = (lf.lfItalic? "\\i1":lpszNotfink),
			lpszUnderline = (lf.lfUnderline? "\\ul1":lpszNotfink),
			lpszStrikeOut = (lf.lfStrikeOut? "\\strike1":lpszNotfink);
	lpszMessageRTFCur += mir_snprintf(lpszMessageRTFCur, (szMessageRTF.GetLength()-(lpszMessageRTFCur-lpszBase)), "\\viewkind4\\uc1\\pard\\cf1\\f0\\fs%lu%s%s%s%s", dwFontSize, lpszBold, lpszItalic, lpszUnderline, lpszStrikeOut);

	if (!MraSymbolsToRTFTags(lpszMessage, wszMessage.GetLength(), lpszMessageRTFCur, (szMessageRTF.GetLength() - (lpszMessageRTFCur - lpszBase)), &dwtm)) {
		lpszMessageRTFCur += dwtm;
		if ((lpszBase + szMessageRTF.GetLength()) >= (lpszMessageRTFCur + sizeof(PAR) + sizeof(CRLF) + 2)) {
			memcpy(lpszMessageRTFCur, PAR, sizeof(PAR)); lpszMessageRTFCur += (sizeof(PAR) - 1);
			memcpy(lpszMessageRTFCur, CRLF, sizeof(CRLF)); lpszMessageRTFCur += (sizeof(CRLF) - 1);
			memcpy(lpszMessageRTFCur, "}", 2); lpszMessageRTFCur += 2;
			debugLogA("%s\n", szMessageRTF);
			return NO_ERROR;
		}

		return ERROR_BUFFER_OVERFLOW;
	}

	return 0;
}
