/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

#define FONTF_BOLD   1
#define FONTF_ITALIC 2

struct
{
	const wchar_t *szDescr;
	COLORREF defColour;
	const wchar_t *szDefFace;
	uint8_t defStyle;
	char defSize;
	const wchar_t *szBkgName;
}
static const fontOptionsList[] =
{
	{ LPGENW("Outgoing messages"), RGB(106, 106, 106), L"Arial", 0, -12, LPGENW("Outgoing background")},
	{ LPGENW("Incoming messages"), RGB(0, 0, 0), L"Arial", 0, -12, LPGENW("Incoming background")},
	{ LPGENW("Outgoing name"), RGB(89, 89, 89), L"Arial", FONTF_BOLD, -12, LPGENW("Outgoing background")},
	{ LPGENW("Outgoing time"), RGB(0, 0, 0), L"Terminal", FONTF_BOLD, -9, LPGENW("Outgoing background")},
	{ LPGENW("Outgoing colon"), RGB(89, 89, 89), L"Arial", 0, -11, LPGENW("Outgoing background")},
	{ LPGENW("Incoming name"), RGB(215, 0, 0), L"Arial", FONTF_BOLD, -12, LPGENW("Incoming background")},
	{ LPGENW("Incoming time"), RGB(0, 0, 0), L"Terminal", FONTF_BOLD, -9, LPGENW("Incoming background")},
	{ LPGENW("Incoming colon"), RGB(215, 0, 0), L"Arial", 0, -11, LPGENW("Incoming background")},
	{ LPGENW("Message area"), RGB(0, 0, 0), L"Arial", 0, -12, LPGENW("Input area background")},
	{ LPGENW("Notices"), RGB(90, 90, 160), L"Arial", 0, -12, LPGENW("Incoming background")},
	{ LPGENW("Unused"), RGB(0, 0, 255), L"Arial", 0, -12, LPGENW("Outgoing background")},
	{ LPGENW("Unused"), RGB(0, 0, 255), L"Arial", 0, -12, LPGENW("Incoming background")},
	{ LPGENW("Info bar contact name"), RGB(0, 0, 0), L"Arial", FONTF_BOLD, -19, LPGENW("Info bar background")},
	{ LPGENW("Info bar status message"), RGB(50, 50, 50), L"Arial", FONTF_ITALIC, -11, LPGENW("Info bar background")}
};

int fontOptionsListSize = _countof(fontOptionsList);

struct
{
	const wchar_t *szName;
	const char *szSettingName;
	int systemColor;
}
static const colourOptionsList[] =
{
	{ LPGENW("Background"), SRMSGSET_BKGCOLOUR, COLOR_WINDOW },
	{ LPGENW("Input area background"), SRMSGSET_INPUTBKGCOLOUR, COLOR_WINDOW },
	{ LPGENW("Incoming background"), SRMSGSET_INCOMINGBKGCOLOUR, COLOR_WINDOW },
	{ LPGENW("Outgoing background"), SRMSGSET_OUTGOINGBKGCOLOUR, COLOR_WINDOW },
	{ LPGENW("Info bar background"), SRMSGSET_INFOBARBKGCOLOUR, COLOR_3DLIGHT },
	{ LPGENW("Line between messages"), SRMSGSET_LINECOLOUR, COLOR_3DLIGHT },
};

int FontServiceFontsChanged(WPARAM, LPARAM)
{
	LoadMsgLogIcons();
	LoadInfobarFonts();
	Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
	return 0;
}

