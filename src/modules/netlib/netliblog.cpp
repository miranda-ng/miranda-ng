/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"
#include "netlib.h"
#include "../srfile/file.h"

#define MS_NETLIB_LOGWIN "Netlib/Log/Win"

extern HANDLE hConnectionHeaderMutex;

#define TIMEFORMAT_NONE         0
#define TIMEFORMAT_HHMMSS       1
#define TIMEFORMAT_MILLISECONDS 2
#define TIMEFORMAT_MICROSECONDS 3
struct {
	HWND   hwndOpts;
	int    toOutputDebugString;
	int    toFile;
	int    toLog;
	TCHAR* szFile;
	TCHAR* szUserFile;
	int    timeFormat;
	int    showUser;
	int    dumpSent,dumpRecv,dumpProxy,dumpSsl;
	int    textDumps,autoDetectText;
	CRITICAL_SECTION cs;
	int    save;
} logOptions = {0};

typedef struct {
	const char* pszHead;
	const char* pszMsg;
} LOGMSG;

static __int64 mirandaStartTime,perfCounterFreq;
static int bIsActive = TRUE;
static HANDLE hLogEvent = NULL;

static const TCHAR* szTimeFormats[] =
{
	_T( "No times" ),
	_T( "Standard hh:mm:ss times" ),
	_T( "Times in milliseconds" ),
	_T( "Times in microseconds" )
};

