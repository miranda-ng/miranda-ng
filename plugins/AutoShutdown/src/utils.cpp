/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

/************************* String *********************************/

// mir_free() the return value
char* u2a(const WCHAR *pszUnicode)
{
	int codepage,cch;
	char *psz;
	DWORD flags;

	if (pszUnicode==NULL) return NULL;
	codepage=CallService(MS_LANGPACK_GETCODEPAGE,0,0);
	/* without WC_COMPOSITECHECK some characters might get out strange (see MS blog) */
	cch=WideCharToMultiByte(codepage,flags=WC_COMPOSITECHECK,pszUnicode,-1,NULL,0,NULL,NULL);
	if (!cch) cch=WideCharToMultiByte(codepage,flags=0,pszUnicode,-1,NULL,0,NULL,NULL);
	if (!cch) return NULL;

	psz=(char*)mir_alloc(cch);
	if (psz != NULL && !WideCharToMultiByte(codepage,flags,pszUnicode,-1,psz,cch,NULL,NULL)){
		mir_free(psz);
		return NULL;
	}
	return psz;
}

void TrimString(TCHAR *pszStr)
{
	int i;
	TCHAR *psz,szChars[]=_T(" \r\n\t");
	for(i=0;i<SIZEOF(szChars);++i) {
		/* trim end */
		psz=&pszStr[mir_tstrlen(pszStr)-1];
		while(pszStr[0] && *psz==szChars[i]) {
			*psz=0;
			psz=CharPrev(pszStr,psz);
		}
		/* trim beginning */
		for(psz=pszStr;(*psz && *psz==szChars[i]);psz=CharNext(psz));
		memmove(pszStr,psz,(mir_tstrlen(psz)+1)*sizeof(TCHAR));
	}
}

/************************* Error Output ***************************/

static void MessageBoxIndirectFree(void *param)
{
	MSGBOXPARAMSA *mbp = (MSGBOXPARAMSA*)param;
	MessageBoxIndirectA(mbp);
	mir_free((char*)mbp->lpszCaption); /* does NULL check */
	mir_free((char*)mbp->lpszText);    /* does NULL check */
	mir_free(mbp);
}

void ShowInfoMessage(BYTE flags,const char *pszTitle,const char *pszTextFmt,...)
{
	char szText[256]; /* max for systray */
	MSGBOXPARAMSA *mbp;

	va_list va;
	va_start(va,pszTextFmt);
	mir_vsnprintf(szText,SIZEOF(szText),pszTextFmt,va);
	va_end(va);

	if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY msn;
		msn.cbSize=sizeof(msn);
		msn.szProto=NULL;
		msn.szInfoTitle=(char*)pszTitle;
		msn.szInfo=(char*)szText;
		msn.uTimeout=30000; /* max timeout */
		msn.dwInfoFlags=flags;
		if (!CallServiceSync(MS_CLIST_SYSTRAY_NOTIFY,0,(LPARAM)&msn))
			return; /* success */
	}

	mbp=(MSGBOXPARAMSA*)mir_calloc(sizeof(*mbp));
	if (mbp==NULL) return;
	mbp->cbSize=sizeof(*mbp);
	mbp->lpszCaption=mir_strdup(pszTitle);
	mbp->lpszText=mir_strdup(szText);
	mbp->dwStyle=MB_OK|MB_SETFOREGROUND|MB_TASKMODAL;
	mbp->dwLanguageId=LANGIDFROMLCID((LCID)CallService(MS_LANGPACK_GETLOCALE,0,0));
	switch(flags&NIIF_ICON_MASK) {
		case NIIF_INFO:    mbp->dwStyle|=MB_ICONINFORMATION; break;
		case NIIF_WARNING: mbp->dwStyle|=MB_ICONWARNING; break;
		case NIIF_ERROR:   mbp->dwStyle|=MB_ICONERROR;
	}
	mir_forkthread(MessageBoxIndirectFree, mbp);
}

// LocalFree() the return value
char* GetWinErrorDescription(DWORD dwLastError)
{
	char *buf=NULL;
	DWORD flags=FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM;
	if (!FormatMessageA(flags,NULL,dwLastError,LANGIDFROMLCID((LCID)CallService(MS_LANGPACK_GETLOCALE,0,0)),(char*)&buf,0,NULL))
		if (GetLastError()==ERROR_RESOURCE_LANG_NOT_FOUND)
			FormatMessageA(flags,NULL,dwLastError,0,(char*)&buf,0,NULL);
	return buf;
}

/************************* Time ***********************************/

BOOL SystemTimeToTimeStamp(SYSTEMTIME *st,time_t *timestamp)
{
	struct tm ts;
	ts.tm_isdst = -1;             /* daylight saving time (-1=compute) */
	ts.tm_sec = st->wSecond;      /* 0-59 */
	ts.tm_min = st->wMinute;      /* 0-59 */
	ts.tm_hour = st->wHour;       /* 0-23 */
	ts.tm_mday = st->wDay;        /* 1-31 */
	ts.tm_wday = st->wDayOfWeek;  /* 0-6 (Sun-Sat) */
	ts.tm_mon = st->wMonth-1;     /* 0-11 (Jan-Dec) */
	ts.tm_year = st->wYear-1900;  /* current year minus 1900 */
	ts.tm_yday=0;               /* 0-365 (Jan1=0) */
	*timestamp = mktime(&ts);
	return (*timestamp != -1);
}

