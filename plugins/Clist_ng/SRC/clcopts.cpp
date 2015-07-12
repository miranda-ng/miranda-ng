/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_n plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: clcopts.cpp 136 2010-10-12 18:55:36Z silvercircle $
 *
 */

#include <commonheaders.h>
#include "../coolsb/coolscroll.h"

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

struct CheckBoxToStyleEx_t {
	int id;
	DWORD flag;
	int not_t;
} static const checkBoxToStyleEx[] = {
	{IDC_DISABLEDRAGDROP, CLS_EX_DISABLEDRAGDROP, 0}, {IDC_NOTEDITLABELS, CLS_EX_EDITLABELS, 1},
	{IDC_SHOWSELALWAYS, CLS_EX_SHOWSELALWAYS, 0}, {IDC_TRACKSELECT, CLS_EX_TRACKSELECT, 0},
	{IDC_DIVIDERONOFF, CLS_EX_DIVIDERONOFF, 0}, {IDC_NOTNOTRANSLUCENTSEL, CLS_EX_NOTRANSLUCENTSEL, 1},
	{IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING, 1}
};

struct CheckBoxToGroupStyleEx_t {
	int id;
	DWORD flag;
	int not_t;
} static const checkBoxToGroupStyleEx[] = {
	{IDC_SHOWGROUPCOUNTS, CLS_EX_SHOWGROUPCOUNTS, 0}, {IDC_HIDECOUNTSWHENEMPTY, CLS_EX_HIDECOUNTSWHENEMPTY, 0},
	{IDC_LINEWITHGROUPS, CLS_EX_LINEWITHGROUPS, 0}, {IDC_QUICKSEARCHVISONLY, CLS_EX_QUICKSEARCHVISONLY, 0},
	{IDC_SORTGROUPSALPHA, CLS_EX_SORTGROUPSALPHA, 0}
};

struct CheckBoxValues_t {
	DWORD style;
	wchar_t *szDescr;
};

static const struct CheckBoxValues_t greyoutValues[] = {
	{ GREYF_UNFOCUS,  LPGENT("Not focused")},
	{ MODEF_OFFLINE,  LPGENT("Offline")},
	{ PF2_ONLINE,     LPGENT("Online")},
	{ PF2_SHORTAWAY,  LPGENT("Away")},
	{ PF2_LONGAWAY,   LPGENT("NA")},
	{ PF2_LIGHTDND,   LPGENT("Occupied")},
	{ PF2_HEAVYDND,   LPGENT("DND")},
	{ PF2_FREECHAT,   LPGENT("Free for chat")},
	{ PF2_INVISIBLE,  LPGENT("Invisible")},
	{ PF2_OUTTOLUNCH, LPGENT("Out to lunch")},
	{ PF2_ONTHEPHONE, LPGENT("On the phone")}
};

static const struct CheckBoxValues_t offlineValues[] = {
	{ MODEF_OFFLINE,  LPGENT("Offline")},
	{ PF2_ONLINE,     LPGENT("Online")},
	{ PF2_SHORTAWAY,  LPGENT("Away")},
	{ PF2_LONGAWAY,   LPGENT("NA")},
	{ PF2_LIGHTDND,   LPGENT("Occupied")},
	{ PF2_HEAVYDND,   LPGENT("DND")},
	{ PF2_FREECHAT,   LPGENT("Free for chat")},
	{ PF2_INVISIBLE,  LPGENT("Invisible")},
	{ PF2_OUTTOLUNCH, LPGENT("Out to lunch")},
	{ PF2_ONTHEPHONE, LPGENT("On the phone")}
};

static HIMAGELIST himlCheckBoxes = 0;

static HIMAGELIST CreateStateImageList()
{
	if(0 == himlCheckBoxes) {
		HICON hIconNoTick = Skin_LoadIcon(SKINICON_OTHER_NOTICK);
		HICON hIconTick = Skin_LoadIcon(SKINICON_OTHER_TICK);

		himlCheckBoxes = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
		ImageList_AddIcon(himlCheckBoxes, hIconNoTick);
		ImageList_AddIcon(himlCheckBoxes, hIconTick);
		ImageList_AddIcon(himlCheckBoxes, hIconNoTick);
	}
	return(himlCheckBoxes);
}


