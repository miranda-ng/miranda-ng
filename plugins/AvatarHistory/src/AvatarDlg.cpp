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

HGENMENU hMenu = nullptr;
static INT_PTR CALLBACK AvatarDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int ShowSaveDialog(HWND hwnd, wchar_t* fn, MCONTACT hContact = NULL);

bool ProtocolEnabled(const char *proto);
int FillAvatarListFromDB(HWND list, MCONTACT hContact);
int FillAvatarListFromFolder(HWND list, MCONTACT hContact);
int FillAvatarListFromFiles(HWND list, MCONTACT hContact);
int CleanupAvatarPic(HWND hwnd);
bool UpdateAvatarPic(HWND hwnd);
wchar_t * GetContactFolder(wchar_t *fn, MCONTACT hContact);
BOOL ResolveShortcut(wchar_t *shortcut, wchar_t *file);

static INT_PTR ShowDialogSvc(WPARAM wParam, LPARAM lParam);

struct AvatarDialogData
{
	MCONTACT hContact;
	wchar_t fn[MAX_PATH];
	HWND parent;
};


class ListEntry
{
public:
	ListEntry()
	{
		hDbEvent = NULL;
		filename = nullptr;
		filelink = nullptr;
	}

	~ListEntry()
	{
		mir_free(filename);
		mir_free(filelink);
	}

	MEVENT hDbEvent;
	wchar_t *filename;
	wchar_t *filelink;
};

static void __cdecl AvatarDialogThread(AvatarDialogData *data)
{
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_AVATARDLG), data->parent, AvatarDlgProc, (LPARAM)data);
}

int OpenAvatarDialog(MCONTACT hContact, char* fn)
{
	HWND hAvatarWindow = WindowList_Find(hAvatarWindowsList, hContact);
	if (hAvatarWindow) {
		SetForegroundWindow(hAvatarWindow);
		SetFocus(hAvatarWindow);
		return 0;
	}

	AvatarDialogData *avdlg = (AvatarDialogData*)calloc(1, sizeof(AvatarDialogData));
	avdlg->hContact = hContact;
	if (fn == nullptr)
		avdlg->fn[0] = '\0';
	else
		MultiByteToWideChar(CP_ACP, 0, fn, -1, avdlg->fn, _countof(avdlg->fn));

	mir_forkThread<AvatarDialogData>(AvatarDialogThread, avdlg);
	return 0;
}

void EnableDisableControls(HWND hwnd)
{
	HWND list = GetDlgItem(hwnd, IDC_AVATARLIST);

	int cursel = SendMessage(list, LB_GETCURSEL, 0, 0);
	int count = SendMessage(list, LB_GETCOUNT, 0, 0);

	if (cursel == LB_ERR) {
		EnableWindow(GetDlgItem(hwnd, IDC_BACK), count > 0);
		EnableWindow(GetDlgItem(hwnd, IDC_NEXT), count > 0);
	}
	else {
		EnableWindow(GetDlgItem(hwnd, IDC_BACK), cursel > 0);
		EnableWindow(GetDlgItem(hwnd, IDC_NEXT), cursel < count - 1);
	}
}

