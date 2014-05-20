/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

#include "headers.h"

HWND hwndBox = NULL;

LRESULT CALLBACK AvatarTrackBarWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AlphaTrackBarWndProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//effekt name for drop down box
LIST<TCHAR> g_lstPopupVfx(5, _tcsicmp);
void OptAdv_RegisterVfx(char *name)
{
	g_lstPopupVfx.insert(mir_a2t(name));
}

void OptAdv_UnregisterVfx()
{
	for (int i=0; i < g_lstPopupVfx.getCount(); ++i)
		mir_free(g_lstPopupVfx[i]);
	g_lstPopupVfx.destroy();
}

//Main Dialog Proc
void LoadOption_AdvOpts()
{
	//History
	PopupOptions.EnableHistory = db_get_b(NULL, MODULNAME, "EnableHistory", TRUE);
	PopupOptions.HistorySize = db_get_w(NULL, MODULNAME, "HistorySize", SETTING_HISTORYSIZE_DEFAULT);
	PopupOptions.UseHppHistoryLog = db_get_b(NULL, MODULNAME, "UseHppHistoryLog", TRUE);

	//Avatars
	PopupOptions.avatarBorders = db_get_b(NULL, MODULNAME, "AvatarBorders", TRUE);
	PopupOptions.avatarPNGBorders = db_get_b(NULL, MODULNAME, "AvatarPNGBorders", FALSE);
	PopupOptions.avatarRadius = db_get_b(NULL, MODULNAME, "AvatarRadius", 2);
	PopupOptions.avatarSize = db_get_w(NULL, MODULNAME, "AvatarSize", SETTING_AVTSIZE_DEFAULT);
	PopupOptions.EnableAvatarUpdates = db_get_b(NULL, MODULNAME, "EnableAvatarUpdates", FALSE);

	//Monitor
	PopupOptions.Monitor = db_get_b(NULL, MODULNAME, "Monitor", SETTING_MONITOR_DEFAULT);

	//Transparency
	PopupOptions.UseTransparency = db_get_b(NULL, MODULNAME, "UseTransparency", TRUE);
	PopupOptions.Alpha = db_get_b(NULL, MODULNAME, "Alpha", SETTING_ALPHA_DEFAULT);
	PopupOptions.OpaqueOnHover = db_get_b(NULL, MODULNAME, "OpaqueOnHover", TRUE);

	//Effects
	PopupOptions.UseAnimations = db_get_b(NULL, MODULNAME, "UseAnimations", TRUE);
	PopupOptions.UseEffect = db_get_b(NULL, MODULNAME, "Fade", TRUE);
	PopupOptions.Effect = (LPTSTR)DBGetContactSettingStringX(NULL, MODULNAME, "Effect", "", DBVT_TCHAR);
	PopupOptions.FadeIn = db_get_dw(NULL, MODULNAME, "FadeInTime", SETTING_FADEINTIME_DEFAULT);
	PopupOptions.FadeOut = db_get_dw(NULL, MODULNAME, "FadeOutTime", SETTING_FADEOUTTIME_DEFAULT);

	//other old stuff
	PopupOptions.MaxPopups = db_get_w(NULL, MODULNAME, "MaxPopups", 20);
}