static UINT sortCtrlIDs[] = {IDC_SORTPRIMARY, IDC_SORTTHEN, IDC_SORTFINALLY, 0};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	int i;

	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for(i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateTS(values[i].szDescr);
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
	while(tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if(((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return(flags);
}

/*
 * load current values into the given profile
 */

void DSP_LoadFromDefaults(TDisplayProfile *p)
{
	int         i;
	DBVARIANT   dbv = {0};
	char        szKey[5];

	if (0 == p)
		return;

	p->dwExtraImageMask = cfg::dat.dwExtraImageMask;
	p->exIconScale = cfg::dat.exIconScale;
	p->bCenterStatusIcons = cfg::dat.bCenterStatusIcons;
	p->dwFlags = cfg::dat.dwFlags;
	p->bDimIdle = cfg::getByte("CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE);
	p->avatarBorder = cfg::dat.avatarBorder;
	p->avatarSize = cfg::dat.avatarSize;
	p->dualRowMode = cfg::dat.dualRowMode;
	p->bNoOfflineAvatars = cfg::dat.bNoOfflineAvatars;
	p->bShowLocalTime = cfg::dat.bShowLocalTime;
	p->bShowLocalTimeSelective = cfg::dat.bShowLocalTimeSelective;
	p->clcExStyle = cfg::getDword("CLC", "ExStyle", pcli->pfnGetDefaultExStyle());
	p->clcOfflineModes = cfg::getDword("CLC", "OfflineModes", CLCDEFAULT_OFFLINEMODES);
	p->bDontSeparateOffline = cfg::dat.bDontSeparateOffline;
	p->sortOrder[0] = cfg::dat.sortOrder[0];
	p->sortOrder[1] = cfg::dat.sortOrder[1];
	p->sortOrder[2] = cfg::dat.sortOrder[2];
	p->bUseDCMirroring = cfg::dat.bUseDCMirroring;
	p->bCenterGroupNames = cfg::getByte("CLCExt", "EXBK_CenterGroupnames", 0);
	p->bGroupAlign = cfg::dat.bGroupAlign;
	p->avatarPadding = cfg::dat.avatarPadding;

	p->bLeftMargin = cfg::getByte("CLC", "LeftMargin", CLCDEFAULT_LEFTMARGIN);
	p->bRightMargin =  cfg::getByte("CLC", "RightMargin", CLCDEFAULT_LEFTMARGIN);
	p->bRowSpacing = cfg::dat.bRowSpacing;
	p->bGroupIndent = cfg::getByte("CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT);
	p->bRowHeight = cfg::getByte("CLC", "RowHeight", CLCDEFAULT_ROWHEIGHT);
	p->bGroupRowHeight = cfg::getByte("CLC", "GRowHeight", CLCDEFAULT_ROWHEIGHT);
	CopyMemory(p->exIconOrder, cfg::dat.exIconOrder, EXICON_COUNT);

	for(i = 0; i < NR_DSPOVERRIDES; i++) {
		mir_snprintf(szKey, 3, "%d", i);
		ZeroMemory(&p->dspOverride[i], sizeof(TDspOverride));
		if(0 == db_get(0, DSP_PROFILES_MODULE, szKey, &dbv)) {
			if(dbv.type == DBVT_BLOB && dbv.cpbVal > 0 && dbv.cpbVal <= sizeof(TDspOverride))
				CopyMemory((void *)&p->dspOverride[i], (void *)dbv.pbVal, dbv.cpbVal);
			db_free(&dbv);
		}
	}
}

/*
 * apply a display profile
 */

void DSP_Apply(TDisplayProfile *p)
{
	int   oldexIconScale = cfg::dat.exIconScale;
	int   i;
	DWORD exStyle;
	char  temp[EXICON_COUNT + 1], szKey[5];
	/*
	 * icons page
	 */
	cfg::dat.dwFlags &= ~(CLUI_FRAME_STATUSICONS | CLUI_SHOWVISI | CLUI_FRAME_OVERLAYICONS | CLUI_FRAME_SELECTIVEICONS);
	cfg::dat.dwExtraImageMask = p->dwExtraImageMask;
	cfg::dat.exIconScale = p->exIconScale;
	cfg::dat.bCenterStatusIcons = p->bCenterStatusIcons;

	cfg::writeDword("CLUI", "ximgmask", cfg::dat.dwExtraImageMask);
	cfg::writeByte("CLC", "ExIconScale", (BYTE)cfg::dat.exIconScale);
	cfg::writeByte("CLC", "si_centered", (BYTE)cfg::dat.bCenterStatusIcons);
	cfg::writeByte("CLC", "ShowIdle", (BYTE)p->bDimIdle);

	CopyMemory(cfg::dat.exIconOrder, p->exIconOrder, EXICON_COUNT);
	CopyMemory(temp, p->exIconOrder, EXICON_COUNT);
	temp[EXICON_COUNT] = 0;
	cfg::writeString(NULL, "CLUI", "exIconOrder", temp);

	/*
	 * advanced (avatars & 2nd row)
	 */

	cfg::dat.dwFlags &= ~(CLUI_FRAME_AVATARSLEFT | CLUI_FRAME_AVATARSRIGHT | CLUI_FRAME_AVATARSRIGHTWITHNICK |
						  CLUI_FRAME_AVATARS |
						  CLUI_FRAME_ALWAYSALIGNNICK | CLUI_FRAME_SHOWSTATUSMSG | CLUI_FRAME_AVATARBORDER);

	cfg::dat.avatarSize = p->avatarSize;
	cfg::dat.avatarBorder = p->avatarBorder;
	cfg::dat.dualRowMode = p->dualRowMode;
	cfg::dat.bNoOfflineAvatars = p->bNoOfflineAvatars;
	cfg::dat.bShowLocalTime = p->bShowLocalTime;
	cfg::dat.bShowLocalTimeSelective = p->bShowLocalTimeSelective;

	if(cfg::dat.hBrushAvatarBorder)
		DeleteObject(cfg::dat.hBrushAvatarBorder);
	cfg::dat.hBrushAvatarBorder = CreateSolidBrush(cfg::dat.avatarBorder);

	/*
	 * items page
	 */

	cfg::dat.dwFlags &= ~CLUI_STICKYEVENTS;

	cfg::dat.sortOrder[0] = p->sortOrder[0];
	cfg::dat.sortOrder[1] = p->sortOrder[1];
	cfg::dat.sortOrder[2] = p->sortOrder[2];
	cfg::dat.bDontSeparateOffline = p->bDontSeparateOffline;
	cfg::writeByte("CList", "DontSeparateOffline", (BYTE)cfg::dat.bDontSeparateOffline);
	cfg::writeDword("CLC", "OfflineModes", p->clcOfflineModes);

	cfg::writeDword("CList", "SortOrder",
					MAKELONG(MAKEWORD(cfg::dat.sortOrder[0], cfg::dat.sortOrder[1]),
							 MAKEWORD(cfg::dat.sortOrder[2], 0)));

	cfg::dat.bUseDCMirroring = p->bUseDCMirroring;
	cfg::writeByte("CLC", "MirrorDC", cfg::dat.bUseDCMirroring);

	/*
	 * groups page
	 */

	cfg::dat.dwFlags &= ~CLUI_FRAME_NOGROUPICON;
	cfg::dat.bGroupAlign = p->bGroupAlign;
	cfg::writeByte("CLC", "GroupAlign", cfg::dat.bGroupAlign);
	cfg::writeByte("CLCExt", "EXBK_CenterGroupnames", (BYTE)p->bCenterGroupNames);

	exStyle = cfg::getDword("CLC", "ExStyle", pcli->pfnGetDefaultExStyle());
	for(i = 0; i < sizeof(checkBoxToGroupStyleEx) / sizeof(checkBoxToGroupStyleEx[0]); i++)
		exStyle &= ~(checkBoxToGroupStyleEx[i].flag);

	exStyle |= p->clcExStyle;
	cfg::writeDword("CLC", "ExStyle", exStyle);
	cfg::dat.avatarPadding = p->avatarPadding;
	cfg::writeByte("CList", "AvatarPadding", cfg::dat.avatarPadding);

	cfg::dat.bRowSpacing = p->bRowSpacing;
	cfg::writeByte("CLC", "RowGap", cfg::dat.bRowSpacing);

	cfg::writeByte("CLC", "LeftMargin", (BYTE)p->bLeftMargin);
	cfg::writeByte("CLC", "RightMargin", (BYTE)p->bRightMargin);
	cfg::writeByte("CLC", "GroupIndent", (BYTE)p->bGroupIndent);
	cfg::writeByte("CLC", "RowHeight", (BYTE)p->bRowHeight);
	cfg::writeByte("CLC", "GRowHeight", (BYTE)p->bGroupRowHeight);

	if(cfg::dat.sortOrder[0] == SORTBY_LASTMSG || cfg::dat.sortOrder[1] == SORTBY_LASTMSG || cfg::dat.sortOrder[2] == SORTBY_LASTMSG) {
		int i;

		for(i = 0; i < cfg::nextCacheEntry; i++)
			cfg::eCache[i].dwLastMsgTime = INTSORT_GetLastMsgTime(cfg::eCache[i].hContact);
	}

	cfg::writeByte("CLC", "ShowLocalTime", (BYTE)cfg::dat.bShowLocalTime);
	cfg::writeByte("CLC", "SelectiveLocalTime", (BYTE)cfg::dat.bShowLocalTimeSelective);
	cfg::writeDword("CLC", "avatarborder", cfg::dat.avatarBorder);
	cfg::writeWord(NULL, "CList", "AvatarSize", (WORD)cfg::dat.avatarSize);
	cfg::writeByte("CLC", "DualRowMode", cfg::dat.dualRowMode);
	cfg::writeByte("CList", "NoOfflineAV", (BYTE)cfg::dat.bNoOfflineAvatars);

	KillTimer(pcli->hwndContactTree, TIMERID_REFRESH);
	if(cfg::dat.bShowLocalTime)
		SetTimer(pcli->hwndContactTree, TIMERID_REFRESH, 65000, NULL);

	cfg::dat.dwFlags |= p->dwFlags;
	cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);

	for(i = 0; i < cfg::nextCacheEntry; i++)
		cfg::eCache[i].dwXMask = CalcXMask(cfg::eCache[i].hContact);

	if(oldexIconScale != cfg::dat.exIconScale) {
		ImageList_RemoveAll(CLUI::hExtraImages);
		ImageList_SetIconSize(CLUI::hExtraImages, cfg::dat.exIconScale, cfg::dat.exIconScale);
		IcoLibReloadIcons();
	}
	pcli->pfnClcOptionsChanged();
	pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);

	for(i = 0; i < NR_DSPOVERRIDES; i++) {
		mir_snprintf(szKey, 3, "%d", i);
		db_set_blob(0, DSP_PROFILES_MODULE, szKey, (void *)&p->dspOverride[i], sizeof(TDspOverride));
	}

	CopyMemory(&CLC::dsp_default, p, sizeof(TDisplayProfile));
}

void GetDefaultFontSetting(int i, LOGFONTW *lf, COLORREF *colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, FALSE);
	*colour = GetSysColor(COLOR_WINDOWTEXT);
	switch(i) {
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

INT_PTR CALLBACK cfg::DlgProcDspGroups(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG: {
			TranslateDialogDefault(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always Left"));
			SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always Right"));
			SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Automatic (RTL)"));
			return(TRUE);
		}
		case WM_COMMAND:
			if((LOWORD(wParam) == IDC_ROWHEIGHT || LOWORD(wParam) == IDC_AVATARPADDING || LOWORD(wParam) == IDC_ROWGAP || LOWORD(wParam) == IDC_RIGHTMARGIN || LOWORD(wParam) == IDC_LEFTMARGIN || LOWORD(wParam) == IDC_SMOOTHTIME || LOWORD(wParam) == IDC_GROUPINDENT || LOWORD(wParam) == IDC_GROUPROWHEIGHT)
					&& (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
				return(0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_USER + 100: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				DWORD exStyle = p->clcExStyle;
				int   i;
				for(i = 0; i < sizeof(checkBoxToGroupStyleEx) / sizeof(checkBoxToGroupStyleEx[0]); i++)
					CheckDlgButton(hwndDlg, checkBoxToGroupStyleEx[i].id, (exStyle & checkBoxToGroupStyleEx[i].flag) ^(checkBoxToGroupStyleEx[i].flag * checkBoxToGroupStyleEx[i].not_t) ? BST_CHECKED : BST_UNCHECKED);

				CheckDlgButton(hwndDlg, IDC_NOGROUPICON, (p->dwFlags & CLUI_FRAME_NOGROUPICON) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CENTERGROUPNAMES, p->bCenterGroupNames);
				SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_SETCURSEL, p->bGroupAlign, 0);
				SendDlgItemMessage(hwndDlg, IDC_AVATARPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
				SendDlgItemMessage(hwndDlg, IDC_AVATARPADDINGSPIN, UDM_SETPOS, 0, p->avatarPadding);

				SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
				SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, p->bLeftMargin);
				SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(64, 0));
				SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, p->bRightMargin);
				SendDlgItemMessage(hwndDlg, IDC_ROWGAPSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
				SendDlgItemMessage(hwndDlg, IDC_ROWGAPSPIN, UDM_SETPOS, 0, p->bRowSpacing);
				SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
				SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETPOS, 0, p->bGroupIndent);
				SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 8));
				SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_SETPOS, 0, p->bRowHeight);
				SendDlgItemMessage(hwndDlg, IDC_GROUPROWHEIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 8));
				SendDlgItemMessage(hwndDlg, IDC_GROUPROWHEIGHTSPIN, UDM_SETPOS, 0, p->bGroupRowHeight);
			}
			return(0);
		}
		case WM_USER + 200: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				int i;
				DWORD exStyle = 0;
				LRESULT curSel;
				BOOL    translated;

				for(i = 0; i < sizeof(checkBoxToGroupStyleEx) / sizeof(checkBoxToGroupStyleEx[0]); i++) {
					if((IsDlgButtonChecked(hwndDlg, checkBoxToGroupStyleEx[i].id) == 0) == checkBoxToGroupStyleEx[i].not_t)
						exStyle |= checkBoxToGroupStyleEx[i].flag;
				}
				p->clcExStyle = exStyle;
				p->dwFlags |= (IsDlgButtonChecked(hwndDlg, IDC_NOGROUPICON) ? CLUI_FRAME_NOGROUPICON : 0);
				p->bCenterGroupNames = IsDlgButtonChecked(hwndDlg, IDC_CENTERGROUPNAMES) ? 1 : 0;
				curSel = SendDlgItemMessage(hwndDlg, IDC_GROUPALIGN, CB_GETCURSEL, 0, 0);
				if(curSel != CB_ERR)
					p->bGroupAlign = (BYTE)curSel;

				p->avatarPadding = (BYTE)GetDlgItemInt(hwndDlg, IDC_AVATARPADDING, &translated, FALSE);
				p->bLeftMargin = (BYTE)SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0);
				p->bRightMargin = (BYTE)SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_GETPOS, 0, 0);
				p->bRowSpacing = (BYTE)SendDlgItemMessage(hwndDlg, IDC_ROWGAPSPIN, UDM_GETPOS, 0, 0);
				p->bGroupIndent = (BYTE)SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0);
				p->bRowHeight = (BYTE)SendDlgItemMessage(hwndDlg, IDC_ROWHEIGHTSPIN, UDM_GETPOS, 0, 0);
				p->bGroupRowHeight = (BYTE)SendDlgItemMessage(hwndDlg, IDC_GROUPROWHEIGHTSPIN, UDM_GETPOS, 0, 0);
			}
			return(0);
		}
		case WM_NOTIFY:
			switch(((LPNMHDR) lParam)->idFrom) {
				case 0:
					switch(((LPNMHDR) lParam)->code) {
						case PSN_APPLY: {
							return(TRUE);
						}
					}
					break;
			}
			break;
	}
	return(FALSE);
}