BOOL TimeStampToSystemTime(time_t timestamp,SYSTEMTIME *st)
{
	struct tm ts = {0};
	errno_t err = localtime_s(&ts, &timestamp);  /* statically alloced, local time correction */
	if (err != 0)
		return FALSE;

	st->wMilliseconds = 0;                 /* 0-999 (not given in tm) */
	st->wSecond = (WORD)ts.tm_sec;       /* 0-59 */
	st->wMinute = (WORD)ts.tm_min;       /* 0-59 */
	st->wHour = (WORD)ts.tm_hour;        /* 0-23 */
	st->wDay = (WORD)ts.tm_mday;         /* 1-31 */
	st->wDayOfWeek = (WORD)ts.tm_wday;   /* 0-6 (Sun-Sat) */
	st->wMonth = (WORD)(ts.tm_mon+1);    /* 1-12 (Jan-Dec) */
	st->wYear = (WORD)(ts.tm_year+1900); /* 1601-30827 */
	return TRUE;
}

BOOL GetFormatedCountdown(TCHAR *pszOut,int nSize,time_t countdown)
{
	static BOOL fInited=FALSE;
	static int (WINAPI *pfnGetDurationFormat)(LCID,DWORD,const SYSTEMTIME*,double,WCHAR*,WCHAR*,int);
	/* Init */
	if (!fInited && IsWinVerVistaPlus()) {
		*(PROC*)&pfnGetDurationFormat=GetProcAddress(GetModuleHandleA("KERNEL32"),"GetDurationFormat");
		fInited=TRUE;
	}
	/* WinVista */
	if (pfnGetDurationFormat != NULL) {
		SYSTEMTIME st;
		LCID locale;
		locale=(LCID)CallService(MS_LANGPACK_GETLOCALE,0,0);
		if (TimeStampToSystemTime(countdown,&st))
			if (pfnGetDurationFormat(locale,0,&st,0,NULL,pszOut,nSize))
				return TRUE;
		return FALSE;
	} else
	/* Win9x/NT/XP */
		return StrFromTimeInterval(pszOut,nSize,(countdown>(MAXDWORD/1000))?MAXDWORD:(countdown*1000),10) != 0;
	return FALSE;
}

BOOL GetFormatedDateTime(TCHAR *pszOut,int nSize,time_t timestamp,BOOL fShowDateEvenToday)
{
	SYSTEMTIME st,stNow;
	LCID locale;
	locale=(LCID)CallService(MS_LANGPACK_GETLOCALE,0,0);
	GetLocalTime(&stNow);
	TimeStampToSystemTime(timestamp,&st);
	/* today: no need to show the date */
	if (!fShowDateEvenToday && st.wDay==stNow.wDay && st.wMonth==stNow.wMonth && st.wYear==stNow.wYear)
		return GetTimeFormat(locale,((st.wSecond==0)?TIME_NOSECONDS:0)|TIME_FORCE24HOURFORMAT,&st,NULL,pszOut,nSize) != 0;
	/* show both date and time */
	{	TCHAR szDate[128],szTime[128];
		if (!GetTimeFormat(locale,((st.wSecond==0)?TIME_NOSECONDS:0)|TIME_FORCE24HOURFORMAT,&st,NULL,szTime,SIZEOF(szTime)))
			return FALSE;
		if (!GetDateFormat(locale,DATE_SHORTDATE,&st,NULL,szDate,SIZEOF(szDate)))
			return FALSE;
		mir_sntprintf(pszOut,nSize,_T("%s %s"),szTime,szDate);
		return TRUE;
	}
}

/************************* Skin ***********************************/

HANDLE IcoLib_AddIconRes(const char *pszDbName,const TCHAR *pszSection,const TCHAR *pszDesc,HINSTANCE hInst,WORD idRes,BOOL fLarge)
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(hInst,szFileName,SIZEOF(szFileName));

	SKINICONDESC sid = { sizeof(sid) };
	sid.pszName = (char*)pszDbName;
	sid.ptszSection = (TCHAR*)pszSection;
	sid.ptszDescription = (TCHAR*)pszDesc;
	sid.ptszDefaultFile = szFileName;
	sid.iDefaultIndex = -idRes;
	sid.cx = GetSystemMetrics(fLarge?SM_CXICON:SM_CXSMICON);
	sid.cy = GetSystemMetrics(fLarge?SM_CYICON:SM_CYSMICON);
	sid.flags = SIDF_SORTED | SIDF_ALL_TCHAR;
	return Skin_AddIcon(&sid);
}

void AddHotkey()
{
	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.dwFlags = HKD_TCHAR;
	hkd.pszName = "AutoShutdown_Toggle";
	hkd.ptszDescription = LPGENT("Toggle Automatic Shutdown");
	hkd.ptszSection = LPGENT("Main");
	hkd.pszService = "AutoShutdown/MenuCommand";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'T') | HKF_MIRANDA_LOCAL;
	hkd.lParam = FALSE;
	Hotkey_Register(&hkd);
}
