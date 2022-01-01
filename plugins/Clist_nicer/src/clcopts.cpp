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
#include "coolscroll.h"

#include <m_extraicons.h>

#define DBFONTF_BOLD		1
#define DBFONTF_ITALIC		2
#define DBFONTF_UNDERLINE	4

void cfgSetFlag(HWND hwndDlg, int ctrlId, uint32_t dwMask)
{
	if (IsDlgButtonChecked(hwndDlg, ctrlId))
		cfg::dat.dwFlags |= dwMask;
	else
		cfg::dat.dwFlags &= ~dwMask;
	db_set_dw(0, "CLUI", "Frameflags", cfg::dat.dwFlags);
}

void GetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, FALSE);
	*colour = GetSysColor(COLOR_WINDOWTEXT);
	switch (i) {
	case FONTID_GROUPS:
		lf->lfWeight = FW_BOLD;
		break;
	case FONTID_GROUPCOUNTS:
		lf->lfHeight = (int)(lf->lfHeight * .75);
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
	case FONTID_OFFINVIS:
	case FONTID_INVIS:
		lf->lfItalic = !lf->lfItalic;
		break;
	case FONTID_DIVIDERS:
		lf->lfHeight = (int)(lf->lfHeight * .75);
		break;
	case FONTID_NOTONLIST:
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static UINT sortCtrlIDs[] = { IDC_SORTPRIMARY, IDC_SORTTHEN, IDC_SORTFINALLY };

class CDspItemsDlg : public CRowItemsBaseDlg
{
	CCtrlCombo comboAlign;

public:
	CDspItemsDlg() :
		CRowItemsBaseDlg(IDD_OPT_DSPITEMS),
		comboAlign(this, IDC_CLISTALIGN)
	{
	}

	bool OnInitDialog() override
	{
		for (auto &it : sortCtrlIDs) {
			SendDlgItemMessage(m_hwnd, it, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Nothing"));
			SendDlgItemMessage(m_hwnd, it, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Name"));
			SendDlgItemMessage(m_hwnd, it, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Protocol"));
			SendDlgItemMessage(m_hwnd, it, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Status"));
			SendDlgItemMessage(m_hwnd, it, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Last message"));
			SendDlgItemMessage(m_hwnd, it, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Message frequency"));
		}

		comboAlign.AddString(TranslateT("Never"));
		comboAlign.AddString(TranslateT("Always"));
		comboAlign.AddString(TranslateT("For RTL only"));
		comboAlign.AddString(TranslateT("RTL TEXT only"));

		CheckDlgButton(m_hwnd, IDC_EVENTSONTOP, (cfg::dat.dwFlags & CLUI_STICKYEVENTS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DONTSEPARATE, cfg::dat.bDontSeparateOffline ? BST_CHECKED : BST_UNCHECKED);
		for (int i = 0; i < _countof(sortCtrlIDs); i++)
			SendDlgItemMessage(m_hwnd, sortCtrlIDs[i], CB_SETCURSEL, cfg::dat.sortOrder[i], 0);

		comboAlign.SetCurSel(cfg::dat.bUseDCMirroring);
		return true;
	}

	bool OnApply() override
	{
		for (int i = 0; i < _countof(sortCtrlIDs); i++) {
			LRESULT curSel = SendDlgItemMessage(m_hwnd, sortCtrlIDs[i], CB_GETCURSEL, 0, 0);
			if (curSel == 0 || curSel == CB_ERR)
				cfg::dat.sortOrder[i] = 0;
			else
				cfg::dat.sortOrder[i] = (uint8_t)curSel;
		}
		g_plugin.setDword("SortOrder", MAKELONG(MAKEWORD(cfg::dat.sortOrder[0], cfg::dat.sortOrder[1]), MAKEWORD(cfg::dat.sortOrder[2], 0)));

		cfg::dat.bDontSeparateOffline = IsDlgButtonChecked(m_hwnd, IDC_DONTSEPARATE) ? 1 : 0;
		g_plugin.setByte("DontSeparateOffline", (uint8_t)cfg::dat.bDontSeparateOffline);

		cfgSetFlag(m_hwnd, IDC_EVENTSONTOP, CLUI_STICKYEVENTS);

		cfg::dat.bUseDCMirroring = comboAlign.GetCurSel();
		db_set_b(0, "CLC", "MirrorDC", cfg::dat.bUseDCMirroring);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CheckBoxToGroupStyleEx_t
{
	int id;
	uint32_t flag;
	int not_t;
}
static const checkBoxToGroupStyleEx[] =
{
	{ IDC_SHOWGROUPCOUNTS, CLS_EX_SHOWGROUPCOUNTS, 0 },
	{ IDC_HIDECOUNTSWHENEMPTY, CLS_EX_HIDECOUNTSWHENEMPTY, 0 },
	{ IDC_LINEWITHGROUPS, CLS_EX_LINEWITHGROUPS, 0 },
	{ IDC_QUICKSEARCHVISONLY, CLS_EX_QUICKSEARCHVISONLY, 0 },
	{ IDC_SORTGROUPSALPHA, CLS_EX_SORTGROUPSALPHA, 0 }
};

class CDspGroupsDlg : public CRowItemsBaseDlg
{

public:
	CDspGroupsDlg() :
		CRowItemsBaseDlg(IDD_OPT_DSPGROUPS)
	{
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always left"));
		SendDlgItemMessage(m_hwnd, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always right"));
		SendDlgItemMessage(m_hwnd, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Automatic (RTL)"));

		uint32_t exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());
		for (auto &it : checkBoxToGroupStyleEx)
			CheckDlgButton(m_hwnd, it.id, (exStyle & it.flag) ^ (it.flag * it.not_t) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_NOGROUPICON, (cfg::dat.dwFlags & CLUI_FRAME_NOGROUPICON) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CENTERGROUPNAMES, db_get_b(0, "CLCExt", "EXBK_CenterGroupnames", 0) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(m_hwnd, IDC_GROUPALIGN, CB_SETCURSEL, cfg::dat.bGroupAlign, 0);

		SendDlgItemMessage(m_hwnd, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
		SendDlgItemMessage(m_hwnd, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLC", "LeftMargin", CLCDEFAULT_LEFTMARGIN));
		SendDlgItemMessage(m_hwnd, IDC_RIGHTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
		SendDlgItemMessage(m_hwnd, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLC", "RightMargin", CLCDEFAULT_LEFTMARGIN));
		SendDlgItemMessage(m_hwnd, IDC_ROWGAPSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
		SendDlgItemMessage(m_hwnd, IDC_ROWGAPSPIN, UDM_SETPOS, 0, cfg::dat.bRowSpacing);
		SendDlgItemMessage(m_hwnd, IDC_GROUPINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(m_hwnd, IDC_GROUPINDENTSPIN, UDM_SETPOS, 0, db_get_b(0, "CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT));
		SendDlgItemMessage(m_hwnd, IDC_ROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 8));
		SendDlgItemMessage(m_hwnd, IDC_ROWHEIGHTSPIN, UDM_SETPOS, 0, db_get_b(0, "CLC", "RowHeight", CLCDEFAULT_ROWHEIGHT));
		SendDlgItemMessage(m_hwnd, IDC_GROUPROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 8));
		SendDlgItemMessage(m_hwnd, IDC_GROUPROWHEIGHTSPIN, UDM_SETPOS, 0, db_get_b(0, "CLC", "GRowHeight", CLCDEFAULT_ROWHEIGHT));
		SendDlgItemMessage(m_hwnd, IDC_AVATARPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
		SendDlgItemMessage(m_hwnd, IDC_AVATARPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.avatarPadding);
		return true;
	}

	bool OnApply() override
	{
		uint32_t exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());

		for (auto &it : checkBoxToGroupStyleEx) {
			if ((IsDlgButtonChecked(m_hwnd, it.id) == 0) == it.not_t)
				exStyle |= it.flag;
			else
				exStyle &= ~(it.flag);
		}
		db_set_dw(0, "CLC", "ExStyle", exStyle);

		cfgSetFlag(m_hwnd, IDC_NOGROUPICON, CLUI_FRAME_NOGROUPICON);

		db_set_b(0, "CLCExt", "EXBK_CenterGroupnames", IsDlgButtonChecked(m_hwnd, IDC_CENTERGROUPNAMES) ? 1 : 0);

		LRESULT curSel = SendDlgItemMessage(m_hwnd, IDC_GROUPALIGN, CB_GETCURSEL, 0, 0);
		if (curSel != CB_ERR) {
			cfg::dat.bGroupAlign = (uint8_t)curSel;
			db_set_b(0, "CLC", "GroupAlign", cfg::dat.bGroupAlign);
		}

		cfg::dat.bRowSpacing = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_ROWGAPSPIN, UDM_GETPOS, 0, 0);
		db_set_b(0, "CLC", "RowGap", cfg::dat.bRowSpacing);

		BOOL translated;
		cfg::dat.avatarPadding = (uint8_t)GetDlgItemInt(m_hwnd, IDC_AVATARPADDING, &translated, FALSE);
		g_plugin.setByte("AvatarPadding", cfg::dat.avatarPadding);

		db_set_b(0, "CLC", "LeftMargin", (uint8_t)SendDlgItemMessage(m_hwnd, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "RightMargin", (uint8_t)SendDlgItemMessage(m_hwnd, IDC_RIGHTMARGINSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "GroupIndent", (uint8_t)SendDlgItemMessage(m_hwnd, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "RowHeight", (uint8_t)SendDlgItemMessage(m_hwnd, IDC_ROWHEIGHTSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "GRowHeight", (uint8_t)SendDlgItemMessage(m_hwnd, IDC_GROUPROWHEIGHTSPIN, UDM_GETPOS, 0, 0));
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static UINT avatar_controls[] = { IDC_ALIGNMENT, IDC_AVATARSBORDER, IDC_AVATARSROUNDED, IDC_AVATARBORDERCLR, IDC_ALWAYSALIGNNICK, IDC_AVATARHEIGHT, IDC_AVATARSIZESPIN };

class CDspAdvancedDlg : public CRowItemsBaseDlg
{
	CCtrlCheck chkLocalTime, chkAvaRound, chkAvaBorder;
	CCtrlColor clrAvaBorder;

public:
	CDspAdvancedDlg() :
		CRowItemsBaseDlg(IDD_OPT_DSPADVANCED),
		chkAvaRound(this, IDC_AVATARSROUNDED),
		chkAvaBorder(this, IDC_AVATARSBORDER),
		chkLocalTime(this, IDC_SHOWLOCALTIME),
		clrAvaBorder(this, IDC_AVATARBORDERCLR)
	{
		chkAvaRound.OnChange = Callback(this, &CDspAdvancedDlg::onChange_AvatarsRounded);
		chkAvaBorder.OnChange = Callback(this, &CDspAdvancedDlg::onChange_AvatarsBorder);
		chkLocalTime.OnChange = Callback(this, &CDspAdvancedDlg::onChange_LocalTime);
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Never"));
		SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
		SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("When space allows it"));
		SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("When needed"));

		SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("With nickname - left"));
		SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Far left"));
		SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Far right"));
		SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("With nickname - right"));

		Utils::enableDlgControl(m_hwnd, IDC_CLISTAVATARS, cfg::dat.bAvatarServiceAvail);
		for (auto &ctrlId : avatar_controls)
			Utils::enableDlgControl(m_hwnd, ctrlId, cfg::dat.bAvatarServiceAvail);

		CheckDlgButton(m_hwnd, IDC_NOAVATARSOFFLINE, cfg::dat.bNoOfflineAvatars ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_SETCURSEL, cfg::dat.dualRowMode, 0);
		CheckDlgButton(m_hwnd, IDC_CLISTAVATARS, (cfg::dat.dwFlags & CLUI_FRAME_AVATARS) ? BST_CHECKED : BST_UNCHECKED);

		chkAvaBorder.SetState((cfg::dat.dwFlags & CLUI_FRAME_AVATARBORDER) != 0);
		chkAvaRound.SetState((cfg::dat.dwFlags & CLUI_FRAME_ROUNDAVATAR) != 0);
		CheckDlgButton(m_hwnd, IDC_ALWAYSALIGNNICK, (cfg::dat.dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWSTATUSMSG, (cfg::dat.dwFlags & CLUI_FRAME_SHOWSTATUSMSG) ? BST_CHECKED : BST_UNCHECKED);

		clrAvaBorder.SetColor(cfg::dat.avatarBorder);

		SendDlgItemMessage(m_hwnd, IDC_RADIUSSPIN, UDM_SETRANGE, 0, MAKELONG(10, 2));
		SendDlgItemMessage(m_hwnd, IDC_RADIUSSPIN, UDM_SETPOS, 0, cfg::dat.avatarRadius);

		SendDlgItemMessage(m_hwnd, IDC_AVATARSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 16));
		SendDlgItemMessage(m_hwnd, IDC_AVATARSIZESPIN, UDM_SETPOS, 0, cfg::dat.avatarSize);

		chkLocalTime.SetState(cfg::dat.bShowLocalTime);

		if (cfg::dat.dwFlags & CLUI_FRAME_AVATARSLEFT)
			SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_SETCURSEL, 1, 0);
		else if (cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHT)
			SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_SETCURSEL, 2, 0);
		else if (cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHTWITHNICK)
			SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_SETCURSEL, 3, 0);
		else
			SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_SETCURSEL, 0, 0);
		return true;
	}

	bool OnApply() override
	{
		LRESULT sel = SendDlgItemMessage(m_hwnd, IDC_ALIGNMENT, CB_GETCURSEL, 0, 0);
		if (sel != CB_ERR) {
			cfg::dat.dwFlags &= ~(CLUI_FRAME_AVATARSLEFT | CLUI_FRAME_AVATARSRIGHT | CLUI_FRAME_AVATARSRIGHTWITHNICK);
			if (sel == 1)
				cfg::dat.dwFlags |= CLUI_FRAME_AVATARSLEFT;
			else if (sel == 2)
				cfg::dat.dwFlags |= CLUI_FRAME_AVATARSRIGHT;
			else if (sel == 3)
				cfg::dat.dwFlags |= CLUI_FRAME_AVATARSRIGHTWITHNICK;
		}

		cfgSetFlag(m_hwnd, IDC_CLISTAVATARS, CLUI_FRAME_AVATARS);
		cfgSetFlag(m_hwnd, IDC_AVATARSBORDER, CLUI_FRAME_AVATARBORDER);
		cfgSetFlag(m_hwnd, IDC_AVATARSROUNDED, CLUI_FRAME_ROUNDAVATAR);
		cfgSetFlag(m_hwnd, IDC_ALWAYSALIGNNICK, CLUI_FRAME_ALWAYSALIGNNICK);
		cfgSetFlag(m_hwnd, IDC_SHOWSTATUSMSG, CLUI_FRAME_SHOWSTATUSMSG);

		cfg::dat.avatarBorder = clrAvaBorder.GetColor();
		db_set_dw(0, "CLC", "avatarborder", cfg::dat.avatarBorder);

		BOOL translated;
		cfg::dat.avatarRadius = GetDlgItemInt(m_hwnd, IDC_RADIUS, &translated, FALSE);
		db_set_dw(0, "CLC", "avatarradius", cfg::dat.avatarRadius);

		cfg::dat.avatarSize = GetDlgItemInt(m_hwnd, IDC_AVATARHEIGHT, &translated, FALSE);
		g_plugin.setWord("AvatarSize", (uint16_t)cfg::dat.avatarSize);

		cfg::dat.bNoOfflineAvatars = IsDlgButtonChecked(m_hwnd, IDC_NOAVATARSOFFLINE) ? TRUE : FALSE;
		g_plugin.setByte("NoOfflineAV", (uint8_t)cfg::dat.bNoOfflineAvatars);

		cfg::dat.bShowLocalTime = chkLocalTime.GetState();
		db_set_b(0, "CLC", "ShowLocalTime", (uint8_t)cfg::dat.bShowLocalTime);

		cfg::dat.bShowLocalTimeSelective = IsDlgButtonChecked(m_hwnd, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT) ? 1 : 0;
		db_set_b(0, "CLC", "SelectiveLocalTime", (uint8_t)cfg::dat.bShowLocalTimeSelective);

		KillTimer(g_clistApi.hwndContactTree, TIMERID_REFRESH);
		if (cfg::dat.bShowLocalTime)
			SetTimer(g_clistApi.hwndContactTree, TIMERID_REFRESH, 65000, nullptr);

		cfg::dat.dualRowMode = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_GETCURSEL, 0, 0);
		if (cfg::dat.dualRowMode == CB_ERR)
			cfg::dat.dualRowMode = 0;
		db_set_b(0, "CLC", "DualRowMode", cfg::dat.dualRowMode);
		return true;
	}

	void onChange_LocalTime(CCtrlCheck *)
	{
		Utils::enableDlgControl(m_hwnd, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, chkLocalTime.GetState());
	}

	void onChange_AvatarsRounded(CCtrlCheck *)
	{
		bool bEnable = chkAvaRound.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_RADIUS, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_RADIUSSPIN, bEnable);
	}

	void onChange_AvatarsBorder(CCtrlCheck *)
	{
		clrAvaBorder.Enable(chkAvaBorder.GetState());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CDspIconsDlg : public CRowItemsBaseDlg
{

public:
	CDspIconsDlg() :
		CRowItemsBaseDlg(IDD_OPT_ICONS)
	{
	}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_XSTATUSASSTATUS, (cfg::dat.dwFlags & CLUI_FRAME_USEXSTATUSASSTATUS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_SHOWSTATUSICONS, (cfg::dat.dwFlags & CLUI_FRAME_STATUSICONS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWMETA, (cfg::dat.dwFlags & CLUI_USEMETAICONS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_OVERLAYICONS, (cfg::dat.dwFlags & CLUI_FRAME_OVERLAYICONS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SELECTIVEICONS, (cfg::dat.dwFlags & CLUI_FRAME_SELECTIVEICONS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_STATUSICONSCENTERED, cfg::dat.bCenterStatusIcons ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_IDLE, db_get_b(0, "CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		cfgSetFlag(m_hwnd, IDC_SHOWSTATUSICONS, CLUI_FRAME_STATUSICONS);
		cfgSetFlag(m_hwnd, IDC_SHOWMETA, CLUI_USEMETAICONS);
		cfgSetFlag(m_hwnd, IDC_OVERLAYICONS, CLUI_FRAME_OVERLAYICONS);
		cfgSetFlag(m_hwnd, IDC_XSTATUSASSTATUS, CLUI_FRAME_USEXSTATUSASSTATUS);
		cfgSetFlag(m_hwnd, IDC_SELECTIVEICONS, CLUI_FRAME_SELECTIVEICONS);

		db_set_b(0, "CLC", "ShowIdle", IsDlgButtonChecked(m_hwnd, IDC_IDLE) ? 1 : 0);

		cfg::dat.bCenterStatusIcons = IsDlgButtonChecked(m_hwnd, IDC_STATUSICONSCENTERED) ? 1 : 0;
		db_set_b(0, "CLC", "si_centered", (uint8_t)cfg::dat.bCenterStatusIcons);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void InitClistOptions(WPARAM);
void InitSkinOptions(WPARAM);

int ClcOptInit(WPARAM wParam, LPARAM)
{
	InitClistOptions(wParam);
	InitSkinOptions(wParam);

	// Contact rows tabs
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Contact list");
	odp.szTitle.a = LPGEN("Row items");

	odp.szTab.a = LPGEN("Contacts");
	odp.pDialog = new CDspItemsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Groups and layout");
	odp.pDialog = new CDspGroupsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Advanced");
	odp.pDialog = new CDspAdvancedDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Icons");
	odp.pDialog = new CDspIconsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
