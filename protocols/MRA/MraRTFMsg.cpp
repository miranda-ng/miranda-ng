#include "Mra.h"
#include "MraRTFMsg.h"



#define COLORTABLE_COUNT	8
#define RTF_COLORTBLCOLOURS "\\red255\\green0\\blue0;\\red0\\green255\\blue0;\\red0\\green0\\blue255;\\red255\\green0\\blue255;\\red255\\green255\\blue0;\\red0\\green255\\blue255;\\red0\\green0\\blue0;\\red255\\green255\\blue255;" 
static const LPSTR lpszColours[COLORTABLE_COUNT]=
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
#define SIMBOLS_COUNT	19
static const LPSTR lpszSimbols[SIMBOLS_COUNT]=
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
	"[color=red]",
	"[color=green]",
	"[color=blue]",
	"[color=magenta]",
	"[color=yellow]",
	"[color=cyan]",
	"[color=black]",
	"[color=white]",
};

static const SIZE_T dwcSimbolsCount[SIMBOLS_COUNT]=
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

static const LPSTR lpszRTFTags[SIMBOLS_COUNT]=
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

static const SIZE_T dwcRTFTagsCount[SIMBOLS_COUNT]=
{
	4,
	2,
	2,
	2,
	4,
	1,
	5,
	1,
	4,
	1,
	1,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
};


DWORD MraTextToRTFData(LPSTR lpszMessage,SIZE_T dwMessageSize,LPSTR lpszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize);


BOOL MraIsMessageFlashAnimation(LPWSTR lpwszMessage,SIZE_T dwMessageSize)
{
	BOOL bRet=FALSE;
	LPWSTR lpwszFounded;

	dwMessageSize*=sizeof(WCHAR);
	lpwszFounded=(LPWSTR)MemoryFind(0,lpwszMessage,dwMessageSize,L"<SMILE>id=flas",28);
	if (lpwszFounded)
	{
		if (MemoryFind(((lpwszFounded-lpwszMessage)+32),lpwszMessage,dwMessageSize,L"'</SMILE>",18)) bRet=TRUE;
	}
return(bRet);
}



