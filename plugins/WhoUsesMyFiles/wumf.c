#define MAX_CHARS 4096
#define WS_WINDOW_STYLE WS_OVERLAPPED
#define NAME "WUMF"
#define WM_MYCMD 0x0401

#include "wumf.h"
static PWumf list = NULL;
static PWumf lst = NULL;

extern WUMF_OPTIONS WumfOptions;
extern HANDLE hInst;
extern HWND hDlg;
extern char ModuleName[];

HANDLE hLog = INVALID_HANDLE_VALUE;
BOOL wumf();

static int DlgResizer(HWND hwndDlg,LPARAM lParam,UTILRESIZECONTROL *urc) {
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
	lvi.iItem=ListView_GetItemCount(hList)+1;
	lvi.mask= LVIF_PARAM|LVIF_TEXT;
	lvi.pszText = w->szID;
	lvi.cchTextMax = strlen(w->szID);
	lvi.lParam = (LPARAM)w;
	ListView_InsertItem(hList,&lvi);
}

void ShowList(PWumf l, HWND hList)
{
	PWumf w;
	w = l;
	while(w)
	{
		AddToList(hList,w);
		w = w->next;
	}
}

VOID OnGetDispInfo(NMLVDISPINFO *plvdi) 
{
	PWumf w;
	w = (PWumf)(plvdi->item.lParam);
	switch (plvdi->item.iSubItem)
    {
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
	    default:
			break;
	}
} 


int CALLBACK ConnDlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    UTILRESIZEDIALOG urd={0};
    LV_COLUMN lvc = { 0 };
	char col0[] = "ID";
	char col1[] = "User";
	char col2[] = "File";
	char col3[] = "Access";
//	char buff[256];
	HWND hList;
    switch( Msg )
    {
        case WM_INITDIALOG:
        	hList = GetDlgItem(hWnd, IDC_CONNLIST);
//			ListView_DeleteAllItems(hList);
        	ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);
        	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;
        	lvc.fmt = LVCFMT_LEFT;
        	lvc.cx = 40;
        	lvc.pszText = col0;
        	lvc.cchTextMax = sizeof(col0);
        	ListView_InsertColumn(hList, 0, &lvc);
        	lvc.cx = 50;
        	lvc.pszText = col1;
        	lvc.cchTextMax = sizeof(col1);
        	ListView_InsertColumn(hList, 1, &lvc);
        	lvc.cx = 250;
        	lvc.pszText = col2;
        	lvc.cchTextMax = sizeof(col2);
        	ListView_InsertColumn(hList, 2, &lvc);
        	lvc.cx = 50;
        	lvc.pszText = col3;
        	lvc.cchTextMax = sizeof(col3);
        	ListView_InsertColumn(hList, 3, &lvc);
			KillTimer(NULL, 777);
        	lst = cpy_list(&list);
        	SetTimer(NULL, 777, TIME,(TIMERPROC) TimerProc);
        	ShowList(lst, hList);
        	Utils_RestoreWindowPosition(hWnd, NULL, ModuleName,"conn");
	        break;
        case WM_CLOSE:
            PostMessage( hWnd, WM_COMMAND, IDCANCEL, 0l );
    	    break;
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDOK:
                case IDCANCEL:
                    PostMessage( hWnd, WM_DESTROY, 0, 0l );
                break;
            }
	        break;
        case WM_SIZE:
			urd.cbSize=sizeof(urd);
			urd.hwndDlg=hWnd;
			urd.hInstance=hInst;
			urd.lpTemplate=MAKEINTRESOURCE(IDD_CONNLIST);
			urd.lParam=(LPARAM)NULL;
			urd.pfnResizer=DlgResizer;
			ResizeDialog(0,(LPARAM)&urd);
			Utils_SaveWindowPosition(hWnd, NULL, ModuleName,"conn");
			return TRUE;
   		case WM_MOVE:
   		    Utils_SaveWindowPosition(hWnd, NULL, ModuleName,"conn");
   			break;
		case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->code) 
            { 
                case LVN_GETDISPINFO: 
                	OnGetDispInfo((NMLVDISPINFO *) lParam); 
                    break; 
			}
			break;
		case WM_DESTROY:
			del_all(&lst);
			PostQuitMessage(0);
			break;
        default:
            return FALSE;
    }
    return TRUE;
}

void LogWumf(PWumf w)
{
	char str[256];
	LPTSTR lpstr;
	char lpDateStr[20];
	char lpTimeStr[20];
	SYSTEMTIME time;
	DWORD bytes;
	
	if(!WumfOptions.LogFolders && (w->dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return;

	if(hLog == INVALID_HANDLE_VALUE || hLog == NULL)
	{
		hLog = CreateFile(WumfOptions.LogFile, 
						GENERIC_WRITE,
						FILE_SHARE_READ, 
						(LPSECURITY_ATTRIBUTES) NULL,
						OPEN_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL,
						(HANDLE) NULL);
		if(hLog == INVALID_HANDLE_VALUE)
		{
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			    NULL,
			    GetLastError(),
			    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			    (LPTSTR) &lpstr,
			    0,
			    NULL);
			wsprintf(str, "Can't open log file %s\nError:%s", WumfOptions.LogFile, lpstr);
			LocalFree(lpstr);		
			MessageBox( NULL, str, "Error opening file", MB_OK | MB_ICONSTOP);
			WumfOptions.LogToFile = FALSE;
		}
	}
	GetLocalTime(&time);
	GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE, &time,NULL, lpDateStr, 20);
	GetTimeFormat(LOCALE_USER_DEFAULT,TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER, &time,NULL, lpTimeStr, 20);
	wsprintf(str ,"%s %s %20s\t%s\r\n\0",lpDateStr, lpTimeStr, w->szUser, w->szPath);
	SetFilePointer (hLog, 0, NULL, FILE_END) ; ;
	WriteFile(hLog, str ,strlen(str),&bytes,NULL);
}

