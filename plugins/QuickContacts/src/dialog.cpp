/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#define IDC_ICO 12344

#define IDC_ENTER 2000	// Pseudo control to handle enter in the main window

HIMAGELIST hIml;

long main_dialog_open = 0;
HWND hwndMain = nullptr;

// array where the contacts are put into
struct c_struct
{
	wchar_t szname[120];
	wchar_t szgroup[50];
	MCONTACT hcontact;
	wchar_t proto[20];

	c_struct()
	{
		szname[0] = 0;
		szgroup[0] = 0;
		hcontact = 0;
		proto[0] = 0;
	}
};

LIST<c_struct> contacts(200);
long max_proto_width;


// Get the name the contact has in list
// This was not made to be called by more than one thread!
wchar_t tmp_list_name[120];

wchar_t *GetListName(c_struct *cs)
{
	if (opts.group_append && cs->szgroup[0] != '\0') {
		mir_snwprintf(tmp_list_name, L"%s (%s)", cs->szname, cs->szgroup);
		return tmp_list_name;
	}
	else {
		return cs->szname;
	}
}


int lstreq(wchar_t *a, wchar_t *b, size_t len = -1)
{
	a = CharLower(wcsdup(a));
	b = CharLower(wcsdup(b));
	int ret;
	if (len > 0)
		ret = wcsncmp(a, b, len);
	else
		ret = mir_wstrcmp(a, b);
	free(a);
	free(b);
	return ret;
}


// simple sorting function to have
// the contact array in alphabetical order
void SortArray(void)
{
	int loop, doop;
	c_struct *cs_temp;

	SortedList *sl = (SortedList *)&contacts;
	for (loop = 0; loop < contacts.getCount(); loop++) {
		for (doop = loop + 1; doop < contacts.getCount(); doop++) {
			int cmp = lstreq(contacts[loop]->szname, contacts[doop]->szname);
			if (cmp > 0) {
				cs_temp = contacts[loop];
				sl->items[loop] = contacts[doop];
				sl->items[doop] = cs_temp;
			}
			else if (cmp == 0) {
				if (lstreq(contacts[loop]->proto, contacts[doop]->proto) > 0) {
					cs_temp = contacts[loop];
					sl->items[loop] = contacts[doop];
					sl->items[doop] = cs_temp;
				}
			}

		}
	}
}


int GetStatus(MCONTACT hContact, char *proto = nullptr)
{
	if (proto == nullptr)
		proto = Proto_GetBaseAccountName(hContact);

	if (proto == nullptr)
		return ID_STATUS_OFFLINE;

	return db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
}


void FreeContacts()
{
	for (auto &it : contacts)
		delete it;
	contacts.destroy();
}


void LoadContacts(HWND hwndDlg, BOOL show_all)
{
	BOOL metacontactsEnabled = db_mc_isEnabled();

	// Read last-sent-to contact from db and set handle as window-userdata
	HANDLE hlastsent = (HANDLE)g_plugin.getDword("LastSentTo", -1);
	SetWindowLongPtr(hwndMain, GWLP_USERDATA, (LONG_PTR)hlastsent);

	// enumerate all contacts and write them to the array
	// item data of listbox-strings is the array position
	FreeContacts();

	for (auto &hContact : Contacts()) {
		char *pszProto = Proto_GetBaseAccountName(hContact);
		if (pszProto == nullptr)
			continue;

		// Get meta
		MCONTACT hMeta = NULL;
		if (metacontactsEnabled) {
			if ((!show_all && opts.hide_subcontacts) || opts.group_append)
				hMeta = db_mc_getMeta(hContact);
		}
		else if (!mir_strcmp(META_PROTO, pszProto))
			continue;

		if (!show_all) {
			// Check if is offline and have to show
			if (GetStatus(hContact, pszProto) <= ID_STATUS_OFFLINE) {
				// See if has to show
				char setting[128];
				mir_snprintf(setting, "ShowOffline%s", pszProto);

				if (!g_plugin.getByte(setting, FALSE))
					continue;

				// Check if proto offline
				else if (opts.hide_from_offline_proto && Proto_GetStatus(pszProto) <= ID_STATUS_OFFLINE)
					continue;

			}

			// Check if is subcontact
			if (opts.hide_subcontacts && hMeta != NULL) {
				if (!opts.keep_subcontacts_from_offline)
					continue;

				if (GetStatus(hMeta, META_PROTO) > ID_STATUS_OFFLINE)
					continue;

				char setting[128];
				mir_snprintf(setting, "ShowOffline%s", META_PROTO);
				if (g_plugin.getByte(setting, FALSE))
					continue;
			}
		}

		// Add to list

		// Get group
		c_struct *contact = new c_struct();

		if (opts.group_append) {
			ptrW wszGroup(Clist_GetGroup(hMeta == NULL ? hContact : hMeta));
			if (wszGroup)
				wcsncpy_s(contact->szgroup, wszGroup, _TRUNCATE);
		}

		// Make contact name
		wchar_t *tmp = (wchar_t *)Clist_GetContactDisplayName(hContact);
		mir_wstrncpy(contact->szname, tmp, _countof(contact->szname));

		PROTOACCOUNT *acc = Proto_GetAccount(pszProto);
		if (acc != nullptr)
			mir_wstrncpy(contact->proto, acc->tszAccountName, _countof(contact->proto));

		contact->hcontact = hContact;
		contacts.insert(contact);
	}

	SortArray();

	SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_RESETCONTENT, 0, 0);
	for (int loop = 0; loop < contacts.getCount(); loop++)
		SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_ADDSTRING, 0, (LPARAM)GetListName(contacts[loop])), loop);
}