INT_PTR CALLBACK DlgProcPopupAdvOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR tstr[64];
	static bool bDlgInit = false;	//some controls send WM_COMMAND before or during WM_INITDIALOG
	UINT idCtrl;

	switch (msg) {
	case WM_INITDIALOG:
		//Create preview box:
		{
			hwndBox = CreateWindowEx(
				WS_EX_TOOLWINDOW|WS_EX_TOPMOST,		// dwStyleEx
				_T(BOXPREVIEW_WNDCLASS),			// Class name
				NULL,								// Title
				DS_SETFONT|DS_FIXEDSYS|WS_POPUP,	// dwStyle
				CW_USEDEFAULT,						// x
				CW_USEDEFAULT,						// y
				CW_USEDEFAULT,						// Width
				CW_USEDEFAULT,						// Height
				HWND_DESKTOP,						// Parent
				NULL,								// menu handle
				hInst,								// Instance
				(LPVOID)0);
			ShowWindow(hwndBox, SW_HIDE);
		}
		//Group: History
		{
			CheckDlgButton(hwnd, IDC_ENABLE_HISTORY, PopupOptions.EnableHistory);
			SetDlgItemInt(hwnd, IDC_HISTORYSIZE, PopupOptions.HistorySize, FALSE);
			SendDlgItemMessage(hwnd, IDC_HISTORYSIZE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwnd, IDC_HISTORYSIZE), 0);
			SendDlgItemMessage(hwnd, IDC_HISTORYSIZE_SPIN, UDM_SETRANGE, 0, MAKELONG(SETTING_HISTORYSIZE_MAX, 1));
			CheckDlgButton(hwnd, IDC_HPPLOG, PopupOptions.UseHppHistoryLog);

			HWND hCtrl = GetDlgItem(hwnd, IDC_SHOWHISTORY);
			SendMessage(hCtrl, BUTTONSETASFLATBTN, TRUE, 0);
			SendMessage(hCtrl, BUTTONADDTOOLTIP, (WPARAM)Translate("Popup History"), 0);
			SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(ICO_HISTORY, 0));

			EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC1),	PopupOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_HISTORYSIZE),		PopupOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_HISTORYSIZE_SPIN),PopupOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC2),	PopupOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_SHOWHISTORY),		PopupOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_HPPLOG),			PopupOptions.EnableHistory && gbHppInstalled);
		}
		//Group: Avatars
		{
			//Borders
			CheckDlgButton(hwnd, IDC_AVT_BORDER, PopupOptions.avatarBorders);
			CheckDlgButton(hwnd, IDC_AVT_PNGBORDER, PopupOptions.avatarPNGBorders);
			EnableWindow(GetDlgItem(hwnd, IDC_AVT_PNGBORDER), PopupOptions.avatarBorders);
			//Radius
			SetDlgItemInt(hwnd, IDC_AVT_RADIUS, PopupOptions.avatarRadius, FALSE);
			SendDlgItemMessage(hwnd, IDC_AVT_RADIUS_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG((PopupOptions.avatarSize / 2), 0));
			//Size
			mir_subclassWindow(GetDlgItem(hwnd, IDC_AVT_SIZE_SLIDE), AvatarTrackBarWndProc);

			SendDlgItemMessage(hwnd, IDC_AVT_SIZE_SLIDE, TBM_SETRANGE, FALSE,
				MAKELONG(SETTING_AVTSIZE_MIN, SETTING_AVTSIZE_MAX));
			SendDlgItemMessage(hwnd, IDC_AVT_SIZE_SLIDE, TBM_SETPOS, TRUE,
				max(PopupOptions.avatarSize, SETTING_AVTSIZE_MIN));
			SetDlgItemInt(hwnd, IDC_AVT_SIZE, PopupOptions.avatarSize, FALSE);
			//Request avatars
			CheckDlgButton(hwnd, IDC_AVT_REQUEST, PopupOptions.EnableAvatarUpdates);
		}
		//Group: Monitor
		{
			BOOL bMonitor = 0;

			bMonitor = GetSystemMetrics(SM_CMONITORS) > 1;

			CheckDlgButton(hwnd, IDC_MIRANDAWND, bMonitor ? (PopupOptions.Monitor == MN_MIRANDA) : TRUE);
			CheckDlgButton(hwnd, IDC_ACTIVEWND, bMonitor ? (PopupOptions.Monitor == MN_ACTIVE) : FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_GRP_MULTIMONITOR), bMonitor);
			EnableWindow(GetDlgItem(hwnd, IDC_MULTIMONITOR_DESC), bMonitor);
			EnableWindow(GetDlgItem(hwnd, IDC_MIRANDAWND), bMonitor);
			EnableWindow(GetDlgItem(hwnd, IDC_ACTIVEWND), bMonitor);
		}
		//Group: Transparency
		{
			//win2k+
			CheckDlgButton(hwnd, IDC_TRANS, PopupOptions.UseTransparency);
			SendDlgItemMessage(hwnd, IDC_TRANS_SLIDER, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
			SendDlgItemMessage(hwnd, IDC_TRANS_SLIDER, TBM_SETPOS, TRUE, PopupOptions.Alpha);
			mir_subclassWindow(GetDlgItem(hwnd, IDC_TRANS_SLIDER), AlphaTrackBarWndProc);
			mir_sntprintf(tstr, SIZEOF(tstr), _T("%d%%"), Byte2Percentile(PopupOptions.Alpha));
			SetDlgItemText(hwnd, IDC_TRANS_PERCENT, tstr);
			CheckDlgButton(hwnd, IDC_TRANS_OPAQUEONHOVER, PopupOptions.OpaqueOnHover);
			{
				BOOL how = TRUE;

				EnableWindow(GetDlgItem(hwnd, IDC_TRANS), how);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_TXT1), how && PopupOptions.UseTransparency);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_SLIDER), how && PopupOptions.UseTransparency);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_PERCENT), how && PopupOptions.UseTransparency);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_OPAQUEONHOVER), how && PopupOptions.UseTransparency);
			}
			ShowWindow(GetDlgItem(hwnd, IDC_TRANS), SW_SHOW);
		}
		//Group: Effects
		{
			//Use Animations
			CheckDlgButton(hwnd, IDC_USEANIMATIONS, PopupOptions.UseAnimations);
			//Fade
			SetDlgItemInt(hwnd, IDC_FADEIN, PopupOptions.FadeIn, FALSE);
			SetDlgItemInt(hwnd, IDC_FADEOUT, PopupOptions.FadeOut, FALSE);
			UDACCEL aAccels[] = {{0,50},{1,100},{3,500}};
			SendDlgItemMessage(hwnd, IDC_FADEIN_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_FADEINTIME_MAX, SETTING_FADEINTIME_MIN));
			SendDlgItemMessage(hwnd, IDC_FADEIN_SPIN, UDM_SETACCEL, (WPARAM)SIZEOF(aAccels), (LPARAM)&aAccels);
			SendDlgItemMessage(hwnd, IDC_FADEOUT_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_FADEOUTTIME_MAX, SETTING_FADEOUTTIME_MIN));
			SendDlgItemMessage(hwnd, IDC_FADEOUT_SPIN, UDM_SETACCEL, (WPARAM)SIZEOF(aAccels), (LPARAM)&aAccels);

			BOOL how = PopupOptions.UseAnimations || PopupOptions.UseEffect;
			EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_TXT1),		how);
			EnableWindow(GetDlgItem(hwnd, IDC_FADEIN),			how);
			EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_SPIN),		how);
			EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_TXT2),		how);
			EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_TXT1),	how);
			EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT),			how);
			EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_SPIN),	how);
			EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_TXT2),	how);
			//effects drop down
			{
				DWORD dwItem, dwActiveItem = 0;

				BOOL how = TRUE;

				EnableWindow(GetDlgItem(hwnd, IDC_EFFECT),		how);
				EnableWindow(GetDlgItem(hwnd, IDC_EFFECT_TXT),	how);

				HWND hCtrl = GetDlgItem(hwnd, IDC_EFFECT);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("No effect")), -2);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Fade in/out")), -1);
				dwActiveItem = (DWORD)PopupOptions.UseEffect;
				for (int i = 0; i < g_lstPopupVfx.getCount(); ++i) {
					dwItem = ComboBox_AddString(hCtrl, TranslateTS(g_lstPopupVfx[i]));
					ComboBox_SetItemData(hCtrl, dwItem, i);
					if (PopupOptions.UseEffect && !lstrcmp(g_lstPopupVfx[i], PopupOptions.Effect))
						dwActiveItem = dwItem;
				}
				SendDlgItemMessage(hwnd, IDC_EFFECT, CB_SETCURSEL, dwActiveItem, 0);
			}
		}

		//later check stuff
		SetDlgItemInt(hwnd, IDC_MAXPOPUPS, PopupOptions.MaxPopups, FALSE);
		SendDlgItemMessage(hwnd, IDC_MAXPOPUPS_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwnd, IDC_MAXPOPUPS), 0);
		SendDlgItemMessage(hwnd, IDC_MAXPOPUPS_SPIN, UDM_SETRANGE, 0, MAKELONG(999, 1));
		TranslateDialogDefault(hwnd);	//do it on end of WM_INITDIALOG
		bDlgInit = true;
		return TRUE;

	case WM_HSCROLL:
		switch (idCtrl = GetDlgCtrlID((HWND)lParam)) {
		case IDC_AVT_SIZE_SLIDE:
			PopupOptions.avatarSize = SendDlgItemMessage(hwnd, IDC_AVT_SIZE_SLIDE, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hwnd, IDC_AVT_SIZE, PopupOptions.avatarSize, FALSE);
			SendDlgItemMessage(hwnd, IDC_AVT_RADIUS_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG((PopupOptions.avatarSize / 2), 0));
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_TRANS_SLIDER:
			PopupOptions.Alpha = (BYTE)SendDlgItemMessage(hwnd, IDC_TRANS_SLIDER, TBM_GETPOS, 0, 0);
			mir_sntprintf(tstr, SIZEOF(tstr), TranslateT("%d%%"), Byte2Percentile(PopupOptions.Alpha));
			SetDlgItemText(hwnd, IDC_TRANS_PERCENT, tstr);
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_COMMAND:
		idCtrl = LOWORD(wParam);
		switch (HIWORD(wParam)) {
		case BN_CLICKED:		//Button controls
			switch (idCtrl) {
			case IDC_ENABLE_HISTORY:
				PopupOptions.EnableHistory = !PopupOptions.EnableHistory;
				EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC1),	PopupOptions.EnableHistory);
				EnableWindow(GetDlgItem(hwnd, IDC_HISTORYSIZE),		PopupOptions.EnableHistory);
				EnableWindow(GetDlgItem(hwnd, IDC_HISTORYSIZE_SPIN),PopupOptions.EnableHistory);
				EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC2),	PopupOptions.EnableHistory);
				EnableWindow(GetDlgItem(hwnd, IDC_SHOWHISTORY),		PopupOptions.EnableHistory);
				EnableWindow(GetDlgItem(hwnd, IDC_HPPLOG), PopupOptions.EnableHistory && gbHppInstalled);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_SHOWHISTORY:
				PopupHistoryShow();
				break;

			case IDC_HPPLOG:
				PopupOptions.UseHppHistoryLog = !PopupOptions.UseHppHistoryLog;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_AVT_BORDER:
				PopupOptions.avatarBorders = !PopupOptions.avatarBorders;
				EnableWindow(GetDlgItem(hwnd, IDC_AVT_PNGBORDER),	PopupOptions.avatarBorders);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_AVT_PNGBORDER:
				PopupOptions.avatarPNGBorders = !PopupOptions.avatarPNGBorders;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_AVT_REQUEST:
				PopupOptions.EnableAvatarUpdates = !PopupOptions.EnableAvatarUpdates;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_MIRANDAWND:
				PopupOptions.Monitor = MN_MIRANDA;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_ACTIVEWND:
				PopupOptions.Monitor = MN_ACTIVE;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_TRANS:
				PopupOptions.UseTransparency = !PopupOptions.UseTransparency;
				{
					BOOL how = TRUE;
					EnableWindow(GetDlgItem(hwnd, IDC_TRANS_TXT1),			how && PopupOptions.UseTransparency);
					EnableWindow(GetDlgItem(hwnd, IDC_TRANS_SLIDER),		how && PopupOptions.UseTransparency);
					EnableWindow(GetDlgItem(hwnd, IDC_TRANS_PERCENT),		how && PopupOptions.UseTransparency);
					EnableWindow(GetDlgItem(hwnd, IDC_TRANS_OPAQUEONHOVER),	how && PopupOptions.UseTransparency);
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_TRANS_OPAQUEONHOVER:
				PopupOptions.OpaqueOnHover = !PopupOptions.OpaqueOnHover;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_USEANIMATIONS:
				PopupOptions.UseAnimations = !PopupOptions.UseAnimations;
				{
					BOOL enable = PopupOptions.UseAnimations || PopupOptions.UseEffect;
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_TXT1),		enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN),			enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_SPIN),		enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_TXT2),		enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_TXT1),	enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT),			enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_SPIN),	enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_TXT2),	enable);
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_PREVIEW:
				PopupPreview();
				break;
			}
			break;

		case CBN_SELCHANGE:
			//lParam = Handle to the control
			switch (idCtrl) {
			case IDC_EFFECT:
				{
					int iEffect = ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
					PopupOptions.UseEffect = (iEffect != -2) ? TRUE : FALSE;
					mir_free(PopupOptions.Effect);
					PopupOptions.Effect = mir_tstrdup((iEffect >= 0) ? g_lstPopupVfx[iEffect] : _T(""));

					BOOL enable = PopupOptions.UseAnimations || PopupOptions.UseEffect;
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_TXT1),		enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN),			enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_SPIN),		enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEIN_TXT2),		enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_TXT1),	enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT),			enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_SPIN),	enable);
					EnableWindow(GetDlgItem(hwnd, IDC_FADEOUT_TXT2),	enable);
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;
			}
			break;

		case EN_CHANGE:			//Edit controls change
			if (!bDlgInit) break;
			//lParam = Handle to the control
			switch (idCtrl) {
			case IDC_MAXPOPUPS:
				{
					int maxPop = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (maxPop > 0) {
						PopupOptions.MaxPopups = maxPop;
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_HISTORYSIZE:
				{
					int histSize = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (histSize > 0 && histSize <= SETTING_HISTORYSIZE_MAX) {
							PopupOptions.HistorySize = histSize;
							SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_AVT_RADIUS:
				{
					int avtRadius = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (avtRadius <= SETTING_AVTSIZE_MAX / 2 ) {
						PopupOptions.avatarRadius = avtRadius;
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_FADEIN:
				{
					int fadeIn = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (fadeIn >= SETTING_FADEINTIME_MIN && fadeIn <= SETTING_FADEINTIME_MAX ) {
							PopupOptions.FadeIn = fadeIn;
							SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			case IDC_FADEOUT:
				{
					int fadeOut = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (fadeOut >= SETTING_FADEOUTTIME_MIN && fadeOut <= SETTING_FADEOUTTIME_MAX) {
							PopupOptions.FadeOut = fadeOut;
							SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			}
			break;

		case EN_KILLFOCUS:		//Edit controls lost fokus
			//lParam = Handle to the control
			switch (idCtrl) {
			case IDC_MAXPOPUPS:
				{
					int maxPop = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (maxPop <= 0)
						PopupOptions.MaxPopups = 20;
					if (maxPop != PopupOptions.MaxPopups) {
						SetDlgItemInt(hwnd, idCtrl, PopupOptions.MaxPopups, FALSE);
						//ErrorMSG(1);
						SetFocus((HWND)lParam);
					}
				}
				break;
			case IDC_HISTORYSIZE:
				{
					int histSize = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (histSize <= 0)
						PopupOptions.HistorySize = SETTING_HISTORYSIZE_DEFAULT;
					else if (histSize > SETTING_HISTORYSIZE_MAX)
						PopupOptions.HistorySize = SETTING_HISTORYSIZE_MAX;
					if (histSize != PopupOptions.HistorySize) {
						SetDlgItemInt(hwnd, idCtrl, PopupOptions.HistorySize, FALSE);
						ErrorMSG(1, SETTING_HISTORYSIZE_MAX);
						SetFocus((HWND)lParam);
					}
				}
				break;
			case IDC_AVT_RADIUS:
				{
					int avtRadius = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (avtRadius > SETTING_AVTSIZE_MAX / 2)
						PopupOptions.avatarRadius = SETTING_AVTSIZE_MAX / 2;
					if (avtRadius != PopupOptions.avatarRadius) {
						SetDlgItemInt(hwnd, idCtrl, PopupOptions.avatarRadius, FALSE);
						ErrorMSG(0, SETTING_AVTSIZE_MAX / 2);
						SetFocus((HWND)lParam);
					}
				}
				break;
			case IDC_FADEIN:
				{
					int fade = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (fade < SETTING_FADEINTIME_MIN)
						PopupOptions.FadeIn = SETTING_FADEINTIME_MIN;
					else if (fade > SETTING_FADEINTIME_MAX)
						PopupOptions.FadeIn = SETTING_FADEINTIME_MAX;
					if (fade != PopupOptions.FadeIn) {
						SetDlgItemInt(hwnd, idCtrl, PopupOptions.FadeIn, FALSE);
						ErrorMSG(SETTING_FADEINTIME_MIN, SETTING_FADEINTIME_MAX);
						SetFocus((HWND)lParam);
					}
				}
				break;
			case IDC_FADEOUT:
				{
					int fade = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
					if (fade < SETTING_FADEOUTTIME_MIN)
						PopupOptions.FadeOut = SETTING_FADEOUTTIME_MIN;
					else if (fade > SETTING_FADEOUTTIME_MAX)
						PopupOptions.FadeOut = SETTING_FADEOUTTIME_MAX;
					if (fade != PopupOptions.FadeOut) {
						SetDlgItemInt(hwnd, idCtrl, PopupOptions.FadeOut, FALSE);
						ErrorMSG(SETTING_FADEOUTTIME_MIN, SETTING_FADEOUTTIME_MAX);
						SetFocus((HWND)lParam);
					}
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadOption_AdvOpts();
				return TRUE;

			case PSN_APPLY:
				//History
				db_set_b(NULL, MODULNAME, "EnableHistory", (BYTE)PopupOptions.EnableHistory);
				db_set_w(NULL, MODULNAME, "HistorySize", PopupOptions.HistorySize);
				PopupHistoryResize();
				db_set_b(NULL, MODULNAME, "UseHppHistoryLog", PopupOptions.UseHppHistoryLog);
				//Avatars
				db_set_b(NULL, MODULNAME, "AvatarBorders", PopupOptions.avatarBorders);
				db_set_b(NULL, MODULNAME, "AvatarPNGBorders", PopupOptions.avatarPNGBorders);
				db_set_b(NULL, MODULNAME, "AvatarRadius", PopupOptions.avatarRadius);
				db_set_w(NULL, MODULNAME, "AvatarSize", PopupOptions.avatarSize);
				db_set_b(NULL, MODULNAME, "EnableAvatarUpdates", PopupOptions.EnableAvatarUpdates);
				//Monitor
				db_set_b(NULL, MODULNAME, "Monitor", PopupOptions.Monitor);
				//Transparency
				db_set_b(NULL, MODULNAME, "UseTransparency", PopupOptions.UseTransparency);
				db_set_b(NULL, MODULNAME, "Alpha", PopupOptions.Alpha);
				db_set_b(NULL, MODULNAME, "OpaqueOnHover", PopupOptions.OpaqueOnHover);

				//Effects
				db_set_b(NULL, MODULNAME, "UseAnimations", PopupOptions.UseAnimations);
				db_set_b(NULL, MODULNAME, "Fade", PopupOptions.UseEffect);
				db_set_ts(NULL, MODULNAME, "Effect", PopupOptions.Effect);
				db_set_dw(NULL, MODULNAME, "FadeInTime", PopupOptions.FadeIn);
				db_set_dw(NULL, MODULNAME, "FadeOutTime", PopupOptions.FadeOut);
				//other old stuff
				db_set_w(NULL, MODULNAME, "MaxPopups", (BYTE)PopupOptions.MaxPopups);
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		bDlgInit = false;
		break;
	}
	return FALSE;
}

LRESULT CALLBACK AvatarTrackBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!IsWindowEnabled(hwnd))
		return mir_callNextSubclass(hwnd, AvatarTrackBarWndProc, msg, wParam, lParam);

	static int oldVal = -1;
	switch (msg) {
	case WM_MOUSEWHEEL:
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (!IsWindowVisible(hwndBox))
			break;

	case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = hwnd;
			_TrackMouseEvent(&tme);

			int newVal = (BYTE)SendMessage(hwnd, TBM_GETPOS, 0, 0);
			if (oldVal != newVal) {
				if (oldVal < 0)
					SetWindowLongPtr(hwndBox, GWLP_USERDATA, 0);

				RECT rc; GetWindowRect(hwnd, &rc);
				SetWindowPos(hwndBox, NULL,
					(rc.left + rc.right - newVal) / 2, rc.bottom + 2, newVal, newVal,
					SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);

				HRGN rgn = CreateRoundRectRgn(0, 0, newVal, newVal, 2 * PopupOptions.avatarRadius, 2 * PopupOptions.avatarRadius);
				SetWindowRgn(hwndBox, rgn, TRUE);
				InvalidateRect(hwndBox, NULL, FALSE);
				oldVal = newVal;
			}
		}
		break;

	case WM_MOUSELEAVE:
		SetWindowRgn(hwndBox, NULL, TRUE);
		ShowWindow(hwndBox, SW_HIDE);
		oldVal = -1;
		break;
	}
	return mir_callNextSubclass(hwnd, AvatarTrackBarWndProc, msg, wParam, lParam);
}

LRESULT CALLBACK AlphaTrackBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!IsWindowEnabled(hwnd))
		return mir_callNextSubclass(hwnd, AlphaTrackBarWndProc, msg, wParam, lParam);

	static int oldVal = -1;
	switch (msg) {
	case WM_MOUSEWHEEL:
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (!IsWindowVisible(hwndBox))
			break;

	case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = hwnd;
			_TrackMouseEvent(&tme);

			int newVal = (BYTE)SendMessage(hwnd, TBM_GETPOS, 0, 0);
			if (oldVal != newVal)
			{

				if (oldVal < 0)
				{
					SetWindowLongPtr(hwndBox, GWLP_USERDATA, 1);
					RECT rc; GetWindowRect(hwnd, &rc);
					SetWindowPos(hwndBox, NULL,
						(rc.left + rc.right - 170) / 2, rc.bottom + 2, 170, 50,
						SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
					SetWindowRgn(hwndBox, NULL, TRUE);
				}
				SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetLayeredWindowAttributes(hwndBox, NULL, newVal, LWA_ALPHA);

				oldVal = newVal;
			}
		}
		break;

	case WM_MOUSELEAVE:
		SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwndBox, NULL, 255, LWA_ALPHA);

		ShowWindow(hwndBox, SW_HIDE);
		oldVal = -1;
		break;
	}
	return mir_callNextSubclass(hwnd, AlphaTrackBarWndProc, msg, wParam, lParam);
}

