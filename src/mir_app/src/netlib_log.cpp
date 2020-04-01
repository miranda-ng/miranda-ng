/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
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
	CMStringW tszFile, tszUserFile;
}
static logOptions = {};

struct LOGMSG
{
	const char* pszHead;
	const char* pszMsg;
};

static __int64 mirandaStartTime, perfCounterFreq;
static int bIsActive = TRUE;
static HANDLE hLogEvent = nullptr;
static HANDLE hLogger = nullptr;

static void InitLog()
{
	if (hLogger) {
		mir_closeLog(hLogger);
		hLogger = nullptr;
	}

	ptrW szBuf(db_get_wsa(0, "Netlib", "File"));
	if (mir_wstrlen(szBuf)) {
		logOptions.tszUserFile = szBuf.get();

		wchar_t path[MAX_PATH];
		PathToAbsoluteW(VARSW(szBuf), path);
		logOptions.tszFile = path;
	}
	else {
		db_set_ws(0, "Netlib", "File", logOptions.tszUserFile = L"%miranda_logpath%\\netlog.txt");
		logOptions.tszFile = VARSW(logOptions.tszUserFile);
	}

	if (logOptions.toFile)
		hLogger = mir_createLog("Netlib", LPGENW("Standard Netlib log"), logOptions.tszFile, 0);
}

static const wchar_t* szTimeFormats[] =
{
	LPGENW("No times"),
	LPGENW("Standard hh:mm:ss times"),
	LPGENW("Times in milliseconds"),
	LPGENW("Times in microseconds")
};

