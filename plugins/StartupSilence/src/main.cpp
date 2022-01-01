/*
Copyright (c) 2012-13 Vladimir Lyubimov
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

//HELPME: need new icons for TTB bottons

#include "stdafx.h"

CMPlugin g_plugin;

HANDLE hTTBarloaded = nullptr;
HANDLE Buttons = nullptr;

uint8_t Enabled;
uint32_t delay;
uint8_t PopUp;
uint32_t PopUpTime;
uint8_t TTBButtons;
uint8_t DefSound;
uint8_t DefPopup;
uint8_t DefEnabled;
uint8_t NonStatusAllow;
uint8_t timer;
char hostname[MAX_PATH] = "";
char EnabledComp[MAX_PATH] = "";
char DelayComp[MAX_PATH] = "";
char PopUpComp[MAX_PATH] = "";
char PopUpTimeComp[MAX_PATH] = "";
char MenuitemComp[MAX_PATH] = "";
char TTBButtonsComp[MAX_PATH] = "";
char DefSoundComp[MAX_PATH] = "";
char DefPopupComp[MAX_PATH] = "";
char DefEnabledComp[MAX_PATH] = "";
char NonStatusAllowComp[MAX_PATH] = "";

static LIST<void> ttbButtons(1);

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {7B856B6A-D48F-4f54-B8D6-C8D86D02FFC2}
	{ 0x7b856b6a, 0xd48f, 0x4f54, { 0xb8, 0xd6, 0xc8, 0xd8, 0x6d, 0x2, 0xff, 0xc2 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static void __cdecl AdvSt(void*)
{
	Thread_SetName("StartupSilenc: AdvSt");

	if ((Enabled == 1)) {
		POPUPDATAW ppd;
		wchar_t *lpwzText = L"";
		db_set_b(0, "Skin", "UseSound", 0);
		Popup_Enable(true);

		if (PopUp == 1) {
			lpwzText = NonStatusAllow == 1 ? ALL_DISABLED_FLT : ALL_DISABLED;
			ppd.lchIcon = IcoLib_GetIconByHandle((NonStatusAllow == 1) ? g_plugin.getIconHandle(IDI_SSFILTEREDON) : g_plugin.getIconHandle(IDI_SSICON));
			ppd.lchContact = NULL;
			ppd.iSeconds = PopUpTime;
			wcsncpy_s(ppd.lpwzText, lpwzText, _TRUNCATE);
			lpwzText = TranslateW(MENU_NAMEW);
			wcsncpy_s(ppd.lpwzContactName, lpwzText, _TRUNCATE);
			PUAddPopupW(&ppd);
		}

		timer = 2;
		Sleep(delay * 1000);
		timer = 0;

		if (PopUp == 1) {
			lpwzText = (DefEnabled == 1 && DefPopup == 1) ? TranslateT(ALL_ENABLED_FLT) : ALL_ENABLED;
			ppd.lchIcon = IcoLib_GetIconByHandle((DefEnabled == 1 && DefPopup == 1) ? g_plugin.getIconHandle(IDI_SSFILTEREDON) : g_plugin.getIconHandle(IDI_SSICON));
			wcsncpy_s(ppd.lpwzText, lpwzText, _TRUNCATE);
			PUAddPopupW(&ppd);
		}
		if (DefEnabled == 1) { //predefined sound setting
			db_set_b(0, "Skin", "UseSound", DefSound);
		}
		else db_set_b(0, "Skin", "UseSound", 1); //or enable sounds every starts
	}
}

int DisablePopup(WPARAM wParam, LPARAM)
{
	if (DefEnabled == 1 && DefPopup == 0)      //All popups are blocked
		return 1;

	if ((NonStatusAllow == 1) // while startup allow popups for unread mail notification from MRA, keepstatus ... other services?
		|| ((DefPopup == 1 && DefEnabled == 1) && timer != 2)) //also filtered only: We do not run next lines every time
																			  //if "Filtered only..." is unchecked --->
	{
		MCONTACT hContact = wParam;
		if (hContact != NULL) {
			char* cp = Proto_GetBaseAccountName(hContact);
			if (!mir_strcmp(cp, "Weather") || !mir_strcmp(cp, "mRadio"))
				return 0;
			return 1;
		}
		else return 0;//or allow popups for unread mail notification from MRA, keepstatus ... other services?
	}
	else if (timer == 2)
		return 1;	//block all popups at startup
	return 0;	//---> just allow all popups with this return
}

void InitSettings()
{
	if (gethostname(hostname, _countof(hostname)) == 0) {
		mir_snprintf(EnabledComp, "%s_Enabled", hostname);
		mir_snprintf(DelayComp, "%s_Delay", hostname);
		mir_snprintf(PopUpComp, "%s_PopUp", hostname);
		mir_snprintf(PopUpTimeComp, "%s_PopUpTime", hostname);
		mir_snprintf(MenuitemComp, "%s_MenuItem", hostname);
		mir_snprintf(TTBButtonsComp, "%s_TTBButtons", hostname);
		mir_snprintf(DefSoundComp, "%s_DefSound", hostname);
		mir_snprintf(DefPopupComp, "%s_DefPopup", hostname);
		mir_snprintf(DefEnabledComp, "%s_DefEnabled", hostname);
		mir_snprintf(NonStatusAllowComp, "%s_NonStatusAllow", hostname);
	}
	//first run on the host, initial setting
	if (!(delay = g_plugin.getDword(DelayComp, 0)))
		DefSettings();
	//or load host settings
	else LoadSettings();
}

void DefSettings()
{
	g_plugin.setDword(DelayComp, 20);
	g_plugin.setByte(EnabledComp, 1);
	g_plugin.setByte(PopUpComp, 1);
	g_plugin.setDword(PopUpTimeComp, 5);
	g_plugin.setByte(MenuitemComp, 1);
	g_plugin.setByte(TTBButtonsComp, 0);
	g_plugin.setByte(DefSoundComp, 1);
	g_plugin.setByte(DefPopupComp, 0);
	g_plugin.setByte(DefEnabledComp, 0);
	g_plugin.setByte(NonStatusAllowComp, 1);
	LoadSettings();
}

void LoadSettings()
{
	Enabled = g_plugin.getByte(EnabledComp);
	delay = g_plugin.getDword(DelayComp);
	PopUp = g_plugin.getByte(PopUpComp);
	PopUpTime = g_plugin.getDword(PopUpTimeComp);
	TTBButtons = g_plugin.getByte(TTBButtonsComp);
	DefSound = g_plugin.getByte(DefSoundComp);
	DefPopup = g_plugin.getByte(DefPopupComp);
	DefEnabled = g_plugin.getByte(DefEnabledComp);
	NonStatusAllow = g_plugin.getByte(NonStatusAllowComp);
	if (PopUpTime < 1)
		PopUpTime = (uint32_t)1;
	if (PopUpTime > 30)
		PopUpTime = (uint32_t)30;
	if (delay < 10)
		delay = (uint32_t)10;
	if (delay > 300)
		delay = (uint32_t)300;
	g_plugin.setDword(DelayComp, delay);
	g_plugin.setDword(PopUpTimeComp, PopUpTime);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR StartupSilenceEnabled(WPARAM, LPARAM)
{
	g_plugin.setByte(EnabledComp, !Enabled);
	LoadSettings();
	UpdateMenu();
	if (PopUp == 1) {
		wchar_t * lpwzText = Enabled == 1 ? S_MODE_CHANGEDON : S_MODE_CHANGEDOFF;
		POPUPDATAW ppd;
		ppd.lchIcon = IcoLib_GetIconByHandle((Enabled == 1) ? g_plugin.getIconHandle(IDI_SSENABLE) : g_plugin.getIconHandle(IDI_SSDISABLE));
		ppd.lchContact = NULL;
		ppd.iSeconds = PopUpTime;
		wcsncpy_s(ppd.lpwzText, lpwzText, _TRUNCATE);
		lpwzText = TranslateW(MENU_NAMEW);
		wcsncpy_s(ppd.lpwzContactName, lpwzText, _TRUNCATE);
		PUAddPopupW(&ppd);
	}
	return 0;
}

static INT_PTR SilenceConnection(WPARAM wParam, LPARAM)
{
	timer = (uint8_t)wParam;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

HGENMENU hSSMenuToggleOnOff;

static INT_PTR InitMenu()
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x9100c881, 0x6f76, 0x4cb5, 0x97, 0x66, 0xeb, 0xf5, 0xc5, 0x22, 0x46, 0x1f);
	mi.position = 100000000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SSICON);
	mi.name.a = MENU_NAME;
	mi.pszService = SS_SERVICE_NAME;
	hSSMenuToggleOnOff = Menu_AddMainMenuItem(&mi);
	UpdateMenu();
	return 0;
}

void UpdateMenu()
{
	if (Enabled == 1)
		Menu_ModifyItem(hSSMenuToggleOnOff, _A2W(DISABLE_SILENCE), g_plugin.getIconHandle(IDI_SSDISABLE));
	else
		Menu_ModifyItem(hSSMenuToggleOnOff, _A2W(ENABLE_SILENCE), g_plugin.getIconHandle(IDI_SSENABLE));

	UpdateTTB();
}

void UpdateTTB()
{
	if (hTTBarloaded != nullptr && TTBButtons == 1)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)Buttons, (Enabled == 1 ? 0 : TTBST_PUSHED));
}

static int CreateTTButtons(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = (Enabled == 1 ? 0 : TTBBF_PUSHED) | TTBBF_VISIBLE | TTBBF_ASPUSHBUTTON;
	ttb.pszService = SS_SERVICE_NAME;
	ttb.hIconHandleDn = g_plugin.getIconHandle(IDI_SSDISABLETTB);
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_SSENABLETTB);
	ttb.name = TTBNAME;
	ttb.pszTooltipUp = SS_IS_ON;
	ttb.pszTooltipDn = SS_IS_OFF;
	Buttons = g_plugin.addTTB(&ttb);
	if (Buttons)
		ttbButtons.insert(Buttons);
	return 0;
}

void RemoveTTButtons()
{
	for (auto &it : ttbButtons.rev_iter())
		CallService(MS_TTB_REMOVEBUTTON, (WPARAM)it, 0);
	ttbButtons.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options

static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		LoadSettings();
		SetDlgItemText(hwndDlg, IDC_HST, mir_a2u(hostname));
		CheckDlgButton(hwndDlg, IDC_DELAY, (Enabled == 1) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_SSSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SSTIME), 0);
		SendDlgItemMessage(hwndDlg, IDC_SSSPIN, UDM_SETRANGE32, 10, 300);
		SendDlgItemMessage(hwndDlg, IDC_SSSPIN, UDM_SETPOS, 0, MAKELONG((delay), 0));
		SendDlgItemMessage(hwndDlg, IDC_SSTIME, EM_LIMITTEXT, (WPARAM)3, 0);

		CheckDlgButton(hwndDlg, IDC_DELAY2, (PopUp == 1) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_SSSPIN2, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SSPOPUPTIME), 0);
		SendDlgItemMessage(hwndDlg, IDC_SSSPIN2, UDM_SETRANGE32, 1, 30);
		SendDlgItemMessage(hwndDlg, IDC_SSSPIN2, UDM_SETPOS, 0, MAKELONG((PopUpTime), 0));
		SendDlgItemMessage(hwndDlg, IDC_SSPOPUPTIME, EM_LIMITTEXT, (WPARAM)3, 0);

		CheckDlgButton(hwndDlg, IDC_TTB, (TTBButtons == 1) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_DEFPOPUP, (DefPopup == 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DEFSOUNDS, (DefSound == 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RESTORE, (DefEnabled == 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NONSTATUSES, (NonStatusAllow == 1) ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SSTIME:
			uint32_t min;
			if ((HWND)lParam != GetFocus() || HIWORD(wParam) != EN_CHANGE) return FALSE;
			min = GetDlgItemInt(hwndDlg, IDC_SSTIME, nullptr, FALSE);
			if (min == 0 && GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SSTIME)))
				SendDlgItemMessage(hwndDlg, IDC_SSSPIN, UDM_SETPOS, 0, MAKELONG((short)1, 0));
			delay = db_set_dw(0, MODULENAME, DelayComp, (uint32_t)(SendDlgItemMessage(hwndDlg, IDC_SSSPIN, UDM_GETPOS, 0, 0)));
			break;

		case IDC_SSPOPUPTIME:
			if ((HWND)lParam != GetFocus() || HIWORD(wParam) != EN_CHANGE) return FALSE;
			min = GetDlgItemInt(hwndDlg, IDC_SSPOPUPTIME, nullptr, FALSE);
			if (min == 0 && GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SSPOPUPTIME)))
				SendDlgItemMessage(hwndDlg, IDC_SSSPIN2, UDM_SETPOS, 0, MAKELONG((short)1, 0));
			PopUpTime = db_set_dw(0, MODULENAME, PopUpTimeComp, (uint32_t)(SendDlgItemMessage(hwndDlg, IDC_SSSPIN2, UDM_GETPOS, 0, 0)));
			break;

		case IDC_DELAY:
			CallService(SS_SERVICE_NAME, 0, 0);
			break;

		case IDC_DELAY2:
			PopUp = db_set_b(0, MODULENAME, PopUpComp, IsDlgButtonChecked(hwndDlg, IDC_DELAY2) == BST_CHECKED);
			break;

		case IDC_TTB:
			if (!hTTBarloaded) {
				MessageBox(nullptr, NEEDTTBMOD, NOTICE, MB_OK);
				CheckDlgButton(hwndDlg, IDC_TTB, BST_UNCHECKED);
				TTBButtons = db_set_b(0, MODULENAME, TTBButtonsComp, 0);
			}
			else TTBButtons = db_set_b(0, MODULENAME, TTBButtonsComp, IsDlgButtonChecked(hwndDlg, IDC_TTB) == BST_CHECKED);
			break;

		case IDC_DEFPOPUP:
			g_plugin.setByte(DefPopupComp, IsDlgButtonChecked(hwndDlg, IDC_DEFPOPUP) == BST_CHECKED);
			DefPopup = g_plugin.getByte(DefPopupComp, 0);
			break;

		case IDC_DEFSOUNDS:
			g_plugin.setByte(DefSoundComp, IsDlgButtonChecked(hwndDlg, IDC_DEFSOUNDS) == BST_CHECKED);
			DefSound = g_plugin.getByte(DefSoundComp, 0);
			break;

		case IDC_RESTORE:
			g_plugin.setByte(DefEnabledComp, IsDlgButtonChecked(hwndDlg, IDC_RESTORE) == BST_CHECKED);
			DefEnabled = g_plugin.getByte(DefEnabledComp, 0);
			break;

		case IDC_NONSTATUSES:
			g_plugin.setByte(NonStatusAllowComp, IsDlgButtonChecked(hwndDlg, IDC_NONSTATUSES) == BST_CHECKED);
			NonStatusAllow = g_plugin.getByte(NonStatusAllowComp, 0);
			break;

		case IDC_RESETDEFAULT:
			DefSettings();
			CheckDlgButton(hwndDlg, IDC_DELAY, (Enabled == 1) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_SSSPIN, UDM_SETPOS, 0, MAKELONG((delay), 0));
			CheckDlgButton(hwndDlg, IDC_DELAY2, (PopUp == 1) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_SSSPIN2, UDM_SETPOS, 0, MAKELONG((PopUpTime), 0));
			CheckDlgButton(hwndDlg, IDC_TTB, (TTBButtons == 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DEFPOPUP, (DefPopup == 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DEFSOUNDS, (DefSound == 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RESTORE, (DefEnabled == 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NONSTATUSES, (NonStatusAllow == 1) ? BST_CHECKED : BST_UNCHECKED);
			break;
		}
		break;
	}
	return FALSE;
}

int InitializeOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SSOPT);
	odp.szGroup.a = LPGEN("Events");//FIXME: move to...Group?
	odp.szTitle.a = MENU_NAME;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOptions;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int ModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_POPUP_FILTER, DisablePopup);
	hTTBarloaded = HookEvent(ME_TTB_MODULELOADED, CreateTTButtons);
	if (TTBButtons == 1 && hTTBarloaded != nullptr) {
		g_plugin.registerIcon("Toolbar/" MENU_NAME, iconttbList, MENU_NAME);
		RemoveTTButtons();
		CreateTTButtons(0, 0);
	}
	return 0;
}

int CMPlugin::Load()
{
	InitSettings();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, InitializeOptions);

	mir_forkthread((pThreadFunc)AdvSt);

	CreateServiceFunction(SS_SERVICE_NAME, StartupSilenceEnabled);
	CreateServiceFunction(SS_SILENCE_CONNECTION, SilenceConnection);

	g_plugin.registerIcon(MENU_NAME, iconList, MENU_NAME);
	InitMenu();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if (hTTBarloaded != nullptr)
		UnhookEvent(hTTBarloaded);

	return 0;
}
