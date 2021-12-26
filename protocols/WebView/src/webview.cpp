/*
* A plugin for Miranda IM which displays web page text in a window Copyright 
* (C) 2005 Vincent Joyce.
* 
* Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
* Richard Hughes, Roland Rabien & Tristan Van de Vreede
* 
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free 
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
* 
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
* 
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc., 59 
* Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
*/

#include "stdafx.h"
#include "webview.h"

char* WndClass = "WEBWnd";
WNDCLASSEX      wincl;
MSG messages;
uint32_t  winheight;
int StartUpDelay = 0;

int       Xposition, Yposition;
int       WindowHeight, WindowWidth;
COLORREF  BackgoundClr, TextClr;

UINT_PTR  timerId;
UINT_PTR  Countdown;
LOGFONT   g_lf;
HFONT     h_font;
HMENU     hMenu;
int       bpStatus;
HGENMENU  hMenuItem1;
HGENMENU  hMenuItemCountdown;

int OptInitialise(WPARAM wParam, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////
void ChangeMenuItem1()
{
	// Enable or Disable auto updates
	LPCTSTR ptszName;
	if (!g_plugin.getByte(DISABLE_AUTOUPDATE_KEY, 0))
		ptszName = LPGENW("Auto update enabled");
	else
		ptszName = LPGENW("Auto update disabled");

	Menu_ModifyItem(hMenuItem1, ptszName, LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_SITE)));
}

/////////////////////////////////////////////////////////////////////////////////////////
void ChangeMenuItemCountdown()
{
	// countdown
	HICON hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_UPDATEALL));

	wchar_t countername[100];
	mir_snwprintf(countername, TranslateT("%d minutes to update"), g_plugin.getDword(COUNTDOWN_KEY, 0));

	Menu_ModifyItem(hMenuItemCountdown, countername, hIcon, CMIF_KEEPUNTRANSLATED);
}

/////////////////////////////////////////////////////////////////////////////////////////
static int CALLBACK EnumFontsProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX*, int, LPARAM lParam)
{
	if (!IsWindow((HWND)lParam))
		return FALSE;
	if (SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, -1, (LPARAM)lpelfe->elfLogFont.lfFaceName) == CB_ERR)
		SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)lpelfe->elfLogFont.lfFaceName);
	return TRUE;
}