DWORD MraTextToRTFData(LPSTR lpszMessage,SIZE_T dwMessageSize,LPSTR lpszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (lpszMessage && dwMessageSize && lpszMessageConverted && dwMessageConvertedBuffSize)
	{
		BYTE tm,bCurByte;
		LPSTR lpszMessageConvertedCur,lpszMessageConvertedMax;
		SIZE_T i;

		lpszMessageConvertedCur=lpszMessageConverted;
		lpszMessageConvertedMax=(lpszMessageConverted+dwMessageConvertedBuffSize);
		for(i=0;(i<dwMessageSize && (lpszMessageConvertedMax-lpszMessageConvertedCur)>3);i++)
		{
			//lpszMessageConvertedCur+=mir_snprintf(lpszMessageConvertedCur,8,"\\'%x",(*((BYTE*)lpszMessage+i)));

			(*((WORD*)lpszMessageConvertedCur))=(*((WORD*)"\\'"));
			bCurByte=(*((BYTE*)lpszMessage+i));
			tm=(bCurByte>>4)&0xf;
			lpszMessageConvertedCur[2]=(tm>9)? ('a'+tm-10):('0'+tm);

			tm=bCurByte&0xf;
			lpszMessageConvertedCur[3]=(tm>9)? ('a'+tm-10):('0'+tm);
			lpszMessageConvertedCur+=4;
			(*((BYTE*)lpszMessageConvertedCur))=0;
		}
		if (pdwMessageConvertedSize) (*pdwMessageConvertedSize)=(lpszMessageConvertedCur-lpszMessageConverted);
	}else{
		if (pdwMessageConvertedSize) (*pdwMessageConvertedSize)=0;
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraSybolsToRTFTags(DWORD dwFlags,LPSTR lpszMessage,SIZE_T dwMessageSize,LPSTR lpszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize)
{
	DWORD dwRetErrorCode=NO_ERROR;
	LPSTR lpszFounded[SIMBOLS_COUNT],lpszMessageConvertedCur,lpszMessageCur,lpszMessageCurPrev,lpszMessageConvertedMax;
	SIZE_T i,dwFirstFoundedIndex=0,dwFoundedCount=0,dwMemPartToCopy;

#ifdef _DEBUG //check table
	for(i=0;i<SIMBOLS_COUNT;i++)
	{
		if (lstrlenA(lpszSimbols[i])!=dwcSimbolsCount[i]) DebugBreak();
		if (lstrlenA(lpszRTFTags[i])!=dwcRTFTagsCount[i]) DebugBreak();
	}
#endif

	lpszMessageCurPrev=lpszMessage;
	lpszMessageConvertedCur=lpszMessageConverted;
	lpszMessageConvertedMax=(lpszMessageConverted+dwMessageConvertedBuffSize);
	for(i=0;i<SIMBOLS_COUNT;i++)
	{// loking for first time
		lpszFounded[i]=(LPSTR)MemoryFind((lpszMessageCurPrev-lpszMessage),lpszMessage,dwMessageSize,lpszSimbols[i],dwcSimbolsCount[i]);
		if (lpszFounded[i]) dwFoundedCount++;
	}

	while(dwFoundedCount)
	{
		for(i=0;i<SIMBOLS_COUNT;i++)
		{// looking for first to replace
			if (lpszFounded[i] && (lpszFounded[i]<lpszFounded[dwFirstFoundedIndex] || lpszFounded[dwFirstFoundedIndex]==NULL)) dwFirstFoundedIndex=i;
		}

		if (lpszFounded[dwFirstFoundedIndex])
		{// founded
			dwMemPartToCopy=(lpszFounded[dwFirstFoundedIndex]-lpszMessageCurPrev);
			if (lpszMessageConvertedMax>(lpszMessageConvertedCur+(dwMemPartToCopy+dwcRTFTagsCount[dwFirstFoundedIndex])))
			{
				//memmove(lpszMessageConvertedCur,lpszMessageCurPrev,dwMemPartToCopy);lpszMessageConvertedCur+=dwMemPartToCopy;
				MraTextToRTFData(lpszMessageCurPrev,dwMemPartToCopy,lpszMessageConvertedCur,(lpszMessageConvertedMax-lpszMessageConvertedCur),&i);lpszMessageConvertedCur+=i;
				memmove(lpszMessageConvertedCur,lpszRTFTags[dwFirstFoundedIndex],dwcRTFTagsCount[dwFirstFoundedIndex]);lpszMessageConvertedCur+=dwcRTFTagsCount[dwFirstFoundedIndex];
				lpszMessageCurPrev=(lpszFounded[dwFirstFoundedIndex]+dwcSimbolsCount[dwFirstFoundedIndex]);

				for(i=0;i<SIMBOLS_COUNT;i++)
				{// loking for next time
					if (lpszFounded[i] && lpszFounded[i]<lpszMessageCurPrev)
					{
						dwFoundedCount--;// вычитаем тут, чтобы учесть схожие смайлы: "):-(" и ":-("
						lpszFounded[i]=(LPSTR)MemoryFind((lpszMessageCurPrev-lpszMessage),lpszMessage,dwMessageSize,lpszSimbols[i],dwcSimbolsCount[i]);
						if (lpszFounded[i]) dwFoundedCount++;
					}
				}
			}else{
				dwRetErrorCode=ERROR_BUFFER_OVERFLOW;
				DebugBreak();
				break;
			}
		}else{// сюда по идее никогда не попадём, на всякий случай.
			DebugBreak();
			break;
		}
	}
	lpszMessageCur=(lpszMessage+dwMessageSize);
	//memmove(lpszMessageConvertedCur,lpszMessageCurPrev,(lpszMessageCur-lpszMessageCurPrev));lpszMessageConvertedCur+=(lpszMessageCur-lpszMessageCurPrev);
	MraTextToRTFData(lpszMessageCurPrev,(lpszMessageCur-lpszMessageCurPrev),lpszMessageConvertedCur,(lpszMessageConvertedMax-lpszMessageConvertedCur),&i);lpszMessageConvertedCur+=i;
	(*((WORD*)lpszMessageConvertedCur))=0;

	if (pdwMessageConvertedSize) (*pdwMessageConvertedSize)=(lpszMessageConvertedCur-lpszMessageConverted);

return(dwRetErrorCode);
}


DWORD MraConvertToRTFW(LPWSTR lpwszMessage,SIZE_T dwMessageSize,LPSTR lpszMessageRTF,SIZE_T dwMessageRTFBuffSize,SIZE_T *pdwMessageRTFSize)
{
	DWORD dwRetErrorCode;

	if (lpwszMessage && lpszMessageRTF)
	{
		if ((dwMessageSize+1024)<=dwMessageRTFBuffSize)
		{
			LPSTR lpszMessage;

			lpszMessage=(LPSTR)MEMALLOC(dwMessageSize+32);
			if (lpszMessage)
			{
				LPSTR lpszMessageRTFCur=lpszMessageRTF;
				SIZE_T dwtm;
				DWORD dwRTFFontColour,dwFontSize;
				LOGFONT lf={0};

				WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszMessage,dwMessageSize,lpszMessage,(dwMessageSize+32),NULL,NULL);

				dwRTFFontColour=DB_Mra_GetDword(NULL,"RTFFontColour",MRA_DEFAULT_RTF_FONT_COLOUR);
				if (DB_Mra_GetContactSettingBlob(NULL,"RTFFont",&lf,sizeof(LOGFONT),NULL))
				{
					/*HDC hDC=GetDC(NULL);// kegl
					dwFontSize=-MulDiv(lf.lfHeight,72,GetDeviceCaps(hDC,LOGPIXELSY));
					ReleaseDC(NULL,hDC);*/
				}else{
					HDC hDC=GetDC(NULL);// kegl
					lf.lfCharSet=MRA_DEFAULT_RTF_FONT_CHARSET;
					lf.lfHeight=-MulDiv(MRA_DEFAULT_RTF_FONT_SIZE,GetDeviceCaps(hDC,LOGPIXELSY),72);
					lstrcpynW(lf.lfFaceName,MRA_DEFAULT_RTF_FONT_NAME,LF_FACESIZE);
					ReleaseDC(NULL,hDC);
				}
				dwFontSize=((-lf.lfHeight)+(((-lf.lfHeight)+4)/8));
/* mail agent: привет
{\rtf1\ansi\ansicpg1251\deff0\deflang1049{\fonttbl{\f0\fnil\fcharset204 Tahoma;}}
{\colortbl ;\red255\green0\blue0;}
\viewkind4\uc1\pard\cf1\f0\fs18\'ef\'f0\'e8\'e2\'e5\'f2\par
}

{\rtf1\ansi\ansicpg1251\deff0\deflang1049{\fonttbl{\f0\fnil\fcharset204 Tahoma;}}
{\colortbl ;\red255\green0\blue0;}
\viewkind4\uc1\pard\cf1\f0\fs18 1\par
2\par
3\par
}
*/
				lpszMessageRTFCur+=mir_snprintf(lpszMessageRTFCur,(dwMessageRTFBuffSize-((SIZE_T)lpszMessageRTFCur-(SIZE_T)lpszMessageRTF)),"{\\rtf1\\ansi\\ansicpg1251\\deff0\\deflang1049{\\fonttbl{\\f0\\fnil\\fcharset%lu %s;}}\r\n",lf.lfCharSet,lf.lfFaceName);

				if (MemoryFind(0,lpszMessage,dwMessageSize,BB_COLOR_TAG,(sizeof(BB_COLOR_TAG)-1)))
				{//BB_COLOR_TAG// добавляем таблицу с цветами, только если текст подкрашен
					lpszMessageRTFCur+=mir_snprintf(lpszMessageRTFCur,(dwMessageRTFBuffSize-((SIZE_T)lpszMessageRTFCur-(SIZE_T)lpszMessageRTF)),"{\\colortbl;\\red%lu\\green%lu\\blue%lu;%s}\r\n",(*((RGBTRIPLE*)&dwRTFFontColour)).rgbtBlue,(*((RGBTRIPLE*)&dwRTFFontColour)).rgbtGreen,(*((RGBTRIPLE*)&dwRTFFontColour)).rgbtRed,RTF_COLORTBLCOLOURS);
				}else{
					lpszMessageRTFCur+=mir_snprintf(lpszMessageRTFCur,(dwMessageRTFBuffSize-((SIZE_T)lpszMessageRTFCur-(SIZE_T)lpszMessageRTF)),"{\\colortbl;\\red%lu\\green%lu\\blue%lu;}\r\n",(*((RGBTRIPLE*)&dwRTFFontColour)).rgbtBlue,(*((RGBTRIPLE*)&dwRTFFontColour)).rgbtGreen,(*((RGBTRIPLE*)&dwRTFFontColour)).rgbtRed);
				}

				{
					LPSTR	lpszNotfink="",
							lpszBold=((lf.lfWeight==FW_BOLD)? "\\b1":lpszNotfink),
							lpszItalic=(lf.lfItalic? "\\i1":lpszNotfink),
							lpszUnderline=(lf.lfUnderline? "\\ul1":lpszNotfink),
							lpszStrikeOut=(lf.lfStrikeOut? "\\strike1":lpszNotfink);
					lpszMessageRTFCur+=mir_snprintf(lpszMessageRTFCur,(dwMessageRTFBuffSize-((SIZE_T)lpszMessageRTFCur-(SIZE_T)lpszMessageRTF)),"\\viewkind4\\uc1\\pard\\cf1\\f0\\fs%lu%s%s%s%s",dwFontSize,lpszBold,lpszItalic,lpszUnderline,lpszStrikeOut);
				}

				if ((dwRetErrorCode=MraSybolsToRTFTags(0,lpszMessage,dwMessageSize,lpszMessageRTFCur,(dwMessageRTFBuffSize-(lpszMessageRTFCur-lpszMessageRTF)),&dwtm))==NO_ERROR)
				{
					lpszMessageRTFCur+=dwtm;
					if ((lpszMessageRTF+dwMessageRTFBuffSize)>=(lpszMessageRTFCur+sizeof(PAR)+sizeof(CRLF)+2))
					{
						memmove(lpszMessageRTFCur,PAR,sizeof(PAR));lpszMessageRTFCur+=(sizeof(PAR)-1);
						memmove(lpszMessageRTFCur,CRLF,sizeof(CRLF));lpszMessageRTFCur+=(sizeof(CRLF)-1);
						memmove(lpszMessageRTFCur,"}",2);lpszMessageRTFCur+=2;
						if (pdwMessageRTFSize) (*pdwMessageRTFSize)=(lpszMessageRTFCur-lpszMessageRTF);
						DebugPrintCRLFA(lpszMessageRTF);
						dwRetErrorCode=NO_ERROR;
					}else{
						if (pdwMessageRTFSize) (*pdwMessageRTFSize)=dwMessageRTFBuffSize+1024;
						dwRetErrorCode=ERROR_BUFFER_OVERFLOW;
						DebugBreak();
					}
				}
				MEMFREE(lpszMessage);
			}
		}else{
			if (pdwMessageRTFSize) (*pdwMessageRTFSize)=dwMessageSize;
			dwRetErrorCode=ERROR_BUFFER_OVERFLOW;
			DebugBreak();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



