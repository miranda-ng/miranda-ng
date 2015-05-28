
#include "stdhdr.h"

#define M_GUESSSAMEASBOXES		(WM_USER + 18)

#define M_REFRESHBKGBOXES		(WM_USER + 20)
#define M_REFRESHBORDERPICKERS	(WM_USER + 21)

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR APIENTRY OptWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg ) {
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

		SendDlgItemMessage(hwndDlg, IDC_WIDTHSPIN, UDM_SETRANGE, 0, MAKELONG(255,0));
		SendDlgItemMessage(hwndDlg, IDC_WIDTHSPIN, UDM_SETPOS, 0, fcOpt.nThumbWidth);

		CheckDlgButton(hwndDlg, IDC_CHK_TIP, (fcOpt.bShowTip ? BST_CHECKED : BST_UNCHECKED));

		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_TIP), bEnableTip);

		EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN), bEnableTip && fcOpt.bShowTip);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN_CMT), bEnableTip && fcOpt.bShowTip);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TIMEIN), bEnableTip && fcOpt.bShowTip);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEINSPIN), bEnableTip && fcOpt.bShowTip);

		SendDlgItemMessage(hwndDlg, IDC_TIMEINSPIN, UDM_SETRANGE, 0, MAKELONG(5000,0));
		SendDlgItemMessage(hwndDlg, IDC_TIMEINSPIN, UDM_SETPOS, 0, fcOpt.TimeIn);

		CheckDlgButton(hwndDlg, IDC_CHK_TOTOP, (fcOpt.bToTop ? BST_CHECKED : BST_UNCHECKED));

		EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TOTOP), fcOpt.bToTop);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TOTOPTIME), fcOpt.bToTop);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TOTOPTIMESPIN), fcOpt.bToTop);

		SendDlgItemMessage(hwndDlg, IDC_TOTOPTIMESPIN, UDM_SETRANGE, 0, MAKELONG(TOTOPTIME_MAX,1));
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

				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_WIDTH ), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_WIDTH ), bChecked);
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

				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN ), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TIMEIN_CMT ), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TIMEIN ), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEINSPIN), bChecked);
			}
			break;

		case IDC_CHK_TOTOP:
			if (BN_CLICKED == HIWORD(wParam)) {
				BOOL bChecked = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TOTOP);

				EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_TOTOP ), bChecked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_TOTOPTIME ), bChecked);
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

				fcOpt.bHideOffline = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_OFFLINE);
				db_set_b(NULL, MODULE, "HideOffline", (BYTE)fcOpt.bHideOffline);

				fcOpt.bHideAll = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_ALL);
				db_set_b(NULL, MODULE, "HideAll", (BYTE)fcOpt.bHideAll);

				fcOpt.bHideWhenFullscreen = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_WHEN_FULSCREEN);
				db_set_b(NULL, MODULE, "HideWhenFullscreen", (BYTE)fcOpt.bHideWhenFullscreen);

				fcOpt.bMoveTogether = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_STICK);
				db_set_b(NULL, MODULE, "MoveTogether", (BYTE)fcOpt.bMoveTogether);

				fcOpt.bFixedWidth = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_WIDTH);
				db_set_b(NULL, MODULE, "FixedWidth", (BYTE)fcOpt.bFixedWidth);
				fcOpt.nThumbWidth	 = GetDlgItemInt(hwndDlg, IDC_TXT_WIDTH, &bSuccess, FALSE);
				db_set_dw(NULL, MODULE, "Width", fcOpt.nThumbWidth );

				if (bEnableTip) {
					fcOpt.bShowTip = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TIP);
					db_set_b(NULL, MODULE, "ShowTip", (BYTE)fcOpt.bShowTip);
					fcOpt.TimeIn	 = GetDlgItemInt(hwndDlg, IDC_TXT_TIMEIN, &bSuccess, FALSE);
					db_set_w(NULL, MODULE, "TimeIn", fcOpt.TimeIn );
				}

				fcOpt.bToTop = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_TOTOP);
				db_set_b(NULL, MODULE, "ToTop", (BYTE)fcOpt.bToTop);
				fcOpt.ToTopTime	 = GetDlgItemInt(hwndDlg, IDC_TXT_TOTOPTIME, &bSuccess, FALSE);
				db_set_w(NULL, MODULE, "ToTopTime", fcOpt.ToTopTime );

				fcOpt.bHideWhenCListShow = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE_WHEN_CLISTSHOW);
				db_set_b(NULL, MODULE, "HideWhenCListShow", (BYTE)fcOpt.bHideWhenCListShow);

				fcOpt.bUseSingleClick = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_SINGLECLK);
				db_set_b(NULL, MODULE, "UseSingleClick", (BYTE)fcOpt.bUseSingleClick);

				fcOpt.bShowIdle = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWIDLE);
				db_set_b(NULL, MODULE, "ShowIdle", (BYTE)fcOpt.bShowIdle);

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

	switch ( uMsg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		// Border
		CheckDlgButton(hwndDlg, IDC_DRAWBORDER, db_get_b(NULL, MODULE, "DrawBorder", FLT_DEFAULT_DRAWBORDER) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, M_REFRESHBORDERPICKERS, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_LTEDGESCOLOR);
		SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_SETCOLOUR, 0, db_get_dw(NULL, MODULE, "LTEdgesColor", FLT_DEFAULT_LTEDGESCOLOR));
		SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_RBEDGESCOLOR);
		SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_SETCOLOUR, 0, db_get_dw(NULL, MODULE, "RBEdgesColor", FLT_DEFAULT_RBEDGESCOLOR));

		// Background
		CheckDlgButton(hwndDlg, IDC_CHK_WIDTH, (fcOpt.bFixedWidth ? BST_CHECKED : BST_UNCHECKED));

		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, FLT_DEFAULT_BKGNDCOLOR);
		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, db_get_dw(NULL, MODULE, "BkColor", FLT_DEFAULT_BKGNDCOLOR));
		CheckDlgButton(hwndDlg, IDC_BITMAP, db_get_b(NULL, MODULE, "BkUseBitmap", FLT_DEFAULT_BKGNDUSEBITMAP) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, M_REFRESHBKGBOXES, 0, 0);
		{
			DBVARIANT dbv;
			if ( !db_get_ts(NULL, MODULE, "BkBitmap", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_FILENAME, dbv.ptszVal);
				db_free(&dbv);
			}

			WORD bmpUse = (WORD)db_get_w(NULL, MODULE, "BkBitmapOpt", FLT_DEFAULT_BKGNDBITMAPOPT);
			CheckDlgButton(hwndDlg, IDC_STRETCHH, ((bmpUse & CLB_STRETCHH) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_STRETCHV, ((bmpUse & CLB_STRETCHV) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_TILEH, ((bmpUse & CLBF_TILEH) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_TILEV, ((bmpUse & CLBF_TILEV) ? BST_CHECKED : BST_UNCHECKED));
			CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, ((bmpUse & CLBF_PROPORTIONAL) ? BST_CHECKED : BST_UNCHECKED));

			SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);

			// Windows 2K/XP
			BYTE btOpacity = (BYTE)db_get_b(NULL, MODULE, "Opacity", 100);
			SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
			SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_SETPOS, TRUE, btOpacity);
				
			mir_snprintf(szPercent, SIZEOF(szPercent), "%d%%", btOpacity);
			SetDlgItemTextA(hwndDlg, IDC_OPACITY, szPercent);

			EnableWindow(GetDlgItem(hwndDlg, IDC_SLIDER_OPACITY), SetLayeredWindowAttributes != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPACITY), SetLayeredWindowAttributes != 0);
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
			fcOpt.thumbAlpha = (BYTE)(( nPos * 255 ) / 100 );
			SetThumbsOpacity(fcOpt.thumbAlpha);

			mir_snprintf(szPercent, SIZEOF(szPercent), "%d%%", nPos);
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
				TCHAR str[MAX_PATH], filter[512];
				GetDlgItemText(hwndDlg, IDC_FILENAME, str, SIZEOF(str));
				BmpFilterGetStrings(filter, SIZEOF(filter));

				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = str;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.nMaxFile = SIZEOF(str);
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = _T("bmp");
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
				fcOpt.thumbAlpha = (BYTE)((double)db_get_b(NULL, MODULE, "Opacity", 100) * 2.55);
				SetThumbsOpacity(fcOpt.thumbAlpha);
				break;

			case PSN_APPLY:
				// Border
				db_set_b(NULL, MODULE, "DrawBorder", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAWBORDER));

				COLORREF col = SendDlgItemMessage(hwndDlg, IDC_LTEDGESCOLOR, CPM_GETCOLOUR, 0, 0);
				db_set_dw(NULL, MODULE, "LTEdgesColor", col);
				col = SendDlgItemMessage(hwndDlg, IDC_RBEDGESCOLOR, CPM_GETCOLOUR, 0, 0);
				db_set_dw(NULL, MODULE, "RBEdgesColor", col);

				db_set_b(NULL, MODULE, "Opacity", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SLIDER_OPACITY, TBM_GETPOS, 0, 0));

				// Backgroud
				col = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);
				db_set_dw(NULL, MODULE, "BkColor", col);

				db_set_b(NULL, MODULE, "BkUseBitmap", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BITMAP));

				TCHAR str[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_FILENAME, str, SIZEOF(str));
				db_set_ts(NULL, MODULE, "BkBitmap", str);

				WORD flags = 0;
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
				db_set_w(NULL, MODULE, "BkBitmapOpt", flags);

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

int OnOptionsInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance   = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FLTCONT);
	odp.pszTitle    = LPGEN("Floating Contacts");
	odp.pszGroup    = LPGEN("Contact list");
	odp.pszTab      = LPGEN("Main Features");
	odp.flags       = ODPF_BOLDGROUPS;
	odp.pfnDlgProc  = OptWndProc;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.pszTab      = LPGEN("Appearance");
	odp.pfnDlgProc  = OptSknWndProc;
	Options_AddPage(wParam, &odp);
	return 0;
}