// Enable buttons for the selected contact
void EnableButtons(HWND hwndDlg, MCONTACT hContact)
{
	if (hContact == NULL) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_USERINFO), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORY), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_MENU), FALSE);

		SendDlgItemMessage(hwndDlg, IDC_ICO, STM_SETICON, 0, 0);
	}
	else {
		// Is a meta?
		MCONTACT hSub = db_mc_getMostOnline(hContact);
		if (hSub != NULL)
			hContact = hSub;

		// Get caps
		INT_PTR caps = 0;

		char *pszProto = Proto_GetBaseAccountName(hContact);
		if (pszProto != nullptr)
			caps = CallProtoService(pszProto, PS_GETCAPS, PFLAGNUM_1, 0);

		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), caps & PF1_IMSEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), caps & PF1_FILESEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_USERINFO), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORY), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_MENU), TRUE);

		HICON ico = ImageList_GetIcon(hIml, Clist_GetContactIcon(hContact), ILD_IMAGE);
		SendDlgItemMessage(hwndDlg, IDC_ICO, STM_SETICON, (WPARAM)ico, 0);
	}
}


// check if the char(s) entered appears in a contacts name
int CheckText(HWND hdlg, wchar_t *sztext, BOOL only_enable = FALSE)
{
	EnableButtons(hwndMain, NULL);

	if (sztext == nullptr || sztext[0] == '\0')
		return 0;

	size_t len = mir_wstrlen(sztext);

	if (only_enable) {
		for (auto &it : contacts) {
			if (lstreq(sztext, it->szname) == 0 || lstreq(sztext, GetListName(it)) == 0) {
				EnableButtons(hwndMain, it->hcontact);
				return 0;
			}
		}
	}
	else {
		for (auto &it : contacts) {
			if (lstreq(sztext, GetListName(it), len) == 0) {
				SetWindowText(hdlg, GetListName(it));
				SendMessage(hdlg, EM_SETSEL, (WPARAM)len, (LPARAM)-1);
				EnableButtons(hwndMain, it->hcontact);
				return 0;
			}
		}
	}

	EnableButtons(hwndMain, NULL);
	return 0;
}

MCONTACT GetSelectedContact(HWND hwndDlg)
{
	// First try selection
	int sel = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETCURSEL, 0, 0);

	if (sel != CB_ERR) {
		int pos = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETITEMDATA, sel, 0);
		if (pos != CB_ERR)
			return contacts[pos]->hcontact;
	}

	// Now try the name
	wchar_t cname[120] = L"";

	GetDlgItemText(hwndDlg, IDC_USERNAME, cname, _countof(cname));

	for (auto &it : contacts)
		if (!mir_wstrcmpi(cname, GetListName(it)))
			return it->hcontact;

	return NULL;
}

// get array position from handle
int GetItemPos(MCONTACT hcontact)
{
	for (auto &it : contacts)
		if (hcontact == it->hcontact)
			return contacts.indexOf(&it);

	return -1;
}


