/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org),
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

static INT_PTR CALLBACK DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcViewModesSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcGenOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

struct CheckBoxToStyleEx_t {
	int id;
	DWORD flag;
	int not_t;
} static const checkBoxToStyleEx[] = {
	{ IDC_DISABLEDRAGDROP, CLS_EX_DISABLEDRAGDROP, 0 }, { IDC_NOTEDITLABELS, CLS_EX_EDITLABELS, 1 },
	{ IDC_SHOWSELALWAYS, CLS_EX_SHOWSELALWAYS, 0 }, { IDC_TRACKSELECT, CLS_EX_TRACKSELECT, 0 },
	{ IDC_DIVIDERONOFF, CLS_EX_DIVIDERONOFF, 0 }, { IDC_NOTNOTRANSLUCENTSEL, CLS_EX_NOTRANSLUCENTSEL, 1 },
	{ IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING, 1 }
};

struct CheckBoxToGroupStyleEx_t {
	int id;
	DWORD flag;
	int not_t;
} static const checkBoxToGroupStyleEx[] = {
	{ IDC_SHOWGROUPCOUNTS, CLS_EX_SHOWGROUPCOUNTS, 0 }, { IDC_HIDECOUNTSWHENEMPTY, CLS_EX_HIDECOUNTSWHENEMPTY, 0 },
	{ IDC_LINEWITHGROUPS, CLS_EX_LINEWITHGROUPS, 0 }, { IDC_QUICKSEARCHVISONLY, CLS_EX_QUICKSEARCHVISONLY, 0 },
	{ IDC_SORTGROUPSALPHA, CLS_EX_SORTGROUPSALPHA, 0 }
};

struct CheckBoxValues_t {
	DWORD style;
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

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	int i;

	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateW(values[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags = 0;
	TVITEM tvi;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2)
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

static void cfgSetFlag(HWND hwndDlg, int ctrlId, DWORD dwMask)
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

class CRowItemsBaseDlg : public CDlgBase
{
	void OnFinish(CDlgBase*)
	{
		Clist_ClcOptionsChanged();
		PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
	}

public:
	CRowItemsBaseDlg(int iDlg) :
		CDlgBase(g_plugin, iDlg)
	{
		m_OnFinishWizard = Callback(this, &CRowItemsBaseDlg::OnFinish);
	}
};

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
				cfg::dat.sortOrder[i] = (BYTE)curSel;
		}
		g_plugin.setDword("SortOrder", MAKELONG(MAKEWORD(cfg::dat.sortOrder[0], cfg::dat.sortOrder[1]), MAKEWORD(cfg::dat.sortOrder[2], 0)));

		cfg::dat.bDontSeparateOffline = IsDlgButtonChecked(m_hwnd, IDC_DONTSEPARATE) ? 1 : 0;
		g_plugin.setByte("DontSeparateOffline", (BYTE)cfg::dat.bDontSeparateOffline);

		cfgSetFlag(m_hwnd, IDC_EVENTSONTOP, CLUI_STICKYEVENTS);

		cfg::dat.bUseDCMirroring = comboAlign.GetCurSel();
		db_set_b(0, "CLC", "MirrorDC", cfg::dat.bUseDCMirroring);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

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