INT_PTR CALLBACK cfg::DlgProcDspItems(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG: {
			int i = 0;
			TranslateDialogDefault(hwndDlg);
			SetWindowLong(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE,
						  GetWindowLong(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

			for(i = 0; sortCtrlIDs[i] != 0; i++) {
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Nothing"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Name"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Protocol"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Status"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Last Message"));
				SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_INSERTSTRING, -1, (LPARAM)TranslateT("Message Frequency"));
			}
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Never"));
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("For RTL only"));
			SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_INSERTSTRING, -1, (LPARAM)TranslateT("RTL TEXT only"));
			return(TRUE);
		}
		case WM_COMMAND:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_USER + 100: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				int i;
				FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), offlineValues, sizeof(offlineValues) / sizeof(offlineValues[0]), p->clcOfflineModes);
				CheckDlgButton(hwndDlg, IDC_EVENTSONTOP, (p->dwFlags & CLUI_STICKYEVENTS) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_DONTSEPARATE, p->bDontSeparateOffline);
				for(i = 0; sortCtrlIDs[i] != 0; i++)
					SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_SETCURSEL, p->sortOrder[i], 0);

				SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_SETCURSEL, p->bUseDCMirroring, 0);
			}
			return(0);
		}
		case WM_USER + 200: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				int     i;
				LRESULT curSel;

				for(i = 0; sortCtrlIDs[i] != 0; i++) {
					curSel = SendDlgItemMessage(hwndDlg, sortCtrlIDs[i], CB_GETCURSEL, 0, 0);
					if(curSel == 0 || curSel == CB_ERR)
						p->sortOrder[i] = 0;
					else
						p->sortOrder[i]	= (BYTE)curSel;
				}
				p->bDontSeparateOffline = IsDlgButtonChecked(hwndDlg, IDC_DONTSEPARATE) ? 1 : 0;
				p->dwFlags |= IsDlgButtonChecked(hwndDlg, IDC_EVENTSONTOP) ? CLUI_STICKYEVENTS : 0;
				p->clcOfflineModes = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS));
				p->bUseDCMirroring = (BYTE)SendDlgItemMessage(hwndDlg, IDC_CLISTALIGN, CB_GETCURSEL, 0, 0);
			}
			return(0);
		}
		case WM_NOTIFY:
			switch(((LPNMHDR) lParam)->idFrom) {
				case IDC_HIDEOFFLINEOPTS:
					if(((LPNMHDR) lParam)->code == NM_CLICK) {
						TVHITTESTINFO hti;
						hti.pt.x = (short) LOWORD(GetMessagePos());
						hti.pt.y = (short) HIWORD(GetMessagePos());
						ScreenToClient(((LPNMHDR) lParam)->hwndFrom, &hti.pt);
						if(TreeView_HitTest(((LPNMHDR) lParam)->hwndFrom, &hti))
							if(hti.flags & TVHT_ONITEMSTATEICON) {
								TVITEM tvi;
								tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
								tvi.hItem = hti.hItem;
								TreeView_GetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
								tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
								TreeView_SetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
					}
					break;
				case 0:
					switch(((LPNMHDR) lParam)->code) {
						case PSN_APPLY: {
							return(TRUE);
						}
					}
					break;
			}
			break;
		case WM_DESTROY: {
			ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), TVSIL_STATE));
			break;
		}
		break;
	}
	return(FALSE);
}