static INT_PTR CALLBACK AvatarDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwnd, IDC_AVATARLIST);

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			AvatarDialogData *data = (struct AvatarDialogData*) lParam;
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)createDefaultOverlayedIcon(TRUE));
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)createDefaultOverlayedIcon(FALSE));
			if (g_plugin.getByte("LogToHistory", AVH_DEF_LOGTOHISTORY))
				FillAvatarListFromDB(hwndList, data->hContact);
			else if (opts.log_store_as_hash)
				FillAvatarListFromFolder(hwndList, data->hContact);
			else
				FillAvatarListFromFiles(hwndList, data->hContact);

			wchar_t *displayName = Clist_GetContactDisplayName(data->hContact);
			if (displayName) {
				wchar_t title[MAX_PATH];
				mir_snwprintf(title, TranslateT("Avatar history for %s"), displayName);
				SetWindowText(hwnd, title);
			}

			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data->hContact);
			UpdateAvatarPic(hwnd);
			CheckDlgButton(hwnd, IDC_LOGUSER, (UINT)db_get_b(data->hContact, MODULENAME, "LogToDisk", BST_INDETERMINATE));
			CheckDlgButton(hwnd, IDC_POPUPUSER, (UINT)db_get_b(data->hContact, MODULENAME, "AvatarPopups", BST_INDETERMINATE));
			CheckDlgButton(hwnd, IDC_HISTORYUSER, (UINT)db_get_b(data->hContact, MODULENAME, "LogToHistory", BST_INDETERMINATE));
			ShowWindow(GetDlgItem(hwnd, IDC_OPENFOLDER), opts.log_per_contact_folders ? SW_SHOW : SW_HIDE);
			Utils_RestoreWindowPositionNoSize(hwnd, NULL, MODULENAME, "AvatarHistoryDialog");
			WindowList_Add(hAvatarWindowsList, hwnd, data->hContact);
			TranslateDialogDefault(hwnd);
			EnableDisableControls(hwnd);
			free(data);
			data = nullptr;
		}
		break;

	case WM_CLOSE:
		CleanupAvatarPic(hwnd);
		EndDialog(hwnd, 0);
		return TRUE;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwnd, NULL, MODULENAME, "AvatarHistoryDialog");
		WindowList_Remove(hAvatarWindowsList, hwnd);
		DestroyIcon((HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(hwnd, WM_SETICON, ICON_SMALL, 0));
		{
			int count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
			for (int i = 0; i < count; i++)
				delete (ListEntry*)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
		}
		break;

	case WM_CONTEXTMENU:
		{
			HWND pic = GetDlgItem(hwnd, IDC_AVATAR);
			int pos;

			if ((HANDLE)wParam == hwndList) {
				POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
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
			else if ((HANDLE)wParam == pic) {
				pos = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
				if (pos == LB_ERR)
					break;
			}
			else break;

			HMENU menu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MENU1));
			HMENU submenu = GetSubMenu(menu, 0);
			TranslateMenu(submenu);

			if (!UpdateAvatarPic(hwnd)) {
				RemoveMenu(submenu, 2, MF_BYPOSITION);
				RemoveMenu(submenu, 0, MF_BYPOSITION);
			}

			POINT p;
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);
			int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwndList, nullptr);
			DestroyMenu(menu);

			ListEntry *le = (ListEntry*)SendMessage(hwndList, LB_GETITEMDATA, pos, 0);
			MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			switch (ret) {
			case ID_AVATARLISTPOPUP_SAVEAS:
				ShowSaveDialog(hwnd, le->filename, hContact);
				break;

			case ID_AVATARLISTPOPUP_DELETE:
				{
					BOOL blDelete;

					if (le->hDbEvent)
						blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this history entry?\nOnly the entry in history will be deleted, bitmap file will be kept!"),
							TranslateT("Delete avatar log?"), MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_TOPMOST) == IDYES;
					else
						blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this avatar shortcut?\nOnly shortcut will be deleted, bitmap file will be kept!"),
							TranslateT("Delete avatar log?"), MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_TOPMOST) == IDYES;

					if (blDelete) {
						if (le->hDbEvent)
							db_event_delete(le->hDbEvent);
						else
							DeleteFile(le->filelink);

						delete le;

						SendMessage(hwndList, LB_DELETESTRING, pos, 0);

						int count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
						if (count > 0) {
							if (pos >= count)
								pos = count - 1;
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
					blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this archived avatar?\nThis will delete the history entry and the bitmap file.\nWARNING: This can affect more than one entry in history!"),
						TranslateT("Delete avatar?"), MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_TOPMOST) == IDYES;
				else
					blDelete = MessageBox(hwnd, TranslateT("Are you sure you wish to delete this archived avatar?\nThis will delete the shortcut and the bitmap file.\nWARNING: This can affect more than one shortcut!"),
						TranslateT("Delete avatar?"), MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_TOPMOST) == IDYES;

				if (blDelete) {
					DeleteFile(le->filename);

					if (le->hDbEvent)
						db_event_delete(le->hDbEvent);
					else
						DeleteFile(le->filelink);

					delete le;

					SendMessage(hwndList, LB_DELETESTRING, pos, 0);

					int count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
					if (count > 0) {
						if (pos >= count)
							pos = count - 1;
						SendMessage(hwndList, LB_SETCURSEL, pos, 0);
					}

					UpdateAvatarPic(hwnd);
					EnableDisableControls(hwnd);
				}
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (HIWORD(wParam) == BN_CLICKED) {
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				g_plugin.setByte(hContact, "AvatarPopups", (uint8_t)IsDlgButtonChecked(hwnd, IDC_POPUPUSER));
				g_plugin.setByte(hContact, "LogToDisk", (uint8_t)IsDlgButtonChecked(hwnd, IDC_LOGUSER));
				g_plugin.setByte(hContact, "LogToHistory", (uint8_t)IsDlgButtonChecked(hwnd, IDC_HISTORYUSER));

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
				wchar_t avfolder[MAX_PATH];
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				GetContactFolder(avfolder, hContact);
				ShellExecute(nullptr, g_plugin.getByte("OpenFolderMethod", 0) ? L"explore" : L"open", avfolder, nullptr, nullptr, SW_SHOWNORMAL);
				return TRUE;
			}
			break;

		case IDC_NEXT:
			if (HIWORD(wParam) == BN_CLICKED) {
				SendMessage(hwndList, LB_SETCURSEL, SendMessage(hwndList, LB_GETCURSEL, 0, 0) + 1, 0);
				UpdateAvatarPic(hwnd);
				EnableDisableControls(hwnd);
				return TRUE;
			}
			break;

		case IDC_BACK:
			if (HIWORD(wParam) == BN_CLICKED) {
				int cursel = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
				if (cursel == LB_ERR)
					SendMessage(hwndList, LB_SETCURSEL, SendMessage(hwndList, LB_GETCOUNT, 0, 0) - 1, 0);
				else
					SendMessage(hwndList, LB_SETCURSEL, cursel - 1, 0);
				UpdateAvatarPic(hwnd);
				EnableDisableControls(hwnd);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}


int AddFileToList(wchar_t *path, wchar_t *lnk, wchar_t *filename, HWND list)
{
	// Add to list
	ListEntry *le = new ListEntry();
	le->filename = mir_wstrdup(path);
	le->filelink = mir_wstrdup(lnk);

	wchar_t *p = wcschr(filename, '.');
	if (p != nullptr)
		p[0] = '\0';
	int max_pos = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)filename);
	SendMessage(list, LB_SETITEMDATA, max_pos, (LPARAM)le);
	return max_pos;
}

