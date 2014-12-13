/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci
Based on work (C) Heiko Schillinger

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "commons.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////
					 
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {F93BA59C-4F48-4F2E-8A91-77A2801527A3}
	{0xf93ba59c, 0x4f48, 0x4f2e, {0x8a, 0x91, 0x77, 0xa2, 0x80, 0x15, 0x27, 0xa3}}
};

HINSTANCE hInst;
HIMAGELIST hIml;
int hLangpack = 0;

HANDLE hModulesLoaded = NULL;
HANDLE hEventAdded = NULL;
HANDLE hHotkeyPressed = NULL;
HANDLE hQSShowDialog = NULL;

long main_dialog_open = 0;
HWND hwndMain = NULL;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int EventAdded(WPARAM wparam, LPARAM lparam);
int HotkeyPressed(WPARAM wParam, LPARAM lParam);
INT_PTR ShowDialog(WPARAM wParam,LPARAM lParam);
void FreeContacts();

int hksModule = 0;
int hksAction = 0;

BOOL hasNewHotkeyModule = FALSE;

#define IDC_ICO 12344

// Functions ////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load() 
{
	mir_getLP(&pluginInfo);

	hQSShowDialog = CreateServiceFunction(MS_QC_SHOW_DIALOG, ShowDialog);

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hEventAdded = HookEvent(ME_DB_EVENT_ADDED, EventAdded);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void) 
{
	FreeContacts();

	DeInitOptions();

	DestroyServiceFunction(hQSShowDialog);

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hEventAdded);

	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	InitOptions();

	// Get number of protocols
	int pcount = 0;
	PROTOACCOUNT** pdesc;

	ProtoEnumAccounts(&pcount,&pdesc);

	opts.num_protos = pcount;

	// Add hotkey to multiple services

	hasNewHotkeyModule = TRUE;

	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.dwFlags = HKD_TCHAR;
	hkd.pszName = "Quick Contacts/Open dialog";
	hkd.ptszDescription = LPGENT("Open dialog");
	hkd.ptszSection = LPGENT("Quick Contacts");
	hkd.pszService = MS_QC_SHOW_DIALOG;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_ALT, 'Q');
	Hotkey_Register(&hkd);

	hkd.pszService = NULL;

	hkd.lParam = HOTKEY_FILE;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'F');
	hkd.pszName = "Quick Contacts/File";
	hkd.ptszDescription = LPGENT("Send file");
	Hotkey_Register(&hkd);

	hkd.lParam = HOTKEY_URL;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'U');
	hkd.pszName = "Quick Contacts/URL";
	hkd.ptszDescription = LPGENT("Send URL");
	Hotkey_Register(&hkd);

	hkd.lParam = HOTKEY_INFO;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'I');
	hkd.pszName = "Quick Contacts/Info";
	hkd.ptszDescription = LPGENT("Open user info");
	Hotkey_Register(&hkd);
		
	hkd.lParam = HOTKEY_HISTORY;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'H');
	hkd.pszName = "Quick Contacts/History";
	hkd.ptszDescription = LPGENT("Open history");
	Hotkey_Register(&hkd);
		
	hkd.lParam = HOTKEY_MENU;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'M');
	hkd.pszName = "Quick Contacts/Menu";
	hkd.ptszDescription = LPGENT("Open contact menu");
	Hotkey_Register(&hkd);
		
	hkd.lParam = HOTKEY_ALL_CONTACTS;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'A');
	hkd.pszName = "Quick Contacts/All Contacts";
	hkd.ptszDescription = LPGENT("Show all contacts");
	Hotkey_Register(&hkd);

	if (ServiceExists(MS_SKIN_ADDHOTKEY))
	{
		SKINHOTKEYDESCEX hk = {0};
		hk.cbSize = sizeof(hk);
		hk.pszSection = Translate("Quick Contacts");
		hk.pszName = Translate("Open dialog");
		hk.pszDescription = Translate("Open dialog");
		hk.pszService = MS_QC_SHOW_DIALOG;
		hk.DefHotKey = 0;
		CallService(MS_SKIN_ADDHOTKEY, 0, (LPARAM)&hk);
	}

	// Get the icons for the listbox
	hIml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST,0,0);

	// Add menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 500100001;
	mi.flags = CMIF_TCHAR;
	mi.ptszName = LPGENT("Quick Contacts...");
	mi.pszService = MS_QC_SHOW_DIALOG;
	Menu_AddMainMenuItem(&mi);
	return 0;
}


