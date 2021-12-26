/*
Wannabe OSD
This plugin tries to become miranda's standard OSD ;-)

(C) 2005 Andrej Krutak

Distributed under GNU's GPL 2 or later
*/

#include "stdafx.h"

COLORREF pencustcolors[16];

const static osdmsg defstr = { L"", 0, RGB(0, 0, 0), nullptr, 0 };

void FillCheckBoxTree(HWND hwndTree, uint32_t style)
{
	logmsg("FillCheckBoxTree");

	TVINSERTSTRUCT tvis = {};
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (uint16_t status = ID_STATUS_OFFLINE; status <= ID_STATUS_MAX; status++) {
		tvis.item.lParam = status - ID_STATUS_OFFLINE;
		tvis.item.pszText = Clist_GetStatusModeDescription(status, 0);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & (1 << tvis.item.lParam)) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

uint32_t MakeCheckBoxTreeFlags(HWND hwndTree)
{
	uint32_t flags = 0;

	logmsg("MakeCheckBoxTreeFlags");

	TVITEM tvi = { 0 };
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if (((tvi.state&TVIS_STATEIMAGEMASK) >> 12 == 2)) flags |= 1 << tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

int selectColor(HWND hwnd, COLORREF *clr)
{
	logmsg("SelectColor");

	CHOOSECOLOR cc = { 0 };
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hwnd;
	cc.hInstance = (HWND)g_plugin.getInst();
	cc.rgbResult = *clr;
	cc.lpCustColors = pencustcolors;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	if (!ChooseColor(&cc))
		return 1;

	*clr = cc.rgbResult;
	return 0;
}

int selectFont(HWND hDlg, LOGFONT *lf)
{
	COLORREF color = RGB(0, 0, 0);

	logmsg("SelectFont");

	HDC hDC = GetDC(hDlg);

	CHOOSEFONT cf;
	memset(&cf, 0, sizeof(CHOOSEFONT));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hDlg;
	cf.hDC = hDC;
	cf.lpLogFont = lf;
	cf.rgbColors = 0;
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_BOTH | CF_FORCEFONTEXIST;
	cf.nFontType = 0;
	cf.rgbColors = color;

	if (!ChooseFont(&cf)) {
		if (cf.hDC)
			DeleteDC(cf.hDC);

		ReleaseDC(hDlg, hDC);
		return 1;
	}

	if (cf.hDC)
		DeleteDC(cf.hDC);

	ReleaseDC(hDlg, hDC);
	return 0;
}

void loadDBSettings(plgsettings *ps)
{
	logmsg("loadDBSettings");

	ps->align = g_plugin.getByte("align", DEFAULT_ALIGN);
	ps->salign = g_plugin.getByte("salign", DEFAULT_SALIGN);
	ps->altShadow = g_plugin.getByte("altShadow", DEFAULT_ALTSHADOW);
	ps->transparent = g_plugin.getByte("transparent", DEFAULT_TRANPARENT);
	ps->showShadow = g_plugin.getByte("showShadow", DEFAULT_SHOWSHADOW);
	ps->messages = g_plugin.getByte("messages", DEFAULT_ANNOUNCEMESSAGES);
	ps->a_user = g_plugin.getByte("a_user", DEFAULT_ANNOUNCESTATUS);
	ps->distance = g_plugin.getByte("distance", DEFAULT_DISTANCE);
	ps->winx = g_plugin.getDword("winx", DEFAULT_WINX);
	ps->winy = g_plugin.getDword("winy", DEFAULT_WINY);
	ps->winxpos = g_plugin.getDword("winxpos", DEFAULT_WINXPOS);
	ps->winypos = g_plugin.getDword("winypos", DEFAULT_WINYPOS);
	ps->alpha = g_plugin.getByte("alpha", DEFAULT_ALPHA);
	ps->showmystatus = g_plugin.getByte("showMyStatus", DEFAULT_SHOWMYSTATUS);
	ps->timeout = g_plugin.getDword("timeout", DEFAULT_TIMEOUT);
	ps->clr_msg = g_plugin.getDword("clr_msg", DEFAULT_CLRMSG);
	ps->clr_status = g_plugin.getDword("clr_status", DEFAULT_CLRSTATUS);
	ps->clr_shadow = g_plugin.getDword("clr_shadow", DEFAULT_CLRSHADOW);
	ps->bkclr = g_plugin.getDword("bkclr", DEFAULT_BKCLR);

	ps->showMsgWindow = g_plugin.getByte("showMessageWindow", DEFAULT_SHOWMSGWIN);
	ps->showWhen = g_plugin.getDword("showWhen", DEFAULT_SHOWWHEN);

	DBVARIANT dbv;
	if (!g_plugin.getWString("message_format", &dbv)) {
		mir_wstrcpy(ps->msgformat, dbv.pwszVal);
		db_free(&dbv);
	}
	else mir_wstrcpy(ps->msgformat, DEFAULT_MESSAGEFORMAT);

	ps->announce = g_plugin.getDword("announce", DEFAULT_ANNOUNCE);

	ps->lf.lfHeight = g_plugin.getDword("fntHeight", DEFAULT_FNT_HEIGHT);
	ps->lf.lfWidth = g_plugin.getDword("fntWidth", DEFAULT_FNT_WIDTH);
	ps->lf.lfEscapement = g_plugin.getDword("fntEscapement", DEFAULT_FNT_ESCAPEMENT);
	ps->lf.lfOrientation = g_plugin.getDword("fntOrientation", DEFAULT_FNT_ORIENTATION);
	ps->lf.lfWeight = g_plugin.getDword("fntWeight", DEFAULT_FNT_WEIGHT);
	ps->lf.lfItalic = g_plugin.getByte("fntItalic", DEFAULT_FNT_ITALIC);
	ps->lf.lfUnderline = g_plugin.getByte("fntUnderline", DEFAULT_FNT_UNDERLINE);
	ps->lf.lfStrikeOut = g_plugin.getByte("fntStrikeout", DEFAULT_FNT_STRIKEOUT);
	ps->lf.lfCharSet = g_plugin.getByte("fntCharSet", DEFAULT_FNT_CHARSET);
	ps->lf.lfOutPrecision = g_plugin.getByte("fntOutPrecision", DEFAULT_FNT_OUTPRECISION);
	ps->lf.lfClipPrecision = g_plugin.getByte("fntClipPrecision", DEFAULT_FNT_CLIPRECISION);
	ps->lf.lfQuality = g_plugin.getByte("fntQuality", DEFAULT_FNT_QUALITY);
	ps->lf.lfPitchAndFamily = g_plugin.getByte("fntPitchAndFamily", DEFAULT_FNT_PITCHANDFAM);

	if (!g_plugin.getWString("fntFaceName", &dbv)) {
		mir_wstrcpy(ps->lf.lfFaceName, dbv.pwszVal);
		db_free(&dbv);
	}
	else
		mir_wstrcpy(ps->lf.lfFaceName, DEFAULT_FNT_FACENAME);
}

void saveDBSettings(plgsettings *ps)
{
	logmsg("saveDBSettings");

	g_plugin.setByte("showShadow", ps->showShadow);
	g_plugin.setByte("altShadow", ps->altShadow);
	g_plugin.setByte("distance", ps->distance);

	g_plugin.setDword("winx", ps->winx);
	g_plugin.setDword("winy", ps->winy);
	g_plugin.setDword("winxpos", ps->winxpos);
	g_plugin.setDword("winypos", ps->winypos);

	g_plugin.setByte("alpha", ps->alpha);
	g_plugin.setDword("timeout", ps->timeout);

	g_plugin.setByte("transparent", ps->transparent);
	g_plugin.setByte("messages", ps->messages);
	g_plugin.setByte("a_user", ps->a_user);
	g_plugin.setWString("message_format", ps->msgformat);

	g_plugin.setByte("align", ps->align);
	g_plugin.setByte("salign", ps->salign);

	g_plugin.setByte("showMyStatus", ps->showmystatus);

	g_plugin.setDword("clr_msg", ps->clr_msg);
	g_plugin.setDword("clr_shadow", ps->clr_shadow);
	g_plugin.setDword("clr_status", ps->clr_status);
	g_plugin.setDword("bkclr", ps->bkclr);

	g_plugin.setDword("fntHeight", ps->lf.lfHeight);
	g_plugin.setDword("fntWidth", ps->lf.lfWidth);
	g_plugin.setDword("fntEscapement", ps->lf.lfEscapement);
	g_plugin.setDword("fntOrientation", ps->lf.lfOrientation);
	g_plugin.setDword("fntWeight", ps->lf.lfWeight);
	g_plugin.setByte("fntItalic", ps->lf.lfItalic);
	g_plugin.setByte("fntUnderline", ps->lf.lfUnderline);
	g_plugin.setByte("fntStrikeout", ps->lf.lfStrikeOut);
	g_plugin.setByte("fntCharSet", ps->lf.lfCharSet);
	g_plugin.setByte("fntOutPrecision", ps->lf.lfOutPrecision);
	g_plugin.setByte("fntClipPrecision", ps->lf.lfClipPrecision);
	g_plugin.setByte("fntQuality", ps->lf.lfQuality);
	g_plugin.setByte("fntPitchAndFamily", ps->lf.lfPitchAndFamily);
	g_plugin.setWString("fntFaceName", ps->lf.lfFaceName);

	g_plugin.setDword("announce", ps->announce);

	g_plugin.setByte("showMessageWindow", ps->showMsgWindow);
	g_plugin.setDword("showWhen", ps->showWhen);
}

INT_PTR CALLBACK OptDlgProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	RECT rect;
	plgsettings *ps; //0: current; 1: original

	logmsg("OptDlgProc");

	switch (msg) {
	case WM_INITDIALOG:
		logmsg("OptDlgProc::INITDIALOG");
		TranslateDialogDefault(hDlg);

		ps = (plgsettings*)malloc(sizeof(plgsettings) * 2);
		loadDBSettings(&ps[0]);
		ps[1] = ps[0];
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ps);
		SetWindowLongPtr(g_hWnd, GWL_STYLE, WS_POPUP | WS_SIZEBOX);
		SetWindowPos(g_hWnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		SetWindowLongPtr(GetDlgItem(hDlg, IDC_TREE1), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hDlg, IDC_TREE1), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_TREE2), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hDlg, IDC_TREE1), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

		CheckDlgButton(hDlg, IDC_RADIO1 + ps->align - 1, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_RADIO10 + 9 - ps->salign, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECK1, ps->altShadow ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK2, ps->showMsgWindow ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK3, ps->transparent ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK4, ps->showShadow ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK5, ps->messages ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(hDlg, IDC_EDIT2, ps->msgformat);

		CheckDlgButton(hDlg, IDC_CHECK6, ps->a_user ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK7, ps->showmystatus ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hDlg, IDC_EDIT1, ps->distance, 0);

		SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETRANGE, 0, MAKELONG(0, 255));
		SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETPOS, TRUE, (LPARAM)ps->alpha);

		{
			wchar_t buf[20];
			mir_snwprintf(buf, L"%d %%", ps->alpha * 100 / 255);
			SetDlgItemText(hDlg, IDC_ALPHATXT, buf);
		}

		SetDlgItemInt(hDlg, IDC_EDIT5, ps->timeout, 0);
		FillCheckBoxTree(GetDlgItem(hDlg, IDC_TREE1), ps->announce);
		FillCheckBoxTree(GetDlgItem(hDlg, IDC_TREE2), ps->showWhen);
		return 0;

	case WM_HSCROLL:
		if (LOWORD(wparam) == SB_ENDSCROLL || LOWORD(wparam) == SB_THUMBPOSITION || LOWORD(wparam) == SB_ENDSCROLL)
			return 0;
		ps = (plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		ps->alpha = SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0);
		{
			wchar_t buf[20];
			mir_snwprintf(buf, L"%d %%", ps->alpha * 100 / 255);
			SetDlgItemText(hDlg, IDC_ALPHATXT, buf);
		}
		goto xxx;
	case WM_DESTROY:
		logmsg("OptDlgProc::DESTROY");
		ps = (plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		ps[0] = ps[1];
		saveDBSettings(&ps[0]);

		SetWindowLongPtr(g_hWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(g_hWnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		SetWindowPos(g_hWnd, nullptr, ps->winxpos, ps->winypos, ps->winx, ps->winy, SWP_NOZORDER | SWP_NOACTIVATE);
		SetLayeredWindowAttributes(g_hWnd, ps->bkclr, ps->alpha, (ps->transparent ? LWA_COLORKEY : 0) | LWA_ALPHA);

		free((void*)GetWindowLongPtr(hDlg, GWLP_USERDATA));
		return 0;
	case WM_COMMAND:
		logmsg("OptDlgProc::COMMAND");
		ps = (plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		switch (LOWORD(wparam)) {
		case IDC_BUTTON7:
			MessageBox(hDlg, TranslateT("Variables:\n  %n : Nick\n  %m : Message\n  %l : New line"), TranslateT("Help"), MB_ICONINFORMATION | MB_OK);
			return 0;
		case IDC_BUTTON5:
			SendMessage(g_hWnd, WM_USER + 1, (WPARAM)TranslateT("Miranda NG is great and this is a long message."), 0);
			break;
		case IDC_BUTTON1:
			selectFont(hDlg, &(ps->lf));
			break;
		case IDC_BUTTON2:
			selectColor(hDlg, &ps->clr_status);
			break;
		case IDC_BUTTON6:
			selectColor(hDlg, &ps->clr_msg);
			break;
		case IDC_BUTTON3:
			selectColor(hDlg, &ps->clr_shadow);
			break;
		case IDC_BUTTON4:
			selectColor(hDlg, &ps->bkclr);
			break;
		case IDC_CHECK4:
			ps->showShadow = IsDlgButtonChecked(hDlg, IDC_CHECK4);
			break;
		case IDC_CHECK1:
			ps->altShadow = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			break;
		case IDC_CHECK2:
			ps->showMsgWindow = IsDlgButtonChecked(hDlg, IDC_CHECK2);
		case IDC_EDIT1:
			ps->distance = GetDlgItemInt(hDlg, IDC_EDIT1, nullptr, 0);
			break;
		case IDC_EDIT5:
			ps->timeout = GetDlgItemInt(hDlg, IDC_EDIT5, nullptr, 0);
			break;
		case IDC_CHECK3:
			ps->transparent = IsDlgButtonChecked(hDlg, IDC_CHECK3);
			break;
		case IDC_CHECK5:
			ps->messages = IsDlgButtonChecked(hDlg, IDC_CHECK5);
			break;
		case IDC_CHECK6:
			ps->a_user = IsDlgButtonChecked(hDlg, IDC_CHECK6);
			break;
		case IDC_CHECK7:
			ps->showmystatus = IsDlgButtonChecked(hDlg, IDC_CHECK7);
			break;
		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
		case IDC_RADIO4:
		case IDC_RADIO5:
		case IDC_RADIO6:
		case IDC_RADIO7:
		case IDC_RADIO8:
		case IDC_RADIO9:
			if (IsDlgButtonChecked(hDlg, LOWORD(wparam)))
				ps->align = LOWORD(wparam) - IDC_RADIO1 + 1;
			break;
		case IDC_RADIO10:
		case IDC_RADIO11:
		case IDC_RADIO12:
		case IDC_RADIO13:
		case IDC_RADIO14:
		case IDC_RADIO15:
		case IDC_RADIO16:
		case IDC_RADIO17:
		case IDC_RADIO18:
			if (IsDlgButtonChecked(hDlg, LOWORD(wparam)))
				ps->salign = 10 - (LOWORD(wparam) - IDC_RADIO10 + 1);
			break;
		}
xxx:
		saveDBSettings(ps);
		SetWindowPos(g_hWnd, nullptr, 0, 0, ps->winx, ps->winy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		SetLayeredWindowAttributes(g_hWnd, 
			g_plugin.getDword("bkclr", DEFAULT_BKCLR), 
			g_plugin.getByte("alpha", DEFAULT_ALPHA), 
			(g_plugin.getByte("transparent", DEFAULT_TRANPARENT) ? LWA_COLORKEY : 0) | LWA_ALPHA);
		InvalidateRect(g_hWnd, nullptr, TRUE);
		SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);

		return 0;

	case WM_NOTIFY:
		logmsg("OptDlgProc::NOTIFY");
		switch (((LPNMHDR)lparam)->code) {
		case TVN_SETDISPINFO:
		case NM_CLICK:
		case NM_RETURN:
		case TVN_SELCHANGED:
			if (((LPNMHDR)lparam)->idFrom == IDC_TREE1)
				SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			break;
		
		case PSN_APPLY:
			ps = (plgsettings*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

			GetWindowRect(g_hWnd, &rect);
			ps->winx = rect.right - rect.left;
			ps->winy = rect.bottom - rect.top;
			ps->winxpos = rect.left;
			ps->winypos = rect.top;
			ps->announce = MakeCheckBoxTreeFlags(GetDlgItem(hDlg, IDC_TREE1));
			ps->showWhen = MakeCheckBoxTreeFlags(GetDlgItem(hDlg, IDC_TREE2));
			GetDlgItemText(hDlg, IDC_EDIT2, ps->msgformat, 255);
			ps[1] = ps[0]; //apply current settings at closing

			saveDBSettings(ps);
			SetLayeredWindowAttributes(g_hWnd, g_plugin.getDword("bkclr", DEFAULT_BKCLR), g_plugin.getByte("alpha", DEFAULT_ALPHA), (g_plugin.getByte("transparent", DEFAULT_TRANPARENT) ? LWA_COLORKEY : 0) | LWA_ALPHA);
			InvalidateRect(g_hWnd, nullptr, TRUE);
			break;
		}
		break;
	}

	return 0;
}

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 150000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG1);
	odp.szGroup.w = LPGENW("Plugins");
	odp.szTitle.w = LPGENW("OSD");
	odp.pfnDlgProc = OptDlgProc;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	g_plugin.addOptions(wparam, &odp);
	return 0;
}