static INT_PTR CALLBACK LogOptionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	wchar_t str[MAX_PATH];

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

		for (auto &it : szTimeFormats)
			SendDlgItemMessage(hwndDlg, IDC_TIMEFORMAT, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));

		SendDlgItemMessage(hwndDlg, IDC_TIMEFORMAT, CB_SETCURSEL, logOptions.timeFormat, 0);
		CheckDlgButton(hwndDlg, IDC_SHOWNAMES, logOptions.showUser ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TOOUTPUTDEBUGSTRING, logOptions.toOutputDebugString ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TOFILE, logOptions.toFile ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hwndDlg, IDC_FILENAME, logOptions.tszUserFile);
		SetDlgItemText(hwndDlg, IDC_PATH, logOptions.tszFile);
		CheckDlgButton(hwndDlg, IDC_SHOWTHISDLGATSTART, db_get_b(0, "Netlib", "ShowLogOptsAtStart", 0) ? BST_CHECKED : BST_UNCHECKED);
		{
			ptrA szRun(db_get_sa(0, "Netlib", "RunAtStart"));
			if (szRun)
				SetDlgItemTextA(hwndDlg, IDC_RUNATSTART, szRun);

			HWND hwndFilter = GetDlgItem(hwndDlg, IDC_FILTER);
			SetWindowLongPtr(hwndFilter, GWL_STYLE, GetWindowLongPtr(hwndFilter, GWL_STYLE) | (TVS_NOHSCROLL | TVS_CHECKBOXES));

			TVINSERTSTRUCT tvis = {};
			tvis.hInsertAfter = TVI_SORT;
			tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;

			for (auto &it : netlibUser) {
				tvis.item.pszText = it->user.szDescriptiveName.w;
				tvis.item.lParam = netlibUser.indexOf(&it);
				tvis.item.state = INDEXTOSTATEIMAGEMASK(it->toLog ? 2 : 1);
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

				wchar_t path[MAX_PATH];
				GetWindowText((HWND)lParam, path, _countof(path));

				PathToAbsoluteW(VARSW(path), path);
				SetDlgItemText(hwndDlg, IDC_PATH, path);
			}
			break;

		case IDC_FILENAMEBROWSE:
		case IDC_RUNATSTARTBROWSE:
			GetWindowText(GetWindow((HWND)lParam, GW_HWNDPREV), str, _countof(str));
			{
				wchar_t filter[200];
				mir_snwprintf(filter, L"%s (*)%c*%c", TranslateT("All files"), 0, 0);

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
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = str;
				ofn.nMaxFile = _countof(str) - 2;
				ofn.nMaxFileTitle = MAX_PATH;
				if (LOWORD(wParam) == IDC_FILENAMEBROWSE) {
					if (!GetSaveFileName(&ofn)) return 1;
				}
				else if (!GetOpenFileName(&ofn))
					return 1;

				if (LOWORD(wParam) == IDC_RUNATSTARTBROWSE && wcschr(str, ' ') != nullptr) {
					memmove(str + 1, str, ((_countof(str) - 2) * sizeof(wchar_t)));
					str[0] = '"';
					mir_wstrcat(str, L"\"");
				}
				SetWindowText(GetWindow((HWND)lParam, GW_HWNDPREV), str);
			}
			break;

		case IDC_RUNNOW:
			GetDlgItemText(hwndDlg, IDC_RUNATSTART, str, _countof(str));
			if (str[0]) {
				STARTUPINFO si = { sizeof(si) };
				PROCESS_INFORMATION pi;
				CreateProcess(nullptr, str, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
			}
			break;

		case IDOK:
			GetDlgItemText(hwndDlg, IDC_RUNATSTART, str, _countof(str));
			db_set_ws(0, "Netlib", "RunAtStart", str);
			db_set_b(0, "Netlib", "ShowLogOptsAtStart", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWTHISDLGATSTART));

			GetDlgItemText(hwndDlg, IDC_FILENAME, str, _countof(str));
			logOptions.tszUserFile = rtrimw(str);
			db_set_ws(0, "Netlib", "File", str);

			GetDlgItemText(hwndDlg, IDC_PATH, str, _countof(str));
			logOptions.tszFile = rtrimw(str);

			db_set_b(0, "Netlib", "DumpRecv", logOptions.dumpRecv = IsDlgButtonChecked(hwndDlg, IDC_DUMPRECV));
			db_set_b(0, "Netlib", "DumpSent", logOptions.dumpSent = IsDlgButtonChecked(hwndDlg, IDC_DUMPSENT));
			db_set_b(0, "Netlib", "DumpProxy", logOptions.dumpProxy = IsDlgButtonChecked(hwndDlg, IDC_DUMPPROXY));
			db_set_b(0, "Netlib", "DumpSsl", logOptions.dumpSsl = IsDlgButtonChecked(hwndDlg, IDC_DUMPSSL));
			db_set_b(0, "Netlib", "TextDumps", logOptions.textDumps = IsDlgButtonChecked(hwndDlg, IDC_TEXTDUMPS));
			db_set_b(0, "Netlib", "AutoDetectText", logOptions.autoDetectText = IsDlgButtonChecked(hwndDlg, IDC_AUTODETECTTEXT));
			db_set_b(0, "Netlib", "TimeFormat", logOptions.timeFormat = SendDlgItemMessage(hwndDlg, IDC_TIMEFORMAT, CB_GETCURSEL, 0, 0));
			db_set_b(0, "Netlib", "ShowUser", logOptions.showUser = IsDlgButtonChecked(hwndDlg, IDC_SHOWNAMES));
			db_set_b(0, "Netlib", "ToOutputDebugString", logOptions.toOutputDebugString = IsDlgButtonChecked(hwndDlg, IDC_TOOUTPUTDEBUGSTRING));
			db_set_b(0, "Netlib", "ToFile", logOptions.toFile = IsDlgButtonChecked(hwndDlg, IDC_TOFILE));
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
						db_set_dw(0, "Netlib", "NLlog", checked);
					}
					else if (tvi.lParam < netlibUser.getCount()) {
						netlibUser[tvi.lParam]->toLog = checked;
						db_set_dw(0, netlibUser[tvi.lParam]->user.szSettingsModule, "NLlog", checked);
					}

					tvi.hItem = TreeView_GetNextSibling(hwndFilter, tvi.hItem);
				}
			}
			InitLog();
			__fallthrough;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_FILTER), TVSIL_STATE));
		logOptions.hwndOpts = nullptr;
		break;
	}
	return FALSE;
}

void NetlibLogShowOptions(void)
{
	if (logOptions.hwndOpts == nullptr)
		logOptions.hwndOpts = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_NETLIBLOGOPTS), nullptr, LogOptionsDlgProc);
	SetForegroundWindow(logOptions.hwndOpts);
}

static INT_PTR ShowOptions(WPARAM, LPARAM)
{
	NetlibLogShowOptions();
	return 0;
}