void FillFontListThread(void *param)
{
	HDC hdc = GetDC((HWND)param);

	LOGFONT lf = { 0 };
	g_lf.lfCharSet = DEFAULT_CHARSET;
	g_lf.lfFaceName[0] = 0;
	g_lf.lfPitchAndFamily = 0;
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontsProc, (LPARAM)GetDlgItem((HWND)param, IDC_TYPEFACE), 0);
	ReleaseDC((HWND)param, hdc);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TxtclrLoop()
{
	for (auto &hContact : Contacts(MODULENAME)) {
		HWND hwndDlg = WindowList_Find(hWindowList, hContact);
		SetDlgItemText(hwndDlg, IDC_DATA, L"");
		InvalidateRect(hwndDlg, nullptr, 1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void BGclrLoop()
{
	for (auto &hContact : Contacts(MODULENAME)) {
		HWND hwndDlg = (WindowList_Find(hWindowList, hContact));
		SetDlgItemText(hwndDlg, IDC_DATA, L"");
		SendDlgItemMessage(hwndDlg, IDC_DATA, EM_SETBKGNDCOLOR, 0, BackgoundClr);
		InvalidateRect(hwndDlg, nullptr, 1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void StartUpdate(void*)
{
	StartUpDelay = 1;
	Sleep(((g_plugin.getDword(START_DELAY_KEY, 0)) * SECOND));

	for (auto &hContact : Contacts(MODULENAME))
		GetData((void*)hContact);

	StartUpDelay = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ContactLoop(void*)
{
	if (StartUpDelay == 0) {
		for (auto &hContact : Contacts(MODULENAME)) {
			GetData((void*)hContact);
			Sleep(10); // avoid 100% CPU
		}
	}

	WAlertPopup(NULL, TranslateT("All Webview sites have been updated."));
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR MarkAllReadMenuCommand(WPARAM, LPARAM)
{
	ChangeContactStatus(1);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
void InitialiseGlobals(void)
{
	Xposition = g_plugin.getDword(Xpos_WIN_KEY, 0);
	Yposition = g_plugin.getDword(Ypos_WIN_KEY, 0);

	if (Yposition == -32000)
		Yposition = 100;

	if (Xposition == -32000)
		Xposition = 100;

	BackgoundClr = g_plugin.getDword(BG_COLOR_KEY, Def_color_bg);
	TextClr = g_plugin.getDword(TXT_COLOR_KEY, Def_color_txt);

	WindowHeight = g_plugin.getDword(WIN_HEIGHT_KEY, Def_win_height);
	WindowWidth = g_plugin.getDword(WIN_WIDTH_KEY, Def_win_width);
}

/////////////////////////////////////////////////////////////////////////////////////////
int Doubleclick(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = wParam;
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(MODULENAME, szProto))
		return 0;

	int action = g_plugin.getByte(hContact, DBLE_WIN_KEY, 1);
	if (action == 0) {
		ptrW url(g_plugin.getWStringA(hContact, "URL"));
		Utils_OpenUrlW(url);

		g_plugin.setWord(hContact, "Status", ID_STATUS_ONLINE);
	}
	else if (action == 1) {
		HWND hwndDlg = WindowList_Find(hWindowList, hContact);
		if (hwndDlg) {
			SetForegroundWindow(hwndDlg);
			SetFocus(hwndDlg);
		}
		else {
			hwndDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DISPLAY_DATA), nullptr, DlgProcDisplayData, (LPARAM)hContact);
			HWND hTopmost = g_plugin.getByte(hContact, ON_TOP_KEY, 0) ? HWND_TOPMOST : HWND_NOTOPMOST;
			SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM)((HICON)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0)));

			if (g_plugin.getByte(SAVE_INDIVID_POS_KEY, 0))
				SetWindowPos(hwndDlg, hTopmost,
					g_plugin.getDword(hContact, "WVx", 100), // Xposition,
					g_plugin.getDword(hContact, "WVy", 100), // Yposition,
					g_plugin.getDword(hContact, "WVwidth", 412), // WindowWidth,
					g_plugin.getDword(hContact, "WVheight", 350), 0); // WindowHeight,
			else
				SetWindowPos(hwndDlg, HWND_TOPMOST, Xposition, Yposition, WindowWidth, WindowHeight, 0);
		}

		ShowWindow(hwndDlg, SW_SHOW);
		SetActiveWindow(hwndDlg);

		if (g_plugin.getByte(UPDATE_ON_OPEN_KEY, 0)) {
			if (g_plugin.getByte(hContact, ENABLE_ALERTS_KEY, 0))
				mir_forkthread(ReadFromFile, (void*)hContact);
			else
				mir_forkthread(GetData, (void*)hContact);
			g_plugin.setWord(hContact, "Status", ID_STATUS_ONLINE);
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
int SendToRichEdit(HWND hWindow, char *truncated, COLORREF rgbText, COLORREF rgbBack)
{
	DBVARIANT       dbv;
	uint32_t  bold = 0;
	uint32_t  italic = 0;
	uint32_t  underline = 0;

	SetDlgItemText(hWindow, IDC_DATA, L"");

	CHARFORMAT2 cfFM;
	memset(&cfFM, 0, sizeof(cfFM));
	cfFM.cbSize = sizeof(CHARFORMAT2);
	cfFM.dwMask = CFM_COLOR | CFM_CHARSET | CFM_FACE | ENM_LINK | ENM_MOUSEEVENTS | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_SIZE;

	if (g_plugin.getByte(FONT_BOLD_KEY, 0))
		bold = CFE_BOLD;

	if (g_plugin.getByte(FONT_ITALIC_KEY, 0))
		italic = CFE_ITALIC;

	if (g_plugin.getByte(FONT_UNDERLINE_KEY, 0))
		underline = CFE_UNDERLINE;

	cfFM.dwEffects = bold | italic | underline;

	if (!g_plugin.getWString(FONT_FACE_KEY, &dbv)) {
		mir_wstrcpy(cfFM.szFaceName, dbv.pwszVal);
		db_free(&dbv);
	}
	else mir_wstrcpy(cfFM.szFaceName, Def_font_face);

	HDC hDC = GetDC(hWindow);
	cfFM.yHeight = (uint8_t)MulDiv(abs(g_lf.lfHeight), 120, GetDeviceCaps(GetDC(hWindow), LOGPIXELSY)) * (g_plugin.getByte(FONT_SIZE_KEY, 14));
	ReleaseDC(hWindow, hDC);

	cfFM.bCharSet = g_plugin.getByte(FONT_SCRIPT_KEY, 0);
	cfFM.bPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	cfFM.crTextColor = rgbText;
	cfFM.crBackColor = rgbBack;
	SendDlgItemMessage(hWindow, IDC_DATA, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)& cfFM);

	SendDlgItemMessage(hWindow, IDC_DATA, EM_SETSEL, 0, -1);
	SendDlgItemMessageA(hWindow, IDC_DATA, EM_REPLACESEL, FALSE, (LPARAM)truncated);
	SendDlgItemMessage(hWindow, IDC_DATA, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfFM);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK timerfunc(HWND, UINT, UINT_PTR, DWORD)
{
	g_plugin.setByte(HAS_CRASHED_KEY, 0);

	if (!(g_plugin.getByte(OFFLINE_STATUS, 1)))
		if (!(g_plugin.getByte(DISABLE_AUTOUPDATE_KEY, 0)))
			mir_forkthread(ContactLoop);

	g_plugin.setDword(COUNTDOWN_KEY, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK Countdownfunc(HWND, UINT, UINT_PTR, DWORD)
{
	uint32_t timetemp = g_plugin.getDword(COUNTDOWN_KEY, 100);
	if (timetemp <= 0) {
		timetemp = g_plugin.getDword(REFRESH_KEY, TIME);
		g_plugin.setDword(COUNTDOWN_KEY, timetemp);
	}

	g_plugin.setDword(COUNTDOWN_KEY, timetemp - 1);

	ChangeMenuItemCountdown();
}

/////////////////////////////////////////////////////////////////////////////////////////
void FontSettings(void)
{
	g_lf.lfHeight = 16;
	g_lf.lfWidth = 0;
	g_lf.lfEscapement = 0;
	g_lf.lfOrientation = 0;
	g_lf.lfWeight = FW_NORMAL;
	g_lf.lfItalic = FALSE;
	g_lf.lfUnderline = FALSE;
	g_lf.lfStrikeOut = 0;
	g_lf.lfCharSet = ANSI_CHARSET;
	g_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	g_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	g_lf.lfQuality = DEFAULT_QUALITY;
	g_lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	mir_wstrcpy(g_lf.lfFaceName, Def_font_face);
}

/////////////////////////////////////////////////////////////////////////////////////////
int ModulesLoaded(WPARAM, LPARAM)
{
	hHookDisplayDataAlert = CreateHookableEvent(ME_DISPLAYDATA_ALERT);
	HookEvent(ME_DISPLAYDATA_ALERT, DataWndAlertCommand);

	hHookAlertPopup = CreateHookableEvent(ME_POPUP_ALERT);
	HookEvent(ME_POPUP_ALERT, PopupAlert);

	hHookErrorPopup = CreateHookableEvent(ME_POPUP_ERROR);
	HookEvent(ME_POPUP_ERROR, ErrorMsgs);

	hHookAlertOSD = CreateHookableEvent(ME_OSD_ALERT);
	HookEvent(ME_OSD_ALERT, OSDAlert);

	FontSettings();
	h_font = CreateFontIndirect(&g_lf);

	// get data on startup
	if (g_plugin.getByte(UPDATE_ONSTART_KEY, 0))
		mir_forkthread(StartUpdate);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR DataWndMenuCommand(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = wParam;
	HWND hwndDlg = WindowList_Find(hWindowList, hContact);
	if (hwndDlg != nullptr) {
		DestroyWindow(hwndDlg);
		return 0;
	}

	HWND hTopmost = g_plugin.getByte(hContact, ON_TOP_KEY, 0) ? HWND_TOPMOST : HWND_NOTOPMOST;
	hwndDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DISPLAY_DATA), nullptr, DlgProcDisplayData, (LPARAM)hContact);
	SendDlgItemMessage(hwndDlg, IDC_STICK_BUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_STICK), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
	if (g_plugin.getByte(SAVE_INDIVID_POS_KEY, 0))
		SetWindowPos(hwndDlg, hTopmost,
			g_plugin.getDword(hContact, "WVx", 100), // Xposition,
			g_plugin.getDword(hContact, "WVy", 100), // Yposition,
			g_plugin.getDword(hContact, "WVwidth", 100), // WindowWidth,
			g_plugin.getDword(hContact, "WVheight", 100), 0); // WindowHeight,
	else
		SetWindowPos(hwndDlg, HWND_TOPMOST, Xposition, Yposition, WindowWidth, WindowHeight, 0);

	ShowWindow(hwndDlg, SW_SHOW);
	SetActiveWindow(hwndDlg);

	if (g_plugin.getByte(UPDATE_ON_OPEN_KEY, 0)) {
		if (g_plugin.getByte(hContact, ENABLE_ALERTS_KEY, 0))
			mir_forkthread(ReadFromFile, (void*)hContact);
		else
			mir_forkthread(GetData, (void*)hContact);
		g_plugin.setWord(hContact, "Status", ID_STATUS_ONLINE);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR UpdateAllMenuCommand(WPARAM, LPARAM)
{
	mir_forkthread(ContactLoop);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR AutoUpdateMCmd(WPARAM, LPARAM)
{
	if (g_plugin.getByte(DISABLE_AUTOUPDATE_KEY, 0))
		g_plugin.setByte(DISABLE_AUTOUPDATE_KEY, 0);
	else
		g_plugin.setByte(DISABLE_AUTOUPDATE_KEY, 1);

	ChangeMenuItem1();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR AddContactMenuCommand(WPARAM, LPARAM)
{
	db_set_s(0, "FindAdd", "LastSearched", MODULENAME);
	CallService(MS_FINDADD_FINDADD, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int OnTopMenuCommand(WPARAM, LPARAM, MCONTACT singlecontact)
{
	int ontop = 0;
	int done = 0;

	if (((g_plugin.getByte(singlecontact, ON_TOP_KEY))) && done == 0) {
		g_plugin.setByte(singlecontact, ON_TOP_KEY, 0);
		ontop = 0;
		done = 1;
	}
	if ((!(g_plugin.getByte(singlecontact, ON_TOP_KEY, 0))) && done == 0) {
		g_plugin.setByte(singlecontact, ON_TOP_KEY, 1);
		ontop = 1;
		done = 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR WebsiteMenuCommand(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = wParam;
	ptrW url(g_plugin.getWStringA(hContact, "URL"));
	if (url)
		Utils_OpenUrlW(url);

	g_plugin.setWord(hContact, "Status", ID_STATUS_ONLINE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int UpdateMenuCommand(WPARAM, LPARAM, MCONTACT singlecontact)
{
	mir_forkthread(GetData, (void*)singlecontact);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int ContactMenuItemUpdateData(WPARAM wParam, LPARAM lParam)
{
	UpdateMenuCommand(wParam, lParam, wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR CntOptionsMenuCommand(WPARAM wParam, LPARAM)
{
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONTACT_OPT), nullptr, DlgProcContactOpt, wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR CntAlertMenuCommand(WPARAM wParam, LPARAM)
{
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ALRT_OPT), nullptr, DlgProcAlertOpt, wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
INT_PTR CountdownMenuCommand(WPARAM, LPARAM)
{
	return 0;
}