// called when a message/file/url was sent
// handle of contact is set as window-userdata
int EventAdded(WPARAM wparam, LPARAM lparam)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get((HANDLE)lparam, &dbei);
	if ( !(dbei.flags & DBEF_SENT) || (dbei.flags & DBEF_READ) 
		|| !db_get_b(NULL, MODULE_NAME, "EnableLastSentTo", 0) 
		|| db_get_w(NULL, MODULE_NAME, "MsgTypeRec", TYPE_GLOBAL) != TYPE_GLOBAL) 
		return 0;

	db_set_dw(NULL, MODULE_NAME, "LastSentTo", (DWORD)(HANDLE)wparam);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define IDC_ENTER 2000	// Pseudo control to handle enter in the main window


// array where the contacts are put into
struct c_struct {
	TCHAR szname[120];
	TCHAR szgroup[50];
	MCONTACT hcontact;
	TCHAR proto[20];

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
TCHAR tmp_list_name[120];

TCHAR *GetListName(c_struct *cs)
{
	if (opts.group_append && cs->szgroup[0] != _T('\0'))
	{
		mir_sntprintf(tmp_list_name, SIZEOF(tmp_list_name), _T("%s (%s)"), cs->szname, cs->szgroup);
		return tmp_list_name;
	}
	else
	{
		return cs->szname;
	}
}


int lstreq(TCHAR *a, TCHAR *b, size_t len = -1)
{
	a = CharLower(_tcsdup(a));
	b = CharLower(_tcsdup(b));
	int ret;
	if (len > 0)
		ret = _tcsncmp(a, b, len);
	else
		ret = _tcscmp(a, b);
	free(a);
	free(b);
	return ret;
}


// simple sorting function to have
// the contact array in alphabetical order
void SortArray(void)
{
	int loop,doop;
	c_struct *cs_temp;

	SortedList *sl = (SortedList *) &contacts;
	for(loop=0;loop<contacts.getCount();loop++)
	{
		for(doop=loop+1;doop<contacts.getCount();doop++)
		{
			int cmp = lstreq(contacts[loop]->szname,contacts[doop]->szname);
			if (cmp > 0)
			{
				cs_temp=contacts[loop];
				sl->items[loop]=contacts[doop];
				sl->items[doop]=cs_temp;
			}
			else if (cmp == 0)
			{
				if(lstreq(contacts[loop]->proto, contacts[doop]->proto) > 0)
				{
					cs_temp=contacts[loop];
					sl->items[loop]=contacts[doop];
					sl->items[doop]=cs_temp;
				}
			}

		}
	}
}


int GetStatus(MCONTACT hContact, char *proto = NULL)
{
	if (proto == NULL)
		proto = GetContactProto(hContact);

	if (proto == NULL)
		return ID_STATUS_OFFLINE;

	return db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
}


void FreeContacts()
{
	for (int i = contacts.getCount() - 1; i >= 0; i--)
	{
		delete contacts[i];
		contacts.remove(i);
	}
}


void LoadContacts(HWND hwndDlg, BOOL show_all)
{
	BOOL metacontactsEnabled = db_mc_isEnabled();

	// Read last-sent-to contact from db and set handle as window-userdata
	HANDLE hlastsent = (HANDLE)db_get_dw(NULL, MODULE_NAME, "LastSentTo", -1);
	SetWindowLongPtr(hwndMain, GWLP_USERDATA, (LONG_PTR)hlastsent);

	// enumerate all contacts and write them to the array
	// item data of listbox-strings is the array position
	FreeContacts();

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *pszProto = GetContactProto(hContact);
		if(pszProto == NULL)
			continue;

		// Get meta
		MCONTACT hMeta = NULL;
		if (metacontactsEnabled)
		{
			if ((!show_all && opts.hide_subcontacts) || opts.group_append)
				hMeta = db_mc_getMeta(hContact);
		}
		else if (!strcmp(META_PROTO, pszProto))
			continue;

		if (!show_all)
		{
			// Check if is offline and have to show
			if (GetStatus(hContact, pszProto) <= ID_STATUS_OFFLINE)
			{
				// See if has to show
				char setting[128];
				mir_snprintf(setting, SIZEOF(setting), "ShowOffline%s", pszProto);

				if (!db_get_b(NULL, MODULE_NAME, setting, FALSE))
					continue;

				// Check if proto offline
				else if (opts.hide_from_offline_proto 
						&& CallProtoService(pszProto, PS_GETSTATUS, 0, 0) <= ID_STATUS_OFFLINE)
					continue;

			}

			// Check if is subcontact
			if (opts.hide_subcontacts && hMeta != NULL) 
			{
				if (!opts.keep_subcontacts_from_offline)
					continue;

				if (GetStatus(hMeta, META_PROTO) > ID_STATUS_OFFLINE)
					continue;

				char setting[128];
				mir_snprintf(setting, SIZEOF(setting), "ShowOffline%s", META_PROTO);
				if (db_get_b(NULL, MODULE_NAME, setting, FALSE))
					continue;
			}
		}

		// Add to list

		// Get group
		c_struct *contact = new c_struct();
			
		if (opts.group_append)
		{
			DBVARIANT dbv;
			if (db_get_ts(hMeta == NULL ? hContact : hMeta, "CList", "Group", &dbv) == 0)
			{
				if (dbv.ptszVal != NULL)
					mir_tstrncpy(contact->szgroup, dbv.ptszVal, SIZEOF(contact->szgroup));

				db_free(&dbv);
			}
		}

		// Make contact name
		TCHAR *tmp = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
		mir_tstrncpy(contact->szname, tmp, SIZEOF(contact->szname));

		PROTOACCOUNT *acc = ProtoGetAccount(pszProto);
		if (acc != NULL)
			mir_tstrncpy(contact->proto, acc->tszAccountName, SIZEOF(contact->proto));

		contact->hcontact = hContact;
		contacts.insert(contact);
	}

