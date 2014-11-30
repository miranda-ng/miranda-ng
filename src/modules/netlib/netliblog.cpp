/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "..\..\core\commonheaders.h"
#include "netlib.h"

#define MS_NETLIB_LOGWIN "Netlib/Log/Win"

extern HANDLE hConnectionHeaderMutex;

#define TIMEFORMAT_NONE         0
#define TIMEFORMAT_HHMMSS       1
#define TIMEFORMAT_MILLISECONDS 2
#define TIMEFORMAT_MICROSECONDS 3
struct {
	HWND hwndOpts;
	int toOutputDebugString;
	int toFile;
	int toLog;
	int timeFormat;
	int showUser;
	int dumpSent, dumpRecv, dumpProxy, dumpSsl;
	int textDumps, autoDetectText;
	CMString tszFile, tszUserFile;
}
static logOptions = { 0 };

struct LOGMSG
{
	const char* pszHead;
	const char* pszMsg;
};

static __int64 mirandaStartTime, perfCounterFreq;
static int bIsActive = TRUE;
static HANDLE hLogEvent = NULL;
static HANDLE hLogger = NULL;

static void InitLog()
{
	if (hLogger) {
		mir_closeLog(hLogger);
		hLogger = NULL;
	}

	ptrT szBuf(db_get_tsa(NULL, "Netlib", "File"));
	if (lstrlen(szBuf)) {
		logOptions.tszUserFile = szBuf;

		TCHAR path[MAX_PATH];
		PathToAbsoluteT(VARST(szBuf), path);
		logOptions.tszFile = path;
	}
	else {
		db_set_ts(NULL, "Netlib", "File", logOptions.tszUserFile = _T("%miranda_logpath%\\netlog.txt"));
		logOptions.tszFile = Utils_ReplaceVarsT(logOptions.tszUserFile);
	}

	if (logOptions.toFile)
		hLogger = mir_createLog("Netlib", LPGENT("Standard netlib log"), logOptions.tszFile, 0);
}

static const TCHAR* szTimeFormats[] =
{
	LPGENT("No times"),
	LPGENT("Standard hh:mm:ss times"),
	LPGENT("Times in milliseconds"),
	LPGENT("Times in microseconds")
};

