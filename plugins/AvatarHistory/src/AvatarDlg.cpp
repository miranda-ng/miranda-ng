/*
Avatar History Plugin
 Copyright (C) 2006  Matthew Wild - Email: mwild1@gmail.com

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "stdafx.h"

HGENMENU hMenu = NULL; 
void __cdecl AvatarDialogThread(void *param);
static INT_PTR CALLBACK AvatarDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
int ShowSaveDialog(HWND hwnd, TCHAR* fn,MCONTACT hContact = NULL);

BOOL ProtocolEnabled(const char *proto);
int FillAvatarListFromDB(HWND list, MCONTACT hContact);
int FillAvatarListFromFolder(HWND list, MCONTACT hContact);
int FillAvatarListFromFiles(HWND list, MCONTACT hContact);
int CleanupAvatarPic(HWND hwnd);
bool UpdateAvatarPic(HWND hwnd);
TCHAR * GetContactFolder(TCHAR *fn, MCONTACT hContact);
BOOL ResolveShortcut(TCHAR *shortcut, TCHAR *file);

static INT_PTR ShowDialogSvc(WPARAM wParam, LPARAM lParam);

struct AvatarDialogData
{
	MCONTACT hContact;
	TCHAR fn[MAX_PATH];
	HWND parent;
};


class ListEntry
{
public:
	ListEntry()
	{
		hDbEvent = NULL;
		filename = NULL;
		filelink = NULL;
	}

	~ListEntry()
	{
		mir_free(filename);
		mir_free(filelink);
	}

	MEVENT hDbEvent;
	TCHAR *filename;
	TCHAR *filelink;
};

int OpenAvatarDialog(MCONTACT hContact, char* fn)
{
	HWND hAvatarWindow = WindowList_Find(hAvatarWindowsList, hContact);
	if (hAvatarWindow)
	{
		SetForegroundWindow(hAvatarWindow);
		SetFocus(hAvatarWindow);
		return 0;
	}

	struct AvatarDialogData *avdlg = (struct AvatarDialogData*)malloc(sizeof(struct AvatarDialogData));
	memset(avdlg, 0, sizeof(struct AvatarDialogData));
	avdlg->hContact = hContact;
	if (fn == NULL)
	{
		avdlg->fn[0] = _T('\0');
	}
	else
	{
		MultiByteToWideChar(CP_ACP, 0, fn, -1, avdlg->fn, SIZEOF(avdlg->fn));
	}

	CloseHandle(mir_forkthread(AvatarDialogThread, (void*)avdlg));
	return 0;
}

void __cdecl AvatarDialogThread(void *param)
{
	struct AvatarDialogData* data = (struct AvatarDialogData*)param;
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AVATARDLG), data->parent, AvatarDlgProc, (LPARAM)param);
}

void EnableDisableControls(HWND hwnd)
{
	HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
	
	int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
	int count = SendMessage(list, LB_GETCOUNT, 0, 0);

	if (cursel == LB_ERR)
	{
		EnableWindow(GetDlgItem(hwnd, IDC_BACK), count > 0);
		EnableWindow(GetDlgItem(hwnd, IDC_NEXT), count > 0);
	}
	else
	{
		EnableWindow(GetDlgItem(hwnd, IDC_BACK), cursel > 0);
		EnableWindow(GetDlgItem(hwnd, IDC_NEXT), cursel < count-1);
	}
}

static INT_PTR CALLBACK AvatarDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwnd, IDC_AVATARLIST);

	switch(uMsg) {
	case WM_INITDIALOG:
		{
			AvatarDialogData *data = (struct AvatarDialogData*) lParam;
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)createDefaultOverlayedIcon(TRUE));
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)createDefaultOverlayedIcon(FALSE));
			if (db_get_b(NULL, MODULE_NAME, "LogToHistory", AVH_DEF_LOGTOHISTORY))
				FillAvatarListFromDB(hwndList, data->hContact);
			else if (opts.log_store_as_hash)
				FillAvatarListFromFolder(hwndList, data->hContact);
			else
				FillAvatarListFromFiles(hwndList, data->hContact);

			TCHAR *displayName = (TCHAR*) CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)data->hContact,GCDNF_TCHAR);
			if (displayName)
			{
				TCHAR title[MAX_PATH];

				mir_sntprintf(title, SIZEOF(title), TranslateT("Avatar History for %s"), displayName);
				SetWindowText(hwnd, title);
			}

			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data->hContact);
			UpdateAvatarPic(hwnd);
			CheckDlgButton(hwnd, IDC_LOGUSER, (UINT)db_get_b(data->hContact, MODULE_NAME, "LogToDisk", BST_INDETERMINATE));
			CheckDlgButton(hwnd, IDC_POPUPUSER, (UINT)db_get_b(data->hContact, MODULE_NAME, "AvatarPopups", BST_INDETERMINATE));
			CheckDlgButton(hwnd, IDC_HISTORYUSER, (UINT)db_get_b(data->hContact, MODULE_NAME, "LogToHistory", BST_INDETERMINATE));
			ShowWindow(GetDlgItem(hwnd, IDC_OPENFOLDER), opts.log_per_contact_folders ? SW_SHOW : SW_HIDE);
			Utils_RestoreWindowPositionNoSize(hwnd,NULL,MODULE_NAME,"AvatarHistoryDialog");
			WindowList_Add(hAvatarWindowsList,hwnd,data->hContact);
			TranslateDialogDefault(hwnd);
			EnableDisableControls(hwnd);
			free(data);
			data = NULL;
		}
		break;

	case WM_CLOSE:
		CleanupAvatarPic(hwnd);
		EndDialog(hwnd, 0);
		return TRUE;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwnd,NULL,MODULE_NAME,"AvatarHistoryDialog");
		WindowList_Remove(hAvatarWindowsList,hwnd);
		DestroyIcon((HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(hwnd, WM_SETICON, ICON_SMALL, 0));
		{
			int count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
			for(int i = 0; i < count; i++)
				delete (ListEntry*)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
		}
		break;

	case WM_CONTEXTMENU:
		{
			HWND pic = GetDlgItem(hwnd, IDC_AVATAR);
			int pos;

			if ((HANDLE)wParam == hwndList) {
				POINT p;
				p.x = LOWORD(lParam); 
				p.y = HIWORD(lParam); 

				ScreenToClient(hwndList, &p);

				pos = SendMessage(hwndList, LB_ITEMFROMPOINT, 0, MAKELONG(p.x, p.y));
				if (HIWORD(pos))
					break;
				pos = LOWORD(pos);

				int count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
				if (pos >= count)
					break;

				SendMessage(hwndList, LB_SETCURSEL, pos, 0);
				EnableDisableControls(hwnd);
			}
			else if ((HANDLE) wParam == pic) {
				pos = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
				if (pos == LB_ERR)
					break;
			}
			else break;

			HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
			HMENU submenu = GetSubMenu(menu, 0);
			TranslateMenu(submenu);

			if (!UpdateAvatarPic(hwnd)) {
				RemoveMenu(submenu, 2, MF_BYPOSITION);
				RemoveMenu(submenu, 0, MF_BYPOSITION);
			}

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 
			int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwndList, NULL);
			DestroyMenu(menu);

			ListEntry *le = (ListEntry*) SendMessage(hwndList, LB_GETITEMDATA, pos, 0);
			MCONTACT hContact = (MCONTACT) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			switch(ret) {
			case ID_AVATARLISTPOPUP_SAVEAS:
				ShowSaveDialog(hwnd, le->filename, hContact);
				break;

			case ID_AVATARLISTPOPUP_DELETE:
				{
					BOOL blDelete;

					if (le->hDbEvent)
						blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this history entry?\nOnly the entry in history will be deleted, bitmap file will be kept!"), 
						TranslateT("Delete avatar log?"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_SETFOREGROUND|MB_TOPMOST) == IDYES;
					else
						blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this avatar shortcut?\nOnly shortcut will be deleted, bitmap file will be kept!"), 
						TranslateT("Delete avatar log?"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_SETFOREGROUND|MB_TOPMOST) == IDYES;

					if (blDelete) {
						if (le->hDbEvent)
							db_event_delete(hContact, le->hDbEvent);
						else
							DeleteFile(le->filelink);

						delete le;

						SendMessage(hwndList, LB_DELETESTRING, pos, 0);

						int count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
						if (count > 0) {
							if (pos >= count)
								pos = count -1;
							SendMessage(hwndList, LB_SETCURSEL, pos, 0);
						}

						UpdateAvatarPic(hwnd);
						EnableDisableControls(hwnd);
					}
				}
				break;

			case ID_AVATARLISTPOPUP_DELETE_BOTH:
				BOOL blDelete;

				if (le->hDbEvent)
					blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this archived avatar?\nThis will delete the history entry and the bitmap file.\nWARNING:This can affect more than one entry in history!"), 
					TranslateT("Delete avatar?"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_SETFOREGROUND|MB_TOPMOST) == IDYES;
				else
					blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this archived avatar?\nThis will delete the shortcut and the bitmap file.\nWARNING:This can affect more than one shortcut!"), 
					TranslateT("Delete avatar?"), MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_SETFOREGROUND|MB_TOPMOST) == IDYES;

				if (blDelete) {
					DeleteFile(le->filename);

					if (le->hDbEvent)
						db_event_delete(hContact, le->hDbEvent);
					else
						DeleteFile(le->filelink);

					delete le;

					SendMessage(hwndList, LB_DELETESTRING, pos, 0);

					int count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
					if (count > 0) {
						if (pos >= count)
							pos = count -1;
						SendMessage(hwndList, LB_SETCURSEL, pos, 0);
					}

					UpdateAvatarPic(hwnd);
					EnableDisableControls(hwnd);
				}
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			if (HIWORD(wParam) == BN_CLICKED) {
				MCONTACT hContact = (MCONTACT) GetWindowLongPtr(hwnd, GWLP_USERDATA);
				db_set_b(hContact, MODULE_NAME, "AvatarPopups", (BYTE) IsDlgButtonChecked(hwnd, IDC_POPUPUSER));
				db_set_b(hContact, MODULE_NAME, "LogToDisk", (BYTE) IsDlgButtonChecked(hwnd, IDC_LOGUSER));
				db_set_b(hContact, MODULE_NAME, "LogToHistory", (BYTE) IsDlgButtonChecked(hwnd, IDC_HISTORYUSER));

				CleanupAvatarPic(hwnd);
				EndDialog(hwnd, 0);
				return TRUE;
			}
			break;

		case IDC_AVATARLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				UpdateAvatarPic(hwnd);
				EnableDisableControls(hwnd);
				return TRUE;
			}
			break;

		case IDC_OPENFOLDER:
			if (HIWORD(wParam) == BN_CLICKED && opts.log_per_contact_folders) {
				TCHAR avfolder[MAX_PATH];
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				GetContactFolder(avfolder, hContact);
				ShellExecute(NULL, db_get_b(NULL, MODULE_NAME, "OpenFolderMethod", 0) ? _T("explore") : _T("open"), avfolder, NULL, NULL, SW_SHOWNORMAL);
				return TRUE;
			}
			break;

		case IDC_NEXT:
			if (HIWORD(wParam) == BN_CLICKED) {
				SendMessage(hwndList, LB_SETCURSEL, SendMessage(hwndList, LB_GETCURSEL, 0, 0) +1, 0);
				UpdateAvatarPic(hwnd);
				EnableDisableControls(hwnd);
				return TRUE;
			}
			break;

		case IDC_BACK:
			if (HIWORD(wParam) == BN_CLICKED) {
				int cursel = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
				if (cursel == LB_ERR)
					SendMessage(hwndList, LB_SETCURSEL, SendMessage(hwndList, LB_GETCOUNT, 0, 0) -1, 0);
				else
					SendMessage(hwndList, LB_SETCURSEL, cursel -1, 0);
				UpdateAvatarPic(hwnd);
				EnableDisableControls(hwnd);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}


int AddFileToList(TCHAR *path,TCHAR *lnk,TCHAR *filename, HWND list)
{
	// Add to list
	ListEntry *le = new ListEntry();
	le->filename = mir_tstrdup(path);
	le->filelink = mir_tstrdup(lnk);

	TCHAR *p = _tcschr(filename, _T('.'));
	if (p != NULL)
		p[0] = _T('\0');
	int max_pos = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)filename);
	SendMessage(list, LB_SETITEMDATA, max_pos, (LPARAM)le);
	return max_pos;
}

int FillAvatarListFromFiles(HWND list, MCONTACT hContact)
{
	int max_pos = 0;
	TCHAR dir[MAX_PATH], path[MAX_PATH];
	WIN32_FIND_DATA finddata;

	GetContactFolder(dir, hContact);
	mir_sntprintf(path, SIZEOF(path), _T("%s\\*.*"), dir);

	HANDLE hFind = FindFirstFile(path, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do
	{
		if (finddata.cFileName[0] != '.')
		{
			mir_sntprintf(path, SIZEOF(path), _T("%s\\%s"), dir, finddata.cFileName);
			max_pos = AddFileToList(path,finddata.cFileName,finddata.cFileName,list);
		}
	}
		while(FindNextFile(hFind, &finddata));
	FindClose(hFind);
	SendMessage(list, LB_SETCURSEL, max_pos, 0); // Set to first item
	return 0;
}

int FillAvatarListFromFolder(HWND list, MCONTACT hContact)
{
	int max_pos = 0;
	TCHAR dir[MAX_PATH], path[MAX_PATH];
	WIN32_FIND_DATA finddata;

	GetContactFolder(dir, hContact);
	mir_sntprintf(path, SIZEOF(path), _T("%s\\*.lnk"), dir);

	HANDLE hFind = FindFirstFile(path, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do
	{
		if (finddata.cFileName[0] != '.')
		{
			TCHAR lnk[MAX_PATH];
			mir_sntprintf(lnk, SIZEOF(lnk), _T("%s\\%s"), dir, finddata.cFileName);
			if (ResolveShortcut(lnk, path))
				max_pos = AddFileToList(path,lnk,finddata.cFileName,list);
		}
	}
		while(FindNextFile(hFind, &finddata));
	FindClose(hFind);
	SendMessage(list, LB_SETCURSEL, max_pos, 0); // Set to first item
	return 0;
}

int FillAvatarListFromDB(HWND list, MCONTACT hContact)
{
	int max_pos = 0;
	BYTE blob[2048];
	for (MEVENT hDbEvent = db_event_first(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.cbBlob = sizeof(blob);
		dbei.pBlob = blob;
		if (db_event_get(hDbEvent, &dbei) != 0) continue;
		if (dbei.eventType != EVENTTYPE_AVATAR_CHANGE) continue;

		// Get time
		TCHAR date[64];
		DBTIMETOSTRINGT tts = {0};
		tts.szFormat = _T("d s");
		tts.szDest = date;
		tts.cbDest = sizeof(date);
		CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)dbei.timestamp, (LPARAM)&tts);

		// Get file in disk
		TCHAR path[MAX_PATH];
		ptrT tszStoredPath(mir_utf8decodeT((char*)dbei.pBlob));
		PathToAbsoluteT(tszStoredPath, path);

		// Add to list
		ListEntry *le = new ListEntry();
		le->hDbEvent = hDbEvent;
		le->filename = mir_tstrdup(path);
		max_pos = SendMessage(list,LB_ADDSTRING, 0, (LPARAM)date);
		SendMessage(list, LB_SETITEMDATA, max_pos, (LPARAM)le);
	}

	SendMessage(list, LB_SETCURSEL, max_pos, 0); // Set to first item
	return 0;
}

bool UpdateAvatarPic(HWND hwnd)
{
	HWND hwndpic = GetDlgItem(hwnd, IDC_AVATAR);
	if (!hwnd || !hwndpic)
		return false;

	HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);
	int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
	if (cursel < 0)
	{
		SetDlgItemText(hwnd,IDC_AVATARPATH,TranslateT("Avatar History is empty!"));
		return false;
	}
	
	ListEntry *le = (ListEntry*) SendMessage(list, LB_GETITEMDATA, cursel, 0);

	if (!le || !le->filename)
	{
		SetDlgItemText(hwnd,IDC_AVATARPATH,TranslateT("avatar path is null."));
		return 0;
	}
	SetDlgItemText(hwnd,IDC_AVATARPATH,le->filename);

	HBITMAP avpic = (HBITMAP) CallService(MS_IMG_LOAD, (WPARAM)le->filename, IMGL_TCHAR);

	bool found_image = (avpic != NULL);

	avpic = (HBITMAP)SendMessage(hwndpic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)avpic);
	if (avpic)
		DeleteObject(avpic);

	return found_image;
}

int CleanupAvatarPic(HWND hwnd)
{
	HWND hwndpic = GetDlgItem(hwnd, IDC_AVATAR);
	if (!hwnd || !hwndpic)
		return -1;

	HBITMAP avpic = (HBITMAP)SendMessage(hwndpic, STM_GETIMAGE, 0, 0);
	if (avpic)
		DeleteObject(avpic);
	return 0;
}

int PreBuildContactMenu(WPARAM wParam, LPARAM) 
{
	char *proto = GetContactProto(wParam);
	Menu_ShowItem(hMenu, ProtocolEnabled(proto));
	return 0;
}

void InitMenuItem()
{
	CreateServiceFunction(MS_AVATARHISTORY_SHOWDIALOG, ShowDialogSvc);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.ptszName = LPGENT("View Avatar History");
	mi.flags = CMIF_TCHAR;
	mi.position = 1000090010;
	mi.hIcon = createDefaultOverlayedIcon(FALSE);
	mi.pszService = MS_AVATARHISTORY_SHOWDIALOG;
	hMenu = Menu_AddContactMenuItem(&mi);
	DestroyIcon(mi.hIcon);
}

static INT_PTR ShowDialogSvc(WPARAM wParam, LPARAM lParam)
{
	OpenAvatarDialog(wParam, (char*)lParam);
	return 0;
}


int ShowSaveDialog(HWND hwnd, TCHAR* fn, MCONTACT hContact)
{
	TCHAR filter[MAX_PATH];
	TCHAR file[MAX_PATH];
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = hInst;

	CallService(MS_UTILS_GETBITMAPFILTERSTRINGST, MAX_PATH, (LPARAM)filter);
	ofn.lpstrFilter = filter;
	
	ofn.nFilterIndex = 1;
	_tcsncpy_s(file, (_tcsrchr(fn, '\\') + 1), _TRUNCATE);
	ofn.lpstrFile = file;

	TCHAR *displayName = (TCHAR*) CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR);
	TCHAR title[MAX_PATH];
	if (displayName)
	{
		mir_sntprintf(title, SIZEOF(title), TranslateT("Save Avatar for %s"), displayName);
		ofn.lpstrTitle = title;
	}
	else
	{
		ofn.lpstrTitle = TranslateT("Save Avatar");
	}
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_DONTADDTORECENT;
	ofn.lpstrDefExt = _tcsrchr(fn, '.')+1;

	DBVARIANT dbvInitDir = {0};
	if (!db_get_ts(hContact,MODULE_NAME,"SavedAvatarFolder",&dbvInitDir))
	{
		ofn.lpstrInitialDir = dbvInitDir.ptszVal;
		db_free(&dbvInitDir);
	}
	else
	{
		ofn.lpstrInitialDir = _T(".");
	}
	if (GetSaveFileName(&ofn))
	{
		CopyFile(fn, file, FALSE);
		db_set_ts(hContact,MODULE_NAME,"SavedAvatarFolder",file);
	}
	return 0;
}