	SortArray();
			
	SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_RESETCONTENT, 0, 0);
	for(int loop = 0; loop < contacts.getCount(); loop++)
	{
		SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETITEMDATA, 
							(WPARAM)SendDlgItemMessage(hwndDlg, IDC_USERNAME, 
											CB_ADDSTRING, 0, (LPARAM) GetListName(contacts[loop])), 
							(LPARAM)loop);
	}
}


// Enable buttons for the selected contact
void EnableButtons(HWND hwndDlg, MCONTACT hContact)
{
	if (hContact == NULL)
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_URL), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_USERINFO), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORY), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_MENU), FALSE);

		SendDlgItemMessage(hwndDlg, IDC_ICO, STM_SETICON, 0, 0);
	}
	else
	{
		// Is a meta?
		MCONTACT hSub = db_mc_getMostOnline(hContact);
		if (hSub != NULL)
			hContact = hSub;

		// Get caps
		INT_PTR caps = 0;

		char *pszProto = GetContactProto(hContact);
		if (pszProto != NULL)
			caps = CallProtoService(pszProto, PS_GETCAPS, PFLAGNUM_1, 0);

		EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), caps & PF1_IMSEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), caps & PF1_FILESEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_URL), caps & PF1_URLSEND ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_USERINFO), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HISTORY), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_MENU), TRUE);

		HICON ico = ImageList_GetIcon(hIml, CallService(MS_CLIST_GETCONTACTICON, hContact, 0), ILD_IMAGE);
		SendDlgItemMessage(hwndDlg, IDC_ICO, STM_SETICON, (WPARAM) ico, 0);
	}
}


