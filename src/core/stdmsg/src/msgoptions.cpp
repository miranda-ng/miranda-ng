/*

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#include "commonheaders.h"

#include "m_fontservice.h"
#include "m_modernopt.h"

struct FontOptionsList
{
	const TCHAR* szDescr;
	COLORREF     defColour;
	const TCHAR* szDefFace;
	BYTE         defStyle;
	char         defSize;
}
static const fontOptionsList[] =
{
	{ LPGENT("Outgoing messages"), RGB(106, 106, 106), _T("Arial"),    0, -12},
	{ LPGENT("Incoming messages"), RGB(0, 0, 0),       _T("Arial"),    0, -12},
	{ LPGENT("Outgoing name"),     RGB(89, 89, 89),    _T("Arial"),    DBFONTF_BOLD, -12},
	{ LPGENT("Outgoing time"),     RGB(0, 0, 0),       _T("Terminal"), DBFONTF_BOLD, -9},
	{ LPGENT("Outgoing colon"),    RGB(89, 89, 89),    _T("Arial"),    0, -11},
	{ LPGENT("Incoming name"),     RGB(215, 0, 0),     _T("Arial"),    DBFONTF_BOLD, -12},
	{ LPGENT("Incoming time"),     RGB(0, 0, 0),       _T("Terminal"), DBFONTF_BOLD, -9},
	{ LPGENT("Incoming colon"),    RGB(215, 0, 0),     _T("Arial"),    0, -11},
	{ LPGENT("Message area"),      RGB(0, 0, 0),       _T("Arial"),    0, -12},
	{ LPGENT("Other events"),      RGB(90, 90, 160),   _T("Arial"),    0, -12},
};

static BYTE MsgDlgGetFontDefaultCharset(const TCHAR* szFont)
{
  return DEFAULT_CHARSET;
}

bool LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF * colour)
{
	if (i >= SIZEOF(fontOptionsList))
		return false;

	char str[32];

	if (colour) {
		mir_snprintf(str, SIZEOF(str), "SRMFont%dCol", i);
		*colour = db_get_dw(NULL, SRMMMOD, str, fontOptionsList[i].defColour);
	}
	if (lf) {
		mir_snprintf(str, SIZEOF(str), "SRMFont%dSize", i);
		lf->lfHeight = (char)db_get_b(NULL, SRMMMOD, str, fontOptionsList[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "SRMFont%dSty", i);
		int style = db_get_b(NULL, SRMMMOD, str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & DBFONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "SRMFont%d", i);

		DBVARIANT dbv;
		if (db_get_ts(NULL, SRMMMOD, str, &dbv))
			_tcsncpy(lf->lfFaceName, fontOptionsList[i].szDefFace, SIZEOF(lf->lfFaceName)-1);
		else {
			mir_tstrncpy(lf->lfFaceName, dbv.ptszVal, SIZEOF(lf->lfFaceName));
			db_free(&dbv);
		}
		mir_snprintf(str, SIZEOF(str), "SRMFont%dSet", i);
		lf->lfCharSet = db_get_b(NULL, SRMMMOD, str, MsgDlgGetFontDefaultCharset(lf->lfFaceName));
	}
	return true;
}

void RegisterSRMMFonts(void)
{
	char idstr[10];

	FontIDT fontid = { sizeof(fontid) };
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID;
	for (int i = 0; i < SIZEOF(fontOptionsList); i++) {
		strcpy(fontid.dbSettingsGroup, SRMMMOD);
		_tcscpy(fontid.group, LPGENT("Message log"));
		_tcscpy(fontid.name, fontOptionsList[i].szDescr);
		mir_snprintf(idstr, SIZEOF(idstr), "SRMFont%d", i);
		strcpy(fontid.prefix, idstr);
		fontid.order = i;

		fontid.flags &= ~FIDF_CLASSMASK;
		fontid.flags |= (fontOptionsList[i].defStyle == DBFONTF_BOLD) ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL;

		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		_tcscpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace);
		fontid.deffontsettings.charset = MsgDlgGetFontDefaultCharset(fontOptionsList[i].szDefFace);
		_tcsncpy(fontid.backgroundGroup, LPGENT("Message log"),SIZEOF(fontid.backgroundGroup));
		_tcsncpy(fontid.backgroundName, LPGENT("Background"),SIZEOF(fontid.backgroundName));
		FontRegisterT(&fontid);
	}

	ColourIDT colourid = { sizeof(colourid) };
	strcpy(colourid.dbSettingsGroup, SRMMMOD);
	strcpy(colourid.setting, SRMSGSET_BKGCOLOUR);
	colourid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	_tcscpy(colourid.name, LPGENT("Background"));
	_tcscpy(colourid.group, LPGENT("Message log"));
	ColourRegisterT(&colourid);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct CheckBoxValues_t
{
	DWORD  style;
	TCHAR* szDescr;
} statusValues[] =
{
	{ MODEF_OFFLINE, LPGENT("Offline") },
	{ PF2_ONLINE, LPGENT("Online") },
	{ PF2_SHORTAWAY, LPGENT("Away") },
	{ PF2_LONGAWAY, LPGENT("NA") },
	{ PF2_LIGHTDND, LPGENT("Occupied") },
	{ PF2_HEAVYDND, LPGENT("DND") },
	{ PF2_FREECHAT, LPGENT("Free for chat") },
	{ PF2_INVISIBLE, LPGENT("Invisible") },
	{ PF2_OUTTOLUNCH, LPGENT("Out to lunch") },
	{ PF2_ONTHEPHONE, LPGENT("On the phone") }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
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
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DWORD avatarHeight, msgTimeout;

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_POPLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_POPLIST), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
			FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_POPLIST), statusValues, SIZEOF(statusValues), db_get_dw(NULL, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS));
			CheckDlgButton(hwndDlg, IDC_DONOTSTEALFOCUS, db_get_b(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS) ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemInt(hwndDlg, IDC_NFLASHES, db_get_b(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT), FALSE);
			CheckDlgButton(hwndDlg, IDC_SHOWBUTTONLINE, g_dat.flags&SMF_SHOWBTNS ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWINFOLINE, g_dat.flags&SMF_SHOWINFO ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOMIN, db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOMIN, SRMSGDEFSET_AUTOMIN) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOCLOSE, db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SAVEPERCONTACT, db_get_b(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CASCADE, db_get_b(NULL, SRMMMOD, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SENDONENTER, db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SENDONDBLENTER, db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STATUSWIN, db_get_b(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_AVATARSUPPORT, g_dat.flags&SMF_AVATAR ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LIMITAVATARH, db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT) ? BST_CHECKED : BST_UNCHECKED);
			avatarHeight = db_get_dw(NULL, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT);
			SetDlgItemInt(hwndDlg, IDC_AVATARHEIGHT, avatarHeight, FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITAVATARH), IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT))
				EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), FALSE);
			else EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
			CheckDlgButton(hwndDlg, IDC_SHOWSENDBTN, g_dat.flags&SMF_SENDBTN ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHARCOUNT, db_get_b(NULL, SRMMMOD, SRMSGSET_CHARCOUNT, SRMSGDEFSET_CHARCOUNT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CTRLSUPPORT, db_get_b(NULL, SRMMMOD, SRMSGSET_CTRLSUPPORT, SRMSGDEFSET_CTRLSUPPORT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DELTEMP, db_get_b(NULL, SRMMMOD, SRMSGSET_DELTEMP, SRMSGDEFSET_DELTEMP) ? BST_CHECKED : BST_UNCHECKED);
			msgTimeout = db_get_dw(NULL, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
			SetDlgItemInt(hwndDlg, IDC_SECONDS, msgTimeout >= SRMSGSET_MSGTIMEOUT_MIN ? msgTimeout / 1000 : SRMSGDEFSET_MSGTIMEOUT / 1000, FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CASCADE), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SAVEPERCONTACT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CTRLSUPPORT), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AUTOCLOSE));
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AUTOMIN:
			CheckDlgButton(hwndDlg, IDC_AUTOCLOSE, BST_UNCHECKED);
			break;
		case IDC_AUTOCLOSE:
			CheckDlgButton(hwndDlg, IDC_AUTOMIN, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CTRLSUPPORT), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AUTOCLOSE));
			break;
		case IDC_SENDONENTER:
			CheckDlgButton(hwndDlg, IDC_SENDONDBLENTER, BST_UNCHECKED);
			break;
		case IDC_SENDONDBLENTER:
			CheckDlgButton(hwndDlg, IDC_SENDONENTER, BST_UNCHECKED);
			break;
		case IDC_SAVEPERCONTACT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CASCADE), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SAVEPERCONTACT));
			break;
		case IDC_SECONDS:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		case IDC_AVATARSUPPORT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITAVATARH), IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT))
				EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), FALSE);
			else EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
			break;
		case IDC_LIMITAVATARH:
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
			break;
		case IDC_AVATARHEIGHT:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_POPLIST:
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
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_dw(NULL, SRMMMOD, SRMSGSET_POPFLAGS, MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_POPLIST)));
				db_set_b(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DONOTSTEALFOCUS));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWBUTTONLINE));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWINFOLINE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWINFOLINE));
				db_set_b(NULL, SRMMMOD, SRMSGSET_AUTOMIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOMIN));
				db_set_b(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOCLOSE));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SAVEPERCONTACT));
				db_set_b(NULL, SRMMMOD, SRMSGSET_CASCADE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CASCADE));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SENDONENTER, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SENDONENTER));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SENDONDBLENTER, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SENDONDBLENTER));
				db_set_b(NULL, SRMMMOD, SRMSGSET_STATUSICON, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_STATUSWIN));

				db_set_b(NULL, SRMMMOD, SRMSGSET_AVATARENABLE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT));
				db_set_b(NULL, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
				DWORD avatarHeight = GetDlgItemInt(hwndDlg, IDC_AVATARHEIGHT, NULL, TRUE);
				db_set_dw(NULL, SRMMMOD, SRMSGSET_AVHEIGHT, avatarHeight <= 0 ? SRMSGDEFSET_AVHEIGHT : avatarHeight);

				db_set_b(NULL, SRMMMOD, SRMSGSET_SENDBUTTON, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWSENDBTN));
				db_set_b(NULL, SRMMMOD, SRMSGSET_CHARCOUNT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CHARCOUNT));
				db_set_b(NULL, SRMMMOD, SRMSGSET_CTRLSUPPORT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CTRLSUPPORT));
				db_set_b(NULL, SRMMMOD, SRMSGSET_DELTEMP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DELTEMP));
				db_set_b(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, (BYTE)GetDlgItemInt(hwndDlg, IDC_NFLASHES, NULL, TRUE));

				DWORD msgTimeout = GetDlgItemInt(hwndDlg, IDC_SECONDS, NULL, TRUE) * 1000;
				if (msgTimeout < SRMSGSET_MSGTIMEOUT_MIN) msgTimeout = SRMSGDEFSET_MSGTIMEOUT;
				db_set_dw(NULL, SRMMMOD, SRMSGSET_MSGTIMEOUT, msgTimeout);

				ReloadGlobals();
				WindowList_Broadcast(g_dat.hMessageWindowList, DM_OPTIONSAPPLIED, 0, 0);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH hBkgColourBrush;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		switch (db_get_b(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
		case LOADHISTORY_UNREAD:
			CheckDlgButton(hwndDlg, IDC_LOADUNREAD, BST_CHECKED);
			break;
		case LOADHISTORY_COUNT:
			CheckDlgButton(hwndDlg, IDC_LOADCOUNT, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTSPIN), TRUE);
			break;
		case LOADHISTORY_TIME:
			CheckDlgButton(hwndDlg, IDC_LOADTIME, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMEN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMESPIN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STMINSOLD), TRUE);
			break;
		}
		SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETPOS, 0, db_get_w(NULL, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
		SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETRANGE, 0, MAKELONG(12 * 60, 0));
		SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETPOS, 0, db_get_w(NULL, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

		CheckDlgButton(hwndDlg, IDC_SHOWLOGICONS, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWNAMES, !db_get_b(NULL, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWTIMES, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSECS), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
		CheckDlgButton(hwndDlg, IDC_SHOWSECS, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWSECS, SRMSGDEFSET_SHOWSECS) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWDATES), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
		CheckDlgButton(hwndDlg, IDC_SHOWDATES, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWFORMATTING, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWFORMAT, SRMSGDEFSET_SHOWFORMAT) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LOADUNREAD:
		case IDC_LOADCOUNT:
		case IDC_LOADTIME:
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTN), IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTSPIN), IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMEN), IsDlgButtonChecked(hwndDlg, IDC_LOADTIME));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_LOADTIME));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STMINSOLD), IsDlgButtonChecked(hwndDlg, IDC_LOADTIME));
			break;

		case IDC_SHOWTIMES:
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSECS), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWDATES), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
			break;

		case IDC_LOADCOUNTN:
		case IDC_LOADTIMEN:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				if (IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT))
					db_set_b(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
				else if (IsDlgButtonChecked(hwndDlg, IDC_LOADTIME))
					db_set_b(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
				else
					db_set_b(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
				db_set_w(NULL, SRMMMOD, SRMSGSET_LOADCOUNT, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_GETPOS, 0, 0));
				db_set_w(NULL, SRMMMOD, SRMSGSET_LOADTIME, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWLOGICONS));
				db_set_b(NULL, SRMMMOD, SRMSGSET_HIDENAMES, (BYTE)BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWNAMES));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWTIME, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWSECS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWSECS));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWDATE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWDATES));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWFORMAT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWFORMATTING));

				FreeMsgLogIcons();
				LoadMsgLogIcons();
				ReloadGlobals();
				WindowList_Broadcast(g_dat.hMessageWindowList, DM_OPTIONSAPPLIED, 0, 0);
				return TRUE;
			}
			break;
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBkgColourBrush);
		break;
	}
	return FALSE;
}

static void ResetCList(HWND hwndDlg)
{
	if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_DISABLEGROUPS && !db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, (WPARAM)FALSE, 0);
	else
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, (WPARAM)TRUE, 0);

	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static void RebuildList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	BYTE defType = db_get_b(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW);
	if (hItemNew && defType)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemNew, 1);

	if (hItemUnknown && db_get_b(NULL, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemUnknown, 1);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem && db_get_b(hContact, SRMMMOD, SRMSGSET_TYPING, defType))
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
	}
}

static void SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew)
		db_set_b(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemNew, 0) ? 1 : 0));

	if (hItemUnknown)
		db_set_b(NULL, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemUnknown, 0) ? 1 : 0));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
			db_set_b(hContact, SRMMMOD, SRMSGSET_TYPING, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? 1 : 0));
	}
}

static INT_PTR CALLBACK DlgProcTypeOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hItemNew, hItemUnknown;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CLCINFOITEM cii = { sizeof(cii) };
			cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
			cii.pszText = TranslateT("** New contacts **");
			hItemNew = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)& cii);
			cii.pszText = TranslateT("** Unknown contacts **");
			hItemUnknown = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)& cii);
		}
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE) | (CLS_SHOWHIDDEN) | (CLS_NOHIDEOFFLINE));
		ResetCList(hwndDlg);
		RebuildList(hwndDlg, hItemNew, hItemUnknown);
		CheckDlgButton(hwndDlg, IDC_SHOWNOTIFY, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TYPEWIN, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TYPETRAY, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOTIFYTRAY, db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOTIFYBALLOON, !db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TYPEWIN), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TYPETRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
		if (!ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), FALSE);
			CheckDlgButton(hwndDlg, IDC_NOTIFYTRAY, BST_CHECKED);
			SetDlgItemText(hwndDlg, IDC_NOTIFYBALLOON, TranslateT("Show balloon popup (unsupported system)"));
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TYPETRAY:
			if (IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY)) {
				if (!ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), TRUE);
				}
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), FALSE);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_SHOWNOTIFY:
			EnableWindow(GetDlgItem(hwndDlg, IDC_TYPEWIN), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TYPETRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY)
							 && ServiceExists(MS_CLIST_SYSTRAY_NOTIFY));
			//fall-thru
		case IDC_TYPEWIN:
		case IDC_NOTIFYTRAY:
		case IDC_NOTIFYBALLOON:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR *)lParam)->idFrom) {
		case IDC_CLIST:
			switch (((NMHDR *)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				ResetCList(hwndDlg);
				break;
			case CLN_CHECKCHANGED:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				SaveList(hwndDlg, hItemNew, hItemUnknown);
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPEWIN));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
				db_set_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY));
				ReloadGlobals();
				WindowList_Broadcast(g_dat.hMessageWindowList, DM_OPTIONSAPPLIED, 0, 0);
			}
		}
		break;
	}
	return FALSE;
}

static int OptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGDLG);
	odp.pszTab = LPGEN("Messaging");
	odp.pszTitle = LPGEN("Message sessions");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGLOG);
	odp.pszTab = LPGEN("Messaging log");
	odp.pfnDlgProc = DlgProcLogOptions;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGTYPE);
	odp.pszTab = LPGEN("Typing notify");
	odp.pfnDlgProc = DlgProcTypeOptions;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int ModernOptInitialise(WPARAM wParam, LPARAM lParam)
{
	static int iBoldControls[] =
	{
		IDC_TXT_TITLE1, IDC_TXT_TITLE2, IDC_TXT_TITLE3,
		MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = { sizeof(obj) };
	obj.dwFlags = MODEROPT_FLG_TCHAR | MODEROPT_FLG_NORESIZE;
	obj.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	obj.hInstance = g_hInst;
	obj.iSection = MODERNOPT_PAGE_MSGS;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.iBoldControls = iBoldControls;
	obj.lpzClassicGroup = NULL;
	obj.lpzClassicPage = "Message Sessions";
	obj.lpzClassicTab = "Messaging";
	obj.lpzHelpUrl = "http://wiki.miranda-ng.org/";

	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_MSGDLG);
	obj.pfnDlgProc = DlgProcOptions;
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_MSGLOG);
	obj.pfnDlgProc = DlgProcLogOptions;
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

	return 0;
}

void InitOptions(void)
{
	HookEvent(ME_OPT_INITIALISE, OptInitialise);
	HookEvent(ME_MODERNOPT_INITIALIZE, ModernOptInitialise);
}
