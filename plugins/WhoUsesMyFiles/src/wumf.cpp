#include "wumf.h"

#define MAX_CHARS 4096
#define WS_WINDOW_STYLE WS_OVERLAPPED
#define NAME "WUMF"
#define WM_MYCMD 0x0401

static PWumf list = NULL;
static PWumf lst = NULL;

HANDLE hLogger = NULL;
BOOL wumf();

static int DlgResizer(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	switch(urc->wId) {
	case IDC_CONNLIST:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
	case IDOK:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

void AddToList(HWND hList, PWumf w)
{
	LVITEM lvi = { 0 };	
	lvi.iItem = ListView_GetItemCount(hList)+1;
	lvi.mask = LVIF_PARAM|LVIF_TEXT;
	lvi.pszText = w->szID;
	lvi.lParam = (LPARAM)w;
	ListView_InsertItem(hList,&lvi);
}

void ShowList(PWumf l, HWND hList)
{
	PWumf w = l;
	while(w) {
		AddToList(hList,w);
		w = w->next;
	}
}

VOID OnGetDispInfo(NMLVDISPINFO *plvdi) 
{
	PWumf w = (PWumf)(plvdi->item.lParam);
	switch (plvdi->item.iSubItem) {
	case 0:
		plvdi->item.pszText = w->szID;
		break;
	case 1:
		plvdi->item.pszText = w->szUser;
		break;
	case 2:
		plvdi->item.pszText = w->szPath;
		break;
	case 3:
		plvdi->item.pszText = w->szPerm;
		break;
	}
} 

INT_PTR CALLBACK ConnDlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch( Msg ) {
	case WM_INITDIALOG:
		{
			HWND hList = GetDlgItem(hWnd, IDC_CONNLIST);

			ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);
			LV_COLUMN lvc = { 0 };
			lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;
			lvc.fmt = LVCFMT_LEFT;
			lvc.cx = 40;
			lvc.pszText = _T("ID");
			ListView_InsertColumn(hList, 0, &lvc);
			lvc.cx = 50;
			lvc.pszText = TranslateT("User");
			ListView_InsertColumn(hList, 1, &lvc);
			lvc.cx = 250;
			lvc.pszText = TranslateT("File");
			ListView_InsertColumn(hList, 2, &lvc);
			lvc.cx = 50;
			lvc.pszText = TranslateT("Access");
			ListView_InsertColumn(hList, 3, &lvc);
			KillTimer(NULL, 777);
			lst = cpy_list(&list);
			if (IsUserAnAdmin())
				SetTimer(NULL, 777, TIME, TimerProc);
			else
				MessageBox(NULL, TranslateT("Plugin WhoUsesMyFiles requires admin privileges in order to work."), _T("Miranda NG"), MB_OK);
			ShowList(lst, hList);
		}
		Utils_RestoreWindowPosition(hWnd, NULL, MODULENAME,"conn");
		return TRUE;

	case WM_CLOSE:
		PostMessage( hWnd, WM_COMMAND, IDCANCEL, 0l );
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}
		break;

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hwndDlg = hWnd;
			urd.hInstance = hInst;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_CONNLIST);
			urd.lParam = (LPARAM)NULL;
			urd.pfnResizer = DlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}
		Utils_SaveWindowPosition(hWnd, NULL, MODULENAME,"conn");
		return TRUE;

	case WM_MOVE:
		Utils_SaveWindowPosition(hWnd, NULL, MODULENAME,"conn");
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) { 
		case LVN_GETDISPINFO: 
			OnGetDispInfo((NMLVDISPINFO*)lParam); 
			break; 
		}
		break;

	case WM_DESTROY:
		del_all(&lst);
		PostQuitMessage(0);
		break;
	}
	return FALSE;
}

void LogWumf(PWumf w)
{
	if (!WumfOptions.LogFolders && (w->dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return;

	if (hLogger == NULL) {
		hLogger = mir_createLog("wumf", _T("WhoIsUsingMyFiles log file"), WumfOptions.LogFile, 0);
		if (hLogger == NULL) {
			TCHAR str[256];
			mir_sntprintf(str, SIZEOF(str), _T("Can't open log file %s"), WumfOptions.LogFile);
			MessageBox(NULL, str, TranslateT("Error opening file"), MB_OK | MB_ICONSTOP);
			WumfOptions.LogToFile = FALSE;
			return;
		}
	}
	
	SYSTEMTIME time;
	GetLocalTime(&time);

	TCHAR lpDateStr[20], lpTimeStr[20];
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &time, NULL, lpDateStr, 20);
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER, &time, NULL, lpTimeStr, 20);
	mir_writeLogT(hLogger, _T("%s %s %20s\t%s\r\n"), lpDateStr, lpTimeStr, w->szUser, w->szPath);
}

BOOL wumf()
{
	LPSESSION_INFO_1 s_info = NULL;
	DWORD ent_read = 0, ent_total = 0, res_handle = 0;
	NET_API_STATUS res = NERR_Success;
	if ((res = NetSessionEnum(NULL, NULL, NULL, 1, (LPBYTE *)&s_info, MAX_PREFERRED_LENGTH, &ent_read, &ent_total, &res_handle)) == NERR_Success ||
		res == ERROR_MORE_DATA)
	{
		mark_all(&list, TRUE);
		for(unsigned i = 0; i < ent_read; i++)
			process_session(s_info[ i ]);

		NetApiBufferFree(s_info);
	}
	else printError(res);

	return del_marked(&list);
}

void process_session(SESSION_INFO_1 s_info)
{
	LPFILE_INFO_3 f_info = NULL;
	DWORD ent_read = 0, ent_total = 0, res_handle = 0;
	NET_API_STATUS res = NERR_Success;
	if ((res = NetFileEnum(NULL, NULL, s_info.sesi1_username, 3, (LPBYTE *)&f_info, MAX_PREFERRED_LENGTH, &ent_read, &ent_total, (PDWORD_PTR)&res_handle)) == NERR_Success ||
			res == ERROR_MORE_DATA)
	{
		for(unsigned i=0; i < ent_read; i++)
			process_file(s_info, f_info[ i ]);

		NetApiBufferFree(f_info);
	}
	else printError(res);
}

void process_file(SESSION_INFO_1 s_info, FILE_INFO_3 f_info)
{
	PWumf w = fnd_cell(&list, f_info.fi3_id);
	if (!w) {
		w = new_wumf(f_info.fi3_id, f_info.fi3_username, f_info.fi3_pathname, s_info.sesi1_cname, NULL, 0, f_info.fi3_permissions, GetFileAttributes(f_info.fi3_pathname));
		w->mark = FALSE;
		if (!add_cell(&list, w))
			msg(TranslateT("Error memory allocation"));

		if (WumfOptions.PopupsEnabled) ShowWumfPopup(w);
		if (WumfOptions.LogToFile) LogWumf(w);
	}
	else w->mark = FALSE;
}

void printError(DWORD res) 
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, res, 0, (LPTSTR) &lpMsgBuf, 0, NULL );
	OutputDebugString((LPCTSTR)lpMsgBuf);
	msg((LPCTSTR)lpMsgBuf);
	LocalFree( lpMsgBuf );
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (!wumf())
		KillTimer(NULL, 777);
};

void FreeAll()
{
	del_all(&list);
}
