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

#include "stdafx.h"

CMPlugin g_plugin;

COptPage *g_PreviewOptPage; // we need to show popup even for the NULL contact if g_PreviewOptPage is not NULL (used for popup preview)
BOOL bFingerprintExists = FALSE, bVariablesExists = FALSE;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {B68A8906-748B-435D-930E-21CC6E8F3B3F}
	{0xb68a8906, 0x748b, 0x435d, {0x93, 0xe, 0x21, 0xcc, 0x6e, 0x8f, 0x3b, 0x3f}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bPopups(MODULENAME, "PopupNotify", true)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CALLBACK MenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		return Menu_DrawItem(lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static VOID CALLBACK ShowContactMenu(void *param)
{
	MCONTACT hContact = (ULONG_PTR)param;

	POINT pt;
	HWND hMenuWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"static", _A2W(MODULENAME) L"_MenuWindow", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, nullptr, g_plugin.getInst(), nullptr);
	SetWindowLongPtr(hMenuWnd, GWLP_WNDPROC, (LONG_PTR)MenuWndProc);
	HMENU hMenu = Menu_BuildContactMenu(hContact);
	GetCursorPos(&pt);
	SetForegroundWindow(hMenuWnd);
	Clist_MenuProcessCommand(TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hMenuWnd, nullptr), MPCF_CONTACTMENU, hContact);
	PostMessage(hMenuWnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
	DestroyWindow(hMenuWnd);
}

void Popup_DoAction(HWND hWnd, uint8_t Action, PLUGIN_DATA*)
{
	MCONTACT hContact = PUGetContact(hWnd);
	switch (Action) {
	case PCA_OPENMESSAGEWND: // open message window
		if (hContact && hContact != INVALID_CONTACT_ID)
			CallServiceSync(MS_MSG_SENDMESSAGE, hContact, 0);
		break;

	case PCA_OPENMENU: // open contact menu
		if (hContact && hContact != INVALID_CONTACT_ID)
			CallFunctionAsync(ShowContactMenu, (void*)hContact);
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
	PLUGIN_DATA *pdata = (PLUGIN_DATA*)PUGetPluginData(hWnd);
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
		mir_snwprintf(PopupText.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("changed client to %s (was %s)"), (const wchar_t*)sd->MirVer, (const wchar_t*)sd->OldMirVer);
		PopupText.ReleaseBuffer();
	}
	else {
		mir_snwprintf(PopupText.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("changed client to %s"), (const wchar_t*)sd->MirVer);
		PopupText.ReleaseBuffer();
	}

	PLUGIN_DATA *pdata = (PLUGIN_DATA*)calloc(1, sizeof(PLUGIN_DATA));
	POPUPDATAW ppd;
	ppd.lchContact = sd->hContact;
	char *szProto = Proto_GetBaseAccountName(sd->hContact);
	pdata->hIcon = ppd.lchIcon = Finger_GetClientIcon(sd->MirVer, false);
	_ASSERT(ppd.lchIcon);
	if (!ppd.lchIcon || (INT_PTR)ppd.lchIcon == CALLSERVICE_NOTFOUND) {
		// if we didn't succeed retrieving client icon, show the usual status icon instead
		ppd.lchIcon = Skin_LoadProtoIcon(szProto, db_get_w(sd->hContact, szProto, "Status", ID_STATUS_OFFLINE));
		pdata->hIcon = nullptr;
	}
	wcsncpy(ppd.lpwzContactName, Clist_GetContactDisplayName(sd->hContact), _countof(ppd.lpwzContactName) - 1);
	wcsncpy(ppd.lpwzText, PopupText, _countof(ppd.lpwzText) - 1);
	ppd.colorBack = (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_DEFBGCOLOUR) ? 0 : sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_BGCOLOUR));
	ppd.colorText = (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_DEFTEXTCOLOUR) ? 0 : sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_TEXTCOLOUR));
	ppd.PluginWindowProc = PopupWndProc;
	pdata->PopupLClickAction = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_LCLICK_ACTION);
	pdata->PopupRClickAction = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_RCLICK_ACTION);
	ppd.iSeconds = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_POPUPDELAY);
	ppd.PluginData = pdata;
	PUAddPopupW(&ppd);
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (strcmp(cws->szSetting, DB_MIRVER))
		return 0;

	SHOWPOPUP_DATA sd = { 0 };
	if (g_PreviewOptPage) {
		char szVersion[200];
		Miranda_GetVersionText(szVersion, _countof(szVersion));
		sd.MirVer = _A2T(szVersion);
	}
	else {
		if (!hContact) // exit if hContact == NULL and it's not a popup preview
			return 0;

		char *szProto = Proto_GetBaseAccountName(hContact);
		if (!mir_strcmp(szProto, META_PROTO)) // workaround for metacontacts
			return 0;

		sd.MirVer = db_get_s(hContact, szProto, DB_MIRVER, L"");
		if (sd.MirVer.IsEmpty())
			return 0;
	}
	sd.OldMirVer = db_get_s(hContact, MODULENAME, DB_OLDMIRVER, L"");
	g_plugin.setWString(hContact, DB_OLDMIRVER, sd.MirVer); // we have to write it here, because we modify sd.OldMirVer and sd.MirVer to conform our settings later
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

	if (hContact && Contact::IsHidden(hContactOrMeta))
		return 0;

	int PerContactSetting = hContact ? g_plugin.getByte(hContact, DB_CCN_NOTIFY, NOTIFY_USEGLOBAL) : NOTIFY_ALWAYS; // NOTIFY_ALWAYS for preview
	if (PerContactSetting == NOTIFY_USEGLOBAL && hContactOrMeta != hContact) // subcontact setting has a priority over a metacontact setting
		PerContactSetting = g_plugin.getByte(hContactOrMeta, DB_CCN_NOTIFY, NOTIFY_USEGLOBAL);

	if (PerContactSetting && (PerContactSetting == NOTIFY_ALMOST_ALWAYS || PerContactSetting == NOTIFY_ALWAYS || !PopupOptPage.GetValue(IDC_POPUPOPTDLG_USESTATUSNOTIFYFLAG) || !(db_get_dw(hContactOrMeta, "Ignore", "Mask1", 0) & 0x8))) { // check if we need to notify at all
		sd.hContact = hContact;
		sd.PopupOptPage = &PopupOptPage;
		if (!PopupOptPage.GetValue(IDC_POPUPOPTDLG_VERCHGNOTIFY) || !PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWVER)) {
			if (bFingerprintExists) {
				LPCTSTR ptszOldClient = Finger_GetClientDescr(sd.OldMirVer);
				LPCTSTR ptszClient = Finger_GetClientDescr(sd.MirVer);
				if (ptszOldClient && ptszClient) {
					if (PerContactSetting != NOTIFY_ALMOST_ALWAYS && PerContactSetting != NOTIFY_ALWAYS && !PopupOptPage.GetValue(IDC_POPUPOPTDLG_VERCHGNOTIFY) && !wcscmp(ptszClient, ptszOldClient))
						return 0;

					if (!PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWVER)) {
						sd.MirVer = ptszClient;
						sd.OldMirVer = ptszOldClient;
					}
				}
			}
		}
		if (sd.MirVer == (const wchar_t*)sd.OldMirVer) {
			_ASSERT(hContact);
			return 0;
		}
		if (PerContactSetting == NOTIFY_ALWAYS || (g_plugin.bPopups && (g_PreviewOptPage || PerContactSetting == NOTIFY_ALMOST_ALWAYS || -1 == PcreCheck(sd.MirVer)))) {
			ShowPopup(&sd);
			Skin_PlaySound(CLIENTCHANGED_SOUND);
		}
	}

	if (hContact) {
		TCString ClientName;
		if (PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWPREVCLIENT) && sd.OldMirVer.GetLen()) {
			mir_snwprintf(ClientName.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("%s (was %s)"), (const wchar_t*)sd.MirVer, (const wchar_t*)sd.OldMirVer);
			ClientName.ReleaseBuffer();
		}
		else ClientName = sd.MirVer;
	}
	_ASSERT(sd.MirVer.GetLen()); // save the last known MirVer value even if the new one is empty
	return 0;
}