static UINT avatar_controls[] = { IDC_ALIGNMENT, IDC_AVATARSBORDER, IDC_AVATARBORDERCLR, IDC_ALWAYSALIGNNICK, IDC_AVATARHEIGHT, IDC_AVATARSIZESPIN, 0};

INT_PTR CALLBACK cfg::DlgProcDspClasses(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_INSERTSTRING, -1, (LPARAM)TranslateW(L"Offline contacts"));
			SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_INSERTSTRING, -1, (LPARAM)TranslateW(L"Online contacts"));
			SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_INSERTSTRING, -1, (LPARAM)TranslateW(L"Selected contacts"));
			SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_INSERTSTRING, -1, (LPARAM)TranslateW(L"Hovered contacts"));
			SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_INSERTSTRING, -1, (LPARAM)TranslateW(L"Priority contacts"));
			SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_SETCURSEL, 0, 0);
			break;

			/*
			 * sent from the tab parent to init settings
			 */
		case WM_USER + 100: {
			TDisplayProfile* p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, reinterpret_cast<INT_PTR>(p));
				SendMessage(hwndDlg, WM_USER + 300, 0, 0);

				CheckDlgButton(hwndDlg, IDC_DSPCLASS_ENABLE_SELECTED, p->dspOverride[DSP_OVR_SELECTED].fActive ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_DSPCLASS_ENABLE_HOVERED, p->dspOverride[DSP_OVR_HOVERED].fActive ? BST_CHECKED : BST_UNCHECKED);
			}
			return(0);
		}

		/*
		 * sent from the tab parent to collect settings
		 */
		case WM_USER + 200: {
			TDisplayProfile* pNew = reinterpret_cast<TDisplayProfile *>(lParam);
			TDisplayProfile* p = reinterpret_cast<TDisplayProfile *>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			CopyMemory(&pNew->dspOverride[0], &p->dspOverride[0], sizeof(TDspOverride) * NR_DSPOVERRIDES);
			return(0);
		}

		/*
		 * internal - init settings based on the selected override identifier
		 * (stored in IDC_DSPCLASS)
		 */
		case WM_USER + 300: {
			TDisplayProfile* p = reinterpret_cast<TDisplayProfile *>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			if(p) {
				LRESULT uIndex = SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_GETCURSEL, 0, 0);
				if(uIndex >= 0 && uIndex < NR_DSPOVERRIDES) {
					TDspOverride* o = &p->dspOverride[uIndex];
					CheckDlgButton(hwndDlg, IDC_DSPCLASS_AVATAR, o->bAvatar == 0 ? BST_INDETERMINATE : (o->bAvatar == 1 ? BST_CHECKED : BST_UNCHECKED));
					CheckDlgButton(hwndDlg, IDC_DSPCLASS_SECONDLINE, o->bSecondLine == 0 ? BST_INDETERMINATE : (o->bSecondLine == 1 ? BST_CHECKED : BST_UNCHECKED));
					CheckDlgButton(hwndDlg, IDC_DSPCLASS_ICON, o->bIcon == 0 ? BST_INDETERMINATE : (o->bIcon == 1 ? BST_CHECKED : BST_UNCHECKED));
				}
			}
			return(0);
		}

		case WM_COMMAND: {
			if(IDC_DSPCLASS == LOWORD(wParam) && CBN_SELCHANGE == HIWORD(wParam)) {
				SendMessage(hwndDlg, WM_USER + 300, 0, 0);
				break;
			}
			switch(LOWORD(wParam)) {
				case IDC_DSPCLASS_AVATAR:
				case IDC_DSPCLASS_ICON:
				case IDC_DSPCLASS_SECONDLINE:
				case IDC_DSPCLASS_ENABLE_SELECTED:
				case IDC_DSPCLASS_ENABLE_HOVERED: {
					TDisplayProfile* p = reinterpret_cast<TDisplayProfile *>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
					if(p) {
						LRESULT uIndex = SendDlgItemMessage(hwndDlg, IDC_DSPCLASS, CB_GETCURSEL, 0, 0);
						if(uIndex >= 0 && uIndex < NR_DSPOVERRIDES) {
							LRESULT r;
							TDspOverride* o = &p->dspOverride[uIndex];
							r = IsDlgButtonChecked(hwndDlg, IDC_DSPCLASS_AVATAR);
							o->bAvatar = (r == BST_INDETERMINATE ? 0 : (r == BST_CHECKED ? 1 : -1));
							r = IsDlgButtonChecked(hwndDlg, IDC_DSPCLASS_ICON);
							o->bIcon = (r == BST_INDETERMINATE ? 0 : (r == BST_CHECKED ? 1 : -1));
							r = IsDlgButtonChecked(hwndDlg, IDC_DSPCLASS_SECONDLINE);
							o->bSecondLine = (r == BST_INDETERMINATE ? 0 : (r == BST_CHECKED ? 1 : -1));
						}
						p->dspOverride[DSP_OVR_HOVERED].fActive = (IsDlgButtonChecked(hwndDlg, IDC_DSPCLASS_ENABLE_HOVERED) ? true : false);
						p->dspOverride[DSP_OVR_SELECTED].fActive = (IsDlgButtonChecked(hwndDlg, IDC_DSPCLASS_ENABLE_SELECTED) ? true : false);
					}
					break;
				}

				default:
					break;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		default:
			break;
	}
	return(FALSE);
}

