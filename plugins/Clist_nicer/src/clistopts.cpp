/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include "stdafx.h"

void AddToTaskBar(HWND hWnd);

void cfgSetFlag(HWND hwndDlg, int ctrlId, uint32_t dwMask);

extern COLORREF g_CLUISkinnedBkColorRGB;

/////////////////////////////////////////////////////////////////////////////////////////
// Main options : General

class COptGeneralDlg : public CRowItemsBaseDlg
{
	CCtrlCheck chkDontCycle, chkCycle, chkMulti;

public:
	COptGeneralDlg() :
		CRowItemsBaseDlg(IDD_OPT_CLIST),
		chkCycle(this, IDC_CYCLE),
		chkMulti(this, IDC_MULTITRAY),
		chkDontCycle(this, IDC_DONTCYCLE)
	{
		chkCycle.OnChange = chkMulti.OnChange = chkDontCycle.OnChange = Callback(this, &COptGeneralDlg::onChange_Cycle);
	}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_SHOWBOTTOMBUTTONS, cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CLISTSUNKEN, cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_EVENTAREAAUTOHIDE, cfg::dat.dwFlags & CLUI_FRAME_AUTOHIDENOTIFY ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_EVENTAREASUNKEN, (cfg::dat.dwFlags & CLUI_FRAME_EVENTAREASUNKEN) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ALWAYSMULTI, !g_plugin.getByte("AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DISABLEBLINK, g_plugin.getByte("DisableTrayFlash", 0) == 1 ? BST_CHECKED : BST_UNCHECKED);

		chkCycle.SetState(g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_CYCLE);
		chkMulti.SetState(g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI);
		chkDontCycle.SetState(g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_SINGLE);

		SendDlgItemMessage(m_hwnd, IDC_CYCLETIMESPIN, UDM_SETRANGE, 0, MAKELONG(120, 1));
		SendDlgItemMessage(m_hwnd, IDC_CYCLETIMESPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("CycleTime", SETTING_CYCLETIME_DEFAULT), 0));
		{
			ptrA szPrimaryStatus(g_plugin.getStringA("PrimaryStatus"));

			int item = SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Global"));
			SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM)0);

			for (auto &pa : Accounts()) {
				if (!pa->IsEnabled() || CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
					continue;

				item = SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM)pa->tszAccountName);
				SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM)pa);
				if (!mir_strcmp(szPrimaryStatus, pa->szModuleName))
					SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETCURSEL, item, 0);
			}
		}
		if (CB_ERR == (int)SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0))
			SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETCURSEL, 0, 0);

		SendDlgItemMessage(m_hwnd, IDC_BLINKSPIN, UDM_SETRANGE, 0, MAKELONG(0x3FFF, 250));
		SendDlgItemMessage(m_hwnd, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("IconFlashTime", 550), 0));
		CheckDlgButton(m_hwnd, IDC_NOTRAYINFOTIPS, cfg::dat.bNoTrayTips ? 1 : 0);
		CheckDlgButton(m_hwnd, IDC_APPLYLASTVIEWMODE, g_plugin.getByte("AutoApplyLastViewMode", 0) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("AlwaysMulti", (uint8_t)BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_ALWAYSMULTI));
		g_plugin.setByte("TrayIcon", (uint8_t)(chkDontCycle.GetState() ? SETTING_TRAYICON_SINGLE : (chkCycle.GetState() ? SETTING_TRAYICON_CYCLE : SETTING_TRAYICON_MULTI)));
		g_plugin.setWord("CycleTime", (uint16_t)SendDlgItemMessage(m_hwnd, IDC_CYCLETIMESPIN, UDM_GETPOS, 0, 0));
		g_plugin.setWord("IconFlashTime", (uint16_t)SendDlgItemMessage(m_hwnd, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
		g_plugin.setByte("DisableTrayFlash", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_DISABLEBLINK));
		g_plugin.setByte("AutoApplyLastViewMode", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_APPLYLASTVIEWMODE));

		cfgSetFlag(m_hwnd, CLUI_FRAME_EVENTAREASUNKEN, IDC_EVENTAREASUNKEN);
		cfgSetFlag(m_hwnd, CLUI_FRAME_AUTOHIDENOTIFY, IDC_EVENTAREAAUTOHIDE);

		cfgSetFlag(m_hwnd, CLUI_FRAME_SHOWBOTTOMBUTTONS, IDC_SHOWBOTTOMBUTTONS);
		cfgSetFlag(m_hwnd, CLUI_FRAME_CLISTSUNKEN, IDC_CLISTSUNKEN);

		cfg::dat.bNoTrayTips = IsDlgButtonChecked(m_hwnd, IDC_NOTRAYINFOTIPS) ? 1 : 0;
		g_plugin.setByte("NoTrayTips", (uint8_t)cfg::dat.bNoTrayTips);
		{
			int cursel = SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0);
			PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_GETITEMDATA, cursel, 0);
			if (!pa)
				g_plugin.delSetting("PrimaryStatus");
			else
				g_plugin.setString("PrimaryStatus", pa->szModuleName);
		}
		Clist_TrayIconIconsChanged();
		db_set_dw(0, "CLUI", "Frameflags", cfg::dat.dwFlags);
		ConfigureFrame();
		ConfigureCLUIGeometry(1);
		ConfigureEventArea();
		HideShowNotifyFrame();
		SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);
		SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
		Clist_LoadContactTree(); /* this won't do job properly since it only really works when changes happen */
		Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		return true;
	}

	void onChange_Cycle(CCtrlCheck *)
	{
		Utils::enableDlgControl(m_hwnd, IDC_PRIMARYSTATUS, chkDontCycle.GetState());
		Utils::enableDlgControl(m_hwnd, IDC_CYCLETIME, chkCycle.GetState());
		Utils::enableDlgControl(m_hwnd, IDC_CYCLETIMESPIN, chkCycle.GetState());
		Utils::enableDlgControl(m_hwnd, IDC_ALWAYSMULTI, chkMulti.GetState());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// "List layout" options

struct CheckBoxToStyleEx_t
{
	int id;
	uint32_t flag;
	int not_t;
}
static const checkBoxToStyleEx[] = {
	{ IDC_DISABLEDRAGDROP, CLS_EX_DISABLEDRAGDROP, 0 },
	{ IDC_NOTEDITLABELS, CLS_EX_EDITLABELS, 1 },
	{ IDC_SHOWSELALWAYS, CLS_EX_SHOWSELALWAYS, 0 },
	{ IDC_TRACKSELECT, CLS_EX_TRACKSELECT, 0 },
	{ IDC_DIVIDERONOFF, CLS_EX_DIVIDERONOFF, 0 },
	{ IDC_NOTNOTRANSLUCENTSEL, CLS_EX_NOTRANSLUCENTSEL, 1 },
	{ IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING, 1 }
};

struct CheckBoxValues_t
{
	uint32_t style;
	wchar_t *szDescr;
}
static const greyoutValues[] = {
	{ GREYF_UNFOCUS, LPGENW("Not focused") },
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE,    LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY,  LPGENW("Not available") },
	{ PF2_LIGHTDND,  LPGENW("Occupied") },
	{ PF2_HEAVYDND,  LPGENW("Do not disturb") },
	{ PF2_FREECHAT,  LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") },
};

class COptLayoutDlg : public CRowItemsBaseDlg
{
	CCtrlCheck chkSmooth, chkGreyOut;
	CCtrlTreeView greyOut;

public:
	COptLayoutDlg() :
		CRowItemsBaseDlg(IDD_OPT_CLC),
		greyOut(this, IDC_GREYOUTOPTS),
		chkSmooth(this, IDC_NOTNOSMOOTHSCROLLING),
		chkGreyOut(this, IDC_GREYOUT)
	{
		chkSmooth.OnChange = Callback(this, &COptLayoutDlg::onChange_Smooth);
		chkGreyOut.OnChange = Callback(this, &COptLayoutDlg::onChange_GreyOut);
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(greyOut.GetHwnd(), GWL_STYLE, GetWindowLongPtr(greyOut.GetHwnd(), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

		uint32_t exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());
		UDACCEL accel[2] = { { 0, 10 }, { 2, 50 } };
		SendDlgItemMessage(m_hwnd, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
		SendDlgItemMessage(m_hwnd, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, _countof(accel), (LPARAM)&accel);
		SendDlgItemMessage(m_hwnd, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(0, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));

		for (auto &it : checkBoxToStyleEx)
			CheckDlgButton(m_hwnd, it.id, (exStyle & it.flag) ^ (it.flag * it.not_t) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_FULLROWSELECT, (cfg::dat.dwFlags & CLUI_FULLROWSELECT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DBLCLKAVATARS, cfg::dat.bDblClkAvatars ? BST_CHECKED : BST_UNCHECKED);

		chkGreyOut.SetState(db_get_dw(0, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS));

		uint32_t style = db_get_dw(0, "CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS);
		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
		for (auto &it : greyoutValues) {
			tvis.item.lParam = it.style;
			tvis.item.pszText = TranslateW(it.szDescr);
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;
			tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
			greyOut.InsertItem(&tvis);
		}

		CheckDlgButton(m_hwnd, IDC_NOSCROLLBAR, db_get_b(0, "CLC", "NoVScrollBar", 0) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		uint32_t exStyle = db_get_dw(0, "CLC", "ExStyle", CLCDEFAULT_EXSTYLE);

		for (auto &it : checkBoxToStyleEx)
			exStyle &= ~(it.flag);

		for (auto &it : checkBoxToStyleEx)
			if ((IsDlgButtonChecked(m_hwnd, it.id) == 0) == it.not_t)
				exStyle |= it.flag;

		db_set_dw(0, "CLC", "ExStyle", exStyle);

		uint32_t fullGreyoutFlags = 0;
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
		tvi.hItem = greyOut.GetRoot();
		while (tvi.hItem) {
			greyOut.GetItem(&tvi);
			if ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2)
				fullGreyoutFlags |= tvi.lParam;
			tvi.hItem = greyOut.GetNextSibling(tvi.hItem);
		}
		db_set_dw(0, "CLC", "FullGreyoutFlags", fullGreyoutFlags);

		if (chkGreyOut.GetState())
			db_set_dw(0, "CLC", "GreyoutFlags", fullGreyoutFlags);
		else
			db_set_dw(0, "CLC", "GreyoutFlags", 0);

		cfgSetFlag(m_hwnd, IDC_FULLROWSELECT, CLUI_FULLROWSELECT);

		db_set_w(0, "CLC", "ScrollTime", (uint16_t)SendDlgItemMessage(m_hwnd, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "NoVScrollBar", (uint8_t)(IsDlgButtonChecked(m_hwnd, IDC_NOSCROLLBAR) ? 1 : 0));
		cfg::dat.bDblClkAvatars = IsDlgButtonChecked(m_hwnd, IDC_DBLCLKAVATARS) ? TRUE : FALSE;
		db_set_b(0, "CLC", "dblclkav", (uint8_t)cfg::dat.bDblClkAvatars);

		Clist_ClcOptionsChanged();
		CoolSB_SetupScrollBar();
		PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
		return true;
	}

	void onChange_Smooth(CCtrlCheck *)
	{
		Utils::enableDlgControl(m_hwnd, IDC_SMOOTHTIME, chkSmooth.GetState());
	}

	void onChange_GreyOut(CCtrlCheck *)
	{
		greyOut.Enable(chkGreyOut.GetState());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Window options

class COptWindowDlg : public CRowItemsBaseDlg
{
	CCtrlCheck chkAutoHide, chkAutoSize, chkTransparent;

public:
	COptWindowDlg() :
		CRowItemsBaseDlg(IDD_OPT_CLUI),
		chkAutoHide(this, IDC_AUTOHIDE),
		chkAutoSize(this, IDC_AUTOSIZE),
		chkTransparent(this, IDC_TRANSPARENT)
	{
		chkAutoHide.OnChange = Callback(this, &COptWindowDlg::onChange_AutoHide);
		chkAutoSize.OnChange = Callback(this, &COptWindowDlg::onChange_AutoSize);
		chkTransparent.OnChange = Callback(this, &COptWindowDlg::onChange_Transparent);
	}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_BRINGTOFRONT, g_plugin.getByte("BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ALWAYSHIDEONTASKBAR, g_plugin.getByte("AlwaysHideOnTB", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ONTOP, g_plugin.getByte("OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWMAINMENU, db_get_b(0, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CLIENTDRAG, db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_FADEINOUT, cfg::dat.fadeinout ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DROPSHADOW, g_plugin.getByte("WindowShadow", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ONDESKTOP, g_plugin.getByte("OnDesktop", 0) ? BST_CHECKED : BST_UNCHECKED);

		chkAutoSize.SetState(cfg::dat.autosize);

		SendDlgItemMessage(m_hwnd, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Title bar"));
		SendDlgItemMessage(m_hwnd, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Tool Window"));
		SendDlgItemMessage(m_hwnd, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Thin border"));
		SendDlgItemMessage(m_hwnd, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("No border"));
		SendDlgItemMessage(m_hwnd, IDC_BORDERSTYLE, CB_SETCURSEL, db_get_b(0, "CLUI", "WindowStyle", SETTING_WINDOWSTYLE_TOOLWINDOW), 0);

		SendDlgItemMessage(m_hwnd, IDC_MAXSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(m_hwnd, IDC_MAXSIZESPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "MaxSizeHeight", 75));

		SendDlgItemMessage(m_hwnd, IDC_CLIPBORDERSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
		SendDlgItemMessage(m_hwnd, IDC_CLIPBORDERSPIN, UDM_SETPOS, 0, cfg::dat.bClipBorder);

		SendDlgItemMessage(m_hwnd, IDC_CLEFTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(m_hwnd, IDC_CRIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(m_hwnd, IDC_CTOPSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(m_hwnd, IDC_CBOTTOMSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));

		SendDlgItemMessage(m_hwnd, IDC_CLEFTSPIN, UDM_SETPOS, 0, cfg::dat.bCLeft - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0));
		SendDlgItemMessage(m_hwnd, IDC_CRIGHTSPIN, UDM_SETPOS, 0, cfg::dat.bCRight - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0));
		SendDlgItemMessage(m_hwnd, IDC_CTOPSPIN, UDM_SETPOS, 0, cfg::dat.bCTop);
		SendDlgItemMessage(m_hwnd, IDC_CBOTTOMSPIN, UDM_SETPOS, 0, cfg::dat.bCBottom);

		CheckDlgButton(m_hwnd, IDC_AUTOSIZEUPWARD, db_get_b(0, "CLUI", "AutoSizeUpward", 0) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(m_hwnd, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
		SendDlgItemMessage(m_hwnd, IDC_HIDETIMESPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("HideTime", SETTING_HIDETIME_DEFAULT), 0));

		chkAutoHide.SetState(g_plugin.getByte("AutoHide", SETTING_AUTOHIDE_DEFAULT));

		ptrW tszTitle(g_plugin.getWStringA("TitleText"));
		if (tszTitle != NULL)
			SetDlgItemText(m_hwnd, IDC_TITLETEXT, tszTitle);
		else
			SetDlgItemTextA(m_hwnd, IDC_TITLETEXT, MIRANDANAME);

		CheckDlgButton(m_hwnd, IDC_FULLTRANSPARENT, cfg::dat.bFullTransparent ? BST_CHECKED : BST_UNCHECKED);

		chkTransparent.SetState(cfg::dat.isTransparent);

		SendDlgItemMessage(m_hwnd, IDC_TRANSACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
		SendDlgItemMessage(m_hwnd, IDC_TRANSINACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
		SendDlgItemMessage(m_hwnd, IDC_TRANSACTIVE, TBM_SETPOS, TRUE, cfg::dat.alpha);
		SendDlgItemMessage(m_hwnd, IDC_TRANSINACTIVE, TBM_SETPOS, TRUE, cfg::dat.autoalpha);
		UpdateCounters();

		CheckDlgButton(m_hwnd, IDC_ROUNDEDBORDER, cfg::dat.dwFlags & CLUI_FRAME_ROUNDEDFRAME ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(m_hwnd, IDC_FRAMEGAPSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
		SendDlgItemMessage(m_hwnd, IDC_FRAMEGAPSPIN, UDM_SETPOS, 0, (LPARAM)cfg::dat.gapBetweenFrames);
		return true;
	}

	bool OnApply() override
	{
		BOOL translated;
		uint8_t windowStyle = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_BORDERSTYLE, CB_GETCURSEL, 0, 0);

		db_set_b(0, "CLUI", "FadeInOut", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_FADEINOUT));
		cfg::dat.fadeinout = IsDlgButtonChecked(m_hwnd, IDC_FADEINOUT) ? 1 : 0;
		uint8_t oldFading = cfg::dat.fadeinout;
		cfg::dat.fadeinout = FALSE;

		db_set_b(0, "CLUI", "WindowStyle", windowStyle);
		cfg::dat.gapBetweenFrames = GetDlgItemInt(m_hwnd, IDC_FRAMEGAP, &translated, FALSE);

		db_set_dw(0, "CLUIFrames", "GapBetweenFrames", cfg::dat.gapBetweenFrames);
		g_plugin.setByte("OnTop", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ONTOP));
		SetWindowPos(g_clistApi.hwndContactList, IsDlgButtonChecked(m_hwnd, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		cfg::dat.bCLeft = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_CLEFTSPIN, UDM_GETPOS, 0, 0);
		cfg::dat.bCRight = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_CRIGHTSPIN, UDM_GETPOS, 0, 0);
		cfg::dat.bCTop = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_CTOPSPIN, UDM_GETPOS, 0, 0);
		cfg::dat.bCBottom = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_CBOTTOMSPIN, UDM_GETPOS, 0, 0);

		db_set_dw(0, "CLUI", "clmargins", MAKELONG(MAKEWORD(cfg::dat.bCLeft, cfg::dat.bCRight), MAKEWORD(cfg::dat.bCTop, cfg::dat.bCBottom)));
		SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);

		g_plugin.setByte("BringToFront", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_BRINGTOFRONT));
		g_plugin.setByte("AlwaysHideOnTB", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ALWAYSHIDEONTASKBAR));

		if (windowStyle != SETTING_WINDOWSTYLE_DEFAULT) {
			LONG style;
			// Window must be hidden to dynamically remove the taskbar button.
			// See http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/taskbar.asp
			WINDOWPLACEMENT p;
			p.length = sizeof(p);
			GetWindowPlacement(g_clistApi.hwndContactList, &p);
			ShowWindow(g_clistApi.hwndContactList, SW_HIDE);

			style = GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE);
			style |= WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;
			style &= ~WS_EX_APPWINDOW;
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, style);

			SetWindowPlacement(g_clistApi.hwndContactList, &p);
			ShowWindow(g_clistApi.hwndContactList, SW_SHOW);
		}
		else {
			LONG style;
			style = GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE);
			style &= ~(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE);
			if (g_plugin.getByte("AlwaysHideOnTB", 1))
				style &= ~WS_EX_APPWINDOW;
			else {
				style |= WS_EX_APPWINDOW;
				AddToTaskBar(g_clistApi.hwndContactList);
			}
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, style);
		}

		cfg::dat.bClipBorder = (uint8_t)GetDlgItemInt(m_hwnd, IDC_CLIPBORDER, &translated, FALSE);
		db_set_dw(0, "CLUI", "Frameflags", cfg::dat.dwFlags);
		db_set_b(0, "CLUI", "clipborder", cfg::dat.bClipBorder);

		db_set_b(0, "CLUI", "ShowMainMenu", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWMAINMENU));
		db_set_b(0, "CLUI", "ClientAreaDrag", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_CLIENTDRAG));

		ApplyCLUIBorderStyle();

		if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_SHOWMAINMENU))
			SetMenu(g_clistApi.hwndContactList, nullptr);
		else
			SetMenu(g_clistApi.hwndContactList, g_clistApi.hMenuMain);

		wchar_t title[256];
		GetDlgItemText(m_hwnd, IDC_TITLETEXT, title, _countof(title));
		g_plugin.setWString("TitleText", title);
		SetWindowText(g_clistApi.hwndContactList, title);

		cfg::dat.dwFlags = IsDlgButtonChecked(m_hwnd, IDC_ROUNDEDBORDER) ? cfg::dat.dwFlags | CLUI_FRAME_ROUNDEDFRAME : cfg::dat.dwFlags & ~CLUI_FRAME_ROUNDEDFRAME;

		if (cfg::dat.autosize = chkAutoSize.GetState()) {
			SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
			SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);
		}
		db_set_b(0, "CLUI", "AutoSize", cfg::dat.autosize);

		db_set_b(0, "CLUI", "MaxSizeHeight", (uint8_t)GetDlgItemInt(m_hwnd, IDC_MAXSIZEHEIGHT, nullptr, FALSE));
		db_set_b(0, "CLUI", "AutoSizeUpward", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_AUTOSIZEUPWARD));
		g_plugin.setByte("AutoHide", chkAutoHide.GetState());
		g_plugin.setWord("HideTime", (uint16_t)SendDlgItemMessage(m_hwnd, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));

		g_plugin.setByte("Transparent", cfg::dat.isTransparent = chkTransparent.GetState());
		g_plugin.setByte("Alpha", cfg::dat.alpha = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
		g_plugin.setByte("AutoAlpha", cfg::dat.autoalpha = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));
		g_plugin.setByte("WindowShadow", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_DROPSHADOW));
		g_plugin.setByte("OnDesktop", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ONDESKTOP));
		db_set_dw(0, "CLUI", "Frameflags", cfg::dat.dwFlags);
		cfg::dat.bFullTransparent = IsDlgButtonChecked(m_hwnd, IDC_FULLTRANSPARENT) ? 1 : 0;
		db_set_b(0, "CLUI", "fulltransparent", (uint8_t)cfg::dat.bFullTransparent);

		if (cfg::dat.bLayeredHack)
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);

		if (g_CLUISkinnedBkColorRGB)
			cfg::dat.colorkey = g_CLUISkinnedBkColorRGB;
		else if (cfg::dat.bClipBorder == 0 && !(cfg::dat.dwFlags & CLUI_FRAME_ROUNDEDFRAME))
			cfg::dat.colorkey = db_get_dw(0, "CLC", "BkColour", CLCDEFAULT_BKCOLOUR);
		else {
			SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
			cfg::dat.colorkey = RGB(255, 0, 255);
		}
		if (cfg::dat.isTransparent || cfg::dat.bFullTransparent) {
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(g_clistApi.hwndContactList, 0, 255, LWA_ALPHA | LWA_COLORKEY);
			SetLayeredWindowAttributes(g_clistApi.hwndContactList,
				(COLORREF)(cfg::dat.bFullTransparent ? cfg::dat.colorkey : 0),
				(uint8_t)(cfg::dat.isTransparent ? cfg::dat.autoalpha : 255),
				(uint32_t)((cfg::dat.isTransparent ? LWA_ALPHA : 0L) | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0L)));
		}
		else {
			SetLayeredWindowAttributes(g_clistApi.hwndContactList, RGB(0, 0, 0), (uint8_t)255, LWA_ALPHA);
			if (!cfg::dat.bLayeredHack)
				SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		}

		ConfigureCLUIGeometry(1);
		ShowWindow(g_clistApi.hwndContactList, SW_SHOW);
		SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
		SetWindowPos(g_clistApi.hwndContactList, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
		cfg::dat.fadeinout = oldFading;

		Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		return true;
	}

	void onChange_AutoHide(CCtrlCheck *pCheck)
	{
		bool bEnable = pCheck->GetState();
		Utils::enableDlgControl(m_hwnd, IDC_HIDETIME, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_STATIC01, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_HIDETIMESPIN, bEnable);
	}

	void onChange_AutoSize(CCtrlCheck *pCheck)
	{
		bool bEnable = pCheck->GetState();
		Utils::enableDlgControl(m_hwnd, IDC_STATIC21, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_STATIC22, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_MAXSIZEHEIGHT, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_MAXSIZESPIN, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_AUTOSIZEUPWARD, bEnable);
	}

	void onChange_Transparent(CCtrlCheck *pCheck)
	{
		bool bEnable = pCheck->GetState();
		Utils::enableDlgControl(m_hwnd, IDC_STATIC11, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_STATIC12, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_TRANSACTIVE, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_TRANSINACTIVE, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_ACTIVEPERC, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_INACTIVEPERC, bEnable);
	}

	void UpdateCounters()
	{
		char str[10];
		mir_snprintf(str, "%d%%", 100 * SendDlgItemMessage(m_hwnd, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
		SetDlgItemTextA(m_hwnd, IDC_INACTIVEPERC, str);
		mir_snprintf(str, "%d%%", 100 * SendDlgItemMessage(m_hwnd, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
		SetDlgItemTextA(m_hwnd, IDC_ACTIVEPERC, str);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_HSCROLL)
			UpdateCounters();

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// "Background" options page

class COptBackgroundDlg : public CRowItemsBaseDlg
{
	CCtrlCheck chkBitmap;
	CCtrlButton btnBrowse;

public:
	COptBackgroundDlg() :
		CRowItemsBaseDlg(IDD_OPT_CLCBKG),
		chkBitmap(this, IDC_BITMAP),
		btnBrowse(this, IDC_BROWSE)
	{
		chkBitmap.OnChange = Callback(this, &COptBackgroundDlg::onChange_Bitmap);

		btnBrowse.OnClick = Callback(this, &COptBackgroundDlg::onClick_Browse);
	}

	bool OnInitDialog() override
	{
		chkBitmap.SetState(db_get_b(0, "CLC", "UseBitmap", CLCDEFAULT_USEBITMAP));

		CheckDlgButton(m_hwnd, IDC_WINCOLOUR, db_get_b(0, "CLC", "UseWinColours", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SKINMODE, cfg::dat.bWallpaperMode ? BST_CHECKED : BST_UNCHECKED);

		DBVARIANT dbv;
		if (!db_get_ws(0, "CLC", "BkBitmap", &dbv)) {
			wchar_t szPath[MAX_PATH];
			if (PathToAbsoluteW(dbv.pwszVal, szPath))
				SetDlgItemText(m_hwnd, IDC_FILENAME, szPath);

			db_free(&dbv);
		}

		uint16_t bmpUse = db_get_w(0, "CLC", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
		CheckDlgButton(m_hwnd, IDC_STRETCHH, bmpUse & CLB_STRETCHH ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_STRETCHV, bmpUse & CLB_STRETCHV ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_TILEH, bmpUse & CLBF_TILEH ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_TILEV, bmpUse & CLBF_TILEV ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SCROLL, bmpUse & CLBF_SCROLL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_PROPORTIONAL, bmpUse & CLBF_PROPORTIONAL ? BST_CHECKED : BST_UNCHECKED);

		SHAutoComplete(GetDlgItem(m_hwnd, IDC_FILENAME), 1);
		return true;
	}

	bool OnApply() override
	{
		db_set_b(0, "CLC", "UseBitmap", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_BITMAP));
		db_set_b(0, "CLC", "UseWinColours", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_WINCOLOUR));

		char str[MAX_PATH], strrel[MAX_PATH];
		GetDlgItemTextA(m_hwnd, IDC_FILENAME, str, _countof(str));
		if (PathToRelative(str, strrel))
			db_set_s(0, "CLC", "BkBitmap", strrel);
		else
			db_set_s(0, "CLC", "BkBitmap", str);

		uint16_t flags = 0;
		if (IsDlgButtonChecked(m_hwnd, IDC_STRETCHH))
			flags |= CLB_STRETCHH;
		if (IsDlgButtonChecked(m_hwnd, IDC_STRETCHV))
			flags |= CLB_STRETCHV;
		if (IsDlgButtonChecked(m_hwnd, IDC_TILEH))
			flags |= CLBF_TILEH;
		if (IsDlgButtonChecked(m_hwnd, IDC_TILEV))
			flags |= CLBF_TILEV;
		if (IsDlgButtonChecked(m_hwnd, IDC_SCROLL))
			flags |= CLBF_SCROLL;
		if (IsDlgButtonChecked(m_hwnd, IDC_PROPORTIONAL))
			flags |= CLBF_PROPORTIONAL;
		db_set_w(0, "CLC", "BkBmpUse", flags);
		cfg::dat.bWallpaperMode = IsDlgButtonChecked(m_hwnd, IDC_SKINMODE) ? 1 : 0;
		db_set_b(0, "CLUI", "UseBkSkin", (uint8_t)cfg::dat.bWallpaperMode);
		return true;
	}

	void onChange_Bitmap(CCtrlCheck *)
	{
		bool bEnabled = chkBitmap.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_FILENAME, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_BROWSE, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_STRETCHH, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_STRETCHV, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_TILEH, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_TILEV, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_SCROLL, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_PROPORTIONAL, bEnabled);
	}

	void onClick_Browse(CCtrlButton *)
	{
		wchar_t str[MAX_PATH], filter[512];
		GetDlgItemText(m_hwnd, IDC_FILENAME, str, _countof(str));
		Bitmap_GetFilter(filter, _countof(filter));

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = m_hwnd;
		ofn.hInstance = nullptr;
		ofn.lpstrFilter = filter;
		ofn.lpstrFile = str;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.nMaxFile = _countof(str);
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrDefExt = L"bmp";
		if (GetOpenFileName(&ofn))
			SetDlgItemText(m_hwnd, IDC_FILENAME, str);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptStatusDlg : public CRowItemsBaseDlg
{
	CCtrlCheck chkShowSbar;

public:
	COptStatusDlg() :
		CRowItemsBaseDlg(IDD_OPT_SBAR),
		chkShowSbar(this, IDC_SHOWSBAR)
	{
		chkShowSbar.OnChange = Callback(this, &COptStatusDlg::onChange_Show);
	}

	bool OnInitDialog() override
	{
		chkShowSbar.SetState(db_get_b(0, "CLUI", "ShowSBar", 1));

		uint8_t showOpts = db_get_b(0, "CLUI", "SBarShow", 1);
		CheckDlgButton(m_hwnd, IDC_SHOWICON, showOpts & 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWPROTO, showOpts & 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWSTATUS, showOpts & 4 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_RIGHTSTATUS, db_get_b(0, "CLUI", "SBarRightClk", 0) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_RIGHTMIRANDA, BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_RIGHTSTATUS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_EQUALSECTIONS, cfg::dat.bEqualSections ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SBPANELBEVEL, db_get_b(0, "CLUI", "SBarBevel", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWGRIP, db_get_b(0, "CLUI", "ShowGrip", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SKINBACKGROUND, cfg::dat.bSkinnedStatusBar ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWXSTATUS, cfg::dat.bShowXStatusOnSbar ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_MARKLOCKED, db_get_b(0, "CLUI", "sbar_showlocked", 1) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		db_set_b(0, "CLUI", "ShowSBar", (uint8_t)chkShowSbar.GetState());
		db_set_b(0, "CLUI", "SBarShow", (uint8_t)((IsDlgButtonChecked(m_hwnd, IDC_SHOWICON) ? 1 : 0) | (IsDlgButtonChecked(m_hwnd, IDC_SHOWPROTO) ? 2 : 0) | (IsDlgButtonChecked(m_hwnd, IDC_SHOWSTATUS) ? 4 : 0)));
		db_set_b(0, "CLUI", "SBarRightClk", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_RIGHTMIRANDA));
		db_set_b(0, "CLUI", "EqualSections", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_EQUALSECTIONS));
		db_set_b(0, "CLUI", "sb_skinned", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SKINBACKGROUND));
		db_set_b(0, "CLUI", "sbar_showlocked", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_MARKLOCKED));

		cfg::dat.bEqualSections = IsDlgButtonChecked(m_hwnd, IDC_EQUALSECTIONS) ? 1 : 0;
		cfg::dat.bSkinnedStatusBar = IsDlgButtonChecked(m_hwnd, IDC_SKINBACKGROUND) ? 1 : 0;
		cfg::dat.bShowXStatusOnSbar = IsDlgButtonChecked(m_hwnd, IDC_SHOWXSTATUS) ? 1 : 0;
		db_set_b(0, "CLUI", "xstatus_sbar", (uint8_t)cfg::dat.bShowXStatusOnSbar);
		db_set_b(0, "CLUI", "SBarBevel", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SBPANELBEVEL));
		if (db_get_b(0, "CLUI", "ShowGrip", 1) != (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWGRIP)) {
			db_set_b(0, "CLUI", "ShowGrip", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWGRIP));
			ShowWindow(g_clistApi.hwndStatus, SW_HIDE);
			mir_unsubclassWindow(g_clistApi.hwndStatus, NewStatusBarWndProc);
			DestroyWindow(g_clistApi.hwndStatus);

			int flags = WS_CHILD | CCS_BOTTOM;
			flags |= db_get_b(0, "CLUI", "ShowSBar", 1) ? WS_VISIBLE : 0;
			flags |= db_get_b(0, "CLUI", "ShowGrip", 1) ? SBARS_SIZEGRIP : 0;

			HWND parent = GetParent(g_clistApi.hwndStatus);
			g_clistApi.hwndStatus = CreateWindow(STATUSCLASSNAME, nullptr, flags, 0, 0, 0, 0, parent, nullptr, g_plugin.getInst(), nullptr);
			mir_subclassWindow(g_clistApi.hwndStatus, NewStatusBarWndProc);
		}
		if (chkShowSbar.GetState()) {
			ShowWindow(g_clistApi.hwndStatus, SW_SHOW);
			SendMessage(g_clistApi.hwndStatus, WM_SIZE, 0, 0);
			cfg::dat.dwFlags |= CLUI_FRAME_SBARSHOW;
		}
		else {
			ShowWindow(g_clistApi.hwndStatus, SW_HIDE);
			cfg::dat.dwFlags &= ~CLUI_FRAME_SBARSHOW;
		}
		db_set_dw(0, "CLUI", "Frameflags", cfg::dat.dwFlags);
		ConfigureCLUIGeometry(1);
		SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
		CluiProtocolStatusChanged(0, nullptr);
		return true;
	}

	void onChange_Show(CCtrlCheck *)
	{
		bool bEnabled = chkShowSbar.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_SHOWICON, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_SHOWPROTO, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_SHOWSTATUS, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_RIGHTSTATUS, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_RIGHTMIRANDA, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_EQUALSECTIONS, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_SBPANELBEVEL, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_SHOWGRIP, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_SKINBACKGROUND, bEnabled);
	}
};

void InitClistOptions(WPARAM wParam)
{
	// Main options tabs
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Contact list");

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new COptGeneralDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("List layout");
	odp.pDialog = new COptLayoutDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Window");
	odp.pDialog = new COptWindowDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Background");
	odp.pDialog = new COptBackgroundDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Status bar");
	odp.pDialog = new COptStatusDlg();
	g_plugin.addOptions(wParam, &odp);
}