		DWORD exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());
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
		DWORD exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());

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
			cfg::dat.bGroupAlign = (BYTE)curSel;
			db_set_b(0, "CLC", "GroupAlign", cfg::dat.bGroupAlign);
		}

		cfg::dat.bRowSpacing = (BYTE)SendDlgItemMessage(m_hwnd, IDC_ROWGAPSPIN, UDM_GETPOS, 0, 0);
		db_set_b(0, "CLC", "RowGap", cfg::dat.bRowSpacing);

		BOOL translated;
		cfg::dat.avatarPadding = (BYTE)GetDlgItemInt(m_hwnd, IDC_AVATARPADDING, &translated, FALSE);
		g_plugin.setByte("AvatarPadding", cfg::dat.avatarPadding);

		db_set_b(0, "CLC", "LeftMargin", (BYTE)SendDlgItemMessage(m_hwnd, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "RightMargin", (BYTE)SendDlgItemMessage(m_hwnd, IDC_RIGHTMARGINSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "GroupIndent", (BYTE)SendDlgItemMessage(m_hwnd, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "RowHeight", (BYTE)SendDlgItemMessage(m_hwnd, IDC_ROWHEIGHTSPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, "CLC", "GRowHeight", (BYTE)SendDlgItemMessage(m_hwnd, IDC_GROUPROWHEIGHTSPIN, UDM_GETPOS, 0, 0));
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static UINT avatar_controls[] = { IDC_ALIGNMENT, IDC_AVATARSBORDER, IDC_AVATARSROUNDED, IDC_AVATARBORDERCLR, IDC_ALWAYSALIGNNICK, IDC_AVATARHEIGHT, IDC_AVATARSIZESPIN, 0 };

class CDspAdvancedDlg : public CRowItemsBaseDlg
{
	CCtrlCheck chkLocalTime, chkAvaRound, chkAvaBorder;

public:
	CDspAdvancedDlg() :
		CRowItemsBaseDlg(IDD_OPT_DSPADVANCED),
		chkAvaRound(this, IDC_AVATARSROUNDED),
		chkAvaBorder(this, IDC_AVATARSBORDER),
		chkLocalTime(this, IDC_SHOWLOCALTIME)
	{
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
		{
			int i = 0;
			if (cfg::dat.bAvatarServiceAvail) {
				Utils::enableDlgControl(m_hwnd, IDC_CLISTAVATARS, TRUE);
				while (avatar_controls[i] != 0)
					Utils::enableDlgControl(m_hwnd, avatar_controls[i++], TRUE);
			}
			else {
				Utils::enableDlgControl(m_hwnd, IDC_CLISTAVATARS, FALSE);
				while (avatar_controls[i] != 0)
					Utils::enableDlgControl(m_hwnd, avatar_controls[i++], FALSE);
			}
		}

		CheckDlgButton(m_hwnd, IDC_NOAVATARSOFFLINE, cfg::dat.bNoOfflineAvatars ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_SETCURSEL, cfg::dat.dualRowMode, 0);
		CheckDlgButton(m_hwnd, IDC_CLISTAVATARS, (cfg::dat.dwFlags & CLUI_FRAME_AVATARS) ? BST_CHECKED : BST_UNCHECKED);

		chkAvaBorder.SetState((cfg::dat.dwFlags & CLUI_FRAME_AVATARBORDER) != 0);
		chkAvaRound.SetState((cfg::dat.dwFlags & CLUI_FRAME_ROUNDAVATAR) != 0);
		CheckDlgButton(m_hwnd, IDC_ALWAYSALIGNNICK, (cfg::dat.dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWSTATUSMSG, (cfg::dat.dwFlags & CLUI_FRAME_SHOWSTATUSMSG) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(m_hwnd, IDC_AVATARBORDERCLR, CPM_SETCOLOUR, 0, cfg::dat.avatarBorder);

		SendDlgItemMessage(m_hwnd, IDC_RADIUSSPIN, UDM_SETRANGE, 0, MAKELONG(10, 2));
		SendDlgItemMessage(m_hwnd, IDC_RADIUSSPIN, UDM_SETPOS, 0, cfg::dat.avatarRadius);

		SendDlgItemMessage(m_hwnd, IDC_AVATARSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 16));
		SendDlgItemMessage(m_hwnd, IDC_AVATARSIZESPIN, UDM_SETPOS, 0, cfg::dat.avatarSize);

		onChange_AvatarsBorder(0);
		onChange_AvatarsRounded(0);

		chkLocalTime.SetState(cfg::dat.bShowLocalTime);
		CheckDlgButton(m_hwnd, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, cfg::dat.bShowLocalTimeSelective ? BST_CHECKED : BST_UNCHECKED);
		onChange_LocalTime(0);

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

		cfg::dat.avatarBorder = SendDlgItemMessage(m_hwnd, IDC_AVATARBORDERCLR, CPM_GETCOLOUR, 0, 0);
		db_set_dw(0, "CLC", "avatarborder", cfg::dat.avatarBorder);

		BOOL translated;
		cfg::dat.avatarRadius = GetDlgItemInt(m_hwnd, IDC_RADIUS, &translated, FALSE);
		db_set_dw(0, "CLC", "avatarradius", cfg::dat.avatarRadius);

		cfg::dat.avatarSize = GetDlgItemInt(m_hwnd, IDC_AVATARHEIGHT, &translated, FALSE);
		g_plugin.setWord("AvatarSize", (WORD)cfg::dat.avatarSize);

		cfg::dat.bNoOfflineAvatars = IsDlgButtonChecked(m_hwnd, IDC_NOAVATARSOFFLINE) ? TRUE : FALSE;
		g_plugin.setByte("NoOfflineAV", (BYTE)cfg::dat.bNoOfflineAvatars);

		cfg::dat.bShowLocalTime = IsDlgButtonChecked(m_hwnd, IDC_SHOWLOCALTIME) ? 1 : 0;
		db_set_b(0, "CLC", "ShowLocalTime", (BYTE)cfg::dat.bShowLocalTime);

		cfg::dat.bShowLocalTimeSelective = IsDlgButtonChecked(m_hwnd, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT) ? 1 : 0;
		db_set_b(0, "CLC", "SelectiveLocalTime", (BYTE)cfg::dat.bShowLocalTimeSelective);

		KillTimer(g_clistApi.hwndContactTree, TIMERID_REFRESH);
		if (cfg::dat.bShowLocalTime)
			SetTimer(g_clistApi.hwndContactTree, TIMERID_REFRESH, 65000, nullptr);

		cfg::dat.dualRowMode = (BYTE)SendDlgItemMessage(m_hwnd, IDC_DUALROWMODE, CB_GETCURSEL, 0, 0);
		if (cfg::dat.dualRowMode == CB_ERR)
			cfg::dat.dualRowMode = 0;
		db_set_b(0, "CLC", "DualRowMode", cfg::dat.dualRowMode);
		return true;
	}

	void onChange_LocalTime(CCtrlCheck *)
	{
		Utils::enableDlgControl(m_hwnd, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, IsDlgButtonChecked(m_hwnd, IDC_SHOWLOCALTIME));
	}

	void onChange_AvatarsRounded(CCtrlCheck *)
	{
		bool bEnable = chkAvaRound.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_RADIUS, bEnable);
		Utils::enableDlgControl(m_hwnd, IDC_RADIUSSPIN, bEnable);
	}

	void onChange_AvatarsBorder(CCtrlCheck *)
	{
		Utils::enableDlgControl(m_hwnd, IDC_AVATARBORDERCLR, chkAvaBorder.GetState());
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
		db_set_b(0, "CLC", "si_centered", (BYTE)cfg::dat.bCenterStatusIcons);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int opt_clc_main_changed = 0;

static INT_PTR CALLBACK DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		opt_clc_main_changed = 0;
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			DWORD exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());
			UDACCEL accel[2] = { { 0, 10 }, { 2, 50 } };
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, _countof(accel), (LPARAM)&accel);
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(0, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));

			for (auto &it : checkBoxToStyleEx)
				CheckDlgButton(hwndDlg, it.id, (exStyle & it.flag) ^ (it.flag * it.not_t) ? BST_CHECKED : BST_UNCHECKED);
		}
		CheckDlgButton(hwndDlg, IDC_FULLROWSELECT, (cfg::dat.dwFlags & CLUI_FULLROWSELECT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_DBLCLKAVATARS, cfg::dat.bDblClkAvatars ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_GREYOUT, db_get_dw(0, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS) ? BST_CHECKED : BST_UNCHECKED);
		Utils::enableDlgControl(hwndDlg, IDC_SMOOTHTIME, IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		Utils::enableDlgControl(hwndDlg, IDC_GREYOUTOPTS, IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), greyoutValues, sizeof(greyoutValues) / sizeof(greyoutValues[0]), db_get_dw(0, "CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS));
		CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, db_get_b(0, "CLC", "NoVScrollBar", 0) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_VSCROLL:
		opt_clc_main_changed = 1;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTNOSMOOTHSCROLLING)
			Utils::enableDlgControl(hwndDlg, IDC_SMOOTHTIME, IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		if (LOWORD(wParam) == IDC_GREYOUT)
			Utils::enableDlgControl(hwndDlg, IDC_GREYOUTOPTS, IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		if (LOWORD(wParam) == IDC_SMOOTHTIME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		opt_clc_main_changed = 1;
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_GREYOUTOPTS:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						opt_clc_main_changed = 1;
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
			}
			break;

		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				if (!opt_clc_main_changed)
					return TRUE;

				DWORD exStyle = db_get_dw(0, "CLC", "ExStyle", CLCDEFAULT_EXSTYLE);

				for (auto &it : checkBoxToStyleEx)
					exStyle &= ~(it.flag);

				for (auto &it : checkBoxToStyleEx)
					if ((IsDlgButtonChecked(hwndDlg, it.id) == 0) == it.not_t)
						exStyle |= it.flag;

				db_set_dw(0, "CLC", "ExStyle", exStyle);

				DWORD fullGreyoutFlags = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS));
				db_set_dw(0, "CLC", "FullGreyoutFlags", fullGreyoutFlags);
				if (IsDlgButtonChecked(hwndDlg, IDC_GREYOUT))
					db_set_dw(0, "CLC", "GreyoutFlags", fullGreyoutFlags);
				else
					db_set_dw(0, "CLC", "GreyoutFlags", 0);

				cfgSetFlag(hwndDlg, IDC_FULLROWSELECT, CLUI_FULLROWSELECT);

				db_set_w(0, "CLC", "ScrollTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "NoVScrollBar", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));
				cfg::dat.bDblClkAvatars = IsDlgButtonChecked(hwndDlg, IDC_DBLCLKAVATARS) ? TRUE : FALSE;
				db_set_b(0, "CLC", "dblclkav", (BYTE)cfg::dat.bDblClkAvatars);

				Clist_ClcOptionsChanged();
				CoolSB_SetupScrollBar();
				PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
				opt_clc_main_changed = 0;
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int opt_clc_bkg_changed = 0;

static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		opt_clc_bkg_changed = 0;
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_BITMAP, db_get_b(0, "CLC", "UseBitmap", CLCDEFAULT_USEBITMAP) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, WM_USER + 10, 0, 0);
		CheckDlgButton(hwndDlg, IDC_WINCOLOUR, db_get_b(0, "CLC", "UseWinColours", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SKINMODE, cfg::dat.bWallpaperMode ? BST_CHECKED : BST_UNCHECKED);
		{
			DBVARIANT dbv;
			if (!db_get_ws(0, "CLC", "BkBitmap", &dbv)) {
				wchar_t szPath[MAX_PATH];
				if (PathToAbsoluteW(dbv.pwszVal, szPath))
					SetDlgItemText(hwndDlg, IDC_FILENAME, szPath);

				db_free(&dbv);
			}

			WORD bmpUse = db_get_w(0, "CLC", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
			CheckDlgButton(hwndDlg, IDC_STRETCHH, bmpUse & CLB_STRETCHH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STRETCHV, bmpUse & CLB_STRETCHV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEH, bmpUse & CLBF_TILEH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEV, bmpUse & CLBF_TILEV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SCROLL, bmpUse & CLBF_SCROLL ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, bmpUse & CLBF_PROPORTIONAL ? BST_CHECKED : BST_UNCHECKED);

			SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
		}
		return TRUE;

	case WM_USER + 10:
		Utils::enableDlgControl(hwndDlg, IDC_FILENAME, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_BROWSE, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_STRETCHH, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_STRETCHV, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_TILEH, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_TILEV, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_SCROLL, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		Utils::enableDlgControl(hwndDlg, IDC_PROPORTIONAL, IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BROWSE) {
			wchar_t str[MAX_PATH], filter[512];
			GetDlgItemText(hwndDlg, IDC_FILENAME, str, _countof(str));
			Bitmap_GetFilter(filter, _countof(filter));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.hInstance = nullptr;
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = _countof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = L"bmp";
			if (!GetOpenFileName(&ofn))
				break;
			SetDlgItemText(hwndDlg, IDC_FILENAME, str);
		}
		else if (LOWORD(wParam) == IDC_FILENAME && HIWORD(wParam) != EN_CHANGE)
			break;

		if (LOWORD(wParam) == IDC_BITMAP)
			SendMessage(hwndDlg, WM_USER + 10, 0, 0);
		if (LOWORD(wParam) == IDC_WINCOLOUR)
			SendMessage(hwndDlg, WM_USER + 11, 0, 0);
		if (LOWORD(wParam) == IDC_FILENAME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		opt_clc_bkg_changed = 1;
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				if (!opt_clc_bkg_changed)
					return TRUE;

				db_set_b(0, "CLC", "UseBitmap", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BITMAP));
				db_set_b(0, "CLC", "UseWinColours", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_WINCOLOUR));

				char str[MAX_PATH], strrel[MAX_PATH];
				GetDlgItemTextA(hwndDlg, IDC_FILENAME, str, _countof(str));
				if (PathToRelative(str, strrel))
					db_set_s(0, "CLC", "BkBitmap", strrel);
				else
					db_set_s(0, "CLC", "BkBitmap", str);

				WORD flags = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHH))
					flags |= CLB_STRETCHH;
				if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHV))
					flags |= CLB_STRETCHV;
				if (IsDlgButtonChecked(hwndDlg, IDC_TILEH))
					flags |= CLBF_TILEH;
				if (IsDlgButtonChecked(hwndDlg, IDC_TILEV))
					flags |= CLBF_TILEV;
				if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL))
					flags |= CLBF_SCROLL;
				if (IsDlgButtonChecked(hwndDlg, IDC_PROPORTIONAL))
					flags |= CLBF_PROPORTIONAL;
				db_set_w(0, "CLC", "BkBmpUse", flags);
				cfg::dat.bWallpaperMode = IsDlgButtonChecked(hwndDlg, IDC_SKINMODE) ? 1 : 0;
				db_set_b(0, "CLUI", "UseBkSkin", (BYTE)cfg::dat.bWallpaperMode);

				Clist_ClcOptionsChanged();
				PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
				opt_clc_bkg_changed = 0;
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

int ClcOptInit(WPARAM wParam, LPARAM)
{
	////////////////////////////////////////////////////////////////////////////
	// Main options tabs

	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1000000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Contact list");
	odp.szTab.a = LPGEN("General");
	odp.pfnDlgProc = DlgProcGenOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLIST);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("List layout");
	odp.pfnDlgProc = DlgProcClcMainOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLC);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Window");
	odp.pfnDlgProc = DlgProcCluiOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLUI);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Background");
	odp.pfnDlgProc = DlgProcClcBkgOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLCBKG);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Status bar");
	odp.pfnDlgProc = DlgProcSBarOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SBAR);
	g_plugin.addOptions(wParam, &odp);

	////////////////////////////////////////////////////////////////////////////
	// Other options

	odp.position = 0;
	odp.szGroup.a = LPGEN("Skins");
	odp.szTitle.a = LPGEN("Contact list");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.pfnDlgProc = OptionsDlgProc;
	g_plugin.addOptions(wParam, &odp);

	////////////////////////////////////////////////////////////////////////////
	// Contact rows tabs

	odp.szGroup.a = LPGEN("Contact list");
	odp.szTitle.a = LPGEN("Row items");

	odp.pfnDlgProc = 0;
	odp.pszTemplate = 0;
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