INT_PTR CALLBACK cfg::DlgProcDspAdvanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG: {
			int i = 0;

			TranslateDialogDefault(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Never"));
			SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
			SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("When needed"));

			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("With Nickname - left"));
			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Far left"));
			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Far right"));
			SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("With Nickname - right"));

			Utils::enableDlgControl(hwndDlg, IDC_CLISTAVATARS, TRUE);
			while(avatar_controls[i] != 0)
				Utils::enableDlgControl(hwndDlg, avatar_controls[i++], TRUE);
			return(TRUE);
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_CLISTAVATARS:
					if((HWND)lParam != GetFocus())
						return(0);
					break;
				case IDC_SHOWLOCALTIME:
					Utils::enableDlgControl(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIME));
					break;
				case IDC_AVATARSBORDER:
					Utils::enableDlgControl(hwndDlg, IDC_AVATARBORDERCLR, IsDlgButtonChecked(hwndDlg, IDC_AVATARSBORDER) ? TRUE : FALSE);
					break;
			}
			if((LOWORD(wParam) == IDC_AVATARHEIGHT) && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
				return(0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_USER + 100: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				CheckDlgButton(hwndDlg, IDC_NOAVATARSOFFLINE, p->bNoOfflineAvatars);
				SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_SETCURSEL, (WPARAM)p->dualRowMode, 0);
				CheckDlgButton(hwndDlg, IDC_CLISTAVATARS, (p->dwFlags & CLUI_FRAME_AVATARS) ? BST_CHECKED : BST_UNCHECKED);

				CheckDlgButton(hwndDlg, IDC_AVATARSBORDER, (p->dwFlags & CLUI_FRAME_AVATARBORDER) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_ALWAYSALIGNNICK, (p->dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_SHOWSTATUSMSG, (p->dwFlags & CLUI_FRAME_SHOWSTATUSMSG) ? BST_CHECKED : BST_UNCHECKED);

				SendDlgItemMessage(hwndDlg, IDC_AVATARBORDERCLR, CPM_SETCOLOUR, 0, p->avatarBorder);

				SendDlgItemMessage(hwndDlg, IDC_AVATARSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 16));
				SendDlgItemMessage(hwndDlg, IDC_AVATARSIZESPIN, UDM_SETPOS, 0, p->avatarSize);

				Utils::enableDlgControl(hwndDlg, IDC_AVATARBORDERCLR, IsDlgButtonChecked(hwndDlg, IDC_AVATARSBORDER) ? TRUE : FALSE);

				CheckDlgButton(hwndDlg, IDC_SHOWLOCALTIME, p->bShowLocalTime ? 1 : 0);
				CheckDlgButton(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, p->bShowLocalTimeSelective ? 1 : 0);
				Utils::enableDlgControl(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT, IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIME));

				if(p->dwFlags & CLUI_FRAME_AVATARSLEFT)
					SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 1, 0);
				else if(p->dwFlags & CLUI_FRAME_AVATARSRIGHT)
					SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 2, 0);
				else if(p->dwFlags & CLUI_FRAME_AVATARSRIGHTWITHNICK)
					SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 3, 0);
				else
					SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_SETCURSEL, 0, 0);
			}
			return(0);
		}
		case WM_USER + 200: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				LRESULT sel = SendDlgItemMessage(hwndDlg, IDC_ALIGNMENT, CB_GETCURSEL, 0, 0);
				BOOL    translated;

				if(sel != CB_ERR) {
					if(sel == 1)
						p->dwFlags |= CLUI_FRAME_AVATARSLEFT;
					else if(sel == 2)
						p->dwFlags |= CLUI_FRAME_AVATARSRIGHT;
					else if(sel == 3)
						p->dwFlags |= CLUI_FRAME_AVATARSRIGHTWITHNICK;
				}

				p->dwFlags |= ((IsDlgButtonChecked(hwndDlg, IDC_CLISTAVATARS) ? CLUI_FRAME_AVATARS : 0) |
							   (IsDlgButtonChecked(hwndDlg, IDC_AVATARSBORDER) ? CLUI_FRAME_AVATARBORDER : 0) |
							   (IsDlgButtonChecked(hwndDlg, IDC_ALWAYSALIGNNICK) ? CLUI_FRAME_ALWAYSALIGNNICK : 0) |
							   (IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUSMSG) ? CLUI_FRAME_SHOWSTATUSMSG : 0));

				p->avatarBorder = SendDlgItemMessage(hwndDlg, IDC_AVATARBORDERCLR, CPM_GETCOLOUR, 0, 0);
				p->avatarSize = GetDlgItemInt(hwndDlg, IDC_AVATARHEIGHT, &translated, FALSE);
				p->bNoOfflineAvatars = IsDlgButtonChecked(hwndDlg, IDC_NOAVATARSOFFLINE) ? TRUE : FALSE;
				p->bShowLocalTime = IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIME) ? 1 : 0;
				p->bShowLocalTimeSelective = IsDlgButtonChecked(hwndDlg, IDC_SHOWLOCALTIMEONLYWHENDIFFERENT) ? 1 : 0;

				p->dualRowMode = (BYTE)SendDlgItemMessage(hwndDlg, IDC_DUALROWMODE, CB_GETCURSEL, 0, 0);
				if(p->dualRowMode == CB_ERR)
					p->dualRowMode = 0;
			}
			return(0);
		}
		case WM_NOTIFY:
			switch(((LPNMHDR) lParam)->code) {
				case PSN_APPLY: {
					return(TRUE);
				}
			}
			break;
	}
	return(FALSE);
}

ORDERTREEDATA OrderTreeData[] = {
	{EXTRA_ICON_RES0, _T("Reserved, unused"), 9, TRUE, 0},
	{EXTRA_ICON_EMAIL, _T("E-mail"), 0, TRUE, 0},
	{EXTRA_ICON_RES1, _T("Reserved #1"), 7, TRUE, 0},
	{EXTRA_ICON_SMS, _T("Telephone"), 2, TRUE, 0},
	{EXTRA_ICON_ADV1, _T("Advanced #1 (X-Status)"), 3, TRUE, 0},
	{EXTRA_ICON_ADV2, _T("Advanced #2"), 4, TRUE, 0},
	{EXTRA_ICON_WEB, _T("Homepage"), 1, TRUE, 0},
	{EXTRA_ICON_CLIENT, _T("Client (fingerprint required)"), 10, TRUE, 0},
	{EXTRA_ICON_RES2, _T("Reserved #2"), 8, TRUE, 0},
	{EXTRA_ICON_ADV3, _T("Advanced #3"), 5, TRUE, 0},
	{EXTRA_ICON_ADV4, _T("Advanced #4"), 6, TRUE, 0},
};

static int dragging = 0;
static HTREEITEM hDragItem = NULL;

static int FillOrderTree(HWND hwndDlg, HWND hwndTree, TDisplayProfile *p)
{
	int i = 0;
	TVINSERTSTRUCT tvis = {0};
	TreeView_DeleteAllItems(hwndTree);
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;

	for(i = 0; i < EXICON_COUNT; i++) {
		int iIndex = (int)(p->exIconOrder[i] - 1);
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.lParam = (LPARAM)(&(OrderTreeData[iIndex]));
		tvis.item.pszText = TranslateTS(OrderTreeData[iIndex].Name);
		OrderTreeData[iIndex].Visible = (p->dwExtraImageMask & (1 << OrderTreeData[iIndex].ID)) ? TRUE : FALSE;
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK(OrderTreeData[iIndex].Visible ? 1 : 2);
		TreeView_InsertItem(hwndTree, &tvis);
	}
	return(0);
}