static INT_PTR CALLBACK LogOptionsDlgProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		logOptions.hwndOpts=hwndDlg;
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg,IDC_DUMPRECV,logOptions.dumpRecv?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_DUMPSENT,logOptions.dumpSent?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_DUMPPROXY,logOptions.dumpProxy?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_DUMPSSL,logOptions.dumpSsl?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_TEXTDUMPS,logOptions.textDumps?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_AUTODETECTTEXT,logOptions.autoDetectText?BST_CHECKED:BST_UNCHECKED);
		{	int i;
			for( i=0; i < SIZEOF(szTimeFormats); i++ )
				SendDlgItemMessage(hwndDlg,IDC_TIMEFORMAT,CB_ADDSTRING,0,(LPARAM)TranslateTS( szTimeFormats[i] ));
		}
		SendDlgItemMessage(hwndDlg,IDC_TIMEFORMAT,CB_SETCURSEL,logOptions.timeFormat,0);
		CheckDlgButton(hwndDlg,IDC_SHOWNAMES,logOptions.showUser?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_TOOUTPUTDEBUGSTRING,logOptions.toOutputDebugString?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_TOFILE,logOptions.toFile?BST_CHECKED:BST_UNCHECKED);
		SetDlgItemText(hwndDlg,IDC_FILENAME,logOptions.szUserFile);
		SetDlgItemText(hwndDlg,IDC_PATH,logOptions.szFile);
		CheckDlgButton(hwndDlg,IDC_SHOWTHISDLGATSTART,DBGetContactSettingByte(NULL, "Netlib", "ShowLogOptsAtStart",0)?BST_CHECKED:BST_UNCHECKED);
		{	DBVARIANT dbv;
			if(!DBGetContactSettingString(NULL, "Netlib", "RunAtStart",&dbv)) {
				SetDlgItemTextA(hwndDlg,IDC_RUNATSTART,dbv.pszVal);
				DBFreeVariant(&dbv);
			}
		}
		logOptions.save = 0;
		{
			TVINSERTSTRUCT tvis = {0};
			int i;
			HWND hwndFilter = GetDlgItem(hwndDlg,IDC_FILTER);

			SetWindowLongPtr(hwndFilter, GWL_STYLE, GetWindowLongPtr(hwndFilter, GWL_STYLE) | (TVS_NOHSCROLL | TVS_CHECKBOXES));

			tvis.hParent=NULL;
			tvis.hInsertAfter=TVI_SORT;
			tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
			tvis.item.stateMask=TVIS_STATEIMAGEMASK;

			for (i = 0; i < netlibUser.getCount(); ++i)
			{
				tvis.item.pszText=netlibUser[i]->user.ptszDescriptiveName;
				tvis.item.lParam=i;
				tvis.item.state=INDEXTOSTATEIMAGEMASK( (netlibUser[i]->toLog) ? 2 : 1 );
				TreeView_InsertItem(hwndFilter, &tvis);
			}
			tvis.item.lParam=-1;
			tvis.item.pszText=TranslateT("(Miranda Core Logging)");
			tvis.item.state=INDEXTOSTATEIMAGEMASK( (logOptions.toLog) ? 2 : 1 );
			TreeView_InsertItem(hwndFilter, &tvis);
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
/*
		case IDC_DUMPRECV:
		case IDC_DUMPSENT:
		case IDC_DUMPPROXY:
		case IDC_TEXTDUMPS:
		case IDC_AUTODETECTTEXT:
		case IDC_TIMEFORMAT:
		case IDC_SHOWNAMES:
		case IDC_TOOUTPUTDEBUGSTRING:
		case IDC_TOFILE:
		case IDC_SHOWTHISDLGATSTART:
		case IDC_RUNATSTART:
			break;
*/
		case IDC_FILENAME:
			if(HIWORD(wParam)!=EN_CHANGE) break;
			if((HWND)lParam==GetFocus())
				CheckDlgButton(hwndDlg,IDC_TOFILE,BST_CHECKED);

			{	
				TCHAR path[MAX_PATH];
				GetWindowText((HWND)lParam, path, MAX_PATH);

				TCHAR *pszNewPath = Utils_ReplaceVarsT(path);
				pathToAbsoluteT(pszNewPath, path, NULL);
				SetDlgItemText(hwndDlg, IDC_PATH, path);
				mir_free(pszNewPath);
			}
			break;
		case IDC_FILENAMEBROWSE:
		case IDC_RUNATSTARTBROWSE:
		{	TCHAR str[MAX_PATH+2];
			OPENFILENAME ofn={0};
			TCHAR filter[512],*pfilter;

			GetWindowText(GetWindow((HWND)lParam,GW_HWNDPREV),str,SIZEOF(str));
			ofn.lStructSize=OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner=hwndDlg;
			ofn.Flags=OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
			if (LOWORD(wParam)==IDC_FILENAMEBROWSE) {
				ofn.lpstrTitle=TranslateT("Select where log file will be created");
			} else {
				ofn.Flags|=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
				ofn.lpstrTitle=TranslateT("Select program to be run");
			}
			_tcscpy(filter,TranslateT("All Files"));
			_tcscat(filter,_T(" (*)"));
			pfilter=filter+lstrlen(filter)+1;
			_tcscpy(pfilter,_T("*"));
			pfilter=pfilter+lstrlen(pfilter)+1;
			*pfilter='\0';
			ofn.lpstrFilter=filter;
			ofn.lpstrFile=str;
			ofn.nMaxFile=SIZEOF(str)-2;
			ofn.nMaxFileTitle=MAX_PATH;
			if (LOWORD(wParam)==IDC_FILENAMEBROWSE) {
				if(!GetSaveFileName(&ofn)) return 1;
			} else {
				if(!GetOpenFileName(&ofn)) return 1;
			}
			if(LOWORD(wParam)==IDC_RUNATSTARTBROWSE && _tcschr(str,' ')!=NULL) {
				MoveMemory(str+1,str,SIZEOF(str)-2);
				str[0]='"';
				lstrcat(str,_T("\""));
			}
			SetWindowText(GetWindow((HWND)lParam,GW_HWNDPREV),str);
			break;
		}
		case IDC_RUNNOW:
			{	TCHAR str[MAX_PATH+1];
				STARTUPINFO si={0};
				PROCESS_INFORMATION pi;
				GetDlgItemText(hwndDlg,IDC_RUNATSTART,str,MAX_PATH);
				si.cb=sizeof(si);
				if(str[0]) CreateProcess(NULL,str,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
			}
			break;
		case IDC_SAVE:
			logOptions.save = 1;
			//
		case IDOK:
			{
				TCHAR str[MAX_PATH];

				GetDlgItemText(hwndDlg, IDC_RUNATSTART, str, MAX_PATH);
				DBWriteContactSettingTString(NULL, "Netlib", "RunAtStart",str);
				DBWriteContactSettingByte(NULL, "Netlib", "ShowLogOptsAtStart",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWTHISDLGATSTART));

				EnterCriticalSection(&logOptions.cs);

				mir_free(logOptions.szUserFile);
				GetWindowText(GetDlgItem(hwndDlg,IDC_FILENAME), str, MAX_PATH );
				logOptions.szUserFile = mir_tstrdup(str);

				mir_free(logOptions.szFile);
				GetWindowText(GetDlgItem(hwndDlg,IDC_PATH), str, MAX_PATH );
				logOptions.szFile = mir_tstrdup(str);

				logOptions.dumpRecv=IsDlgButtonChecked(hwndDlg,IDC_DUMPRECV);
				logOptions.dumpSent=IsDlgButtonChecked(hwndDlg,IDC_DUMPSENT);
				logOptions.dumpProxy=IsDlgButtonChecked(hwndDlg,IDC_DUMPPROXY);
				logOptions.dumpSsl=IsDlgButtonChecked(hwndDlg,IDC_DUMPSSL);
				logOptions.textDumps=IsDlgButtonChecked(hwndDlg,IDC_TEXTDUMPS);
				logOptions.autoDetectText=IsDlgButtonChecked(hwndDlg,IDC_AUTODETECTTEXT);
				logOptions.timeFormat=SendDlgItemMessage(hwndDlg,IDC_TIMEFORMAT,CB_GETCURSEL,0,0);
				logOptions.showUser=IsDlgButtonChecked(hwndDlg,IDC_SHOWNAMES);
				logOptions.toOutputDebugString=IsDlgButtonChecked(hwndDlg,IDC_TOOUTPUTDEBUGSTRING);
				logOptions.toFile=IsDlgButtonChecked(hwndDlg,IDC_TOFILE);

				LeaveCriticalSection(&logOptions.cs);
			}
			{
				HWND hwndFilter = GetDlgItem(logOptions.hwndOpts, IDC_FILTER);
				TVITEM tvi={0};
				BOOL checked;

				tvi.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_STATE|TVIF_TEXT;
				tvi.hItem=TreeView_GetRoot(hwndFilter);

				while(tvi.hItem)
				{
					TreeView_GetItem(hwndFilter,&tvi);
					checked = ((tvi.state&TVIS_STATEIMAGEMASK)>>12==2);

					if (tvi.lParam == -1) {
						logOptions.toLog = checked;
					if ( logOptions.save )
						DBWriteContactSettingDword(NULL, "Netlib", "NLlog",checked);
					}
					else
					if (tvi.lParam < netlibUser.getCount()) {
						netlibUser[tvi.lParam]->toLog = checked;
						if ( logOptions.save )
							DBWriteContactSettingDword(NULL,netlibUser[tvi.lParam]->user.szSettingsModule,"NLlog",checked);
					}

					tvi.hItem=TreeView_GetNextSibling(hwndFilter,tvi.hItem);
				}
			}

			if ( logOptions.save ) {
				DBWriteContactSettingByte(NULL, "Netlib", "DumpRecv",(BYTE)logOptions.dumpRecv);
				DBWriteContactSettingByte(NULL, "Netlib", "DumpSent",(BYTE)logOptions.dumpSent);
				DBWriteContactSettingByte(NULL, "Netlib", "DumpProxy",(BYTE)logOptions.dumpProxy);
				DBWriteContactSettingByte(NULL, "Netlib", "DumpSsl",(BYTE)logOptions.dumpSsl);
				DBWriteContactSettingByte(NULL, "Netlib", "TextDumps",(BYTE)logOptions.textDumps);
				DBWriteContactSettingByte(NULL, "Netlib", "AutoDetectText",(BYTE)logOptions.autoDetectText);
				DBWriteContactSettingByte(NULL, "Netlib", "TimeFormat",(BYTE)logOptions.timeFormat);
				DBWriteContactSettingByte(NULL, "Netlib", "ShowUser",(BYTE)logOptions.showUser);
				DBWriteContactSettingByte(NULL, "Netlib", "ToOutputDebugString",(BYTE)logOptions.toOutputDebugString);
				DBWriteContactSettingByte(NULL, "Netlib", "ToFile",(BYTE)logOptions.toFile);
				DBWriteContactSettingTString(NULL, "Netlib", "File", logOptions.szFile ? logOptions.szUserFile: _T(""));
				logOptions.save = 0;
			}
			else
				DestroyWindow(hwndDlg);

			break;
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_FILTER), TVSIL_STATE));
		logOptions.hwndOpts=NULL;
		break;
	}
	return FALSE;
}