static INT_PTR CALLBACK LogOptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR str[MAX_PATH];

	switch (message) {
	case WM_INITDIALOG:
		logOptions.hwndOpts = hwndDlg;
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_DUMPRECV, logOptions.dumpRecv ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DUMPSENT, logOptions.dumpSent ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DUMPPROXY, logOptions.dumpProxy ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DUMPSSL, logOptions.dumpSsl ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TEXTDUMPS, logOptions.textDumps ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTODETECTTEXT, logOptions.autoDetectText ? BST_CHECKED : BST_UNCHECKED);
		{
			for (int i=0; i < SIZEOF(szTimeFormats); i++)
				SendDlgItemMessage(hwndDlg, IDC_TIMEFORMAT, CB_ADDSTRING, 0, (LPARAM)TranslateTS(szTimeFormats[i]));
		}
		SendDlgItemMessage(hwndDlg, IDC_TIMEFORMAT, CB_SETCURSEL, logOptions.timeFormat, 0);
		CheckDlgButton(hwndDlg, IDC_SHOWNAMES, logOptions.showUser ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TOOUTPUTDEBUGSTRING, logOptions.toOutputDebugString ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TOFILE, logOptions.toFile ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hwndDlg, IDC_FILENAME, logOptions.tszUserFile);
		SetDlgItemText(hwndDlg, IDC_PATH, logOptions.tszFile);
		CheckDlgButton(hwndDlg, IDC_SHOWTHISDLGATSTART, db_get_b(NULL, "Netlib", "ShowLogOptsAtStart", 0) ? BST_CHECKED : BST_UNCHECKED);
		{
			ptrA szRun(db_get_sa(NULL, "Netlib", "RunAtStart"));
			if (szRun)
				SetDlgItemTextA(hwndDlg, IDC_RUNATSTART, szRun);

			HWND hwndFilter = GetDlgItem(hwndDlg, IDC_FILTER);
			SetWindowLongPtr(hwndFilter, GWL_STYLE, GetWindowLongPtr(hwndFilter, GWL_STYLE) | (TVS_NOHSCROLL | TVS_CHECKBOXES));

			TVINSERTSTRUCT tvis = { 0 };
			tvis.hInsertAfter = TVI_SORT;
			tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;

			for (int i = 0; i < netlibUser.getCount(); i++) {
				tvis.item.pszText = netlibUser[i]->user.ptszDescriptiveName;
				tvis.item.lParam = i;
				tvis.item.state = INDEXTOSTATEIMAGEMASK((netlibUser[i]->toLog) ? 2 : 1);
				TreeView_InsertItem(hwndFilter, &tvis);
			}
			tvis.item.lParam = -1;
			tvis.item.pszText = TranslateT("(Miranda core logging)");
			tvis.item.state = INDEXTOSTATEIMAGEMASK((logOptions.toLog) ? 2 : 1);
			TreeView_InsertItem(hwndFilter, &tvis);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FILENAME:
			if (HIWORD(wParam) == EN_CHANGE) {
				if ((HWND)lParam == GetFocus())
					CheckDlgButton(hwndDlg, IDC_TOFILE, BST_CHECKED);

				TCHAR path[MAX_PATH];
				GetWindowText((HWND)lParam, path, SIZEOF(path));

				PathToAbsoluteT(VARST(path), path);
				SetDlgItemText(hwndDlg, IDC_PATH, path);
			}
			break;

		case IDC_FILENAMEBROWSE:
		case IDC_RUNATSTARTBROWSE:
			GetWindowText(GetWindow((HWND)lParam, GW_HWNDPREV), str, SIZEOF(str));
			{
				TCHAR filter[512];
				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwndDlg;
				ofn.Flags = OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
				if (LOWORD(wParam) == IDC_FILENAMEBROWSE)
					ofn.lpstrTitle = TranslateT("Select where log file will be created");
				else {
					ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					ofn.lpstrTitle = TranslateT("Select program to be run");
				}
				_tcscpy(filter, TranslateT("All files"));
				_tcscat(filter, _T(" (*)"));
				TCHAR *pfilter = filter + lstrlen(filter) + 1;
				_tcscpy(pfilter, _T("*"));
				pfilter = pfilter + lstrlen(pfilter) + 1;
				*pfilter = '\0';
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = str;
				ofn.nMaxFile = SIZEOF(str) - 2;
				ofn.nMaxFileTitle = MAX_PATH;
				if (LOWORD(wParam) == IDC_FILENAMEBROWSE) {
					if (!GetSaveFileName(&ofn)) return 1;
				}
				else if (!GetOpenFileName(&ofn))
					return 1;

				if (LOWORD(wParam) == IDC_RUNATSTARTBROWSE && _tcschr(str, ' ') != NULL) {
					MoveMemory(str + 1, str, SIZEOF(str) - 2);
					str[0] = '"';
					lstrcat(str, _T("\""));
				}
				SetWindowText(GetWindow((HWND)lParam, GW_HWNDPREV), str);
			}
			break;

		case IDC_RUNNOW:
			GetDlgItemText(hwndDlg, IDC_RUNATSTART, str, MAX_PATH);
			if (str[0]) {
				STARTUPINFO si = { sizeof(si) };
				PROCESS_INFORMATION pi;
				CreateProcess(NULL, str, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			}
			break;

		case IDOK:
			GetDlgItemText(hwndDlg, IDC_RUNATSTART, str, MAX_PATH);
			db_set_ts(NULL, "Netlib", "RunAtStart", str);
			db_set_b(NULL, "Netlib", "ShowLogOptsAtStart", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWTHISDLGATSTART));

			GetWindowText(GetDlgItem(hwndDlg, IDC_FILENAME), str, SIZEOF(str));
			logOptions.tszUserFile = rtrimt(str);
			db_set_ts(NULL, "Netlib", "File", str);

			GetWindowText(GetDlgItem(hwndDlg, IDC_PATH), str, SIZEOF(str));
			logOptions.tszFile = rtrimt(str);

			db_set_b(NULL, "Netlib", "DumpRecv", logOptions.dumpRecv = IsDlgButtonChecked(hwndDlg, IDC_DUMPRECV));
			db_set_b(NULL, "Netlib", "DumpSent", logOptions.dumpSent = IsDlgButtonChecked(hwndDlg, IDC_DUMPSENT));
			db_set_b(NULL, "Netlib", "DumpProxy", logOptions.dumpProxy = IsDlgButtonChecked(hwndDlg, IDC_DUMPPROXY));
			db_set_b(NULL, "Netlib", "DumpSsl", logOptions.dumpSsl = IsDlgButtonChecked(hwndDlg, IDC_DUMPSSL));
			db_set_b(NULL, "Netlib", "TextDumps", logOptions.textDumps = IsDlgButtonChecked(hwndDlg, IDC_TEXTDUMPS));
			db_set_b(NULL, "Netlib", "AutoDetectText", logOptions.autoDetectText = IsDlgButtonChecked(hwndDlg, IDC_AUTODETECTTEXT));
			db_set_b(NULL, "Netlib", "TimeFormat", logOptions.timeFormat = SendDlgItemMessage(hwndDlg, IDC_TIMEFORMAT, CB_GETCURSEL, 0, 0));
			db_set_b(NULL, "Netlib", "ShowUser", logOptions.showUser = IsDlgButtonChecked(hwndDlg, IDC_SHOWNAMES));
			db_set_b(NULL, "Netlib", "ToOutputDebugString", logOptions.toOutputDebugString = IsDlgButtonChecked(hwndDlg, IDC_TOOUTPUTDEBUGSTRING));
			db_set_b(NULL, "Netlib", "ToFile", logOptions.toFile = IsDlgButtonChecked(hwndDlg, IDC_TOFILE));
			{
				HWND hwndFilter = GetDlgItem(logOptions.hwndOpts, IDC_FILTER);
				TVITEM tvi = { 0 };
				BOOL checked;

				tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE | TVIF_TEXT;
				tvi.hItem = TreeView_GetRoot(hwndFilter);

				while (tvi.hItem) {
					TreeView_GetItem(hwndFilter, &tvi);
					checked = ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2);

					if (tvi.lParam == -1) {
						logOptions.toLog = checked;
						db_set_dw(NULL, "Netlib", "NLlog", checked);
					}
					else if (tvi.lParam < netlibUser.getCount()) {
						netlibUser[tvi.lParam]->toLog = checked;
						db_set_dw(NULL, netlibUser[tvi.lParam]->user.szSettingsModule, "NLlog", checked);
					}

					tvi.hItem = TreeView_GetNextSibling(hwndFilter, tvi.hItem);
				}
			}
			InitLog();
			// fall through
		case IDCANCEL:
			DestroyWindow(hwndDlg);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_FILTER), TVSIL_STATE));
		logOptions.hwndOpts = NULL;
		break;
	}
	return FALSE;
}