/*
BOOL wumf()
{
	FILE_INFO_3 *buf, *cur;
	SESSION_INFO_0 *sinfo;
	DWORD read, total, resumeh, rc, i, sess = 0L;
	wchar_t server[20];
	char user[512], path[512], comp[512];
	char* UNC = NULL;
	PWumf w = NULL;

    mbstowcs( server, "\\\\.", 8);
	resumeh = 0;

	mark_all(&list, TRUE);
	do
	{
		buf = NULL;
		rc = NetFileEnum( (LPWSTR)server, 
			NULL,
			NULL, 3,
			(BYTE **) &buf, 2048, &read, &total, &resumeh );
		if ( rc != ERROR_MORE_DATA && rc != ERROR_SUCCESS )
			break;
		for ( i = 0, cur = buf; i < read; ++ i, ++ cur)
		{
		    w = fnd_cell(&list, cur->fi3_id);
		    if(!w)
		    {
		        wcstombs(user, (wchar_t *) cur->fi3_username, 512);
		        wcstombs(path, (wchar_t *) cur->fi3_pathname, 512);

		        w = new_wumf(cur->fi3_id, user, path, comp, UNC, sess, cur->fi3_permissions,GetFileAttributes(path));
		        w->mark = FALSE;
		        if(!add_cell(&list, w)){
		        	 msg("Error memory allocation");
		        	 return FALSE;
		        };

		        if(WumfOptions.PopupsEnabled) ShowWumfPopUp(w);
		    	if(WumfOptions.LogToFile) LogWumf(w);
		    }
	    	else 
				w->mark = FALSE;
		}
		if(buf != NULL) NetApiBufferFree( buf );
	} while(rc == ERROR_MORE_DATA);
	return del_marked(&list);
};*/

BOOL wumf()
{
	LPSESSION_INFO_1 s_info = NULL;
	DWORD ent_read = 0, ent_total = 0, res_handle = 0, i = 0;
	NET_API_STATUS res = NERR_Success;
	if( (res = NetSessionEnum(NULL, NULL, NULL, 1, (LPBYTE *)&s_info, MAX_PREFERRED_LENGTH, &ent_read, &ent_total, &res_handle)) == NERR_Success ||
		res == ERROR_MORE_DATA)
	{
		mark_all(&list, TRUE);
		for(i = 0; i < ent_read; i++)
		{
			process_session(s_info[ i ]);
		}
		NetApiBufferFree(s_info);
	} else {
		printError(res);
	}
	return del_marked(&list);
};

void process_session(SESSION_INFO_1 s_info)
{
	LPFILE_INFO_3 f_info = NULL;
	DWORD ent_read = 0, ent_total = 0, res_handle = 0, i = 0;
	NET_API_STATUS res = NERR_Success;
	if( (res = NetFileEnum(NULL, NULL, s_info.sesi1_username, 3, (LPBYTE *)&f_info, MAX_PREFERRED_LENGTH, &ent_read, &ent_total, &res_handle)) == NERR_Success ||
		res == ERROR_MORE_DATA)
	{
		for(i = 0; i < ent_read; i++)
		{
			process_file(s_info, f_info[ i ]);
		}
		NetApiBufferFree(f_info);
	} else {
		printError(res);
	}
}

void process_file(SESSION_INFO_1 s_info, FILE_INFO_3 f_info)
{
	PWumf w = fnd_cell(&list, f_info.fi3_id);
	if(!w)
	{
		w = new_wumf(f_info.fi3_id, f_info.fi3_username, f_info.fi3_pathname, s_info.sesi1_cname, NULL, 0, f_info.fi3_permissions, GetFileAttributes(f_info.fi3_pathname));
		w->mark = FALSE;
		if(!add_cell(&list, w)){
			msg("Error memory allocation");
		};
		if(WumfOptions.PopupsEnabled) ShowWumfPopUp(w);
		if(WumfOptions.LogToFile) LogWumf(w);
	} else {
		w->mark = FALSE;
	}
}

void printError(DWORD res) 
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, res, 0, (LPTSTR) &lpMsgBuf, 0, NULL );
	OutputDebugString(lpMsgBuf);
	msg(lpMsgBuf);
	LocalFree( lpMsgBuf );
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if(!wumf())
		KillTimer(NULL, 777);
};

void FreeAll()
{
	del_all(&list);
};
