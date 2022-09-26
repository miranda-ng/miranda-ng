/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

#include "m_fontservice.h"

int ChatOptionsInitialize(WPARAM);

struct
{
	wchar_t* szDescr;
	COLORREF defColour;
	wchar_t* szDefFace;
	uint8_t  defStyle;
	char     defSize;
}
static fontOptionsList[] =
{
	{ LPGENW("Outgoing messages"), RGB(106, 106, 106), L"Arial",    0, -12},
	{ LPGENW("Incoming messages"), RGB(0, 0, 0),       L"Arial",    0, -12},
	{ LPGENW("Outgoing name"),     RGB(89, 89, 89),    L"Arial",    DBFONTF_BOLD, -12},
	{ LPGENW("Outgoing time"),     RGB(0, 0, 0),       L"Terminal", DBFONTF_BOLD, -9},
	{ LPGENW("Outgoing colon"),    RGB(89, 89, 89),    L"Arial",    0, -11},
	{ LPGENW("Incoming name"),     RGB(215, 0, 0),     L"Arial",    DBFONTF_BOLD, -12},
	{ LPGENW("Incoming time"),     RGB(0, 0, 0),       L"Terminal", DBFONTF_BOLD, -9},
	{ LPGENW("Incoming colon"),    RGB(215, 0, 0),     L"Arial",    0, -11},
	{ LPGENW("Message area"),      RGB(0, 0, 0),       L"Arial",    0, -12},
	{ LPGENW("Other events"),      RGB(90, 90, 160),   L"Arial",    0, -12},
};

bool LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF * colour)
{
	if (i >= _countof(fontOptionsList))
		return false;

	char str[32];

	if (colour) {
		mir_snprintf(str, "SRMFont%dCol", i);
		*colour = g_plugin.getDword(str, fontOptionsList[i].defColour);
	}
	if (lf) {
		mir_snprintf(str, "SRMFont%dSize", i);
		lf->lfHeight = (char)g_plugin.getByte(str, Utils_CorrectFontSize(fontOptionsList[i].defSize));
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "SRMFont%dSty", i);
		int style = g_plugin.getByte(str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & DBFONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "SRMFont%d", i);

		ptrW wszFontFace(g_plugin.getWStringA(str));
		if (wszFontFace == nullptr)
			wcsncpy_s(lf->lfFaceName, fontOptionsList[i].szDefFace, _TRUNCATE);
		else
			mir_wstrncpy(lf->lfFaceName, wszFontFace, _countof(lf->lfFaceName));

		mir_snprintf(str, "SRMFont%dSet", i);
		lf->lfCharSet = g_plugin.getByte(str, DEFAULT_CHARSET);
	}
	return true;
}

void RegisterSRMMFonts(void)
{
	FontIDW fontid = {};
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID;
	for (int i = 0; i < _countof(fontOptionsList); i++) {
		strncpy_s(fontid.dbSettingsGroup, SRMMMOD, _TRUNCATE);
		wcsncpy_s(fontid.group, LPGENW("Message sessions") L"/" LPGENW("Message log"), _TRUNCATE);
		wcsncpy_s(fontid.name, fontOptionsList[i].szDescr, _TRUNCATE);

		char idstr[10];
		mir_snprintf(idstr, "SRMFont%d", i);
		strncpy_s(fontid.setting, idstr, _TRUNCATE);
		fontid.order = i;

		fontid.flags &= ~FIDF_CLASSMASK;
		fontid.flags |= (fontOptionsList[i].defStyle == DBFONTF_BOLD) ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL;

		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = Utils_CorrectFontSize(fontOptionsList[i].defSize);
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		wcsncpy_s(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _TRUNCATE);
		fontid.deffontsettings.charset = DEFAULT_CHARSET;
		wcsncpy_s(fontid.backgroundGroup, LPGENW("Message sessions") L"/" LPGENW("Message log"), _TRUNCATE);
		wcsncpy_s(fontid.backgroundName, LPGENW("Background"), _TRUNCATE);
		g_plugin.addFont(&fontid);
	}

	ColourIDW colourid = {};
	strncpy_s(colourid.dbSettingsGroup, SRMMMOD, _TRUNCATE);
	strncpy_s(colourid.setting, SRMSGSET_BKGCOLOUR, _TRUNCATE);
	colourid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	wcsncpy_s(colourid.name, LPGENW("Background"), _TRUNCATE);
	wcsncpy_s(colourid.group, LPGENW("Message sessions") L"/" LPGENW("Message log"), _TRUNCATE);
	g_plugin.addColor(&colourid);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	uint32_t style;
	wchar_t* szDescr;
}
static statusValues[] =
{
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE,    LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY,  LPGENW("Not available") },
	{ PF2_LIGHTDND,  LPGENW("Occupied") },
	{ PF2_HEAVYDND,  LPGENW("Do not disturb") },
	{ PF2_FREECHAT,  LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") }
};