void NetlibLogShowOptions(void)
{
	if (logOptions.hwndOpts == NULL)
		logOptions.hwndOpts = CreateDialog(hInst, MAKEINTRESOURCE(IDD_NETLIBLOGOPTS), NULL, LogOptionsDlgProc);
	SetForegroundWindow(logOptions.hwndOpts);
}

static INT_PTR ShowOptions(WPARAM, LPARAM)
{
	NetlibLogShowOptions();
	return 0;
}

static INT_PTR NetlibLog(WPARAM wParam, LPARAM lParam)
{
	if (!bIsActive)
		return 0;

	DWORD dwOriginalLastError = GetLastError();

	NetlibUser *nlu = (NetlibUser*)wParam;
	const char *pszMsg = (const char*)lParam;
	if ((nlu != NULL && GetNetlibHandleType(nlu) != NLH_USER) || pszMsg == NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	/* if the Netlib user handle is NULL, just pretend its not */
	if (!(nlu != NULL ? nlu->toLog : logOptions.toLog))
		return 1;

	LARGE_INTEGER liTimeNow;
	char szTime[32], szHead[128];
	switch (logOptions.timeFormat) {
	case TIMEFORMAT_HHMMSS:
		GetTimeFormatA(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER, NULL, NULL, szTime, SIZEOF(szTime));
		strcat(szTime, " ");
		break;

	case TIMEFORMAT_MILLISECONDS:
		QueryPerformanceCounter(&liTimeNow);
		liTimeNow.QuadPart -= mirandaStartTime;
		mir_snprintf(szTime, SIZEOF(szTime), "%I64u.%03I64u ", liTimeNow.QuadPart / perfCounterFreq,
			1000 * (liTimeNow.QuadPart % perfCounterFreq) / perfCounterFreq);
		break;

	case TIMEFORMAT_MICROSECONDS:
		QueryPerformanceCounter(&liTimeNow);
		liTimeNow.QuadPart -= mirandaStartTime;
		mir_snprintf(szTime, SIZEOF(szTime), "%I64u.%06I64u ", liTimeNow.QuadPart / perfCounterFreq,
			1000000 * (liTimeNow.QuadPart % perfCounterFreq) / perfCounterFreq);
		break;

	default:
		szTime[0] = '\0';
		break;
	}

	char *szUser = (logOptions.showUser) ? (nlu == NULL ? NULL : nlu->user.szSettingsModule) : NULL;
	if (szUser)
		mir_snprintf(szHead, SIZEOF(szHead) - 1, "[%s%04X] [%s] ", szTime, GetCurrentThreadId(), szUser);
	else
		mir_snprintf(szHead, SIZEOF(szHead) - 1, "[%s%04X] ", szTime, GetCurrentThreadId());

	if (logOptions.toOutputDebugString) {
		if (szHead[0])
			OutputDebugStringA(szHead);
		OutputDebugStringA(pszMsg);
		OutputDebugStringA("\n");
	}

	if (logOptions.toFile && !logOptions.tszFile.IsEmpty()) {
		size_t len = strlen(pszMsg);
		mir_writeLogA(hLogger, "%s%s%s", szHead, pszMsg, pszMsg[len-1] == '\n' ? "" : "\r\n");
	}

	LOGMSG logMsg = { szHead, pszMsg };
	NotifyFastHook(hLogEvent, (WPARAM)nlu, (LPARAM)&logMsg);

	SetLastError(dwOriginalLastError);
	return 1;
}

static INT_PTR NetlibLogW(WPARAM wParam, LPARAM lParam)
{
	const wchar_t *pszMsg = (const wchar_t*)lParam;
	char* szMsg = Utf8EncodeW(pszMsg);
	INT_PTR res = NetlibLog(wParam, (LPARAM)szMsg);
	mir_free(szMsg);
	return res;
}

void NetlibLogf(NetlibUser* nlu, const char *fmt, ...)
{
	if (nlu == NULL) {
		if (!logOptions.toLog)
			return;
	}
	else if (!nlu->toLog)
		return;

	va_list va;
	char szText[1024];

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);

	NetlibLog((WPARAM)nlu, (LPARAM)szText);
}