INT_PTR CALLBACK CCNErrorDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
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
			g_plugin.setByte(DB_NO_FINGERPRINT_ERROR, 1);
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ModuleLoad(WPARAM, LPARAM)
{
	bFingerprintExists = ServiceExists(MS_FP_SAMECLIENTST) && ServiceExists(MS_FP_GETCLIENTICONT);
	bVariablesExists = ServiceExists(MS_VARS_FORMATSTRING);
	return 0;
}

static int MirandaLoaded(WPARAM, LPARAM)
{
	ModuleLoad(0, 0);
	COptPage PopupOptPage(g_PopupOptPage);
	PopupOptPage.DBToMem();
	RecompileRegexps(*(TCString*)PopupOptPage.GetValue(IDC_POPUPOPTDLG_IGNORESTRINGS));

	// seems that Fingerprint is not installed
	if (!bFingerprintExists && !g_plugin.getByte(DB_NO_FINGERPRINT_ERROR, 0))
		CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CCN_ERROR), nullptr, CCNErrorDlgProc);

	return 0;
}

int CMPlugin::Load()
{
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OptionsDlgInit);
	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULESLOADED, MirandaLoaded);

	addPopupOption(LPGEN("Client change notifications"), bPopups);

	addSound(CLIENTCHANGED_SOUND, nullptr, LPGENW("ClientChangeNotify: Client changed"));

	InitOptions();

	if (g_plugin.getByte(DB_SETTINGSVER, 0) < 1) {
		TCString Str = db_get_s(0, MODULENAME, DB_IGNORESUBSTRINGS, L"");
		if (Str.GetLen()) // fix incorrect regexp from v0.1.1.0
			g_plugin.setWString(DB_IGNORESUBSTRINGS, Str.Replace(L"/Miranda[0-9A-F]{8}/", L"/[0-9A-F]{8}(\\W|$)/"));

		g_plugin.setByte(DB_SETTINGSVER, 1);
	}
	return 0;
}