void RegisterFontServiceFonts()
{
	FontIDW fid = {};
	wcsncpy_s(fid.group, LPGENW("Messaging"), _TRUNCATE);
	wcsncpy_s(fid.backgroundGroup, LPGENW("Messaging"), _TRUNCATE);
	strncpy(fid.dbSettingsGroup, SRMM_MODULE, _countof(fid.dbSettingsGroup));
	fid.flags = FIDF_DEFAULTVALID;
	for (int i = 0; i < _countof(fontOptionsList); i++) {
		fid.order = i;

		char szTemp[100];
		mir_snprintf(szTemp, "SRMFont%d", i);
		strncpy(fid.setting, szTemp, _countof(fid.setting));
		wcsncpy(fid.name, fontOptionsList[i].szDescr, _countof(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].defColour;
		fid.deffontsettings.size = fontOptionsList[i].defSize;
		fid.deffontsettings.style = fontOptionsList[i].defStyle;
		fid.deffontsettings.charset = DEFAULT_CHARSET;
		wcsncpy(fid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _countof(fid.deffontsettings.szFace));
		wcsncpy(fid.backgroundName, fontOptionsList[i].szBkgName, _countof(fid.backgroundName));
		g_plugin.addFont(&fid);
	}

	ColourIDW cid = {};
	wcsncpy_s(cid.group, LPGENW("Messaging"), _TRUNCATE);
	strncpy(cid.dbSettingsGroup, SRMM_MODULE, _countof(fid.dbSettingsGroup));
	for (int i = 0; i < _countof(colourOptionsList); i++) {
		cid.order = i;
		wcsncpy(cid.name, colourOptionsList[i].szName, _countof(cid.name));
		strncpy(cid.setting, colourOptionsList[i].szSettingName, _countof(cid.setting));
		cid.defcolour = GetSysColor(colourOptionsList[i].systemColor);
		g_plugin.addColor(&cid);
	}
}

int IconsChanged(WPARAM, LPARAM)
{
	ReleaseIcons();
	LoadGlobalIcons();
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	Srmm_Broadcast(DM_REMAKELOG, 0, 0);
	Srmm_Broadcast(DM_CHANGEICONS, 0, 1);
	return 0;
}

int SmileySettingsChanged(WPARAM wParam, LPARAM)
{
	Srmm_Broadcast(DM_REMAKELOG, wParam, 0);
	return 0;
}

void LoadMsgDlgFont(int i, LOGFONT *lf, COLORREF *colour)
{
	char str[32];

	if (colour) {
		mir_snprintf(str, "%s%dCol", "SRMFont", i);
		*colour = g_plugin.getDword(str, fontOptionsList[i].defColour);
	}

	if (lf) {
		mir_snprintf(str, "%s%dSize", "SRMFont", i);
		lf->lfHeight = (char)g_plugin.getByte(str, fontOptionsList[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "%s%dSty", "SRMFont", i);
		int style = g_plugin.getByte(str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "%s%d", "SRMFont", i);

		ptrW tszFace(g_plugin.getWStringA(str));
		if (tszFace == nullptr)
			mir_wstrcpy(lf->lfFaceName, fontOptionsList[i].szDefFace);
		else
			wcsncpy(lf->lfFaceName, tszFace, _countof(lf->lfFaceName));

		mir_snprintf(str, "%s%dSet", "SRMFont", i);
		lf->lfCharSet = g_plugin.getByte(str, DEFAULT_CHARSET);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

class CBaseOptionDlg : public CDlgBase
{
	void ApplyOptions(void*)
	{
		ReloadGlobals();
		WindowList_Broadcast(g_dat.hParentWindowList, DM_OPTIONSAPPLIED, 0, 0);
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
		Chat_UpdateOptions();
	}

public:
	CBaseOptionDlg(int dlgId) :
		CDlgBase(g_plugin, dlgId)
	{
		m_OnFinishWizard = Callback(this, &CBaseOptionDlg::ApplyOptions);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CheckBoxValues_t
{
	uint32_t style;
	const wchar_t *szDescr;
};

static const struct CheckBoxValues_t statusValues[] =
{
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE, LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY, LPGENW("Not available") },
	{ PF2_LIGHTDND, LPGENW("Occupied") },
	{ PF2_HEAVYDND, LPGENW("Do not disturb") },
	{ PF2_FREECHAT, LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") }
};

class CMainOptionsDlg : public CBaseOptionDlg
{
	void FillCheckBoxTree(const struct CheckBoxValues_t *values, int nValues, uint32_t style)
	{
		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
		for (int i = 0; i < nValues; i++) {
			tvis.item.lParam = values[i].style;
			tvis.item.pszText = TranslateW(values[i].szDescr);
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;
			tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
			m_tree.InsertItem(&tvis);
		}
	}

	uint32_t MakeCheckBoxTreeFlags()
	{
		uint32_t flags = 0;
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
		tvi.hItem = m_tree.GetRoot();
		while (tvi.hItem) {
			m_tree.GetItem(&tvi);
			if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
				flags |= tvi.lParam;
			tvi.hItem = m_tree.GetNextSibling(tvi.hItem);
		}
		return flags;
	}

	CCtrlSpin spinTimeout;
	CCtrlCheck chkAutoMin, chkAutoPopup, chkCascade, chkSavePerContact, chkStayMinimized;
	CCtrlCheck chkSaveDrafts, chkDelTemp, chkHideContainer;
	CCtrlCombo cmbSendMode;
	CCtrlTreeView m_tree;

public:
	CMainOptionsDlg() :
		CBaseOptionDlg(IDD_OPT_MSGDLG),
		m_tree(this, IDC_POPLIST),
		spinTimeout(this, IDC_SECONDSSPIN, 60, 4),
		chkAutoMin(this, IDC_AUTOMIN),
		chkCascade(this, IDC_CASCADE),
		chkDelTemp(this, IDC_DELTEMP),
		cmbSendMode(this, IDC_SENDMODE),
		chkAutoPopup(this, IDC_AUTOPOPUP),
		chkSaveDrafts(this, IDC_SAVEDRAFTS),
		chkHideContainer(this, IDC_HIDECONTAINERS),
		chkStayMinimized(this, IDC_STAYMINIMIZED),
		chkSavePerContact(this, IDC_SAVEPERCONTACT)
	{
		CreateLink(spinTimeout, g_plugin.iMsgTimeout);
		CreateLink(chkCascade, g_plugin.bCascade);
		CreateLink(chkAutoMin, g_plugin.bAutoMin);
		CreateLink(chkAutoPopup, g_plugin.bAutoPopup);
		CreateLink(chkSaveDrafts, g_plugin.bSaveDrafts);
		CreateLink(chkHideContainer, g_plugin.bHideContainer);
		CreateLink(chkStayMinimized, g_plugin.bStayMinimized);
		CreateLink(chkSavePerContact, g_plugin.bSavePerContact);

		chkCascade.OnChange = Callback(this, &CMainOptionsDlg::onChange_Cascade);
		chkAutoPopup.OnChange = Callback(this, &CMainOptionsDlg::onChange_AutoPopup);
		chkSavePerContact.OnChange = Callback(this, &CMainOptionsDlg::onChange_SavePerContact);
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(m_tree.GetHwnd(), GWL_STYLE, (GetWindowLongPtr(m_tree.GetHwnd(), GWL_STYLE) & ~WS_BORDER) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		FillCheckBoxTree(statusValues, _countof(statusValues), g_plugin.iPopFlags);

		cmbSendMode.AddString(TranslateT("Enter"));
		cmbSendMode.AddString(TranslateT("Double 'Enter'"));
		cmbSendMode.AddString(TranslateT("Ctrl+Enter"));
		cmbSendMode.AddString(TranslateT("Shift+Enter"));
		cmbSendMode.SetCurSel(g_dat.sendMode);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.iPopFlags = MakeCheckBoxTreeFlags();

		g_plugin.setByte(SRMSGSET_SENDMODE, cmbSendMode.GetCurSel());
		return true;
	}

	void onChange_AutoPopup(CCtrlCheck*)
	{
		bool bChecked = chkAutoPopup.GetState();
		m_tree.Enable(bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STAYMINIMIZED), bChecked);
	}

	void onChange_Cascade(CCtrlCheck *pCheck)
	{
		chkSavePerContact.SetState(!pCheck->GetState());
	}

	void onChange_SavePerContact(CCtrlCheck *pCheck)
	{
		chkCascade.SetState(!pCheck->GetState());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CTabsOptionsDlg : public CBaseOptionDlg
{
	CCtrlCheck chkUseTabs, chkLimitTabs, chkLimitChatTabs, chkLimitNames, chkSeparateChats;
	CCtrlCheck chkTabCloseButton, chkHideOneTab, chkTabsAtBottom, chkSwitchToActive;
	CCtrlSpin  spinNames, spinTabs, spinChatTabs;

public:
	CTabsOptionsDlg() :
		CBaseOptionDlg(IDD_OPT_MSGTABS),
		chkUseTabs(this, IDC_USETABS),
		chkLimitTabs(this, IDC_LIMITTABS),
		chkLimitNames(this, IDC_LIMITNAMES),
		chkLimitChatTabs(this, IDC_LIMITCHATSTABS),
		chkHideOneTab(this, IDC_HIDEONETAB),
		chkSeparateChats(this, IDC_SEPARATECHATSCONTAINERS),
		chkTabsAtBottom(this, IDC_TABSATBOTTOM),
		chkSwitchToActive(this, IDC_SWITCHTOACTIVE),
		chkTabCloseButton(this, IDC_TABCLOSEBUTTON),
		spinNames(this, IDC_LIMITNAMESLENSPIN, 100),
		spinTabs(this, IDC_LIMITTABSNUMSPIN, 100, 1),
		spinChatTabs(this, IDC_LIMITCHATSTABSNUMSPIN, 100, 1)
	{
		CreateLink(chkUseTabs, g_plugin.bUseTabs);
		CreateLink(chkLimitTabs, g_plugin.bLimitTabs);
		CreateLink(chkLimitNames, g_plugin.bLimitNames);
		CreateLink(chkLimitChatTabs, g_plugin.bLimitChatTabs);
		CreateLink(chkHideOneTab, g_plugin.bHideOneTab);
		CreateLink(chkSeparateChats, g_plugin.bSeparateChats);
		CreateLink(chkTabsAtBottom, g_plugin.bTabsAtBottom);
		CreateLink(chkSwitchToActive, g_plugin.bSwitchToActive);
		CreateLink(chkTabCloseButton, g_plugin.bTabCloseButton);
		
		CreateLink(spinNames, g_plugin.iLimitNames);
		CreateLink(spinTabs, g_plugin.iLimitTabs);
		CreateLink(spinChatTabs, g_plugin.iLimitChatTabs);

		chkUseTabs.OnChange = Callback(this, &CTabsOptionsDlg::onChange_UseTabs);
		chkLimitTabs.OnChange = Callback(this, &CTabsOptionsDlg::onChange_LimitTabs);
		chkLimitNames.OnChange = Callback(this, &CTabsOptionsDlg::onChange_LimitNames);
		chkLimitChatTabs.OnChange = Callback(this, &CTabsOptionsDlg::onChange_LimitChatTabs);
		chkSeparateChats.OnChange = Callback(this, &CTabsOptionsDlg::onChange_SeparateChats);
	}
		
	void onChange_UseTabs(CCtrlCheck*)
	{
		int bChecked = chkUseTabs.GetState();
		chkHideOneTab.Enable(bChecked);
		chkTabsAtBottom.Enable(bChecked);
		chkSwitchToActive.Enable(bChecked);
		chkTabCloseButton.Enable(bChecked);
		chkLimitNames.Enable(bChecked);
		chkSeparateChats.Enable(bChecked);
		chkLimitTabs.Enable(bChecked);

		onChange_LimitTabs(0);
		onChange_LimitNames(0);
		onChange_SeparateChats(0);
	}

	void onChange_LimitTabs(CCtrlCheck*)
	{
		int bChecked = chkUseTabs.GetState() && chkLimitTabs.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LIMITTABSNUM), bChecked);
		spinTabs.Enable(bChecked);
	}

	void onChange_SeparateChats(CCtrlCheck*)
	{
		int bChecked = chkUseTabs.GetState() && chkSeparateChats.GetState();
		chkLimitChatTabs.Enable(bChecked);

		onChange_LimitChatTabs(0);
	}

	void onChange_LimitChatTabs(CCtrlCheck*)
	{
		int bChecked = chkUseTabs.GetState() && chkSeparateChats.GetState() && chkLimitChatTabs.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LIMITCHATSTABSNUM), bChecked);
		spinChatTabs.Enable(bChecked);
	}

	void onChange_LimitNames(CCtrlCheck*)
	{
		int bChecked = chkUseTabs.GetState() && chkLimitNames.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LIMITNAMESLEN), bChecked);
		spinNames.Enable(bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHARS), bChecked);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CLayoutOptionsDlg : public CBaseOptionDlg
{
	CCtrlSpin spinInput;
	CCtrlCheck chkTransparency, chkShowTitlebar, chkShowStatusBar, chkShowToolbar, chkShowInfobar, chkShowProgress, chkShowAvatar;

public:
	CLayoutOptionsDlg() : 
		CBaseOptionDlg(IDD_OPT_LAYOUT),
		chkTransparency(this, IDC_TRANSPARENCY),
		chkShowAvatar(this, IDC_AVATARSUPPORT),
		chkShowInfobar(this, IDC_SHOWINFOBAR),
		chkShowToolbar(this, IDC_SHOWTOOLBAR),
		chkShowTitlebar(this, IDC_SHOWTITLEBAR),
		chkShowProgress(this, IDC_SHOWPROGRESS),
		chkShowStatusBar(this, IDC_SHOWSTATUSBAR),
		spinInput(this, IDC_INPUTLINESSPIN, 100, 1)
	{
		CreateLink(spinInput, g_plugin.iAutoResizeLines);
		CreateLink(chkShowAvatar, g_plugin.bShowAvatar);
		CreateLink(chkShowInfobar, g_plugin.bShowInfoBar);
		CreateLink(chkShowToolbar, g_plugin.bShowToolBar);
		CreateLink(chkShowTitlebar, g_plugin.bShowTitleBar);
		CreateLink(chkShowProgress, g_plugin.bShowProgress);
		CreateLink(chkShowStatusBar, g_plugin.bShowStatusBar);
		CreateLink(chkTransparency, g_plugin.bUseTransparency);

		chkTransparency.OnChange = Callback(this, &CLayoutOptionsDlg::onChange_Transparency);
		chkShowTitlebar.OnChange = Callback(this, &CLayoutOptionsDlg::onChange_ShowTitlebar);
	}

	bool OnInitDialog() override
	{
		SetWindowText(GetDlgItem(m_hwnd, IDC_TITLEFORMAT), g_dat.wszTitleFormat);

		SendDlgItemMessage(m_hwnd, IDC_ATRANSPARENCYVALUE, TBM_SETRANGE, FALSE, MAKELONG(0, 255));
		SendDlgItemMessage(m_hwnd, IDC_ATRANSPARENCYVALUE, TBM_SETPOS, TRUE, g_plugin.iActiveAlpha);
		SendDlgItemMessage(m_hwnd, IDC_ITRANSPARENCYVALUE, TBM_SETRANGE, FALSE, MAKELONG(0, 255));
		SendDlgItemMessage(m_hwnd, IDC_ITRANSPARENCYVALUE, TBM_SETPOS, TRUE, g_plugin.iInactiveAlpha);

		char str[10];
		mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(m_hwnd, IDC_ATRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 255));
		SetDlgItemTextA(m_hwnd, IDC_ATRANSPARENCYPERC, str);

		mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(m_hwnd, IDC_ITRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 255));
		SetDlgItemTextA(m_hwnd, IDC_ITRANSPARENCYPERC, str);
		return true;
	}

	bool OnApply() override
	{
		GetWindowText(GetDlgItem(m_hwnd, IDC_TITLEFORMAT), g_dat.wszTitleFormat, _countof(g_dat.wszTitleFormat));
		g_plugin.setWString(SRMSGSET_WINDOWTITLE, g_dat.wszTitleFormat);

		g_plugin.iActiveAlpha = SendDlgItemMessage(m_hwnd, IDC_ATRANSPARENCYVALUE, TBM_GETPOS, 0, 0);
		g_plugin.iInactiveAlpha = SendDlgItemMessage(m_hwnd, IDC_ITRANSPARENCYVALUE, TBM_GETPOS, 0, 0);

		LoadInfobarFonts();
		return true;
	}

	void onChange_Transparency(CCtrlCheck*)
	{
		int bChecked = chkTransparency.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_ATRANSPARENCYVALUE), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ATRANSPARENCYPERC), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ITRANSPARENCYVALUE), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ITRANSPARENCYPERC), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TRANSPARENCYTEXT1), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TRANSPARENCYTEXT2), bChecked);
	}

	void onChange_ShowTitlebar(CCtrlCheck*)
	{
		bool bChecked = chkShowTitlebar.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_TITLEFORMAT), bChecked);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_HSCROLL) {
			char str[10];
			mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(m_hwnd, IDC_ATRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 256));
			SetDlgItemTextA(m_hwnd, IDC_ATRANSPARENCYPERC, str);
			
			mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(m_hwnd, IDC_ITRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 256));
			SetDlgItemTextA(m_hwnd, IDC_ITRANSPARENCYPERC, str);
			SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CLogOptionsDlg : public CBaseOptionDlg
{
	CCtrlSpin spinCount, spinTime, spinIndent;
	CCtrlCheck chkLoadUnread, chkLoadCount, chkLoadTime;
	CCtrlCheck chkShowIcons, chkShowTime, chkShowSecs, chkShowDate, chkLongDate, chkRelativeDate;
	CCtrlCheck chkGroupMsg, chkIndentText, chkHideNames, chkMarkFollowups, chkMsgOnNewline, chkDrawLines;
	CCtrlRichEdit m_rtf;
	CCtrlHyperlink m_fonts;

	void OnChange() override
	{
		m_rtf.SetText(L"");

		struct GlobalMessageData gdat = {};
		gdat.flags.bShowIcons = chkShowIcons.GetState();
		gdat.flags.bHideNames = chkHideNames.GetState();
		gdat.flags.bShowTime = chkShowTime.GetState();
		gdat.flags.bShowSeconds = chkShowSecs.GetState();
		gdat.flags.bShowDate = chkShowDate.GetState();
		gdat.flags.bLongDate = chkLongDate.GetState();
		gdat.flags.bRelativeDate = chkRelativeDate.GetState();
		gdat.flags.bGroupMessages = chkGroupMsg.GetState();
		gdat.flags.bMarkFollowups = chkMarkFollowups.GetState();
		gdat.flags.bMsgOnNewline = chkMsgOnNewline.GetState();
		gdat.flags.bDrawLines = chkDrawLines.GetState();
		gdat.flags.bIndentText = chkIndentText.GetState();
		gdat.indentSize = spinIndent.GetPosition();

		PARAFORMAT2 pf2;
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_OFFSET;
		pf2.dxOffset = (gdat.flags.bIndentText) ? gdat.indentSize * 1440 / g_dat.logPixelSX : 0;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		StreamInTestEvents(m_rtf.GetHwnd(), &gdat);
	}

public:
	CLogOptionsDlg() :
		CBaseOptionDlg(IDD_OPT_MSGLOG),
		m_rtf(this, IDC_SRMM_LOG),
		m_fonts(this, IDC_FONTSCOLORS),
		chkShowTime(this, IDC_SHOWTIMES),
		chkShowSecs(this, IDC_SHOWSECONDS),
		chkShowDate(this, IDC_SHOWDATES),
		chkLongDate(this, IDC_USELONGDATE),
		chkRelativeDate(this, IDC_USERELATIVEDATE),
		chkGroupMsg(this, IDC_GROUPMESSAGES),
		chkMarkFollowups(this, IDC_MARKFOLLOWUPS),
		chkDrawLines(this, IDC_DRAWLINES),
		chkShowIcons(this, IDC_SHOWLOGICONS),
		chkIndentText(this, IDC_INDENTTEXT),
		chkHideNames(this, IDC_HIDENAMES),
		chkMsgOnNewline(this, IDC_MESSAGEONNEWLINE),
		chkLoadTime(this, IDC_LOADTIME),
		chkLoadCount(this, IDC_LOADCOUNT),
		chkLoadUnread(this, IDC_LOADUNREAD),
		spinTime(this, IDC_LOADTIMESPIN, 12 * 60),
		spinCount(this, IDC_LOADCOUNTSPIN, 100),
		spinIndent(this, IDC_INDENTSPIN, 999)
	{
		m_fonts.OnClick = Callback(this, &CLogOptionsDlg::onClick_Fonts);

		CreateLink(chkShowTime, g_plugin.bShowTime);
		CreateLink(chkShowSecs, g_plugin.bShowSeconds);
		CreateLink(chkShowDate, g_plugin.bShowDate);
		CreateLink(chkLongDate, g_plugin.bLongDate);
		CreateLink(chkGroupMsg, g_plugin.bGroupMessages);
		CreateLink(chkShowIcons, g_plugin.bShowIcons);
		CreateLink(chkHideNames, g_plugin.bHideNames);
		CreateLink(chkDrawLines, g_plugin.bDrawLines);
		CreateLink(chkIndentText, g_plugin.bIndentText);
		CreateLink(chkMsgOnNewline, g_plugin.bMsgOnNewline);
		CreateLink(chkRelativeDate, g_plugin.bRelativeDate);
		CreateLink(chkMarkFollowups, g_plugin.bMarkFollowups);

		CreateLink(spinTime, g_plugin.iLoadTime);
		CreateLink(spinCount, g_plugin.iLoadCount);
		CreateLink(spinIndent, g_plugin.iIndentSize);

		chkLoadTime.OnChange = chkLoadCount.OnChange = chkLoadUnread.OnChange = Callback(this, &CLogOptionsDlg::onChange_Time);
		chkShowDate.OnChange = Callback(this, &CLogOptionsDlg::onChange_Dates);
		chkShowTime.OnChange = Callback(this, &CLogOptionsDlg::onChange_Times);
		chkGroupMsg.OnChange = Callback(this, &CLogOptionsDlg::onChange_GroupMsg);
		chkIndentText.OnChange = Callback(this, &CLogOptionsDlg::onChange_IndentText);
	}

	bool OnInitDialog() override
	{
		switch (g_plugin.iHistoryMode) {
		case LOADHISTORY_UNREAD:
			chkLoadUnread.SetState(true);
			break;
		case LOADHISTORY_COUNT:
			chkLoadCount.SetState(true);
			break;
		case LOADHISTORY_TIME:
			chkLoadTime.SetState(true);
			break;
		}

		PARAFORMAT2 pf2;
		memset(&pf2, 0, sizeof(pf2));
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_OFFSETINDENT | PFM_RIGHTINDENT;
		pf2.dxStartIndent = 30;
		pf2.dxRightIndent = 30;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		m_rtf.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
		m_rtf.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
		m_rtf.SendMsg(EM_AUTOURLDETECT, TRUE, 0);
		m_rtf.SetReadOnly(true);

		OnChange();
		return true;
	}

	bool OnApply() override
	{
		if (chkLoadCount.GetState())
			g_plugin.iHistoryMode = LOADHISTORY_COUNT;
		else if (chkLoadTime.GetState())
			g_plugin.iHistoryMode = LOADHISTORY_TIME;
		else
			g_plugin.iHistoryMode = LOADHISTORY_UNREAD;

		FreeMsgLogIcons();
		LoadMsgLogIcons();
		return true;
	}

	void onClick_Fonts(CCtrlHyperlink*)
	{
		g_plugin.openOptions(L"Customize", L"Fonts and colors");
	}

	void onChange_Time(CCtrlCheck*)
	{
		int bChecked = chkLoadCount.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTN), bChecked);
		spinCount.Enable(bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTTEXT2), bChecked);

		bChecked = chkLoadTime.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMEN), bChecked);
		spinTime.Enable(bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STMINSOLD), bChecked);
	}

	void onChange_Times(CCtrlCheck*)
	{
		int bChecked = chkShowTime.GetState();
		chkShowSecs.Enable(bChecked);
		chkShowDate.Enable(bChecked);

		onChange_Dates(0);
	}

	void onChange_Dates(CCtrlCheck*)
	{
		int bChecked = chkShowDate.GetState() && chkShowTime.GetState();
		chkLongDate.Enable(bChecked);
		chkRelativeDate.Enable(bChecked);
	}

	void onChange_GroupMsg(CCtrlCheck*)
	{
		chkMarkFollowups.Enable(chkGroupMsg.GetState());
	}

	void onChange_IndentText(CCtrlCheck*)
	{
		int bChecked = chkIndentText.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_INDENTSIZE), bChecked);
		spinIndent.Enable(bChecked);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CTypeOptionsDlg : public CDlgBase
{
	HANDLE hItemNew, hItemUnknown;

	CCtrlClc m_list;
	CCtrlCheck chkTyping, chkTypingWin, chkTypingTray, chkTypingBalloon, chkTypingClist, chkTypingSwitch;

public:
	CTypeOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGTYPE),
		m_list(this, IDC_CLIST),
		chkTyping(this, IDC_SHOWNOTIFY),
		chkTypingWin(this, IDC_TYPEWIN),
		chkTypingTray(this, IDC_TYPETRAY),
		chkTypingClist(this, IDC_NOTIFYTRAY),
		chkTypingSwitch(this, IDC_TYPINGSWITCH),
		chkTypingBalloon(this, IDC_NOTIFYBALLOON)
	{
		CreateLink(chkTyping, g_plugin.bShowTyping);
		CreateLink(chkTypingWin, g_plugin.bShowTypingWin);
		CreateLink(chkTypingTray, g_plugin.bShowTypingTray);
		CreateLink(chkTypingClist, g_plugin.bShowTypingClist);
		CreateLink(chkTypingSwitch, g_plugin.bShowTypingSwitch);

		m_list.OnListRebuilt = Callback(this, &CTypeOptionsDlg::onRebuildClist);
		m_list.OnOptionsChanged = Callback(this, &CTypeOptionsDlg::onResetClist);

		chkTyping.OnChange = Callback(this, &CTypeOptionsDlg::onChange_Notify);
		chkTypingTray.OnChange = Callback(this, &CTypeOptionsDlg::onChange_Tray);
	}

	bool OnInitDialog() override
	{
		CLCINFOITEM cii = { sizeof(cii) };
		cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
		cii.pszText = TranslateT("** New contacts **");
		hItemNew = m_list.AddInfoItem(&cii);
		cii.pszText = TranslateT("** Unknown contacts **");
		hItemUnknown = m_list.AddInfoItem(&cii);

		SetWindowLongPtr(m_list.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_list.GetHwnd(), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_NOHIDEOFFLINE);
		onResetClist(0);

		chkTypingBalloon.SetState(!g_plugin.bShowTypingClist);
		return true;
	}

	bool OnApply() override
	{
		if (hItemNew)
			g_plugin.bTypingNew = m_list.GetCheck(hItemNew);

		if (hItemUnknown)
			g_plugin.bTypingUnknown = m_list.GetCheck(hItemUnknown);

		for (auto &hContact : Contacts()) {
			HANDLE hItem = m_list.FindContact(hContact);
			if (hItem)
				g_plugin.setByte(hContact, SRMSGSET_TYPING, m_list.GetCheck(hItem));
		}

		ReloadGlobals();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
		return true;
	}

	void onChange_Notify(CCtrlCheck *)
	{
		bool bChecked = chkTyping.GetState();
		chkTypingWin.Enable(bChecked);
		chkTypingTray.Enable(bChecked);
		chkTypingClist.Enable(bChecked);
		chkTypingBalloon.Enable(bChecked);
	}

	void onChange_Tray(CCtrlCheck *)
	{
		bool bChecked = chkTypingTray.GetState();
		chkTypingClist.Enable(bChecked);
		chkTypingBalloon.Enable(bChecked);
	}

	void onRebuildClist(CCtrlClc *)
	{
		uint8_t defType = g_plugin.bTypingNew;
		if (hItemNew && defType)
			m_list.SetCheck(hItemNew, 1);

		if (hItemUnknown && g_plugin.bTypingUnknown)
			m_list.SetCheck(hItemUnknown, 1);

		for (auto &hContact : Contacts()) {
			HANDLE hItem = m_list.FindContact(hContact);
			if (hItem && g_plugin.getByte(hContact, SRMSGSET_TYPING, defType))
				m_list.SetCheck(hItem, 1);
		}
	}

	void onResetClist(CCtrlClc *)
	{
		m_list.SetUseGroups(Clist::UseGroups);
		m_list.SetHideEmptyGroups(true);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

void ChatOptInitialize(WPARAM);

int OptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.szTitle.a = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;

	odp.pDialog = new CMainOptionsDlg();
	odp.szTab.a = LPGEN("General");
	g_plugin.addOptions(wParam, &odp);

	odp.pDialog = new CTabsOptionsDlg();
	odp.szTab.a = LPGEN("Tabs");
	g_plugin.addOptions(wParam, &odp);

	odp.pDialog = new CLayoutOptionsDlg();
	odp.szTab.a = LPGEN("Layout");
	g_plugin.addOptions(wParam, &odp);

	odp.pDialog = new CLogOptionsDlg();
	odp.szTab.a = LPGEN("Event log");
	g_plugin.addOptions(wParam, &odp);

	////////////////////////////////////////////////////////////////////////////////////////
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Typing notify");
	odp.pDialog = new CTypeOptionsDlg();
	g_plugin.addOptions(wParam, &odp);

	////////////////////////////////////////////////////////////////////////////////////////
	ChatOptInitialize(wParam);
	return 0;
}