static int SaveOrderTree(HWND hwndDlg, HWND hwndTree, TDisplayProfile *p)
{
	HTREEITEM ht;
	TVITEM tvi = {0};
	int  iIndex = 0;

	tvi.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
	ht = TreeView_GetRoot(hwndTree);

	p->dwExtraImageMask = 0;

	do {
		ORDERTREEDATA *it = NULL;
		tvi.hItem = ht;
		TreeView_GetItem(hwndTree, &tvi);
		it = (ORDERTREEDATA *)(tvi.lParam);

		p->exIconOrder[iIndex] = it->ID + 1;
		p->dwExtraImageMask |= (it->Visible ? (1 << it->ID) : 0);
		ht = TreeView_GetNextSibling(hwndTree, ht);
		iIndex++;
	} while(ht);
	return(0);
}

INT_PTR CALLBACK cfg::DlgProcXIcons(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG: {
			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_EXTRAORDER), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_EXTRAORDER), GWL_STYLE) | (TVS_NOHSCROLL | TVS_CHECKBOXES));

			HIMAGELIST himlOptions = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)CreateStateImageList());
			ImageList_Destroy(himlOptions);
			return(TRUE);
		}
		case WM_COMMAND:
			if((LOWORD(wParam) == IDC_EXICONSCALE) && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
				return(0);

			if(IDC_RESETXICONS == LOWORD(wParam)) {
				TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
				if(p) {
					for(int i = 0; i < EXICON_COUNT; i++) {
						OrderTreeData[i].Visible = TRUE;
						p->exIconOrder[i] = i + 1;
					}
					p->dwExtraImageMask = 0xffffffff;
					FillOrderTree(hwndDlg, GetDlgItem(hwndDlg, IDC_EXTRAORDER), p);
				}
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_USER + 100: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (INT_PTR)p);
				CheckDlgButton(hwndDlg, IDC_SHOWSTATUSICONS, (p->dwFlags & CLUI_FRAME_STATUSICONS) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_SHOWVISIBILITY, (p->dwFlags & CLUI_SHOWVISI) ? BST_CHECKED : BST_UNCHECKED);

				CheckDlgButton(hwndDlg, IDC_OVERLAYICONS, (p->dwFlags & CLUI_FRAME_OVERLAYICONS) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_SELECTIVEICONS, (p->dwFlags & CLUI_FRAME_SELECTIVEICONS) ? BST_CHECKED : BST_UNCHECKED);

				CheckDlgButton(hwndDlg, IDC_STATUSICONSCENTERED, p->bCenterStatusIcons ? 1 : 0);
				CheckDlgButton(hwndDlg, IDC_IDLE, p->bDimIdle ? BST_CHECKED : BST_UNCHECKED);

				SendDlgItemMessage(hwndDlg, IDC_EXICONSCALESPIN, UDM_SETRANGE, 0, MAKELONG(20, 8));
				SendDlgItemMessage(hwndDlg, IDC_EXICONSCALESPIN, UDM_SETPOS, 0, (LPARAM)p->exIconScale);
				FillOrderTree(hwndDlg, GetDlgItem(hwndDlg, IDC_EXTRAORDER), p);
			}
			return(0);
		}
		case WM_USER + 200: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			if(p) {
				SaveOrderTree(hwndDlg, GetDlgItem(hwndDlg, IDC_EXTRAORDER), p);

				p->exIconScale = SendDlgItemMessage(hwndDlg, IDC_EXICONSCALESPIN, UDM_GETPOS, 0, 0);
				p->exIconScale = (p->exIconScale < 8 || p->exIconScale > 20) ? 16 : p->exIconScale;

				p->dwFlags |= ((IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUSICONS) ? CLUI_FRAME_STATUSICONS : 0) |
							   (IsDlgButtonChecked(hwndDlg, IDC_SHOWVISIBILITY) ? CLUI_SHOWVISI : 0) |
							   (IsDlgButtonChecked(hwndDlg, IDC_OVERLAYICONS) ? CLUI_FRAME_OVERLAYICONS : 0) |
							   (IsDlgButtonChecked(hwndDlg, IDC_SELECTIVEICONS) ? CLUI_FRAME_SELECTIVEICONS : 0));

				p->bDimIdle = IsDlgButtonChecked(hwndDlg, IDC_IDLE) ? 1 : 0;
				p->bCenterStatusIcons = IsDlgButtonChecked(hwndDlg, IDC_STATUSICONSCENTERED) ? 1 : 0;
			}
			return(0);
		}
		case WM_NOTIFY:
			if(((LPNMHDR) lParam)->idFrom == IDC_EXTRAORDER) {
				switch(((LPNMHDR)lParam)->code) {
					case TVN_BEGINDRAGA:
					case TVN_BEGINDRAGW:
						SetCapture(hwndDlg);
						dragging = 1;
						hDragItem = ((LPNMTREEVIEWA)lParam)->itemNew.hItem;
						TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_EXTRAORDER), hDragItem);
						break;
					case NM_CLICK: {
						TVHITTESTINFO hti;
						hti.pt.x = (short)LOWORD(GetMessagePos());
						hti.pt.y = (short)HIWORD(GetMessagePos());
						ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
						if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
							if(hti.flags & TVHT_ONITEMSTATEICON) {
								TVITEM tvi;
								tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
								tvi.hItem = hti.hItem;
								TreeView_GetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
								tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
								((ORDERTREEDATA *)tvi.lParam)->Visible = (tvi.iImage == 2 ? FALSE : TRUE);
								if(((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
									((ORDERTREEDATA *)tvi.lParam)->Visible = TRUE;
								else
									((ORDERTREEDATA *)tvi.lParam)->Visible = FALSE;
								TreeView_SetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}

					}
				}
				break;
			}
			switch(((LPNMHDR) lParam)->code) {
				case PSN_APPLY: {
					return(TRUE);
				}
			}
			break;

		case WM_MOUSEMOVE: {
			if(!dragging)
				break;
			{
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(lParam);
				hti.pt.y = (short)HIWORD(lParam);
				ClientToScreen(hwndDlg, &hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &hti.pt);
				TreeView_HitTest(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &hti);
				if(hti.flags&(TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
					HTREEITEM it = hti.hItem;
					hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_EXTRAORDER)) / 2;
					TreeView_HitTest(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &hti);
					if(!(hti.flags & TVHT_ABOVE))
						TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_EXTRAORDER), hti.hItem, 1);
					else
						TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_EXTRAORDER), it, 0);
				} else {
					if(hti.flags & TVHT_ABOVE) SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
					if(hti.flags & TVHT_BELOW) SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
					TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_EXTRAORDER), NULL, 0);
				}
			}
		}
		break;
		case WM_LBUTTONUP: {
			if(!dragging) break;
			TreeView_SetInsertMark(GetDlgItem(hwndDlg, IDC_EXTRAORDER), NULL, 0);
			dragging = 0;
			ReleaseCapture();
			{
				TVHITTESTINFO hti;
				TVITEM tvi;
				hti.pt.x = (short)LOWORD(lParam);
				hti.pt.y = (short)HIWORD(lParam);
				ClientToScreen(hwndDlg, &hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &hti.pt);
				hti.pt.y -= TreeView_GetItemHeight(GetDlgItem(hwndDlg, IDC_EXTRAORDER)) / 2;
				TreeView_HitTest(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &hti);
				if(hDragItem == hti.hItem) break;
				if(hti.flags & TVHT_ABOVE) hti.hItem = TVI_FIRST;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = hDragItem;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &tvi);
				if(hti.flags&(TVHT_ONITEM | TVHT_ONITEMRIGHT) || (hti.hItem == TVI_FIRST)) {
					TVINSERTSTRUCT tvis;
					wchar_t name[128];
					tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvis.item.stateMask = 0xFFFFFFFF;
					tvis.item.pszText = name;
					tvis.item.cchTextMax = sizeof(name);
					tvis.item.hItem = hDragItem;
					tvis.item.iImage = tvis.item.iSelectedImage = ((ORDERTREEDATA *)tvi.lParam)->Visible;
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &tvis.item);
					TreeView_DeleteItem(GetDlgItem(hwndDlg, IDC_EXTRAORDER), hDragItem);
					tvis.hParent = NULL;
					tvis.hInsertAfter = hti.hItem;
					TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_EXTRAORDER), TreeView_InsertItem(GetDlgItem(hwndDlg, IDC_EXTRAORDER), &tvis));
					SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
				}
			}
		}
		break;
	}
	return(FALSE);
}