void NetlibLogShowOptions(void)
{
	if(logOptions.hwndOpts==NULL)
		logOptions.hwndOpts=CreateDialog(hMirandaInst,MAKEINTRESOURCE(IDD_NETLIBLOGOPTS),NULL,LogOptionsDlgProc);
	SetForegroundWindow(logOptions.hwndOpts);
}

static INT_PTR ShowOptions(WPARAM, LPARAM)
{
	NetlibLogShowOptions();
	return 0;
}

static INT_PTR NetlibLog(WPARAM wParam, LPARAM lParam)
{
	struct NetlibUser *nlu = (struct NetlibUser*)wParam;
	struct NetlibUser nludummy;
	const char *pszMsg = (const char*)lParam;
	char szTime[32], szHead[128];
	LARGE_INTEGER liTimeNow;
	DWORD dwOriginalLastError;

	if (!bIsActive)
		return 0;

	if ((nlu != NULL && GetNetlibHandleType(nlu) != NLH_USER) || pszMsg == NULL) 
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if (nlu == NULL) /* if the Netlib user handle is NULL, just pretend its not */
	{
		if (!logOptions.toLog)
			return 1;
		nlu = &nludummy;
		nlu->user.szSettingsModule = "(NULL)";
	}
	else if (!nlu->toLog)
		return 1;

	dwOriginalLastError = GetLastError();
	switch (logOptions.timeFormat) 
	{
	case TIMEFORMAT_HHMMSS:
		GetTimeFormatA(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER, 
			NULL, NULL, szTime, SIZEOF(szTime));
		break;

	case TIMEFORMAT_MILLISECONDS:
		QueryPerformanceCounter(&liTimeNow);
		liTimeNow.QuadPart -= mirandaStartTime;
		mir_snprintf(szTime, SIZEOF(szTime), "%I64u.%03I64u", liTimeNow.QuadPart / perfCounterFreq, 
			1000 * (liTimeNow.QuadPart % perfCounterFreq) / perfCounterFreq);
		break;

	case TIMEFORMAT_MICROSECONDS:
		QueryPerformanceCounter(&liTimeNow);
		liTimeNow.QuadPart -= mirandaStartTime;
		mir_snprintf(szTime, SIZEOF(szTime), "%I64u.%06I64u", liTimeNow.QuadPart / perfCounterFreq,
			1000000 * (liTimeNow.QuadPart % perfCounterFreq) / perfCounterFreq);
		break;

	default:
		szTime[0] = '\0';
		break;
	}
	if(logOptions.timeFormat || logOptions.showUser)
		mir_snprintf(szHead, SIZEOF(szHead) - 1, "[%s%s%s] ", szTime, 
			(logOptions.showUser && logOptions.timeFormat) ? " " : "",
			logOptions.showUser ? nlu->user.szSettingsModule : "");
	else
		szHead[0]=0;

	if(logOptions.toOutputDebugString) 
	{
	    if (szHead[0])
			OutputDebugStringA(szHead);
		OutputDebugStringA(pszMsg);
		OutputDebugStringA("\n");
	}

	if (logOptions.toFile && logOptions.szFile[0]) 
	{
		EnterCriticalSection(&logOptions.cs);

		FILE *fp;
		fp = _tfopen(logOptions.szFile, _T("ab"));
		if (!fp) 
		{
			CreatePathToFileT(logOptions.szFile);
			fp = _tfopen(logOptions.szFile, _T("at"));
		}
		if (fp) 
		{
			size_t len = strlen(pszMsg);
			fprintf(fp,"%s%s%s", szHead, pszMsg, pszMsg[len-1] == '\n' ? "" : "\r\n");
			fclose(fp);
		}	
		LeaveCriticalSection(&logOptions.cs);
	}

	if (((THook*)hLogEvent)->subscriberCount) 
	{
		LOGMSG logMsg = { szHead, pszMsg };
		CallHookSubscribers(hLogEvent, (WPARAM)nlu, (LPARAM)&logMsg);
	}

	SetLastError(dwOriginalLastError);
	return 1;
}