// callback function for edit-box of the listbox
// without this the autofill function isn't possible
// this was done like ie does it..as far as spy++ could tell ;)
LRESULT CALLBACK EditProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_CHAR:
		{
			if (wparam < 32 && wparam != VK_BACK)
				break;

			wchar_t sztext[120] = L"";
			uint32_t start;
			uint32_t end;

			int ret = SendMessage(hdlg, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

			GetWindowText(hdlg, sztext, _countof(sztext));

			BOOL at_end = (mir_wstrlen(sztext) == (int)end);

			if (ret != -1) {
				if (wparam == VK_BACK) {
					if (start > 0)
						SendMessage(hdlg, EM_SETSEL, (WPARAM)start - 1, (LPARAM)end);

					sztext[0] = 0;
				}
				else {
					sztext[0] = wparam;
					sztext[1] = 0;
				}

				SendMessage(hdlg, EM_REPLACESEL, 0, (LPARAM)sztext);
				GetWindowText(hdlg, sztext, _countof(sztext));
			}

			CheckText(hdlg, sztext, !at_end);

			return 1;
		}
	case WM_KEYUP:
		{
			wchar_t sztext[120] = L"";

			if (wparam == VK_RETURN) {
				switch (SendMessage(GetParent(hdlg), CB_GETDROPPEDSTATE, 0, 0)) {
				case FALSE:
					SendMessage(GetParent(GetParent(hdlg)), WM_COMMAND, MAKEWPARAM(IDC_ENTER, STN_CLICKED), 0);
					break;

				case TRUE:
					SendMessage(GetParent(hdlg), CB_SHOWDROPDOWN, FALSE, 0);
					break;
				}
			}
			else if (wparam == VK_DELETE) {
				GetWindowText(hdlg, sztext, _countof(sztext));
				CheckText(hdlg, sztext, TRUE);
			}

			return 0;
		}

	case WM_GETDLGCODE:
		return DLGC_WANTCHARS | DLGC_WANTARROWS;
	}

	return mir_callNextSubclass(hdlg, EditProc, msg, wparam, lparam);
}

HACCEL hAcct;
HHOOK hHook;

// This function filters the message queue and translates
// the keyboard accelerators
LRESULT CALLBACK HookProc(int code, WPARAM, LPARAM lparam)
{
	if (code != MSGF_DIALOGBOX)
		return 0;

	MSG *msg = (MSG*)lparam;

	int action = Hotkey_Check(msg, "Quick Contacts");
	if (action != 0) {
		SendMessage(hwndMain, WM_COMMAND, action, 0);
		return 1;
	}

	if (msg->message == WM_KEYDOWN && msg->wParam == VK_ESCAPE) {
		switch (SendDlgItemMessage(hwndMain, IDC_USERNAME, CB_GETDROPPEDSTATE, 0, 0)) {
		case FALSE:
			SendMessage(hwndMain, WM_CLOSE, 0, 0);
			break;

		case TRUE:
			SendDlgItemMessage(hwndMain, IDC_USERNAME, CB_SHOWDROPDOWN, FALSE, 0);
			break;
		}
	}

	return 0;
}

BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}


BOOL MoveWindow(HWND hWnd, const RECT &rect, BOOL bRepaint)
{
	return MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bRepaint);
}


static void FillButton(HWND hwndDlg, int dlgItem, wchar_t *name, wchar_t *key, HICON icon)
{
	wchar_t tmp[256];
	wchar_t *full = tmp;

	if (key == nullptr)
		full = TranslateW(name);
	else
		mir_snwprintf(tmp, L"%s (%s)", TranslateW(name), key);

	SendDlgItemMessage(hwndDlg, dlgItem, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, dlgItem, BUTTONADDTOOLTIP, (LPARAM)full, BATF_UNICODE);
	SendDlgItemMessage(hwndDlg, dlgItem, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);
}


static void FillCheckbox(HWND hwndDlg, int dlgItem, wchar_t *name, wchar_t *key)
{
	wchar_t tmp[256];
	wchar_t *full = tmp;

	if (key == nullptr)
		full = TranslateW(name);
	else
		mir_snwprintf(tmp, L"%s (%s)", TranslateW(name), key);

	SetDlgItemText(hwndDlg, dlgItem, full);
}


static INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_USERNAME), &rc);
			ScreenToClient(hwndDlg, &rc);

			CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE,
				rc.left - 20, rc.top + (rc.bottom - rc.top - 16) / 2, 16, 16, hwndDlg, (HMENU)IDC_ICO,
				g_plugin.getInst(), nullptr);

			hHook = SetWindowsHookEx(WH_MSGFILTER, HookProc, nullptr, GetCurrentThreadId());

			// Combo
			SendDlgItemMessage(hwndDlg, IDC_USERNAME, EM_LIMITTEXT, (WPARAM)119, 0);
			mir_subclassWindow(GetWindow(GetDlgItem(hwndDlg, IDC_USERNAME), GW_CHILD), EditProc);

			// Buttons
			FillCheckbox(hwndDlg, IDC_SHOW_ALL_CONTACTS, LPGENW("Show all contacts"), NULL);
			FillButton(hwndDlg, IDC_MESSAGE, LPGENW("Send message"), nullptr, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
			FillButton(hwndDlg, IDC_FILE, LPGENW("Send file"), NULL, Skin_LoadIcon(SKINICON_EVENT_FILE));
			FillButton(hwndDlg, IDC_USERINFO, LPGENW("Open user info"), NULL, Skin_LoadIcon(SKINICON_OTHER_USERDETAILS));
			FillButton(hwndDlg, IDC_HISTORY, LPGENW("Open history"), NULL, Skin_LoadIcon(SKINICON_OTHER_HISTORY));
			FillButton(hwndDlg, IDC_MENU, LPGENW("Open contact menu"), NULL, Skin_LoadIcon(SKINICON_OTHER_DOWNARROW));

			SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETEXTENDEDUI, TRUE, 0);

			Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULENAME, "window");

			LoadContacts(hwndDlg, FALSE);

			EnableButtons(hwndDlg, NULL);
			if (g_plugin.getByte("EnableLastSentTo", 0)) {
				int pos = GetItemPos(g_plugin.getDword("LastSentTo", -1));
				if (pos != -1) {
					SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETCURSEL, (WPARAM)pos, 0);
					EnableButtons(hwndDlg, contacts[pos]->hcontact);
				}
			}

			SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USERNAME:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int pos = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETCURSEL, 0, 0);
				EnableButtons(hwndDlg, pos < contacts.getCount() ? contacts[pos]->hcontact : NULL);
			}
			break;

		case IDC_ENTER:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
					break;

				Clist_ContactDoubleClicked(hContact);

				g_plugin.setDword("LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;
		case IDC_MESSAGE:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL) {
					SetDlgItemText(hwndDlg, IDC_USERNAME, L"");
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_MESSAGE)))
					break;

				CallService(MS_MSG_SENDMESSAGEW, hContact, 0);

				g_plugin.setDword("LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
				break;
			}

		case HOTKEY_FILE:
		case IDC_FILE:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL) {
					SetDlgItemText(hwndDlg, IDC_USERNAME, L"");
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FILE)))
					break;

				CallService(MS_FILE_SENDFILE, hContact, 0);

				g_plugin.setDword("LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case HOTKEY_INFO:
		case IDC_USERINFO:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL) {
					SetDlgItemText(hwndDlg, IDC_USERNAME, L"");
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_USERINFO)))
					break;

				CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);

				g_plugin.setDword("LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case HOTKEY_HISTORY:
		case IDC_HISTORY:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL) {
					SetDlgItemText(hwndDlg, IDC_USERNAME, L"");
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_HISTORY)))
					break;

				CallService(MS_HISTORY_SHOWCONTACTHISTORY, hContact, 0);

				g_plugin.setDword("LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case HOTKEY_MENU:
		case IDC_MENU:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL) {
					SetDlgItemText(hwndDlg, IDC_USERNAME, L"");
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_MENU)))
					break;

				RECT rc;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_MENU), &rc);
				HMENU hMenu = Menu_BuildContactMenu(hContact);
				int ret = TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, nullptr);
				DestroyMenu(hMenu);

				if (ret) {
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					Clist_MenuProcessCommand(LOWORD(ret), MPCF_CONTACTMENU, hContact);
				}

				g_plugin.setDword("LastSentTo", (uint32_t)hContact);
			}
			break;

		case HOTKEY_ALL_CONTACTS:
		case IDC_SHOW_ALL_CONTACTS:
			{
				// Get old text
				HWND hEdit = GetWindow(GetWindow(hwndDlg, GW_CHILD), GW_CHILD);
				wchar_t sztext[120] = L"";

				if (SendMessage(hEdit, EM_GETSEL, 0, 0) != -1)
					SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)L"");

				GetWindowText(hEdit, sztext, _countof(sztext));

				// Fill combo			
				BOOL all = IsDlgButtonChecked(hwndDlg, IDC_SHOW_ALL_CONTACTS);

				if (LOWORD(wParam) == HOTKEY_ALL_CONTACTS) {
					// Toggle checkbox
					all = !all;
					CheckDlgButton(hwndDlg, IDC_SHOW_ALL_CONTACTS, all ? BST_CHECKED : BST_UNCHECKED);
				}

				LoadContacts(hwndDlg, all);

				// Return selection
				CheckText(hEdit, sztext);
			}
		}
		break;

	case WM_CLOSE:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "window");
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		UnhookWindowsHookEx(hHook);
		hwndMain = nullptr;
		FreeContacts();
		InterlockedExchange(&main_dialog_open, 0);
		break;

	case WM_DRAWITEM:
		{
			// add icons and protocol to listbox
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

			// Handle contact menu
			if (lpdis->CtlID != IDC_USERNAME) {
				if (lpdis->CtlType == ODT_MENU)
					return Menu_DrawItem(lParam);
				break;
			}

			// Handle combo
			if (lpdis->itemID == -1)
				break;

			TEXTMETRIC tm;
			int icon_width = 0, icon_height = 0;
			RECT rc;

			GetTextMetrics(lpdis->hDC, &tm);
			ImageList_GetIconSize(hIml, &icon_width, &icon_height);

			COLORREF clrfore = SetTextColor(lpdis->hDC, GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
			COLORREF clrback = SetBkColor(lpdis->hDC, GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

			// Draw icon
			rc.left = lpdis->rcItem.left + 5;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - icon_height) / 2;
			ImageList_Draw(hIml, Clist_GetContactIcon(contacts[lpdis->itemData]->hcontact), lpdis->hDC, rc.left, rc.top, ILD_NORMAL);

			// Make rect for text
			rc.left += icon_width + 5;
			rc.right = lpdis->rcItem.right - 1;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;
			rc.bottom = rc.top + tm.tmHeight;

			// Draw Protocol
			if (opts.num_protos > 1) {
				if (max_proto_width == 0) {
					// Has to be done, else the DC isnt the right one
					// Dont ask me why
					for (auto &it : contacts) {
						RECT rcc = { 0, 0, 0x7FFF, 0x7FFF };
						DrawText(lpdis->hDC, it->proto, -1, &rcc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_CALCRECT);
						max_proto_width = max(max_proto_width, rcc.right - rcc.left);
					}

					// Fix max_proto_width
					if (opts.group_append && opts.group_column)
						max_proto_width = min(max_proto_width, (rc.right - rc.left) / 5);
					else if (opts.group_append)
						max_proto_width = min(max_proto_width, (rc.right - rc.left) / 4);
					else
						max_proto_width = min(max_proto_width, (rc.right - rc.left) / 3);
				}

				RECT rc_tmp = rc;
				rc_tmp.left = rc_tmp.right - max_proto_width;
				DrawText(lpdis->hDC, contacts[lpdis->itemData]->proto, -1, &rc_tmp, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
				rc.right = rc_tmp.left - 5;
			}

			// Draw group
			if (opts.group_append && opts.group_column) {
				RECT rc_tmp = rc;

				if (opts.group_column_left) {
					rc_tmp.right = rc_tmp.left + (rc.right - rc.left) / 3;
					rc.left = rc_tmp.right + 5;
				}
				else {
					rc_tmp.left = rc_tmp.right - (rc.right - rc.left) / 3;
					rc.right = rc_tmp.left - 5;
				}

				DrawText(lpdis->hDC, contacts[lpdis->itemData]->szgroup, -1, &rc_tmp, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
			}

			// Draw text
			wchar_t *name;
			if (opts.group_append && !opts.group_column)
				name = GetListName(contacts[lpdis->itemData]);
			else
				name = contacts[lpdis->itemData]->szname;

			DrawText(lpdis->hDC, name, -1, &rc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			// Restore old colors
			SetTextColor(lpdis->hDC, clrfore);
			SetBkColor(lpdis->hDC, clrback);
		}
		return TRUE;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;

			// Handle contact menu
			if (lpmis->CtlID != IDC_USERNAME) {
				if (lpmis->CtlType == ODT_MENU)
					return Menu_MeasureItem(lParam);
				break;
			}

			// Handle combo

			TEXTMETRIC tm;
			int icon_width = 0, icon_height = 0;

			GetTextMetrics(GetDC(hwndDlg), &tm);
			ImageList_GetIconSize(hIml, &icon_width, &icon_height);

			lpmis->itemHeight = max(icon_height, tm.tmHeight);

			return TRUE;
		}
	}

	return FALSE;
}

// Show the main dialog
INT_PTR ShowDialog(WPARAM, LPARAM)
{
	// Get the icons for the listbox
	hIml = Clist_GetImageList();

	if (!main_dialog_open) {
		InterlockedExchange(&main_dialog_open, 1);

		hwndMain = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MAIN), nullptr, MainDlgProc);
	}

	// Show it
	SetForegroundWindow(hwndMain);
	SetFocus(hwndMain);
	ShowWindow(hwndMain, SW_SHOW);
	return 0;
}