static TDisplayProfile dsp_current;

INT_PTR CALLBACK cfg::DlgProcDspProfiles(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int iInit = TRUE;
	static HWND hwndTab;
	static int iTabCount;

	switch(msg) {
		case WM_INITDIALOG: {
			TCITEM tci;
			RECT rcClient;
			int oPage = cfg::getByte("CLUI", "opage_d", 0);

			TranslateDialogDefault(hwnd);

			GetClientRect(hwnd, &rcClient);
			hwndTab = GetDlgItem(hwnd, IDC_OPTIONSTAB);
			iInit = TRUE;
			tci.mask = TCIF_PARAM | TCIF_TEXT;

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_DSPITEMS), hwnd, DlgProcDspItems);
			tci.pszText = TranslateT("Contacts");
			TabCtrl_InsertItem(hwndTab, 0, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 0 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_DSPGROUPS), hwnd, DlgProcDspGroups);
			tci.pszText = TranslateT("Groups and layout");
			TabCtrl_InsertItem(hwndTab, 1, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 1 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_XICONS), hwnd, DlgProcXIcons);
			tci.pszText = TranslateT("Icons");
			TabCtrl_InsertItem(hwndTab, 2, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 2 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_DSPADVANCED), hwnd, DlgProcDspAdvanced);
			tci.pszText = TranslateT("Advanced");
			TabCtrl_InsertItem(hwndTab, 3, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 3 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_DSPCLASSES), hwnd, DlgProcDspClasses);
			tci.pszText = TranslateT("Display classes");
			TabCtrl_InsertItem(hwndTab, 4, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 4 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB), oPage);

			DSP_LoadFromDefaults(&CLC::dsp_default);
			CopyMemory(&dsp_current, &CLC::dsp_default, sizeof(TDisplayProfile));

			iTabCount =  TabCtrl_GetItemCount(hwndTab);

			SendMessage(hwnd, WM_USER + 100, 0, (LPARAM)&CLC::dsp_default);

			iInit = FALSE;
			return(FALSE);
		}

		/*
		 * distribute a WM_USER message to all child windows so they can update their pages from the
		 * display profile structure
		 * LPARAM = DISPLAYPROFILE *
		 */

		case WM_USER + 100: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);

			if(p) {
				int i;
				TCITEM item = {0};
				item.mask = TCIF_PARAM;

				for(i = 0; i < iTabCount; i++) {
					TabCtrl_GetItem(hwndTab, i, &item);
					if(item.lParam && IsWindow((HWND)item.lParam))
						SendMessage((HWND)item.lParam, WM_USER + 100, 0, (LPARAM)p);
				}
			}
			return(0);
		}

		/*
		 * collect the settings from the pages into a DISPLAYPROFILE struct
		 */
		case WM_USER + 200: {
			TDisplayProfile *p = reinterpret_cast<TDisplayProfile *>(lParam);
			int i;
			TCITEM item = {0};
			item.mask = TCIF_PARAM;

			for(i = 0; i < iTabCount; i++) {
				TabCtrl_GetItem(hwndTab, i, &item);
				if(item.lParam && IsWindow((HWND)item.lParam))
					SendMessage((HWND)item.lParam, WM_USER + 200, 0, (LPARAM)p);
			}
			return(0);
		}

		case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
			if(!iInit)
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch(((LPNMHDR)lParam)->code) {
						case PSN_APPLY: {
							TDisplayProfile p;

							ZeroMemory(&p, sizeof(p));
							SendMessage(hwnd, WM_USER + 200, 0, (LPARAM)&p);
							DSP_Apply(&p);
						}
						break;
					}
					break;
				case IDC_OPTIONSTAB:
					switch(((LPNMHDR)lParam)->code) {
						case TCN_SELCHANGING: {
							TCITEM tci;
							tci.mask = TCIF_PARAM;
							TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)), &tci);
							ShowWindow((HWND)tci.lParam, SW_HIDE);
						}
						break;
						case TCN_SELCHANGE: {
							TCITEM tci;
							tci.mask = TCIF_PARAM;
							TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)), &tci);
							ShowWindow((HWND)tci.lParam, SW_SHOW);
							cfg::writeByte("CLUI", "opage_d", (BYTE)TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)));
						}
						break;
					}
					break;

			}
			break;
	}
	return(FALSE);
}

INT_PTR CALLBACK cfg::TabOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int iInit = TRUE;

	switch(msg) {
		case WM_INITDIALOG: {
			TCITEM tci;
			RECT rcClient;
			int oPage = cfg::getByte("CLUI", "opage_m", 0);

			if(oPage > 3)
				oPage = 0;

			GetClientRect(hwnd, &rcClient);
			iInit = TRUE;
			tci.mask = TCIF_PARAM | TCIF_TEXT;
			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_CLIST), hwnd, cfg::DlgProcGenOpts);
			tci.pszText = TranslateT("General");
			TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 0 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_CLC), hwnd, cfg::DlgProcClcMainOpts);
			tci.pszText = TranslateT("List layout");
			TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 1, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 1 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_CLUI), hwnd, cfg::DlgProcCluiOpts);
			tci.pszText = TranslateT("Window");
			TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 2, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 2 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_SBAR), hwnd, cfg::DlgProcSBarOpts);
			tci.pszText = TranslateT("Status Bar");
			TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 3, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 30, 1);
			ShowWindow((HWND)tci.lParam, oPage == 3 ? SW_SHOW : SW_HIDE);
			if(IS_THEMED)
				Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB), oPage);
			iInit = FALSE;
			return(FALSE);
		}

		case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
			if(!iInit)
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch(((LPNMHDR)lParam)->code) {
						case PSN_APPLY: {
							TCITEM tci;
							int i, count;
							tci.mask = TCIF_PARAM;
							count = TabCtrl_GetItemCount(GetDlgItem(hwnd, IDC_OPTIONSTAB));
							for(i = 0; i < count; i++) {
								TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), i, &tci);
								SendMessage((HWND)tci.lParam, WM_NOTIFY, 0, lParam);
							}
						}
						break;
					}
					break;
				case IDC_OPTIONSTAB:
					switch(((LPNMHDR)lParam)->code) {
						case TCN_SELCHANGING: {
							TCITEM tci;
							tci.mask = TCIF_PARAM;
							TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)), &tci);
							ShowWindow((HWND)tci.lParam, SW_HIDE);
						}
						break;
						case TCN_SELCHANGE: {
							TCITEM tci;
							tci.mask = TCIF_PARAM;
							TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)), &tci);
							ShowWindow((HWND)tci.lParam, SW_SHOW);
							cfg::writeByte("CLUI", "opage_m", (BYTE)TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)));
						}
						break;
					}
					break;

			}
			break;
	}
	return(FALSE);
}

int ClcOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	//odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = g_hInst;
	odp.pszGroup = LPGEN("Contact List");

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DSPPROFILES);
	odp.pszTitle = LPGEN("Contact rows");
	odp.pfnDlgProc = cfg::DlgProcDspProfiles;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FLOATING);
	odp.pszTitle = LPGEN("Floating contacts");
	odp.pfnDlgProc = cfg::DlgProcFloatingContacts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.pszGroup = LPGEN("Skins");
	odp.pszTitle = LPGEN("Contact list skin");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = cfg::SkinOptionsDlgProc;
	Options_AddPage(wParam, &odp);

	odp.position = -1000000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSDIALOG);
	odp.pszGroup = NULL;
	odp.pszTitle = LPGEN("Contact List");
	odp.pfnDlgProc = cfg::TabOptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return(0);
}

static int opt_clc_main_changed = 0;

INT_PTR CALLBACK cfg::DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			opt_clc_main_changed = 0;
			SetWindowLong(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE, GetWindowLong(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

			{
				int i;
				DWORD exStyle = cfg::getDword("CLC", "ExStyle", pcli->pfnGetDefaultExStyle());
				UDACCEL accel[2] = {
					{0, 10}, {2, 50}
				};
				SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
				SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, sizeof(accel) / sizeof(accel[0]), (LPARAM) &accel);
				SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0, MAKELONG(cfg::getWord("CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));

				for(i = 0; i < sizeof(checkBoxToStyleEx) / sizeof(checkBoxToStyleEx[0]); i++)
					CheckDlgButton(hwndDlg, checkBoxToStyleEx[i].id, (exStyle & checkBoxToStyleEx[i].flag) ^(checkBoxToStyleEx[i].flag * checkBoxToStyleEx[i].not_t) ? BST_CHECKED : BST_UNCHECKED);
			}
			CheckDlgButton(hwndDlg, IDC_FULLROWSELECT, (cfg::dat.dwFlags & CLUI_FULLROWSELECT) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_DBLCLKAVATARS, cfg::dat.bDblClkAvatars);
			CheckDlgButton(hwndDlg, IDC_GREYOUT, cfg::getDword("CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS) ? BST_CHECKED : BST_UNCHECKED);
			Utils::enableDlgControl(hwndDlg, IDC_SMOOTHTIME, IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
			Utils::enableDlgControl(hwndDlg, IDC_GREYOUTOPTS, IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
			FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), greyoutValues, sizeof(greyoutValues) / sizeof(greyoutValues[0]), cfg::getDword("CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS));
			CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, cfg::getByte("CLC", "NoVScrollBar", 0) ? BST_CHECKED : BST_UNCHECKED);

			return(TRUE);
		case WM_VSCROLL:
			opt_clc_main_changed = 1;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_NOTNOSMOOTHSCROLLING)
				Utils::enableDlgControl(hwndDlg, IDC_SMOOTHTIME, IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
			if(LOWORD(wParam) == IDC_GREYOUT)
				Utils::enableDlgControl(hwndDlg, IDC_GREYOUTOPTS, IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			opt_clc_main_changed = 1;
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR) lParam)->idFrom) {
				case IDC_GREYOUTOPTS:
					if(((LPNMHDR) lParam)->code == NM_CLICK) {
						TVHITTESTINFO hti;
						hti.pt.x = (short) LOWORD(GetMessagePos());
						hti.pt.y = (short) HIWORD(GetMessagePos());
						ScreenToClient(((LPNMHDR) lParam)->hwndFrom, &hti.pt);
						if(TreeView_HitTest(((LPNMHDR) lParam)->hwndFrom, &hti))
							if(hti.flags & TVHT_ONITEMSTATEICON) {
								TVITEM tvi;
								tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
								tvi.hItem = hti.hItem;
								TreeView_GetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
								tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
								TreeView_SetItem(((LPNMHDR) lParam)->hwndFrom, &tvi);
								opt_clc_main_changed = 1;
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
					}
					break;
				case 0:
					switch(((LPNMHDR) lParam)->code) {
						case PSN_APPLY: {
							int i;
							DWORD exStyle = cfg::getDword("CLC", "ExStyle", CLCDEFAULT_EXSTYLE);

							if(!opt_clc_main_changed)
								return(TRUE);

							for(i = 0; i < sizeof(checkBoxToStyleEx) / sizeof(checkBoxToStyleEx[0]); i++)
								exStyle &= ~(checkBoxToStyleEx[i].flag);

							for(i = 0; i < sizeof(checkBoxToStyleEx) / sizeof(checkBoxToStyleEx[0]); i++) {
								if((IsDlgButtonChecked(hwndDlg, checkBoxToStyleEx[i].id) == 0) == checkBoxToStyleEx[i].not_t)
									exStyle |= checkBoxToStyleEx[i].flag;
							}
							cfg::writeDword("CLC", "ExStyle", exStyle);
						} {
							DWORD fullGreyoutFlags = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS));
							cfg::writeDword("CLC", "FullGreyoutFlags", fullGreyoutFlags);
							if(IsDlgButtonChecked(hwndDlg, IDC_GREYOUT))
								cfg::writeDword("CLC", "GreyoutFlags", fullGreyoutFlags);
							else
								cfg::writeDword("CLC", "GreyoutFlags", 0);
						}
						cfg::writeWord("CLC", "ScrollTime", (WORD) SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
						cfg::writeByte("CLC", "NoVScrollBar", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));
						cfg::dat.dwFlags = IsDlgButtonChecked(hwndDlg, IDC_FULLROWSELECT) ? cfg::dat.dwFlags | CLUI_FULLROWSELECT : cfg::dat.dwFlags & ~CLUI_FULLROWSELECT;
						cfg::dat.bDblClkAvatars = IsDlgButtonChecked(hwndDlg, IDC_DBLCLKAVATARS) ? TRUE : FALSE;
						cfg::writeByte("CLC", "dblclkav", (BYTE)cfg::dat.bDblClkAvatars);
						cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);

						pcli->pfnClcOptionsChanged();
						CoolSB_SetupScrollBar(pcli->hwndContactTree);
						PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
						opt_clc_main_changed = 0;
						return(TRUE);
					}
					break;
			}
			break;
		case WM_DESTROY:
			ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), TVSIL_STATE));
			break;
	}
	return(FALSE);
}