void NetlibDumpData(NetlibConnection *nlc, PBYTE buf, int len, int sent, int flags)
{
	char szTitleLine[128];
	char *szBuf;
	bool useStack = false;

	// This section checks a number of conditions and aborts
	// the dump if the data should not be written to the log

	// Check packet flags
	if (flags & (MSG_PEEK | MSG_NODUMP))
		return;

	// Check user's log settings
	if (!(logOptions.toOutputDebugString || GetSubscribersCount(hLogEvent) != 0 || (logOptions.toFile && !logOptions.tszFile.IsEmpty())))
		return;
	if ((sent && !logOptions.dumpSent) || (!sent && !logOptions.dumpRecv))
		return;
	if ((flags & MSG_DUMPPROXY) && !logOptions.dumpProxy)
		return;
	if ((flags & MSG_DUMPSSL) && !logOptions.dumpSsl)
		return;

	WaitForSingleObject(hConnectionHeaderMutex, INFINITE);
	NetlibUser *nlu = nlc ? nlc->nlu : NULL;
	int titleLineLen = mir_snprintf(szTitleLine, SIZEOF(szTitleLine), "(%p:%u) Data %s%s\r\n",
		nlc, nlc ? nlc->s : 0, sent ? "sent" : "received", flags & MSG_DUMPPROXY ? " (proxy)" : "");
	ReleaseMutex(hConnectionHeaderMutex);

	// check filter settings
	if (nlu == NULL) {
		if (!logOptions.toLog)
			return;
	}
	else if (!nlu->toLog)
		return;

	bool isText = true;
	if (!logOptions.textDumps)
		isText = false;
	else if (!(flags & MSG_DUMPASTEXT)) {
		if (logOptions.autoDetectText) {
			for (int i = 0; i < len; i++) {
				if ((buf[i] < ' ' && buf[i] != '\t' && buf[i] != '\r' && buf[i] != '\n') || buf[i] >= 0x80) {
					isText = false;
					break;
				}
			}
		}
		else isText = false;
	}

	// Text data
	if (isText) {
		int sz = titleLineLen + len + 1;
		useStack = sz <= 8192;
		szBuf = (char*)(useStack ? alloca(sz) : mir_alloc(sz));
		CopyMemory(szBuf, szTitleLine, titleLineLen);
		CopyMemory(szBuf + titleLineLen, (const char*)buf, len);
		szBuf[titleLineLen + len] = '\0';
	}
	// Binary data
	else {
		int line, col, colsInLine;
		int sz = titleLineLen + ((len + 16) >> 4) * 78 + 1;
		useStack = sz <= 8192;

		szBuf = (char*)(useStack ? alloca(sz) : mir_alloc(sz));
		CopyMemory(szBuf, szTitleLine, titleLineLen);
		char *pszBuf = szBuf + titleLineLen;
		for (line = 0;; line += 16) {
			colsInLine = min(16, len - line);

			if (colsInLine == 16) {
				PBYTE p = buf + line;
				pszBuf += wsprintfA(
					pszBuf, "%08X: %02X %02X %02X %02X-%02X %02X %02X %02X-%02X %02X %02X %02X-%02X %02X %02X %02X  ",
					line, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]); //!!!!!!!!!!
			}
			else {
				pszBuf += wsprintfA(pszBuf, "%08X: ", line); //!!!!!!!!!!
				// Dump data as hex
				for (col = 0; col < colsInLine; col++)
					pszBuf += wsprintfA(pszBuf, "%02X%c", buf[line + col], ((col & 3) == 3 && col != 15) ? '-' : ' '); //!!!!!!!!!!
				// Fill out last line with blanks
				for (; col < 16; col++) {
					lstrcpyA(pszBuf, "   ");
					pszBuf += 3;
				}
				*pszBuf++ = ' ';
			}

			for (col = 0; col < colsInLine; col++)
				*pszBuf++ = (buf[line + col] < ' ') ? '.' : (char)buf[line + col];

			if (len - line <= 16)
				break;

			*pszBuf++ = '\r'; // End each line with a break
			*pszBuf++ = '\n'; // End each line with a break
		}
		*pszBuf = '\0';
	}

	NetlibLog((WPARAM)nlu, (LPARAM)szBuf);
	if (!useStack)
		mir_free(szBuf);
}