class COptionMainDlg : public CDlgBase
{
	CCtrlEdit  edtNFlash, edtAvatarH, edtSecs;
	CCtrlCheck chkAutoMin, chkAutoClose, chkSavePerContact, chkDoNotStealFocus, chkStatusWin;
	CCtrlCheck chkDelTemp, chkCascade, chkCharCount, chkCtrlSupport;
	CCtrlCheck chkAvatar, chkLimitAvatar;
	CCtrlCheck chkShowSend, chkShowButtons;

	CCtrlTreeView tree;

	void FillCheckBoxTree(uint32_t style)
	{
		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE;
		for (auto &it : statusValues) {
			tvis.item.lParam = it.style;
			tvis.item.pszText = TranslateW(it.szDescr);
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;
			tvis.item.iImage = (style & it.style) != 0;
			tree.InsertItem(&tvis);
		}
	}

	uint32_t MakeCheckBoxTreeFlags()
	{
		uint32_t flags = 0;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE;
		tvi.hItem = tree.GetRoot();
		while (tvi.hItem) {
			tree.GetItem(&tvi);
			if (tvi.iImage)
				flags |= tvi.lParam;
			tvi.hItem = tree.GetNextSibling(tvi.hItem);
		}
		return flags;
	}

public:
	COptionMainDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGDLG),
		tree(this, IDC_POPLIST),
		edtSecs(this, IDC_SECONDS),
		edtNFlash(this, IDC_NFLASHES),
		edtAvatarH(this, IDC_AVATARHEIGHT),
		chkAvatar(this, IDC_AVATARSUPPORT),
		chkDelTemp(this, IDC_DELTEMP),
		chkAutoMin(this, IDC_AUTOMIN),
		chkCascade(this, IDC_CASCADE),
		chkShowSend(this, IDC_SHOWSENDBTN),
		chkAutoClose(this, IDC_AUTOCLOSE),
		chkCharCount(this, IDC_CHARCOUNT),
		chkStatusWin(this, IDC_STATUSWIN),
		chkLimitAvatar(this, IDC_LIMITAVATARH),
		chkShowButtons(this, IDC_SHOWBUTTONLINE),
		chkCtrlSupport(this, IDC_CTRLSUPPORT),
		chkSavePerContact(this, IDC_SAVEPERCONTACT),
		chkDoNotStealFocus(this, IDC_DONOTSTEALFOCUS)
	{
		tree.SetFlags(MTREE_CHECKBOX);

		chkAvatar.OnChange = Callback(this, &COptionMainDlg::onChange_Avatar);
		chkAutoMin.OnChange = Callback(this, &COptionMainDlg::onChange_AutoMin);
		chkAutoClose.OnChange = Callback(this, &COptionMainDlg::onChange_AutoClose);
		chkLimitAvatar.OnChange = Callback(this, &COptionMainDlg::onChange_LimitAvatar);
		chkSavePerContact.OnChange = Callback(this, &COptionMainDlg::onChange_SavePerContact);

		CreateLink(edtNFlash, g_plugin.nFlashMax);
		CreateLink(edtAvatarH, g_plugin.iAvatarHeight);

		CreateLink(chkAvatar, g_plugin.bShowAvatar);
		CreateLink(chkLimitAvatar, g_plugin.bLimitAvatarHeight);

		CreateLink(chkDelTemp, g_plugin.bDeleteTempCont);
		CreateLink(chkCascade, g_plugin.bCascade);
		CreateLink(chkAutoMin, g_plugin.bAutoMin);
		CreateLink(chkAutoClose, g_plugin.bAutoClose);
		CreateLink(chkShowSend, g_plugin.bSendButton);
		CreateLink(chkCharCount, g_plugin.bShowReadChar);
		CreateLink(chkStatusWin, g_plugin.bUseStatusWinIcon);
		CreateLink(chkShowButtons, g_plugin.bShowButtons);
		CreateLink(chkCtrlSupport, g_plugin.bCtrlSupport);
		CreateLink(chkSavePerContact, g_plugin.bSavePerContact);
		CreateLink(chkDoNotStealFocus, g_plugin.bDoNotStealFocus);
	}

	bool OnInitDialog() override
	{
		FillCheckBoxTree(g_plugin.popupFlags);

		uint32_t msgTimeout = g_plugin.msgTimeout;
		edtSecs.SetInt((msgTimeout >= 5000) ? msgTimeout / 1000 : 5);
		
		chkCascade.Enable(!g_plugin.bSavePerContact);
		chkCtrlSupport.Enable(!g_plugin.bAutoClose);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.popupFlags = MakeCheckBoxTreeFlags();

		uint32_t msgTimeout = edtSecs.GetInt() * 1000;
		if (msgTimeout < 5000)
			msgTimeout = 5000;
		g_plugin.msgTimeout = msgTimeout;

		Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
		return true;
	}

	void onChange_AutoMin(CCtrlCheck*)
	{
		if (m_bInitialized)
			chkAutoClose.SetState(false);
		chkCtrlSupport.Enable(!chkAutoClose.GetState());
	}

	void onChange_AutoClose(CCtrlCheck*)
	{
		if (m_bInitialized)
			chkAutoMin.SetState(false);
		chkCtrlSupport.Enable(!chkAutoClose.GetState());
	}

	void onChange_SavePerContact(CCtrlCheck*)
	{
		chkCascade.Enable(!chkSavePerContact.GetState());
	}

	void onChange_Avatar(CCtrlCheck*)
	{
		bool bEnabled = chkAvatar.GetState();
		edtAvatarH.Enable(bEnabled);
		chkLimitAvatar.Enable(bEnabled);
	}

	void onChange_LimitAvatar(CCtrlCheck*)
	{
		edtAvatarH.Enable(chkLimitAvatar.GetState());
	}

	void onChange_Tree(CCtrlTreeView::TEventInfo*)
	{
		NotifyChange();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptionLogDlg : public CDlgBase
{
	HBRUSH hBkgColourBrush;

	CCtrlCheck chkLoadUnread, chkLoadCount, chkLoadTime, chkDate, chkTime, chkSecs, chkIcons;
	CCtrlCheck chkShowNames, chkFormat;
	CCtrlSpin  spinCount, spinTime;

public:
	COptionLogDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGLOG),
		chkSecs(this, IDC_SHOWSECS),
		chkDate(this, IDC_SHOWDATES),
		chkTime(this, IDC_SHOWTIMES),
		chkIcons(this, IDC_SHOWLOGICONS),
		chkFormat(this, IDC_SHOWFORMATTING),
		chkLoadTime(this, IDC_LOADTIME),
		chkShowNames(this, IDC_SHOWNAMES),
		chkLoadCount(this, IDC_LOADCOUNT),
		chkLoadUnread(this, IDC_LOADUNREAD),
		
		spinTime(this, IDC_LOADTIMESPIN, 12 * 60),
		spinCount(this, IDC_LOADCOUNTSPIN, 100)
	{
		chkTime.OnChange = Callback(this, &COptionLogDlg::onChange_Time);
		chkLoadUnread.OnChange = chkLoadCount.OnChange = chkLoadTime.OnChange = Callback(this, &COptionLogDlg::onChange_Load);

		CreateLink(chkSecs, g_plugin.bShowSecs);
		CreateLink(chkDate, g_plugin.bShowDate);
		CreateLink(chkTime, g_plugin.bShowTime);
		CreateLink(chkIcons, g_plugin.bShowIcons);
		CreateLink(chkFormat, g_plugin.bShowFormat);
		CreateLink(chkShowNames, g_plugin.bShowNames);
	}
	
	bool OnInitDialog() override
	{
		switch (g_plugin.iLoadHistory) {
		case LOADHISTORY_UNREAD:
			chkLoadUnread.SetState(true);
			break;
		case LOADHISTORY_COUNT:
			chkLoadCount.SetState(true);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTN), TRUE);
			spinCount.Enable();
			break;
		case LOADHISTORY_TIME:
			chkLoadTime.SetState(true);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMEN), TRUE);
			spinTime.Enable();
			EnableWindow(GetDlgItem(m_hwnd, IDC_STMINSOLD), TRUE);
			break;
		}

		spinCount.SetPosition(g_plugin.nLoadCount);
		spinTime.SetPosition(g_plugin.nLoadTime);
		return true;
	}

	bool OnApply() override
	{
		if (chkLoadCount.GetState())
			g_plugin.iLoadHistory = LOADHISTORY_COUNT;
		else if (chkLoadTime.GetState())
			g_plugin.iLoadHistory = LOADHISTORY_TIME;
		else
			g_plugin.iLoadHistory = LOADHISTORY_UNREAD;
		g_plugin.nLoadCount = spinCount.GetPosition();
		g_plugin.nLoadTime = spinTime.GetPosition();

		FreeMsgLogIcons();
		LoadMsgLogIcons();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
		return true;
	}

	void OnDestroy() override
	{
		DeleteObject(hBkgColourBrush);
	}

	void onChange_Load(CCtrlCheck*)
	{
		bool bEnabled = chkLoadCount.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTN), bEnabled);
		spinCount.Enable(bEnabled);

		bEnabled = chkLoadTime.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMEN), bEnabled);
		spinTime.Enable(bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STMINSOLD), bEnabled);
	}

	void onChange_Time(CCtrlCheck*)
	{
		bool bEnabled = chkTime.GetState();
		chkSecs.Enable(bEnabled);
		chkDate.Enable(bEnabled);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptionTypingDlg : public CDlgBase
{
	HANDLE hItemNew, hItemUnknown;

	CCtrlClc clist;
	CCtrlCheck chkType, chkTypeWin, chkTypeTray, chkTypeClist, chkTypeBalloon;

public:
	COptionTypingDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGTYPE),
		clist(this, IDC_CLIST),
		chkType(this, IDC_SHOWNOTIFY),
		chkTypeWin(this, IDC_TYPEWIN),
		chkTypeTray(this, IDC_TYPETRAY),
		chkTypeClist(this, IDC_NOTIFYTRAY),
		chkTypeBalloon(this, IDC_NOTIFYBALLOON)
	{
		chkType.OnChange = Callback(this, &COptionTypingDlg::onChange_ShowNotify);
		chkTypeTray.OnChange = Callback(this, &COptionTypingDlg::onChange_Tray);

		CreateLink(chkType, g_plugin.bShowTyping);
		CreateLink(chkTypeWin, g_plugin.bShowTypingWin);
		CreateLink(chkTypeTray, g_plugin.bShowTypingTray);
		CreateLink(chkTypeClist, g_plugin.bShowTypingClist);
	}

	void ResetCList(CCtrlClc::TEventInfo* = nullptr)
	{
		clist.SetUseGroups(Clist::UseGroups);
		clist.SetHideEmptyGroups(1);
	}

	void RebuildList(CCtrlClc::TEventInfo* = nullptr)
	{
		uint8_t defType = g_plugin.bTypingNew;
		if (hItemNew && defType)
			clist.SetCheck(hItemNew, 1);

		if (hItemUnknown && g_plugin.bTypingUnknown)
			clist.SetCheck(hItemUnknown, 1);

		for (auto &hContact : Contacts()) {
			HANDLE hItem = clist.FindContact(hContact);
			if (hItem && g_plugin.getByte(hContact, SRMSGSET_TYPING, defType))
				clist.SetCheck(hItem, 1);
		}
	}

	void SaveList()
	{
		if (hItemNew)
			g_plugin.bTypingNew = clist.GetCheck(hItemNew);

		if (hItemUnknown)
			g_plugin.bTypingUnknown = clist.GetCheck(hItemUnknown);

		for (auto &hContact : Contacts()) {
			HANDLE hItem = clist.FindContact(hContact);
			if (hItem)
				g_plugin.setByte(hContact, SRMSGSET_TYPING, clist.GetCheck(hItem));
		}
	}

	bool OnInitDialog() override
	{
		CLCINFOITEM cii = { sizeof(cii) };
		cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
		cii.pszText = TranslateT("** New contacts **");
		hItemNew = clist.AddInfoItem(&cii);
		cii.pszText = TranslateT("** Unknown contacts **");
		hItemUnknown = clist.AddInfoItem(&cii);

		SetWindowLongPtr(clist.GetHwnd(), GWL_STYLE, GetWindowLongPtr(clist.GetHwnd(), GWL_STYLE) | (CLS_SHOWHIDDEN) | (CLS_NOHIDEOFFLINE));
		ResetCList();

		clist.OnListRebuilt = Callback(this, &COptionTypingDlg::RebuildList);
		clist.OnOptionsChanged = Callback(this, &COptionTypingDlg::ResetCList);
		return true;
	}

	bool OnApply() override
	{
		SaveList();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
		return true;
	}

	void onChange_Tray(CCtrlCheck*)
	{
		bool bStatus = chkTypeTray.GetState() != 0;
		chkTypeClist.Enable(bStatus);
		chkTypeBalloon.Enable(bStatus);
	}

	void onChange_ShowNotify(CCtrlCheck*)
	{
		bool bStatus = chkType.GetState();
		chkTypeWin.Enable(bStatus);
		chkTypeTray.Enable(bStatus);

		bool bTrayStatus = chkTypeTray.GetState();
		chkTypeClist.Enable(bStatus && bTrayStatus);
		chkTypeBalloon.Enable(bStatus && bTrayStatus);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsTabDlg : public CDlgBase
{
	CCtrlCheck m_chkTabs, m_chkTabsBottom, m_chkTabsClose;

public:
	COptionsTabDlg() :
		CDlgBase(g_plugin, IDD_OPT_TABS),
		m_chkTabs(this, IDC_USETABS),
		m_chkTabsBottom(this, IDC_TABSBOTTOM),
		m_chkTabsClose(this, IDC_CLOSETABS)
	{
		m_chkTabs.OnChange = Callback(this, &COptionsTabDlg::onChange_Tabs);
	}

	bool OnInitDialog() override
	{
		m_chkTabs.SetState(g_Settings.bTabsEnable);
		m_chkTabsBottom.SetState(g_Settings.bTabsAtBottom);
		m_chkTabsClose.SetState(g_Settings.bTabCloseOnDblClick);
		return true;
	}

	bool OnApply() override
	{
		uint8_t bOldValue = db_get_b(0, CHAT_MODULE, "Tabs", 1);

		db_set_b(0, CHAT_MODULE, "Tabs", m_chkTabs.GetState());
		db_set_b(0, CHAT_MODULE, "TabBottom", m_chkTabsBottom.GetState());
		db_set_b(0, CHAT_MODULE, "TabCloseOnDblClick", m_chkTabsClose.GetState());

		g_chatApi.ReloadSettings();

		if (bOldValue != db_get_b(0, CHAT_MODULE, "Tabs", 1)) {
			if (g_pTabDialog != nullptr)
				g_pTabDialog->Close();
			g_Settings.bTabsEnable = db_get_b(0, CHAT_MODULE, "Tabs", 1) != 0;
		}
		else Chat_UpdateOptions();
		return true;
	}

	void onChange_Tabs(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->GetState() != 0;
		m_chkTabsBottom.Enable(bEnabled);
		m_chkTabsClose.Enable(bEnabled);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

int OptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.szTab.a = LPGEN("Messaging");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTitle.a = LPGEN("Message sessions");
	odp.pDialog = new COptionMainDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Messaging log");
	odp.pDialog = new COptionLogDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Typing notify");
	odp.pDialog = new COptionTypingDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Tabs");
	odp.pDialog = new COptionsTabDlg();
	g_plugin.addOptions(wParam, &odp);

	ChatOptionsInitialize(wParam);
	return 0;
}
