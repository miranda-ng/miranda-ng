#include "stdafx.h"
#pragma comment(lib, "shlwapi.lib")

#define M_GUESSSAMEASBOXES		(WM_USER + 18)

#define M_REFRESHBKGBOXES		(WM_USER + 20)
#define M_REFRESHBORDERPICKERS	(WM_USER + 21)

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR APIENTRY OptWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		// Properties
		CheckDlgButton(hwndDlg, IDC_CHK_HIDE_OFFLINE, (fcOpt.bHideOffline ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHK_HIDE_ALL, (fcOpt.bHideAll ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHK_HIDE_WHEN_FULSCREEN, (fcOpt.bHideWhenFullscreen ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHK_STICK, (fcOpt.bMoveTogether ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHK_WIDTH, (fcOpt.bFixedWidth ? BST_CHECKED : BST_UNCHECKED));

		EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_WIDTH), fcOpt.bFixedWidth);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_WIDTH), fcOpt.bFixedWidth);
		EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTHSPIN), fcOpt.bFixedWidth);

		SendDlgItemMessage(hwndDlg, IDC_WIDTHSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_WIDTHSPIN, UDM_SETPOS, 0, fcOpt.nThumbWidth);

		CheckDlgButton(hwndDlg, IDC_CHK_TIP, (fcOpt.bShowTip ? BST_CHECKED : BST_UNCHECKED));

		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_TIP), bEnableTip);

		EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN), bEnableTip && fcOpt.bShowTip);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN_CMT), bEnableTip && fcOpt.bShowTip);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TIMEIN), bEnableTip && fcOpt.bShowTip);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEINSPIN), bEnableTip && fcOpt.bShowTip);

		SendDlgItemMessage(hwndDlg, IDC_TIMEINSPIN, UDM_SETRANGE, 0, MAKELONG(5000, 0));
		SendDlgItemMessage(hwndDlg, IDC_TIMEINSPIN, UDM_SETPOS, 0, fcOpt.TimeIn);

		CheckDlgButton(hwndDlg, IDC_CHK_TOTOP, (fcOpt.bToTop ? BST_CHECKED : BST_UNCHECKED));

		EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TOTOP), fcOpt.bToTop);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TOTOPTIME), fcOpt.bToTop);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TOTOPTIMESPIN), fcOpt.bToTop);

		SendDlgItemMessage(hwndDlg, IDC_TOTOPTIMESPIN, UDM_SETRANGE, 0, MAKELONG(TOTOPTIME_MAX, 1));
		SendDlgItemMessage(hwndDlg, IDC_TOTOPTIMESPIN, UDM_SETPOS, 0, fcOpt.ToTopTime);

		CheckDlgButton(hwndDlg, IDC_CHK_HIDE_WHEN_CLISTSHOW, (fcOpt.bHideWhenCListShow ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHK_SINGLECLK, (fcOpt.bUseSingleClick ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHK_SHOWIDLE, (fcOpt.bShowIdle ? BST_CHECKED : BST_UNCHECKED));

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHK_WIDTH:
			if (BN_CLICKED == HIWORD(wParam)) {
				BOOL bChecked = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_WIDTH);

				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_WIDTH), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_WIDTH), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_WIDTHSPIN), bChecked);
			}
			break;

		case IDC_TXT_TIMEIN:
		case IDC_TXT_TOTOPTIME:
		case IDC_TXT_WIDTH:
			if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
				return 0;
			break;

		case IDC_CHK_TIP:
			if (BN_CLICKED == HIWORD(wParam)) {
				BOOL bChecked = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TIP);

				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN_CMT), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TIMEIN), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEINSPIN), bChecked);
			}
			break;

		case IDC_CHK_TOTOP:
			if (BN_CLICKED == HIWORD(wParam)) {
				BOOL bChecked = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TOTOP);

				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TOTOP), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TOTOPTIME), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TOTOPTIMESPIN), bChecked);
			}
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		LPNMHDR phdr = (LPNMHDR)(lParam);
		if (0 == phdr->idFrom) {
			switch (phdr->code) {
			case PSN_APPLY:
				BOOL bSuccess = FALSE;

				fcOpt.bHideOffline = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_OFFLINE);
				g_plugin.setByte("HideOffline", (uint8_t)fcOpt.bHideOffline);

				fcOpt.bHideAll = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_ALL);
				g_plugin.setByte("HideAll", (uint8_t)fcOpt.bHideAll);

				fcOpt.bHideWhenFullscreen = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_WHEN_FULSCREEN);
				g_plugin.setByte("HideWhenFullscreen", (uint8_t)fcOpt.bHideWhenFullscreen);

				fcOpt.bMoveTogether = IsDlgButtonChecked(hwndDlg, IDC_CHK_STICK);
				g_plugin.setByte("MoveTogether", (uint8_t)fcOpt.bMoveTogether);

				fcOpt.bFixedWidth = IsDlgButtonChecked(hwndDlg, IDC_CHK_WIDTH);
				g_plugin.setByte("FixedWidth", (uint8_t)fcOpt.bFixedWidth);
				fcOpt.nThumbWidth = GetDlgItemInt(hwndDlg, IDC_TXT_WIDTH, &bSuccess, FALSE);
				g_plugin.setDword("Width", fcOpt.nThumbWidth);

				if (bEnableTip) {
					fcOpt.bShowTip = IsDlgButtonChecked(hwndDlg, IDC_CHK_TIP);
					g_plugin.setByte("ShowTip", (uint8_t)fcOpt.bShowTip);
					fcOpt.TimeIn = GetDlgItemInt(hwndDlg, IDC_TXT_TIMEIN, &bSuccess, FALSE);
					g_plugin.setWord("TimeIn", fcOpt.TimeIn);
				}

				fcOpt.bToTop = IsDlgButtonChecked(hwndDlg, IDC_CHK_TOTOP);
				g_plugin.setByte("ToTop", (uint8_t)fcOpt.bToTop);
				fcOpt.ToTopTime = GetDlgItemInt(hwndDlg, IDC_TXT_TOTOPTIME, &bSuccess, FALSE);
				g_plugin.setWord("ToTopTime", fcOpt.ToTopTime);

				fcOpt.bHideWhenCListShow = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_WHEN_CLISTSHOW);
				g_plugin.setByte("HideWhenCListShow", (uint8_t)fcOpt.bHideWhenCListShow);

				fcOpt.bUseSingleClick = IsDlgButtonChecked(hwndDlg, IDC_CHK_SINGLECLK);
				g_plugin.setByte("UseSingleClick", (uint8_t)fcOpt.bUseSingleClick);

				fcOpt.bShowIdle = IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWIDLE);
				g_plugin.setByte("ShowIdle", (uint8_t)fcOpt.bShowIdle);

				ApplyOptionsChanges();
				OnStatusChanged();
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR APIENTRY OptSknWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bEnable;
	char szPercent[20];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		// Border
		CheckDlgButton(hwndDlg, IDC_DRAWBORDER, g_plugin.getByte("DrawBorder", FLT_DEFAULT_DRAWBORDER) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, M_REFRESHBORDERPICKERS, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_LTEDGESCOLOR);
		SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_SETCOLOUR, 0, g_plugin.getDword("LTEdgesColor", FLT_DEFAULT_LTEDGESCOLOR));
		SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_RBEDGESCOLOR);
		SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_SETCOLOUR, 0, g_plugin.getDword("RBEdgesColor", FLT_DEFAULT_RBEDGESCOLOR));

		// Background
		CheckDlgButton(hwndDlg, IDC_CHK_WIDTH, (fcOpt.bFixedWidth ? BST_CHECKED : BST_UNCHECKED));

		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_BKGNDCOLOR);
		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, g_plugin.getDword("BkColor", FLT_DEFAULT_BKGNDCOLOR));
		CheckDlgButton(hwndDlg, IDC_BITMAP, g_plugin.getByte("BkUseBitmap", FLT_DEFAULT_BKGNDUSEBITMAP) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, M_REFRESHBKGBOXES, 0, 0);
		{
			ptrW wszBitmap(g_plugin.getWStringA("BkBitmap"));
			if (wszBitmap)
				SetDlgItemText(hwndDlg, IDC_FILENAME, wszBitmap);

			uint16_t bmpUse = (uint16_t)g_plugin.getWord("BkBitmapOpt", FLT_DEFAULT_BKGNDBITMAPOPT);
			CheckDlgButton(hwndDlg, IDC_STRETCHH, ((bmpUse & CLB_STRETCHH) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_STRETCHV, ((bmpUse & CLB_STRETCHV) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_TILEH, ((bmpUse & CLBF_TILEH) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_TILEV, ((bmpUse & CLBF_TILEV) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, ((bmpUse & CLBF_PROPORTIONAL) ? BST_CHECKED : BST_UNCHECKED));

			SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);

			// Windows 2K/XP
			uint8_t btOpacity = (uint8_t)g_plugin.getByte("Opacity", 100);
			SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
			SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_SETPOS, TRUE, btOpacity);

			mir_snprintf(szPercent, "%d%%", btOpacity);
			SetDlgItemTextA(hwndDlg, IDC_OPACITY, szPercent);

			EnableWindow(GetDlgItem(hwndDlg, IDC_SLIDER_OPACITY), SetLayeredWindowAttributes != nullptr);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPACITY), SetLayeredWindowAttributes != nullptr);
		}
		return TRUE;

	case M_REFRESHBKGBOXES:
		bEnable = IsDlgButtonChecked(hwndDlg, IDC_BITMAP);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), bEnable);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), bEnable);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHH), bEnable);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRETCHV), bEnable);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TILEH), bEnable);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TILEV), bEnable);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PROPORTIONAL), bEnable);
		break;

	case M_REFRESHBORDERPICKERS:
		bEnable = IsDlgButtonChecked(hwndDlg, IDC_DRAWBORDER);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LTEDGESCOLOR), bEnable);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RBEDGESCOLOR), bEnable);
		break;

	case WM_HSCROLL:
		if (wParam != TB_ENDTRACK) {
			int nPos = (int)SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_GETPOS, 0, 0);
			fcOpt.thumbAlpha = (uint8_t)((nPos * 255) / 100);
			SetThumbsOpacity(fcOpt.thumbAlpha);

			mir_snprintf(szPercent, "%d%%", nPos);
			SetDlgItemTextA(hwndDlg, IDC_OPACITY, szPercent);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DRAWBORDER:
			SendMessage(hwndDlg, M_REFRESHBORDERPICKERS, 0, 0);
			break;

		case IDC_BROWSE:
		{
			wchar_t str[MAX_PATH], filter[512];
			GetDlgItemText(hwndDlg, IDC_FILENAME, str, _countof(str));
			Bitmap_GetFilter(filter, _countof(filter));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = _countof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = L"bmp";
			if (!GetOpenFileName(&ofn))
				return FALSE;
			SetDlgItemText(hwndDlg, IDC_FILENAME, str);
		}
		break;

		case IDC_FILENAME:
			if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
				return FALSE;
			break;

		case IDC_BITMAP:
			SendMessage(hwndDlg, M_REFRESHBKGBOXES, 0, 0);
			break;
		}

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (0 == ((LPNMHDR)lParam)->idFrom) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				fcOpt.thumbAlpha = (uint8_t)((double)g_plugin.getByte("Opacity", 100) * 2.55);
				SetThumbsOpacity(fcOpt.thumbAlpha);
				break;

			case PSN_APPLY:
				// Border
				g_plugin.setByte("DrawBorder", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DRAWBORDER));

				COLORREF col = SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_GETCOLOUR, 0, 0);
				g_plugin.setDword("LTEdgesColor", col);
				col = SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_GETCOLOUR, 0, 0);
				g_plugin.setDword("RBEdgesColor", col);

				g_plugin.setByte("Opacity", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_GETPOS, 0, 0));

				// Backgroud
				col = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);
				g_plugin.setDword("BkColor", col);

				g_plugin.setByte("BkUseBitmap", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BITMAP));

				wchar_t str[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_FILENAME, str, _countof(str));
				g_plugin.setWString("BkBitmap", str);

				uint16_t flags = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHH))
					flags |= CLB_STRETCHH;
				if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHV))
					flags |= CLB_STRETCHV;
				if (IsDlgButtonChecked(hwndDlg, IDC_TILEH))
					flags |= CLBF_TILEH;
				if (IsDlgButtonChecked(hwndDlg, IDC_TILEV))
					flags |= CLBF_TILEV;
				if (IsDlgButtonChecked(hwndDlg, IDC_PROPORTIONAL))
					flags |= CLBF_PROPORTIONAL;
				g_plugin.setWord("BkBitmapOpt", flags);

				ApplyOptionsChanges();
				OnStatusChanged();
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int OnOptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FLTCONT);
	odp.szTitle.a = LPGEN("Floating Contacts");
	odp.szGroup.a = LPGEN("Contact list");
	odp.szTab.a = LPGEN("Main Features");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptWndProc;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.szTab.a = LPGEN("Appearance");
	odp.pfnDlgProc = OptSknWndProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