// check if the char(s) entered appears in a contacts name
int CheckText(HWND hdlg, TCHAR *sztext, BOOL only_enable = FALSE)
{
	EnableButtons(hwndMain, NULL);

	if(sztext == NULL || sztext[0] == _T('\0'))
		return 0;

	int len = mir_tstrlen(sztext);

	if (only_enable)
	{
		for(int loop=0;loop<contacts.getCount();loop++)
		{
			if(lstreq(sztext, contacts[loop]->szname)==0 || lstreq(sztext, GetListName(contacts[loop]))==0)
			{
				EnableButtons(hwndMain, contacts[loop]->hcontact);
				return 0;
			}
		}
	}
	else
	{
		for(int loop=0;loop<contacts.getCount();loop++)
		{
			if (lstreq(sztext, GetListName(contacts[loop]), len) == 0)
			{
				SendMessage(hdlg, WM_SETTEXT, 0, (LPARAM) GetListName(contacts[loop]));
				SendMessage(hdlg, EM_SETSEL, (WPARAM) len, (LPARAM) -1);
				EnableButtons(hwndMain, contacts[loop]->hcontact);
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

	if (sel != CB_ERR)
	{
		int pos = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETITEMDATA, sel, 0);
		if (pos != CB_ERR)
			return contacts[pos]->hcontact;
	}

	// Now try the name
	TCHAR cname[120] = _T("");

	GetDlgItemText(hwndDlg, IDC_USERNAME, cname, SIZEOF(cname));
			
	for(int loop = 0; loop < contacts.getCount(); loop++)
	{
		if(!mir_tstrcmpi(cname, GetListName(contacts[loop])))
			return contacts[loop]->hcontact;
	}

	return NULL;
}

// get array position from handle
int GetItemPos(MCONTACT hcontact)
{
	for(int loop=0; loop < contacts.getCount(); loop++)
		if(hcontact == contacts[loop]->hcontact)
			return loop;

	return -1;
}


// callback function for edit-box of the listbox
// without this the autofill function isn't possible
// this was done like ie does it..as far as spy++ could tell ;)
LRESULT CALLBACK EditProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg) {
	case WM_CHAR:
		{
			if (wparam<32 && wparam != VK_BACK) 
				break;

			TCHAR sztext[120] = _T("");
			DWORD start;
			DWORD end;

			int ret = SendMessage(hdlg,EM_GETSEL,(WPARAM)&start,(LPARAM)&end);

			SendMessage(hdlg,WM_GETTEXT,(WPARAM)SIZEOF(sztext),(LPARAM)sztext);

			BOOL at_end = (mir_tstrlen(sztext) == (int)end);

			if (ret != -1)
			{
				if (wparam == VK_BACK)
				{
					if (start > 0)
						SendMessage(hdlg,EM_SETSEL,(WPARAM)start-1,(LPARAM)end);

					sztext[0]=0;
				}
				else
				{
					sztext[0]=wparam;
					sztext[1]=0;
				}

				SendMessage(hdlg,EM_REPLACESEL,0,(LPARAM)sztext);
				SendMessage(hdlg,WM_GETTEXT,(WPARAM)SIZEOF(sztext),(LPARAM)sztext);
			}

			CheckText(hdlg, sztext, !at_end);

			return 1;
		}
	case WM_KEYUP:
		{
			TCHAR sztext[120] = _T("");

			if (wparam == VK_RETURN)
			{
				switch(SendMessage(GetParent(hdlg),CB_GETDROPPEDSTATE,0,0))
				{
				case FALSE:
					SendMessage(GetParent(GetParent(hdlg)),WM_COMMAND,MAKEWPARAM(IDC_ENTER,STN_CLICKED),0);
					break;

				case TRUE:
					SendMessage(GetParent(hdlg),CB_SHOWDROPDOWN,FALSE,0);
					break;
				}
			}
			else if (wparam == VK_DELETE)
			{
				SendMessage(hdlg,WM_GETTEXT,(WPARAM)SIZEOF(sztext),(LPARAM)sztext);
				CheckText(hdlg, sztext, TRUE);
			}

			return 0;
		}

	case WM_GETDLGCODE:
		return DLGC_WANTCHARS|DLGC_WANTARROWS;
	}

	return mir_callNextSubclass(hdlg, EditProc, msg, wparam, lparam);
}

HACCEL hAcct;
HHOOK hHook;