int NetlibLog_Worker(NetlibUser *nlu, const char *pszMsg, int flags)
{
	if (!bIsActive)
		return 0;

	DWORD dwOriginalLastError = GetLastError();

	if ((nlu != nullptr && GetNetlibHandleType(nlu) != NLH_USER) || pszMsg == nullptr) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	/* if the Netlib user handle is nullptr, just pretend its not */
	if (!(nlu != nullptr ? nlu->toLog : logOptions.toLog))
		return 1;

	LARGE_INTEGER liTimeNow;
	char szTime[32], szHead[128];
	switch (logOptions.timeFormat) {
	case TIMEFORMAT_HHMMSS:
		GetTimeFormatA(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER, nullptr, nullptr, szTime, _countof(szTime));
		mir_strcat(szTime, " ");
		break;

	case TIMEFORMAT_MILLISECONDS:
		QueryPerformanceCounter(&liTimeNow);
		liTimeNow.QuadPart -= mirandaStartTime;
		mir_snprintf(szTime, "%I64u.%03I64u ", liTimeNow.QuadPart / perfCounterFreq,
			1000 * (liTimeNow.QuadPart % perfCounterFreq) / perfCounterFreq);
		break;

	case TIMEFORMAT_MICROSECONDS:
		QueryPerformanceCounter(&liTimeNow);
		liTimeNow.QuadPart -= mirandaStartTime;
		mir_snprintf(szTime, "%I64u.%06I64u ", liTimeNow.QuadPart / perfCounterFreq,
			1000000 * (liTimeNow.QuadPart % perfCounterFreq) / perfCounterFreq);
		break;

	default:
		szTime[0] = '\0';
		break;
	}

	if (flags & MSG_NOTITLE) 
		szHead[0] = 0; 
	else {
		char *szUser = (logOptions.showUser) ? (nlu == nullptr ? nullptr : nlu->user.szSettingsModule) : nullptr;
		if (szUser)
			mir_snprintf(szHead, "[%s%04X] [%s] ", szTime, GetCurrentThreadId(), szUser);
		else
			mir_snprintf(szHead, "[%s%04X] ", szTime, GetCurrentThreadId());
	}

	if (logOptions.toOutputDebugString) {
		if (szHead[0])
			OutputDebugStringA(szHead);
		OutputDebugStringA(pszMsg);
		OutputDebugStringA("\n");
	}

	if (logOptions.toFile && !logOptions.tszFile.IsEmpty()) {
		size_t len = mir_strlen(pszMsg);
		mir_writeLogA(hLogger, "%s%s%s", szHead, pszMsg, pszMsg[len-1] == '\n' ? "" : "\r\n");
	}

	LOGMSG logMsg = { szHead, pszMsg };
	NotifyFastHook(hLogEvent, (WPARAM)nlu, (LPARAM)&logMsg);

	SetLastError(dwOriginalLastError);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void PROTO_INTERFACE::debugLogA(const char *szFormat, ...)
{
	char buf[4096];
	va_list args;
	va_start(args, szFormat);
	int res = _vsnprintf(buf, _countof(buf), szFormat, args);
	NetlibLog_Worker(m_hNetlibUser, (res != -1) ? buf : CMStringA().FormatV(szFormat, args), 0);
	va_end(args);
}

void PROTO_INTERFACE::debugLogW(const wchar_t *wszFormat, ...)
{
	WCHAR buf[4096];
	va_list args;
	va_start(args, wszFormat);
	int res = _vsnwprintf(buf, _countof(buf), wszFormat, args);
	NetlibLog_Worker(m_hNetlibUser, ptrA(mir_utf8encodeW((res != -1) ? buf : CMStringW().FormatV(wszFormat, args))), 0);
	va_end(args);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_Logf(HNETLIBUSER hUser, _Printf_format_string_ const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char szText[8000];
	mir_vsnprintf(szText, _countof(szText), fmt, va);
	va_end(va);
	return NetlibLog_Worker(hUser, szText, 0);
}

MIR_APP_DLL(int) Netlib_LogfW(HNETLIBUSER hUser, _Printf_format_string_ const wchar_t *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	wchar_t szText[8000];
	mir_vsnwprintf(szText, _countof(szText), fmt, va);
	va_end(va);
	return NetlibLog_Worker(hUser, ptrA(mir_utf8encodeW(szText)), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_Log(HNETLIBUSER hUser, const char *pszStr)
{
	return NetlibLog_Worker(hUser, pszStr, 0);
}

MIR_APP_DLL(int) Netlib_LogW(HNETLIBUSER hUser, const wchar_t *pwszStr)
{
	return NetlibLog_Worker(hUser, ptrA(mir_utf8encodeW(pwszStr)), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Netlib_Dump(HNETLIBCONN nlc, const void *pBuf, size_t len, bool bIsSent, int flags)
{
	// This section checks a number of conditions and aborts
	// the dump if the data should not be written to the log

	// Check packet flags
	if (flags & (MSG_PEEK | MSG_NODUMP))
		return;

	// Check user's log settings
	if (!(logOptions.toOutputDebugString || GetSubscribersCount((THook*)hLogEvent) != 0 || (logOptions.toFile && !logOptions.tszFile.IsEmpty())))
		return;
	if ((bIsSent && !logOptions.dumpSent) || (!bIsSent && !logOptions.dumpRecv))
		return;
	if ((flags & MSG_DUMPPROXY) && !logOptions.dumpProxy)
		return;
	if ((flags & MSG_DUMPSSL) && !logOptions.dumpSsl)
		return;

	CMStringA str;

	WaitForSingleObject(hConnectionHeaderMutex, INFINITE);
	NetlibUser *nlu = nlc ? nlc->nlu : nullptr;

	if (!(flags & MSG_NOTITLE))
		str.Format("(%p:%u) Data %s%s\r\n", nlc, nlc ? (int)nlc->s : 0, bIsSent ? "sent" : "received", flags & MSG_DUMPPROXY ? " (proxy)" : "");
	ReleaseMutex(hConnectionHeaderMutex);

	// check filter settings
	if (nlu == nullptr) {
		if (!logOptions.toLog)
			return;
	}
	else if (!nlu->toLog)
		return;

	const uint8_t *buf = (const uint8_t *)pBuf;

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
		str.Append((const char*)buf, (int)len);
	}
	// Binary data
	else {
		for (int line = 0;; line += 16) {
			auto *p = buf + line;
			int colsInLine = min(16, (int)len - line);
			if (colsInLine == 16)
				str.AppendFormat("%08X: %02X %02X %02X %02X-%02X %02X %02X %02X-%02X %02X %02X %02X-%02X %02X %02X %02X  ",
					line, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
			else {
				str.AppendFormat("%08X: ", line);
				
				// Dump data as hex
				int col;
				for (col = 0; col < colsInLine; col++)
					str.AppendFormat("%02X%c", p[col], ((col & 3) == 3) ? '-' : ' ');

				// Fill out last line with blanks
				for (; col < 16; col++)
					str.Append("   ");

				str.AppendChar(' ');
			}

			for (int col = 0; col < colsInLine; col++)
				str.AppendChar((p[col] < ' ') ? '.' : p[col]);

			if (len - line <= 16)
				break;

			str.AppendChar('\r'); // End each line with a break
			str.AppendChar('\n'); // End each line with a break
		}
	}

	NetlibLog_Worker(nlu, str, flags);
}

/////////////////////////////////////////////////////////////////////////////////////////

void NetlibLogInit(void)
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	perfCounterFreq = li.QuadPart;
	QueryPerformanceCounter(&li);
	mirandaStartTime = li.QuadPart;

	CreateServiceFunction(MS_NETLIB_LOGWIN, ShowOptions);
	hLogEvent = CreateHookableEvent(ME_NETLIB_FASTDUMP);

	logOptions.dumpRecv = db_get_b(0, "Netlib", "DumpRecv", 1);
	logOptions.dumpSent = db_get_b(0, "Netlib", "DumpSent", 1);
	logOptions.dumpProxy = db_get_b(0, "Netlib", "DumpProxy", 1);
	logOptions.dumpSsl = db_get_b(0, "Netlib", "DumpSsl", 0);
	logOptions.textDumps = db_get_b(0, "Netlib", "TextDumps", 1);
	logOptions.autoDetectText = db_get_b(0, "Netlib", "AutoDetectText", 1);
	logOptions.timeFormat = db_get_b(0, "Netlib", "TimeFormat", TIMEFORMAT_HHMMSS);
	logOptions.showUser = db_get_b(0, "Netlib", "ShowUser", 1);
	logOptions.toOutputDebugString = db_get_b(0, "Netlib", "ToOutputDebugString", 0);
	logOptions.toFile = db_get_b(0, "Netlib", "ToFile", 0);
	logOptions.toLog = db_get_dw(0, "Netlib", "NLlog", 1);

	InitLog();

	if (db_get_b(0, "Netlib", "ShowLogOptsAtStart", 0))
		NetlibLogShowOptions();

	ptrW szBuf(db_get_wsa(0, "Netlib", "RunAtStart"));
	if (szBuf) {
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		CreateProcess(nullptr, szBuf, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
	}
}

void NetlibLogShutdown(void)
{
	bIsActive = FALSE;
	DestroyHookableEvent(hLogEvent); hLogEvent = nullptr;
	if (IsWindow(logOptions.hwndOpts))
		DestroyWindow(logOptions.hwndOpts);
}
