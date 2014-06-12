/*
	ClientChangeNotify - Plugin for Miranda IM
	Copyright (c) 2006-2008 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Common.h"

HINSTANCE g_hInstance;
HANDLE    g_hMainThread;
HGENMENU  g_hTogglePopupsMenuItem;
int       hLangpack;

COptPage *g_PreviewOptPage; // we need to show popup even for the NULL contact if g_PreviewOptPage is not NULL (used for popup preview)
BOOL bPopupExists = FALSE, bFingerprintExists = FALSE, bVariablesExists = FALSE;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {B68A8906-748B-435D-930E-21CC6E8F3B3F}
	{0xb68a8906, 0x748b, 0x435d, {0x93, 0xe, 0x21, 0xcc, 0x6e, 0x8f, 0x3b, 0x3f}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInstance = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static int CALLBACK MenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static VOID NTAPI ShowContactMenu(ULONG_PTR wParam)
// wParam = hContact
{
	POINT pt;
	HWND hMenuWnd = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T(MOD_NAME)_T("_MenuWindow"), 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInstance, NULL);
	SetWindowLongPtr(hMenuWnd, GWLP_WNDPROC, (LONG_PTR)MenuWndProc);
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)wParam, 0);
	GetCursorPos(&pt);
	SetForegroundWindow(hMenuWnd);
	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hMenuWnd, NULL), MPCF_CONTACTMENU), (LPARAM)wParam);
	PostMessage(hMenuWnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
	DestroyWindow(hMenuWnd);
}


void Popup_DoAction(HWND hWnd, BYTE Action, PLUGIN_DATA *pdata)
{
	MCONTACT hContact = (MCONTACT)CallService(MS_POPUP_GETCONTACT, (WPARAM)hWnd, 0);
	switch (Action) {
	case PCA_OPENMESSAGEWND: // open message window
		if (hContact && hContact != INVALID_CONTACT_ID)
			CallServiceSync(ServiceExists("SRMsg/LaunchMessageWindow") ? "SRMsg/LaunchMessageWindow" : MS_MSG_SENDMESSAGE, hContact, 0);
		break;

	case PCA_OPENMENU: // open contact menu
		if (hContact && hContact != INVALID_CONTACT_ID)
			QueueUserAPC(ShowContactMenu, g_hMainThread, (ULONG_PTR)hContact);
		break;

	case PCA_OPENDETAILS: // open contact details window
		if (hContact != INVALID_CONTACT_ID)
			CallServiceSync(MS_USERINFO_SHOWDIALOG, hContact, 0);
		break;

	case PCA_OPENHISTORY: // open contact history
		if (hContact != INVALID_CONTACT_ID)
			CallServiceSync(MS_HISTORY_SHOWCONTACTHISTORY, hContact, 0);
		break;

	case PCA_CLOSEPOPUP: // close popup
		PUDeletePopup(hWnd);
		break;

	case PCA_DONOTHING: // do nothing
		break;
	}
}


static LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_DATA *pdata = (PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, 0);
	if (pdata) {
		switch (message) {
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED) { // left mouse button
				Popup_DoAction(hWnd, pdata->PopupLClickAction, pdata);
				return true;
			}
			break;

		case WM_CONTEXTMENU:	// right mouse button
			Popup_DoAction(hWnd, pdata->PopupRClickAction, pdata);
			return true;

		case UM_FREEPLUGINDATA:
			if (pdata->hIcon)
				DestroyIcon(pdata->hIcon);

			free(pdata);
			return false;
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void ShowPopup(SHOWPOPUP_DATA *sd)
{
	TCString PopupText;
	if (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_SHOWPREVCLIENT)) {
		mir_sntprintf(PopupText.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("changed client to %s (was %s)"), (const TCHAR*)sd->MirVer, (const TCHAR*)sd->OldMirVer);
		PopupText.ReleaseBuffer();
	}
	else {
		mir_sntprintf(PopupText.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("changed client to %s"), (const TCHAR*)sd->MirVer);
		PopupText.ReleaseBuffer();
	}

	PLUGIN_DATA *pdata = (PLUGIN_DATA*)calloc(1, sizeof(PLUGIN_DATA));
	POPUPDATAT ppd = {0};
	ppd.lchContact = sd->hContact;
	char *szProto = GetContactProto(sd->hContact);
	pdata->hIcon = ppd.lchIcon = Finger_GetClientIcon(sd->MirVer, false);
	_ASSERT(ppd.lchIcon);
	if (!ppd.lchIcon || (DWORD)ppd.lchIcon == CALLSERVICE_NOTFOUND) {
		// if we didn't succeed retrieving client icon, show the usual status icon instead
		ppd.lchIcon = LoadSkinnedProtoIcon(szProto, db_get_w(sd->hContact, szProto, "Status", ID_STATUS_OFFLINE));
		pdata->hIcon = NULL;
	}
	_tcsncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)sd->hContact, GCDNF_TCHAR), lengthof(ppd.lptzContactName) - 1);
	_tcsncpy(ppd.lptzText, PopupText, lengthof(ppd.lptzText) - 1);
	ppd.colorBack = (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_DEFBGCOLOUR) ? 0 : sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_BGCOLOUR));
	ppd.colorText = (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_DEFTEXTCOLOUR) ? 0 : sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_TEXTCOLOUR));
	ppd.PluginWindowProc = PopupWndProc;
	pdata->PopupLClickAction = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_LCLICK_ACTION);
	pdata->PopupRClickAction = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_RCLICK_ACTION);
	ppd.iSeconds = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_POPUPDELAY);
	ppd.PluginData = pdata;
	PUAddPopupT(&ppd);
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (lstrcmpA(cws->szSetting, DB_MIRVER))
		return 0;

	SHOWPOPUP_DATA sd = {0};
	char *szProto = GetContactProto(hContact);
	if (g_PreviewOptPage)
		sd.MirVer = _T("Miranda NG ICQ 0.93.5.3007");
	else {
		if (!hContact) // exit if hContact == NULL and it's not a popup preview
			return 0;

		_ASSERT(szProto);
		if (!strcmp(szProto, META_PROTO)) // workaround for metacontacts
			return 0;

		sd.MirVer = db_get_s(hContact, szProto, DB_MIRVER, _T(""));
		if (sd.MirVer.IsEmpty())
			return 0;
	}
	sd.OldMirVer = db_get_s(hContact, MOD_NAME, DB_OLDMIRVER, _T(""));
	db_set_ts(hContact, MOD_NAME, DB_OLDMIRVER, sd.MirVer); // we have to write it here, because we modify sd.OldMirVer and sd.MirVer to conform our settings later
	if (sd.OldMirVer.IsEmpty())  // looks like it's the right way to do
		return 0;

	COptPage PopupOptPage;
	if (g_PreviewOptPage)
		PopupOptPage = *g_PreviewOptPage;
	else {
		PopupOptPage = g_PopupOptPage;
		PopupOptPage.DBToMem();
	}

	MCONTACT hContactOrMeta = (hContact) ? db_mc_getMeta(hContact) : 0;
	if (!hContactOrMeta)
		hContactOrMeta = hContact;

	if (hContact && db_get_b(hContactOrMeta, "CList", "Hidden", 0))
		return 0;

	int PerContactSetting = hContact ? db_get_b(hContact, MOD_NAME, DB_CCN_NOTIFY, NOTIFY_USEGLOBAL) : NOTIFY_ALWAYS; // NOTIFY_ALWAYS for preview
	if (PerContactSetting == NOTIFY_USEGLOBAL && hContactOrMeta != hContact) // subcontact setting has a priority over a metacontact setting
		PerContactSetting = db_get_b(hContactOrMeta, MOD_NAME, DB_CCN_NOTIFY, NOTIFY_USEGLOBAL);

	if (PerContactSetting && (PerContactSetting == NOTIFY_ALMOST_ALWAYS || PerContactSetting == NOTIFY_ALWAYS || !PopupOptPage.GetValue(IDC_POPUPOPTDLG_USESTATUSNOTIFYFLAG) || !(db_get_dw(hContactOrMeta, "Ignore", "Mask1", 0) & 0x8))) { // check if we need to notify at all
		sd.hContact = hContact;
		sd.PopupOptPage = &PopupOptPage;
		if (!PopupOptPage.GetValue(IDC_POPUPOPTDLG_VERCHGNOTIFY) || !PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWVER)) {
			if (bFingerprintExists) {
				LPCTSTR ptszOldClient = Finger_GetClientDescr(sd.OldMirVer); 
				LPCTSTR ptszClient = Finger_GetClientDescr(sd.MirVer);
				if (ptszOldClient && ptszClient) {
					if (PerContactSetting != NOTIFY_ALMOST_ALWAYS && PerContactSetting != NOTIFY_ALWAYS && !PopupOptPage.GetValue(IDC_POPUPOPTDLG_VERCHGNOTIFY) && !_tcscmp(ptszClient, ptszOldClient))
						return 0;

					if (!PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWVER)) {
						sd.MirVer = ptszClient;
						sd.OldMirVer = ptszOldClient;
					}
				}
			}
		}
		if (sd.MirVer == (const TCHAR*)sd.OldMirVer) {
			_ASSERT(hContact);
			return 0;
		}
		if (PerContactSetting == NOTIFY_ALWAYS || (PopupOptPage.GetValue(IDC_POPUPOPTDLG_POPUPNOTIFY) && (g_PreviewOptPage || PerContactSetting == NOTIFY_ALMOST_ALWAYS || -1 == PcreCheck(sd.MirVer)))) {
			ShowPopup(&sd);
			SkinPlaySound(CLIENTCHANGED_SOUND);
		}
	}

	if (hContact) {
		TCString ClientName;
		if (PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWPREVCLIENT) && sd.OldMirVer.GetLen()) {
			mir_sntprintf(ClientName.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("%s (was %s)"), (const TCHAR*)sd.MirVer, (const TCHAR*)sd.OldMirVer);
			ClientName.ReleaseBuffer();
		}
		else ClientName = sd.MirVer;
	}
	_ASSERT(sd.MirVer.GetLen()); // save the last known MirVer value even if the new one is empty
	return 0;
}

static INT_PTR srvTogglePopups(WPARAM wParam, LPARAM lParam)
{
	g_PopupOptPage.SetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY, !g_PopupOptPage.GetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY));
	return 0;
}

static int PrebuildMainMenu(WPARAM wParam, LPARAM lParam)
{
	// we have to use ME_CLIST_PREBUILDMAINMENU instead of updating menu items only on settings change, because "popup_enabled" and "popup_disabled" icons are not always available yet in ModulesLoaded
	if (bPopupExists) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_TCHAR | CMIM_NAME | CMIM_ICON;
		if (g_PopupOptPage.GetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY)) {
			mi.ptszName = LPGENT("Disable c&lient change notification");
			mi.hIcon = Skin_GetIcon("popup_enabled");
		}
		else {
			mi.ptszName = LPGENT("Enable c&lient change notification");
			mi.hIcon = Skin_GetIcon("popup_disabled");
		}
		mi.ptszPopupName = LPGENT("Popups");
		Menu_ModifyItem(g_hTogglePopupsMenuItem, &mi);
	}
	return 0;
}

INT_PTR CALLBACK CCNErrorDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		return true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		if (IsDlgButtonChecked(hwndDlg, IDC_DONTREMIND))
			db_set_b(NULL, MOD_NAME, DB_NO_FINGERPRINT_ERROR, 1);
		break;
	}
	return 0;
}

static int ModuleLoad(WPARAM wParam, LPARAM lParam)
{
	bPopupExists = ServiceExists(MS_POPUP_ADDPOPUPT);
	bFingerprintExists = ServiceExists(MS_FP_SAMECLIENTST) && ServiceExists(MS_FP_GETCLIENTICONT);
	bVariablesExists = ServiceExists(MS_VARS_FORMATSTRING);
	return 0;
}

int MirandaLoaded(WPARAM wParam, LPARAM lParam)
{
	InitPcre();
	ModuleLoad(0, 0);
	COptPage PopupOptPage(g_PopupOptPage);
	PopupOptPage.DBToMem();
	RecompileRegexps(*(TCString*)PopupOptPage.GetValue(IDC_POPUPOPTDLG_IGNORESTRINGS));
	HookEvent(ME_OPT_INITIALISE, OptionsDlgInit);
	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	SkinAddNewSoundEx(CLIENTCHANGED_SOUND, NULL, LPGEN("ClientChangeNotify: Client changed"));

	if (bPopupExists) {
		CreateServiceFunction(MS_CCN_TOGGLEPOPUPS, srvTogglePopups);
		HookEvent(ME_CLIST_PREBUILDMAINMENU, PrebuildMainMenu);
	
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_TCHAR;
		if (g_PopupOptPage.GetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY))
			mi.ptszName = LPGENT("Disable c&lient change notification");
		else
			mi.ptszName = LPGENT("Enable c&lient change notification");

		mi.pszService = MS_CCN_TOGGLEPOPUPS;
		mi.ptszPopupName = LPGENT("Popups");
		g_hTogglePopupsMenuItem = Menu_AddMainMenuItem(&mi);
	}

	// seems that Fingerprint is not installed
	if (!bFingerprintExists && !db_get_b(NULL, MOD_NAME, DB_NO_FINGERPRINT_ERROR, 0))
		CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_CCN_ERROR), NULL, CCNErrorDlgProc);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP( &pluginInfo );

	HookEvent(ME_SYSTEM_MODULESLOADED, MirandaLoaded);
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &g_hMainThread, THREAD_SET_CONTEXT, false, 0);
	InitOptions();

	if (db_get_b(NULL, MOD_NAME, DB_SETTINGSVER, 0) < 1) {
		TCString Str;
		Str = db_get_s(NULL, MOD_NAME, DB_IGNORESUBSTRINGS, _T(""));
		if (Str.GetLen()) // fix incorrect regexp from v0.1.1.0
			db_set_ts(NULL, MOD_NAME, DB_IGNORESUBSTRINGS, Str.Replace(_T("/Miranda[0-9A-F]{8}/"), _T("/[0-9A-F]{8}(\\W|$)/")));

		db_set_b(NULL, MOD_NAME, DB_SETTINGSVER, 1);
	}
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	CloseHandle(g_hMainThread);
	UninitPcre();
	return 0;
}