int FillAvatarListFromFiles(HWND list, MCONTACT hContact)
{
	int max_pos = 0;
	wchar_t dir[MAX_PATH], path[MAX_PATH];
	WIN32_FIND_DATA finddata;

	GetContactFolder(dir, hContact);
	mir_snwprintf(path, L"%s\\*.*", dir);

	HANDLE hFind = FindFirstFile(path, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do {
		if (finddata.cFileName[0] != '.') {
			mir_snwprintf(path, L"%s\\%s", dir, finddata.cFileName);
			max_pos = AddFileToList(path, finddata.cFileName, finddata.cFileName, list);
		}
	} while (FindNextFile(hFind, &finddata));
	FindClose(hFind);
	SendMessage(list, LB_SETCURSEL, max_pos, 0); // Set to first item
	return 0;
}

int FillAvatarListFromFolder(HWND list, MCONTACT hContact)
{
	int max_pos = 0;
	wchar_t dir[MAX_PATH], path[MAX_PATH];
	WIN32_FIND_DATA finddata;

	GetContactFolder(dir, hContact);
	mir_snwprintf(path, L"%s\\*.lnk", dir);

	HANDLE hFind = FindFirstFile(path, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do {
		if (finddata.cFileName[0] != '.') {
			wchar_t lnk[MAX_PATH];
			mir_snwprintf(lnk, L"%s\\%s", dir, finddata.cFileName);
			if (ResolveShortcut(lnk, path))
				max_pos = AddFileToList(path, lnk, finddata.cFileName, list);
		}
	} while (FindNextFile(hFind, &finddata));
	FindClose(hFind);
	SendMessage(list, LB_SETCURSEL, max_pos, 0); // Set to first item
	return 0;
}

int FillAvatarListFromDB(HWND list, MCONTACT hContact)
{
	int max_pos = 0;
	DB::ECPTR pCursor(DB::Events(hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		DB::EventInfo dbei;
		dbei.cbBlob = -1;
		if (db_event_get(hDbEvent, &dbei))
			continue;
		if (dbei.eventType != EVENTTYPE_AVATAR_CHANGE)
			continue;

		// Get time
		wchar_t date[64];
		TimeZone_ToStringT(dbei.timestamp, L"d s", date, _countof(date));

		// Get file in disk
		wchar_t path[MAX_PATH];
		ptrW tszStoredPath(mir_utf8decodeW((char*)dbei.pBlob));
		PathToAbsoluteW(tszStoredPath, path);

		// Add to list
		ListEntry *le = new ListEntry();
		le->hDbEvent = hDbEvent;
		le->filename = mir_wstrdup(path);
		max_pos = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)date);
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
	if (cursel < 0) {
		SetDlgItemText(hwnd, IDC_AVATARPATH, TranslateT("Avatar history is empty!"));
		return false;
	}

	ListEntry *le = (ListEntry*)SendMessage(list, LB_GETITEMDATA, cursel, 0);

	if (!le || !le->filename) {
		SetDlgItemText(hwnd, IDC_AVATARPATH, TranslateT("Avatar path is null."));
		return 0;
	}
	SetDlgItemText(hwnd, IDC_AVATARPATH, le->filename);

	HBITMAP avpic = Bitmap_Load(le->filename);

	bool found_image = (avpic != nullptr);

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
	char *proto = Proto_GetBaseAccountName(wParam);
	Menu_ShowItem(hMenu, 0 != ProtocolEnabled(proto));
	return 0;
}

void InitMenuItem()
{
	CreateServiceFunction(MS_AVATARHISTORY_SHOWDIALOG, ShowDialogSvc);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x2fb5c7eb, 0xa606, 0x4145, 0x9e, 0x86, 0x73, 0x88, 0x73, 0x1d, 0xe7, 0x5c);
	mi.name.w = LPGENW("View avatar history");
	mi.flags = CMIF_UNICODE;
	mi.position = 1000090010;
	mi.hIcolibItem = createDefaultOverlayedIcon(FALSE);
	mi.pszService = MS_AVATARHISTORY_SHOWDIALOG;
	hMenu = Menu_AddContactMenuItem(&mi);
	DestroyIcon((HICON)mi.hIcolibItem);
}

static INT_PTR ShowDialogSvc(WPARAM wParam, LPARAM lParam)
{
	OpenAvatarDialog(wParam, (char*)lParam);
	return 0;
}


int ShowSaveDialog(HWND hwnd, wchar_t* fn, MCONTACT hContact)
{
	wchar_t filter[MAX_PATH], file[MAX_PATH];
	Bitmap_GetFilter(filter, _countof(filter));

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = g_plugin.getInst();

	ofn.lpstrFilter = filter;

	ofn.nFilterIndex = 1;
	wcsncpy_s(file, (wcsrchr(fn, '\\') + 1), _TRUNCATE);
	ofn.lpstrFile = file;

	wchar_t *displayName = Clist_GetContactDisplayName(hContact);
	wchar_t title[MAX_PATH];
	if (displayName) {
		mir_snwprintf(title, TranslateT("Save avatar for %s"), displayName);
		ofn.lpstrTitle = title;
	}
	else ofn.lpstrTitle = TranslateT("Save avatar");

	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_DONTADDTORECENT;
	ofn.lpstrDefExt = wcsrchr(fn, '.') + 1;

	ptrW wszInitDir(g_plugin.getWStringA(hContact, "SavedAvatarFolder"));
	if (wszInitDir)
		ofn.lpstrInitialDir = wszInitDir;
	else
		ofn.lpstrInitialDir = L".";

	if (GetSaveFileName(&ofn)) {
		CopyFile(fn, file, FALSE);
		g_plugin.setWString(hContact, "SavedAvatarFolder", file);
	}
	return 0;
}
