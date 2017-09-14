/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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

struct FontOptionsList
{
	const wchar_t* szDescr;
	COLORREF     defColour;
	const wchar_t* szDefFace;
	BYTE         defStyle;
	char         defSize;
}
static const fontOptionsList[] =
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

static BYTE MsgDlgGetFontDefaultCharset(const wchar_t*)
{
  return DEFAULT_CHARSET;
}

bool LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF * colour)
{
	if (i >= _countof(fontOptionsList))
		return false;

	char str[32];

	if (colour) {
		mir_snprintf(str, "SRMFont%dCol", i);
		*colour = db_get_dw(0, SRMMMOD, str, fontOptionsList[i].defColour);
	}
	if (lf) {
		mir_snprintf(str, "SRMFont%dSize", i);
		lf->lfHeight = (char)db_get_b(0, SRMMMOD, str, fontOptionsList[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "SRMFont%dSty", i);
		int style = db_get_b(0, SRMMMOD, str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & DBFONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "SRMFont%d", i);

		ptrW wszFontFace(db_get_wsa(0, SRMMMOD, str));
		if (wszFontFace == nullptr)
			wcsncpy_s(lf->lfFaceName, fontOptionsList[i].szDefFace, _TRUNCATE);
		else
			mir_wstrncpy(lf->lfFaceName, wszFontFace, _countof(lf->lfFaceName));

		mir_snprintf(str, "SRMFont%dSet", i);
		lf->lfCharSet = db_get_b(0, SRMMMOD, str, MsgDlgGetFontDefaultCharset(lf->lfFaceName));
	}
	return true;
}

void RegisterSRMMFonts(void)
{
	char idstr[10];

	FontIDW fontid = { sizeof(fontid) };
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID;
	for (int i = 0; i < _countof(fontOptionsList); i++) {
		strncpy_s(fontid.dbSettingsGroup, SRMMMOD, _TRUNCATE);
		wcsncpy_s(fontid.group, LPGENW("Message log"), _TRUNCATE);
		wcsncpy_s(fontid.name, fontOptionsList[i].szDescr, _TRUNCATE);
		mir_snprintf(idstr, "SRMFont%d", i);
		strncpy_s(fontid.prefix, idstr, _TRUNCATE);
		fontid.order = i;

		fontid.flags &= ~FIDF_CLASSMASK;
		fontid.flags |= (fontOptionsList[i].defStyle == DBFONTF_BOLD) ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL;

		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		wcsncpy_s(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _TRUNCATE);
		fontid.deffontsettings.charset = MsgDlgGetFontDefaultCharset(fontOptionsList[i].szDefFace);
		wcsncpy_s(fontid.backgroundGroup, LPGENW("Message log"), _TRUNCATE);
		wcsncpy_s(fontid.backgroundName, LPGENW("Background"), _TRUNCATE);
		Font_RegisterW(&fontid);
	}

	ColourIDW colourid = { sizeof(colourid) };
	strncpy_s(colourid.dbSettingsGroup, SRMMMOD, _TRUNCATE);
	strncpy_s(colourid.setting, SRMSGSET_BKGCOLOUR, _TRUNCATE);
	colourid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	wcsncpy_s(colourid.name, LPGENW("Background"), _TRUNCATE);
	wcsncpy_s(colourid.group, LPGENW("Message log"), _TRUNCATE);
	Colour_RegisterW(&colourid);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct CheckBoxValues_t
{
	DWORD  style;
	wchar_t* szDescr;
}
statusValues[] =
{
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE, LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY, LPGENW("Not available") },
	{ PF2_LIGHTDND, LPGENW("Occupied") },
	{ PF2_HEAVYDND, LPGENW("Do not disturb") },
	{ PF2_FREECHAT, LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") },
	{ PF2_OUTTOLUNCH, LPGENW("Out to lunch") },
	{ PF2_ONTHEPHONE, LPGENW("On the phone") }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
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
		if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

/////////////////////////////////////////////////////////////////////////////////////////

class COptionMainDlg : public CDlgBase
{
	CCtrlCheck chkAutoMin, chkAutoClose, chkSavePerContact;
	CCtrlCheck chkAvatar, chkLimitAvatar;
	CCtrlCheck chkSendOnEnter, chkSendOnDblEnter;

public:
	COptionMainDlg()
		: CDlgBase(g_hInst, IDD_OPT_MSGDLG),
		chkAvatar(this, IDC_AVATARSUPPORT),
		chkAutoMin(this, IDC_AUTOMIN),
		chkAutoClose(this, IDC_AUTOCLOSE),
		chkLimitAvatar(this, IDC_LIMITAVATARH),
		chkSendOnEnter(this, IDC_SENDONENTER),
		chkSendOnDblEnter(this, IDC_SENDONDBLENTER),
		chkSavePerContact(this, IDC_SAVEPERCONTACT)
	{
		chkAvatar.OnChange = Callback(this, &COptionMainDlg::onChange_Avatar);
		chkAutoMin.OnChange = Callback(this, &COptionMainDlg::onChange_AutoMin);
		chkAutoClose.OnChange = Callback(this, &COptionMainDlg::onChange_AutoClose);
		chkLimitAvatar.OnChange = Callback(this, &COptionMainDlg::onChange_LimitAvatar);
		chkSendOnEnter.OnChange = Callback(this, &COptionMainDlg::onChange_SendOnEnter);
		chkSendOnDblEnter.OnChange = Callback(this, &COptionMainDlg::onChange_SendOnDblEnter);
		chkSavePerContact.OnChange = Callback(this, &COptionMainDlg::onChange_SavePerContact);
	}

	virtual void OnInitDialog() override
	{
		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_POPLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_POPLIST), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		FillCheckBoxTree(GetDlgItem(m_hwnd, IDC_POPLIST), statusValues, _countof(statusValues), db_get_dw(0, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS));

		CheckDlgButton(m_hwnd, IDC_DELTEMP, g_dat.bDeleteTempCont);
		CheckDlgButton(m_hwnd, IDC_AUTOMIN, g_dat.bAutoMin);
		CheckDlgButton(m_hwnd, IDC_CASCADE, g_dat.bCascade);
		CheckDlgButton(m_hwnd, IDC_AUTOCLOSE, g_dat.bAutoClose);
		CheckDlgButton(m_hwnd, IDC_CHARCOUNT, g_dat.bShowReadChar);
		CheckDlgButton(m_hwnd, IDC_STATUSWIN, g_dat.bUseStatusWinIcon);
		CheckDlgButton(m_hwnd, IDC_CTRLSUPPORT, g_dat.bCtrlSupport);
		CheckDlgButton(m_hwnd, IDC_SHOWSENDBTN, g_dat.bSendButton);
		CheckDlgButton(m_hwnd, IDC_SENDONENTER, g_dat.bSendOnEnter);
		CheckDlgButton(m_hwnd, IDC_LIMITAVATARH, db_get_b(0, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT));
		CheckDlgButton(m_hwnd, IDC_SENDONDBLENTER, g_dat.bSendOnDblEnter);
		CheckDlgButton(m_hwnd, IDC_AVATARSUPPORT, g_dat.bShowAvatar);
		CheckDlgButton(m_hwnd, IDC_SHOWBUTTONLINE, g_dat.bShowButtons);
		CheckDlgButton(m_hwnd, IDC_SAVEPERCONTACT, g_dat.bSavePerContact);
		CheckDlgButton(m_hwnd, IDC_DONOTSTEALFOCUS, g_dat.bDoNotStealFocus);

		DWORD avatarHeight = db_get_dw(0, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT);
		SetDlgItemInt(m_hwnd, IDC_NFLASHES, db_get_b(0, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT), FALSE);
		SetDlgItemInt(m_hwnd, IDC_AVATARHEIGHT, avatarHeight, FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LIMITAVATARH), IsDlgButtonChecked(m_hwnd, IDC_AVATARSUPPORT));
		if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_AVATARSUPPORT))
			EnableWindow(GetDlgItem(m_hwnd, IDC_AVATARHEIGHT), FALSE);
		else
			EnableWindow(GetDlgItem(m_hwnd, IDC_AVATARHEIGHT), IsDlgButtonChecked(m_hwnd, IDC_LIMITAVATARH));

		DWORD msgTimeout = db_get_dw(0, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
		SetDlgItemInt(m_hwnd, IDC_SECONDS, msgTimeout >= SRMSGSET_MSGTIMEOUT_MIN ? msgTimeout / 1000 : SRMSGDEFSET_MSGTIMEOUT / 1000, FALSE);

		EnableWindow(GetDlgItem(m_hwnd, IDC_CASCADE), !g_dat.bSavePerContact);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CTRLSUPPORT), !g_dat.bAutoClose);
	}

	virtual void OnApply() override
	{
		db_set_dw(0, SRMMMOD, SRMSGSET_POPFLAGS, MakeCheckBoxTreeFlags(GetDlgItem(m_hwnd, IDC_POPLIST)));
		db_set_b(0, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_DONOTSTEALFOCUS));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWBUTTONLINE));
		db_set_b(0, SRMMMOD, SRMSGSET_AUTOMIN, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_AUTOMIN));
		db_set_b(0, SRMMMOD, SRMSGSET_AUTOCLOSE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_AUTOCLOSE));
		db_set_b(0, SRMMMOD, SRMSGSET_SAVEPERCONTACT, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SAVEPERCONTACT));
		db_set_b(0, SRMMMOD, SRMSGSET_CASCADE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_CASCADE));
		db_set_b(0, SRMMMOD, SRMSGSET_SENDONENTER, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SENDONENTER));
		db_set_b(0, SRMMMOD, SRMSGSET_SENDONDBLENTER, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SENDONDBLENTER));
		db_set_b(0, SRMMMOD, SRMSGSET_STATUSICON, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_STATUSWIN));

		db_set_b(0, SRMMMOD, SRMSGSET_AVATARENABLE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_AVATARSUPPORT));
		db_set_b(0, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_LIMITAVATARH));

		DWORD avatarHeight = GetDlgItemInt(m_hwnd, IDC_AVATARHEIGHT, nullptr, TRUE);
		db_set_dw(0, SRMMMOD, SRMSGSET_AVHEIGHT, avatarHeight <= 0 ? SRMSGDEFSET_AVHEIGHT : avatarHeight);

		db_set_b(0, SRMMMOD, SRMSGSET_SENDBUTTON, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWSENDBTN));
		db_set_b(0, SRMMMOD, SRMSGSET_CHARCOUNT, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_CHARCOUNT));
		db_set_b(0, SRMMMOD, SRMSGSET_CTRLSUPPORT, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_CTRLSUPPORT));
		db_set_b(0, SRMMMOD, SRMSGSET_DELTEMP, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_DELTEMP));
		db_set_b(0, SRMMMOD, SRMSGSET_FLASHCOUNT, (BYTE)GetDlgItemInt(m_hwnd, IDC_NFLASHES, nullptr, TRUE));

		DWORD msgTimeout = GetDlgItemInt(m_hwnd, IDC_SECONDS, nullptr, TRUE) * 1000;
		if (msgTimeout < SRMSGSET_MSGTIMEOUT_MIN) msgTimeout = SRMSGDEFSET_MSGTIMEOUT;
		db_set_dw(0, SRMMMOD, SRMSGSET_MSGTIMEOUT, msgTimeout);

		ReloadGlobals();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
	}

	void onChange_AutoMin(CCtrlCheck*)
	{
		CheckDlgButton(m_hwnd, IDC_AUTOCLOSE, BST_UNCHECKED);
	}

	void onChange_AutoClose(CCtrlCheck*)
	{
		CheckDlgButton(m_hwnd, IDC_AUTOMIN, BST_UNCHECKED);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CTRLSUPPORT), BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_AUTOCLOSE));
	}

	void onChange_SendOnEnter(CCtrlCheck*)
	{
		CheckDlgButton(m_hwnd, IDC_SENDONDBLENTER, BST_UNCHECKED);
	}

	void onChange_SendOnDblEnter(CCtrlCheck*)
	{
		CheckDlgButton(m_hwnd, IDC_SENDONENTER, BST_UNCHECKED);
	}

	void onChange_SavePerContact(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_CASCADE), BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_SAVEPERCONTACT));
	}

	void onChange_Avatar(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_LIMITAVATARH), IsDlgButtonChecked(m_hwnd, IDC_AVATARSUPPORT));
		if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_AVATARSUPPORT))
			EnableWindow(GetDlgItem(m_hwnd, IDC_AVATARHEIGHT), FALSE);
		else EnableWindow(GetDlgItem(m_hwnd, IDC_AVATARHEIGHT), IsDlgButtonChecked(m_hwnd, IDC_LIMITAVATARH));
	}

	void onChange_LimitAvatar(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_AVATARHEIGHT), IsDlgButtonChecked(m_hwnd, IDC_LIMITAVATARH));
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY) {
			LPNMHDR pHdr = (LPNMHDR)lParam;
			if (pHdr->code == NM_CLICK && pHdr->idFrom == IDC_POPLIST) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
					}
				}
			}
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptionLogDlg : public CDlgBase
{
	HBRUSH hBkgColourBrush;

	CCtrlCheck chkLoadUnread, chkLoadCount, chkLoadTime, chkTimes;

public:
	COptionLogDlg()
		: CDlgBase(g_hInst, IDD_OPT_MSGLOG),
		chkTimes(this, IDC_SHOWTIMES),
		chkLoadTime(this, IDC_LOADTIME),
		chkLoadCount(this, IDC_LOADCOUNT),
		chkLoadUnread(this, IDC_LOADUNREAD)
	{
		chkTimes.OnChange = Callback(this, &COptionLogDlg::onChange_Time);
		chkLoadUnread.OnChange = chkLoadCount.OnChange = chkLoadTime.OnChange = Callback(this, &COptionLogDlg::onChange_Load);
	}
	
	virtual void OnInitDialog() override
	{
		switch (db_get_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
		case LOADHISTORY_UNREAD:
			CheckDlgButton(m_hwnd, IDC_LOADUNREAD, BST_CHECKED);
			break;
		case LOADHISTORY_COUNT:
			CheckDlgButton(m_hwnd, IDC_LOADCOUNT, BST_CHECKED);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTN), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTSPIN), TRUE);
			break;
		case LOADHISTORY_TIME:
			CheckDlgButton(m_hwnd, IDC_LOADTIME, BST_CHECKED);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMEN), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMESPIN), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_STMINSOLD), TRUE);
			break;
		}
		SendDlgItemMessage(m_hwnd, IDC_LOADCOUNTSPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(m_hwnd, IDC_LOADCOUNTSPIN, UDM_SETPOS, 0, db_get_w(0, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
		SendDlgItemMessage(m_hwnd, IDC_LOADTIMESPIN, UDM_SETRANGE, 0, MAKELONG(12 * 60, 0));
		SendDlgItemMessage(m_hwnd, IDC_LOADTIMESPIN, UDM_SETPOS, 0, db_get_w(0, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

		CheckDlgButton(m_hwnd, IDC_SHOWLOGICONS, db_get_b(0, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS));
		CheckDlgButton(m_hwnd, IDC_SHOWNAMES, !db_get_b(0, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES));
		CheckDlgButton(m_hwnd, IDC_SHOWTIMES, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME));
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWSECS), IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES));
		CheckDlgButton(m_hwnd, IDC_SHOWSECS, db_get_b(0, SRMMMOD, SRMSGSET_SHOWSECS, SRMSGDEFSET_SHOWSECS));
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWDATES), IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES));
		CheckDlgButton(m_hwnd, IDC_SHOWDATES, db_get_b(0, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE));
		CheckDlgButton(m_hwnd, IDC_SHOWFORMATTING, db_get_b(0, SRMMMOD, SRMSGSET_SHOWFORMAT, SRMSGDEFSET_SHOWFORMAT));
	}

	virtual void OnApply() override
	{
		if (IsDlgButtonChecked(m_hwnd, IDC_LOADCOUNT))
			db_set_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
		else if (IsDlgButtonChecked(m_hwnd, IDC_LOADTIME))
			db_set_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
		else
			db_set_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
		db_set_w(0, SRMMMOD, SRMSGSET_LOADCOUNT, (WORD)SendDlgItemMessage(m_hwnd, IDC_LOADCOUNTSPIN, UDM_GETPOS, 0, 0));
		db_set_w(0, SRMMMOD, SRMSGSET_LOADTIME, (WORD)SendDlgItemMessage(m_hwnd, IDC_LOADTIMESPIN, UDM_GETPOS, 0, 0));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWLOGICONS, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWLOGICONS));
		db_set_b(0, SRMMMOD, SRMSGSET_HIDENAMES, (BYTE)BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_SHOWNAMES));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWTIME, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWSECS, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWSECS));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWDATE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWDATES));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWFORMAT, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWFORMATTING));

		FreeMsgLogIcons();
		LoadMsgLogIcons();
		ReloadGlobals();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
	}

	virtual void OnDestroy() override
	{
		DeleteObject(hBkgColourBrush);
	}

	void onChange_Load(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTN), IsDlgButtonChecked(m_hwnd, IDC_LOADCOUNT));
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTSPIN), IsDlgButtonChecked(m_hwnd, IDC_LOADCOUNT));
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMEN), IsDlgButtonChecked(m_hwnd, IDC_LOADTIME));
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMESPIN), IsDlgButtonChecked(m_hwnd, IDC_LOADTIME));
		EnableWindow(GetDlgItem(m_hwnd, IDC_STMINSOLD), IsDlgButtonChecked(m_hwnd, IDC_LOADTIME));
	}

	void onChange_Time(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWSECS), IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES));
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWDATES), IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptionTypingDlg : public CDlgBase
{
	HANDLE hItemNew, hItemUnknown;

	CCtrlCheck chkTray, chkShowNotify;

public:
	COptionTypingDlg()
		: CDlgBase(g_hInst, IDD_OPT_MSGTYPE),
		chkTray(this, IDC_TYPETRAY),
		chkShowNotify(this, IDC_SHOWNOTIFY)
	{
		chkTray.OnChange = Callback(this, &COptionTypingDlg::onChange_Tray);
		chkShowNotify.OnChange = Callback(this, &COptionTypingDlg::onChange_ShowNotify);
	}

	void ResetCList()
	{
		SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETUSEGROUPS, db_get_b(0, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT), 0);
		SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
	}

	void RebuildList()
	{
		BYTE defType = db_get_b(0, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW);
		if (hItemNew && defType)
			SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemNew, 1);

		if (hItemUnknown && db_get_b(0, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
			SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemUnknown, 1);

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			HANDLE hItem = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem && db_get_b(hContact, SRMMMOD, SRMSGSET_TYPING, defType))
				SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
		}
	}

	void SaveList()
	{
		if (hItemNew)
			db_set_b(0, SRMMMOD, SRMSGSET_TYPINGNEW, (BYTE)(SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemNew, 0) ? 1 : 0));

		if (hItemUnknown)
			db_set_b(0, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, (BYTE)(SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemUnknown, 0) ? 1 : 0));

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			HANDLE hItem = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem)
				db_set_b(hContact, SRMMMOD, SRMSGSET_TYPING, (BYTE)(SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? 1 : 0));
		}
	}

	virtual void OnInitDialog() override
	{
		CLCINFOITEM cii = { sizeof(cii) };
		cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
		cii.pszText = TranslateT("** New contacts **");
		hItemNew = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		cii.pszText = TranslateT("** Unknown contacts **");
		hItemUnknown = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE) | (CLS_SHOWHIDDEN) | (CLS_NOHIDEOFFLINE));
		ResetCList();
		CheckDlgButton(m_hwnd, IDC_SHOWNOTIFY, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING));
		CheckDlgButton(m_hwnd, IDC_TYPEWIN, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN));
		CheckDlgButton(m_hwnd, IDC_TYPETRAY, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN));
		CheckDlgButton(m_hwnd, IDC_NOTIFYTRAY, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST));
		CheckDlgButton(m_hwnd, IDC_NOTIFYBALLOON, !db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST));
		EnableWindow(GetDlgItem(m_hwnd, IDC_TYPEWIN), IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(m_hwnd, IDC_TYPETRAY), IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(m_hwnd, IDC_NOTIFYTRAY), IsDlgButtonChecked(m_hwnd, IDC_TYPETRAY));
		EnableWindow(GetDlgItem(m_hwnd, IDC_NOTIFYBALLOON), IsDlgButtonChecked(m_hwnd, IDC_TYPETRAY));
	}

	virtual void OnApply() override
	{
		SaveList();
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPING, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_TYPEWIN));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_TYPETRAY));
		db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY));
		ReloadGlobals();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
	}

	void onChange_Tray(CCtrlCheck*)
	{
		bool bStatus = chkTray.GetState() != 0;
		EnableWindow(GetDlgItem(m_hwnd, IDC_NOTIFYTRAY), bStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_NOTIFYBALLOON), bStatus);
	}

	void onChange_ShowNotify(CCtrlCheck*)
	{
		bool bStatus = chkShowNotify.GetState() != 0;
		EnableWindow(GetDlgItem(m_hwnd, IDC_TYPEWIN), bStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TYPETRAY), bStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_NOTIFYTRAY), bStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_NOTIFYBALLOON), bStatus);
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY && ((NMHDR *)lParam)->idFrom == IDC_CLIST) {
			switch (((NMHDR *)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				ResetCList();
				break;
			case CLN_CHECKCHANGED:
				SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
				break;
			case CLN_LISTREBUILT:
				RebuildList();
				break;
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsTabDlg : public CDlgBase
{
	CCtrlCheck m_chkTabs, m_chkTabsBottom, m_chkTabsClose, m_chkTabsRestore;

public:
	COptionsTabDlg() :
		CDlgBase(g_hInst, IDD_OPT_TABS),
		m_chkTabs(this, IDC_USETABS),
		m_chkTabsBottom(this, IDC_TABSBOTTOM),
		m_chkTabsClose(this, IDC_CLOSETABS),
		m_chkTabsRestore(this, IDC_RESTORETABS)
	{
		m_chkTabs.OnChange = Callback(this, &COptionsTabDlg::onChange_Tabs);
	}

	virtual void OnInitDialog() override
	{
		m_chkTabs.SetState(db_get_b(0, CHAT_MODULE, "Tabs", 1));
		m_chkTabsBottom.SetState(db_get_b(0, CHAT_MODULE, "TabBottom", 1));
		m_chkTabsClose.SetState(db_get_b(0, CHAT_MODULE, "TabCloseOnDblClick", 1));
		m_chkTabsRestore.SetState(db_get_b(0, CHAT_MODULE, "TabRestore", 1));
		onChange_Tabs(&m_chkTabs);
	}

	virtual void OnApply() override
	{
		BYTE bOldValue = db_get_b(0, CHAT_MODULE, "Tabs", 1);

		db_set_b(0, CHAT_MODULE, "Tabs", m_chkTabs.GetState());
		db_set_b(0, CHAT_MODULE, "TabBottom", m_chkTabsBottom.GetState());
		db_set_b(0, CHAT_MODULE, "TabCloseOnDblClick", m_chkTabsClose.GetState());
		db_set_b(0, CHAT_MODULE, "TabRestore", m_chkTabsRestore.GetState());

		pci->ReloadSettings();

		if (bOldValue != db_get_b(0, CHAT_MODULE, "Tabs", 1)) {
			pci->SM_BroadcastMessage(nullptr, WM_CLOSE, 0, 1, FALSE);
			g_Settings.bTabsEnable = db_get_b(0, CHAT_MODULE, "Tabs", 1) != 0;
		}
		else Chat_UpdateOptions();
	}

	void onChange_Tabs(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->GetState() != 0;
		m_chkTabsBottom.Enable(bEnabled);
		m_chkTabsClose.Enable(bEnabled);
		m_chkTabsRestore.Enable(bEnabled);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int OptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.szTab.a = LPGEN("Messaging");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTitle.a = LPGEN("Message sessions");
	odp.pDialog = new COptionMainDlg();
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Messaging log");
	odp.pDialog = new COptionLogDlg();
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Typing notify");
	odp.pDialog = new COptionTypingDlg();
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Tabs");
	odp.pDialog = new COptionsTabDlg();
	Options_AddPage(wParam, &odp);

	ChatOptionsInitialize(wParam);
	return 0;
}

void InitOptions(void)
{
	HookEvent(ME_OPT_INITIALISE, OptInitialise);
}