static INT_PTR NetlibLogW(WPARAM wParam, LPARAM lParam)
{
	const wchar_t *pszMsg = (const wchar_t*)lParam;
	char* szMsg = Utf8EncodeUcs2(pszMsg);
	INT_PTR res = NetlibLog(wParam, (LPARAM)szMsg);
	mir_free(szMsg);
	return res;
}

void NetlibLogf(NetlibUser* nlu, const char *fmt, ...)
{
	if (nlu == NULL) 
	{
		if (!logOptions.toLog)
			return;
	}
	else if (!nlu->toLog)
		return;

	va_list va;
	char szText[1024];

	va_start(va,fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);

	NetlibLog((WPARAM)nlu, (LPARAM)szText);
}


void NetlibDumpData(struct NetlibConnection *nlc,PBYTE buf,int len,int sent,int flags)
{
	int isText=1;
	char szTitleLine[128];
	char *szBuf;
	int titleLineLen;
	struct NetlibUser *nlu;
    bool useStack = false;

	// This section checks a number of conditions and aborts
	// the dump if the data should not be written to the log

	// Check packet flags
	if (flags & (MSG_PEEK | MSG_NODUMP))
		return;

	// Check user's log settings
	if (!(logOptions.toOutputDebugString ||
		((THook*)hLogEvent)->subscriberCount ||
		(logOptions.toFile && logOptions.szFile[0])))
		return;
	if ((sent && !logOptions.dumpSent) ||
		(!sent && !logOptions.dumpRecv))
		return;
	if ((flags & MSG_DUMPPROXY) && !logOptions.dumpProxy)
		return;
	if ((flags & MSG_DUMPSSL) && !logOptions.dumpSsl)
		return;

	WaitForSingleObject(hConnectionHeaderMutex, INFINITE);
	nlu = nlc ? nlc->nlu : NULL;
	titleLineLen = mir_snprintf(szTitleLine, SIZEOF(szTitleLine), "(%p:%u) Data %s%s\r\n", 
		nlc, nlc ? nlc->s : 0, sent ? "sent" : "received", flags & MSG_DUMPPROXY ? " (proxy)" : "");
	ReleaseMutex(hConnectionHeaderMutex);

	// check filter settings
	if (nlu == NULL) 
	{
		if (!logOptions.toLog)
			return;
	}
	else if (!nlu->toLog)
		return;

	if (!logOptions.textDumps)
		isText = 0;
	else if (!(flags&MSG_DUMPASTEXT))
	{
		if (logOptions.autoDetectText) 
		{
			int i;
			for(i = 0; i<len; i++)
			{
				if ((buf[i]<' ' && buf[i]!='\t' && buf[i]!='\r' && buf[i]!='\n') || buf[i]>=0x80)
				{
					isText = 0;
					break;
				}
			}
		}
		else
			isText = 0;
	}

	// Text data
	if ( isText ) {
        int sz = titleLineLen + len + 1;
        useStack = sz <= 8192;
        szBuf = (char*)(useStack ? alloca(sz) : mir_alloc(sz));
		CopyMemory( szBuf, szTitleLine, titleLineLen );
		CopyMemory( szBuf + titleLineLen, (const char*)buf, len );
		szBuf[titleLineLen + len] = '\0';
	}
	// Binary data
	else {
		int line, col, colsInLine;
		char *pszBuf;
        int sz = titleLineLen + ((len+16)>>4) * 78 + 1;
        useStack = sz <= 8192;

        szBuf = (char*)(useStack ? alloca(sz) : mir_alloc(sz));
		CopyMemory(szBuf, szTitleLine, titleLineLen);
		pszBuf = szBuf + titleLineLen;
		for ( line = 0; ; line += 16 ) {
			colsInLine = min(16, len - line);

			if (colsInLine == 16) {
				PBYTE p = buf + line;
				pszBuf += wsprintfA(
					pszBuf, "%08X: %02X %02X %02X %02X-%02X %02X %02X %02X-%02X %02X %02X %02X-%02X %02X %02X %02X  ",
					line, p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15] );
			}
			else {
				pszBuf += wsprintfA(pszBuf, "%08X: ", line);
				// Dump data as hex
				for (col = 0; col < colsInLine; col++)
					pszBuf += wsprintfA(pszBuf, "%02X%c", buf[line + col], ((col&3)==3 && col != 15)?'-':' ');
				// Fill out last line with blanks
				for ( ; col<16; col++)
					{
					lstrcpyA(pszBuf, "   ");
					pszBuf += 3;
				}
				*pszBuf++ = ' ';
			}

			for (col = 0; col < colsInLine; col++)
				*pszBuf++ = buf[line+col]<' '?'.':(char)buf[line+col];

			if (len-line<=16)
				break;

			*pszBuf++ = '\r'; // End each line with a break
			*pszBuf++ = '\n'; // End each line with a break
		}
		*pszBuf = '\0';
	}

	NetlibLog((WPARAM)nlu,(LPARAM)szBuf);
    if (!useStack) mir_free(szBuf);
}