// This function filters the message queue and translates
// the keyboard accelerators
LRESULT CALLBACK HookProc(int code, WPARAM wparam, LPARAM lparam)
{
	if (code!=MSGF_DIALOGBOX) 
		return 0;

	MSG *msg = (MSG*)lparam;

	if (hasNewHotkeyModule) {
		int action = CallService(MS_HOTKEY_CHECK, (WPARAM) msg, (LPARAM) "Quick Contacts");
		if (action != 0)
		{
			SendMessage(hwndMain, WM_COMMAND, action, 0);
			return 1;
		}
	}
	else {
		HWND htemp = msg->hwnd;
		msg->hwnd = hwndMain;

		if (TranslateAccelerator(msg->hwnd, hAcct, msg))
			return 1;
		
		msg->hwnd=htemp;
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


static void FillButton(HWND hwndDlg, int dlgItem, TCHAR *name, TCHAR *key, HICON icon)
{
	TCHAR tmp[256];
	TCHAR *full = tmp;

	if (key == NULL)
		full = TranslateTS(name);
	else
		mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (%s)"), TranslateTS(name), key);

	SendDlgItemMessage(hwndDlg, dlgItem, BUTTONSETASFLATBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, dlgItem, BUTTONADDTOOLTIP, (LPARAM)full, BATF_TCHAR);
	SendDlgItemMessage(hwndDlg, dlgItem, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);
}


static void FillCheckbox(HWND hwndDlg, int dlgItem, TCHAR *name, TCHAR *key)
{
	TCHAR tmp[256];
	TCHAR *full = tmp;

	if (key == NULL)
		full = TranslateTS(name);
	else
		mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (%s)"), TranslateTS(name), key);

	SendDlgItemMessage(hwndDlg, dlgItem, WM_SETTEXT, 0, (LPARAM)full);
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

			HWND icon = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, 
				rc.left - 20, rc.top + (rc.bottom - rc.top - 16) / 2, 16, 16, hwndDlg, (HMENU) IDC_ICO, 
				hInst, NULL);

			if (!hasNewHotkeyModule)
				hAcct = LoadAccelerators(hInst, MAKEINTRESOURCE(ACCEL_TABLE));

			hHook = SetWindowsHookEx(WH_MSGFILTER, HookProc, 0, GetCurrentThreadId());

			// Combo
			SendDlgItemMessage(hwndDlg, IDC_USERNAME, EM_LIMITTEXT, (WPARAM)119, 0);
			mir_subclassWindow(GetWindow(GetDlgItem(hwndDlg, IDC_USERNAME),GW_CHILD), EditProc);

			// Buttons
			FillCheckbox(hwndDlg, IDC_SHOW_ALL_CONTACTS, LPGENT("Show all contacts"), hasNewHotkeyModule ? NULL : _T("Ctrl+A"));
			FillButton(hwndDlg, IDC_MESSAGE, LPGENT("Send message"), NULL, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
			FillButton(hwndDlg, IDC_FILE, LPGENT("Send file"), hasNewHotkeyModule ? NULL : _T("Ctrl+F"), LoadSkinnedIcon(SKINICON_EVENT_FILE));
			FillButton(hwndDlg, IDC_URL, LPGENT("Send URL"), hasNewHotkeyModule ? NULL : _T("Ctrl+U"), LoadSkinnedIcon(SKINICON_EVENT_URL));
			FillButton(hwndDlg, IDC_USERINFO, LPGENT("Open user info"), hasNewHotkeyModule ? NULL : _T("Ctrl+I"), LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS));
			FillButton(hwndDlg, IDC_HISTORY, LPGENT("Open history"), hasNewHotkeyModule ? NULL : _T("Ctrl+H"), LoadSkinnedIcon(SKINICON_OTHER_HISTORY));
			FillButton(hwndDlg, IDC_MENU, LPGENT("Open contact menu"), hasNewHotkeyModule ? NULL : _T("Ctrl+M"), LoadSkinnedIcon(SKINICON_OTHER_DOWNARROW));

			SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETEXTENDEDUI, TRUE, 0);

			Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE_NAME, "window");

			LoadContacts(hwndDlg, FALSE);

			EnableButtons(hwndDlg, NULL);
			if (db_get_b(NULL, MODULE_NAME, "EnableLastSentTo", 0)) {
				int pos = GetItemPos((MCONTACT)db_get_dw(NULL, MODULE_NAME, "LastSentTo", -1));
				if (pos != -1) {
					SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_SETCURSEL, (WPARAM) pos, 0);
					EnableButtons(hwndDlg, contacts[pos]->hcontact);
				}
			}

			SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
		}
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_USERNAME:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				int pos = SendDlgItemMessage(hwndDlg, IDC_USERNAME, CB_GETCURSEL, 0, 0);
				EnableButtons(hwndDlg, pos < contacts.getCount() ? contacts[pos]->hcontact : NULL);
			}
			break;

		case IDC_ENTER:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
					break;

				CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);

				db_set_dw(NULL, MODULE_NAME, "LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;
		case IDC_MESSAGE:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
				{
					SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_MESSAGE)))
					break;

				CallService(MS_MSG_SENDMESSAGET, hContact, 0);

				db_set_dw(NULL, MODULE_NAME, "LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
				break;
			}

		case HOTKEY_FILE:
		case IDC_FILE:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
				{
					SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FILE)))
					break;

				CallService(MS_FILE_SENDFILE, hContact, 0);

				db_set_dw(NULL, MODULE_NAME, "LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case HOTKEY_URL:
		case IDC_URL:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
				{
					SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_URL)))
					break;

				CallService(MS_URL_SENDURL, hContact, 0);

				db_set_dw(NULL, MODULE_NAME, "LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case HOTKEY_INFO:
		case IDC_USERINFO:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
				{
					SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_USERINFO)))
					break;

				CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);

				db_set_dw(NULL, MODULE_NAME, "LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case HOTKEY_HISTORY:
		case IDC_HISTORY:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
				{
					SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_HISTORY)))
					break;

				CallService(MS_HISTORY_SHOWCONTACTHISTORY, hContact, 0);

				db_set_dw(NULL, MODULE_NAME, "LastSentTo", hContact);
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			}
			break;

		case HOTKEY_MENU:
		case IDC_MENU:
			{
				MCONTACT hContact = GetSelectedContact(hwndDlg);
				if (hContact == NULL)
				{
					SetDlgItemText(hwndDlg, IDC_USERNAME, _T(""));
					SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
					break;
				}

				// Is button enabled?
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_MENU)))
					break;

				RECT rc;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_MENU), &rc);
				HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
				int ret = TrackPopupMenu(hMenu, TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);
				DestroyMenu(hMenu);

				if(ret)
				{
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret),MPCF_CONTACTMENU),(LPARAM) hContact);
				}

				db_set_dw(NULL, MODULE_NAME, "LastSentTo", (DWORD) hContact);
			}
			break;

		case HOTKEY_ALL_CONTACTS:
		case IDC_SHOW_ALL_CONTACTS:
			{
				// Get old text
				HWND hEdit = GetWindow(GetWindow(hwndDlg,GW_CHILD),GW_CHILD);
				TCHAR sztext[120] = _T("");

				if (SendMessage(hEdit, EM_GETSEL, 0, 0) != -1)
					SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)_T(""));

				SendMessage(hEdit, WM_GETTEXT, (WPARAM)SIZEOF(sztext), (LPARAM)sztext);

				// Fill combo			
				BOOL all = IsDlgButtonChecked(hwndDlg, IDC_SHOW_ALL_CONTACTS);

				if (LOWORD(wParam) == HOTKEY_ALL_CONTACTS)
				{
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
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE_NAME, "window");
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		UnhookWindowsHookEx(hHook);
		hwndMain = NULL;
		FreeContacts();
		InterlockedExchange(&main_dialog_open, 0);
		break;

	case WM_DRAWITEM:
		{
			// add icons and protocol to listbox
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

			// Handle contact menu
			if(lpdis->CtlID != IDC_USERNAME) 
			{
				if (lpdis->CtlType == ODT_MENU)
					return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
				else
					break;
			}

			// Handle combo
			if(lpdis->itemID == -1) 
				break;

			TEXTMETRIC tm;
			int icon_width=0, icon_height=0;
			RECT rc;

			GetTextMetrics(lpdis->hDC, &tm);
			ImageList_GetIconSize(hIml, &icon_width, &icon_height);

			COLORREF clrfore = SetTextColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));
			COLORREF clrback = SetBkColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHT:COLOR_WINDOW));

			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

			// Draw icon
			rc.left = lpdis->rcItem.left + 5;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - icon_height) / 2;
			ImageList_Draw(hIml, CallService(MS_CLIST_GETCONTACTICON, (WPARAM)contacts[lpdis->itemData]->hcontact, 0), 
				lpdis->hDC, rc.left, rc.top, ILD_NORMAL);

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
					for (int loop = 0; loop < contacts.getCount(); loop++) {
						RECT rcc = { 0, 0, 0x7FFF, 0x7FFF };
						DrawText(lpdis->hDC, contacts[loop]->proto, -1, &rcc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_CALCRECT);
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
			TCHAR *name;
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
			if(lpmis->CtlID != IDC_USERNAME) 
			{
				if (lpmis->CtlType == ODT_MENU)
					return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);
				else
					break;
			}

			// Handle combo

			TEXTMETRIC tm;
			int icon_width = 0, icon_height=0;

			GetTextMetrics(GetDC(hwndDlg), &tm);
			ImageList_GetIconSize(hIml, &icon_width, &icon_height);

			lpmis->itemHeight = max(icon_height, tm.tmHeight);

			return TRUE;
		}
	}

	return FALSE;
}


// Show the main dialog
INT_PTR ShowDialog(WPARAM wParam, LPARAM lParam) 
{
	if (!main_dialog_open) 
	{
		InterlockedExchange(&main_dialog_open, 1);

		hwndMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	}

	// Show it
	SetForegroundWindow(hwndMain);
	SetFocus(hwndMain);
	ShowWindow(hwndMain, SW_SHOW);

	return 0;
}