void NetlibLogInit(void)
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	perfCounterFreq = li.QuadPart;
	QueryPerformanceCounter(&li);
	mirandaStartTime = li.QuadPart;

	CreateServiceFunction(MS_NETLIB_LOGWIN, ShowOptions);
	CreateServiceFunction(MS_NETLIB_LOG, NetlibLog);
	CreateServiceFunction(MS_NETLIB_LOGW, NetlibLogW);
	hLogEvent = CreateHookableEvent(ME_NETLIB_FASTDUMP);

	logOptions.dumpRecv = db_get_b(NULL, "Netlib", "DumpRecv", 1);
	logOptions.dumpSent = db_get_b(NULL, "Netlib", "DumpSent", 1);
	logOptions.dumpProxy = db_get_b(NULL, "Netlib", "DumpProxy", 1);
	logOptions.dumpSsl = db_get_b(NULL, "Netlib", "DumpSsl", 0);
	logOptions.textDumps = db_get_b(NULL, "Netlib", "TextDumps", 1);
	logOptions.autoDetectText = db_get_b(NULL, "Netlib", "AutoDetectText", 1);
	logOptions.timeFormat = db_get_b(NULL, "Netlib", "TimeFormat", TIMEFORMAT_HHMMSS);
	logOptions.showUser = db_get_b(NULL, "Netlib", "ShowUser", 1);
	logOptions.toOutputDebugString = db_get_b(NULL, "Netlib", "ToOutputDebugString", 0);
	logOptions.toFile = db_get_b(NULL, "Netlib", "ToFile", 0);
	logOptions.toLog = db_get_dw(NULL, "Netlib", "NLlog", 1);

	InitLog();

	if (db_get_b(NULL, "Netlib", "ShowLogOptsAtStart", 0))
		NetlibLogShowOptions();

	ptrT szBuf(db_get_tsa(NULL, "Netlib", "RunAtStart"));
	if (szBuf) {
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		CreateProcess(NULL, szBuf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}
}

void NetlibLogShutdown(void)
{
	bIsActive = FALSE;
	DestroyHookableEvent(hLogEvent); hLogEvent = NULL;
	if (IsWindow(logOptions.hwndOpts))
		DestroyWindow(logOptions.hwndOpts);
}