void NetlibLogInit(void)
{
	DBVARIANT dbv;
	LARGE_INTEGER li;

	QueryPerformanceFrequency( &li );
	perfCounterFreq = li.QuadPart;
	QueryPerformanceCounter( &li );
	mirandaStartTime = li.QuadPart;

	CreateServiceFunction( MS_NETLIB_LOGWIN, ShowOptions );
	CreateServiceFunction( MS_NETLIB_LOG, NetlibLog );
	CreateServiceFunction( MS_NETLIB_LOGW, NetlibLogW );
	hLogEvent = CreateHookableEvent( ME_NETLIB_FASTDUMP );

	InitializeCriticalSection(&logOptions.cs);
	logOptions.dumpRecv = DBGetContactSettingByte( NULL, "Netlib", "DumpRecv", 1 );
	logOptions.dumpSent = DBGetContactSettingByte( NULL, "Netlib", "DumpSent", 1 );
	logOptions.dumpProxy = DBGetContactSettingByte( NULL, "Netlib", "DumpProxy", 1 );
	logOptions.dumpSsl = DBGetContactSettingByte( NULL, "Netlib", "DumpSsl", 0 );
	logOptions.textDumps = DBGetContactSettingByte( NULL, "Netlib", "TextDumps", 1 );
	logOptions.autoDetectText = DBGetContactSettingByte( NULL, "Netlib", "AutoDetectText", 1 );
	logOptions.timeFormat = DBGetContactSettingByte( NULL, "Netlib", "TimeFormat", TIMEFORMAT_HHMMSS );
	logOptions.showUser = DBGetContactSettingByte( NULL, "Netlib", "ShowUser", 1 );
	logOptions.toOutputDebugString = DBGetContactSettingByte( NULL, "Netlib", "ToOutputDebugString", 0 );
	logOptions.toFile = DBGetContactSettingByte( NULL, "Netlib", "ToFile", 0 );
	logOptions.toLog = DBGetContactSettingDword( NULL, "Netlib", "NLlog", 1 );

	if (!DBGetContactSettingTString(NULL, "Netlib", "File", &dbv))
	{
		logOptions.szUserFile = mir_tstrdup(dbv.ptszVal);
		TCHAR *pszNewPath = Utils_ReplaceVarsT(dbv.ptszVal);

		TCHAR path[MAX_PATH];
		pathToAbsoluteT(pszNewPath, path, NULL);
		logOptions.szFile = mir_tstrdup(path);

		mir_free(pszNewPath);
		DBFreeVariant(&dbv);
	}
	else 
	{
		logOptions.szUserFile = mir_tstrdup(_T("%miranda_logpath%\\netlog.txt"));
		logOptions.szFile = Utils_ReplaceVarsT(logOptions.szUserFile);
	}

	if ( logOptions.toFile && logOptions.szFile[0] ) {
		FILE *fp;
		fp = _tfopen( logOptions.szFile, _T("wt"));
		if ( fp )
			fclose(fp);
	}

	if ( DBGetContactSettingByte( NULL, "Netlib", "ShowLogOptsAtStart", 0 ))
		NetlibLogShowOptions();

	if ( !DBGetContactSettingTString( NULL, "Netlib", "RunAtStart", &dbv )) {
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi;
		si.cb = sizeof( si );
		if ( dbv.ptszVal[0] )
			CreateProcess( NULL, dbv.ptszVal, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
		DBFreeVariant( &dbv );
	}
}

void NetlibLogShutdown(void)
{
	bIsActive = FALSE;
	DestroyHookableEvent( hLogEvent ); hLogEvent = NULL;
	if ( IsWindow( logOptions.hwndOpts ))
		DestroyWindow( logOptions.hwndOpts );
	DeleteCriticalSection( &logOptions.cs );
	mir_free( logOptions.szFile );
	mir_free( logOptions.szUserFile );
}
