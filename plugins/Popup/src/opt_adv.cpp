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
void OptAdv_RegisterVfx(char *name) {
	g_lstPopupVfx.insert(mir_a2t(name));
}

void OptAdv_UnregisterVfx() {
	for (int i=0; i < g_lstPopupVfx.getCount(); ++i)
		mir_free(g_lstPopupVfx[i]);
	g_lstPopupVfx.destroy();
}

//Main Dialog Proc
void LoadOption_AdvOpts() {
	//History
	PopUpOptions.EnableHistory			= DBGetContactSettingByte (NULL,MODULNAME, "EnableHistory", TRUE);
	PopUpOptions.HistorySize			= DBGetContactSettingWord (NULL,MODULNAME, "HistorySize", SETTING_HISTORYSIZE_DEFAULT);
	PopUpOptions.UseHppHistoryLog		= DBGetContactSettingByte (NULL,MODULNAME, "UseHppHistoryLog", TRUE);
	//Avatars
	PopUpOptions.avatarBorders			= DBGetContactSettingByte (NULL,MODULNAME, "AvatarBorders", TRUE);
	PopUpOptions.avatarPNGBorders		= DBGetContactSettingByte (NULL,MODULNAME, "AvatarPNGBorders", FALSE);
	PopUpOptions.avatarRadius			= DBGetContactSettingByte (NULL,MODULNAME, "AvatarRadius", 2);
	PopUpOptions.avatarSize				= DBGetContactSettingWord (NULL,MODULNAME, "AvatarSize", SETTING_AVTSIZE_DEFAULT);
	PopUpOptions.EnableAvatarUpdates	= DBGetContactSettingByte (NULL,MODULNAME, "EnableAvatarUpdates", FALSE);
	//Monitor
	PopUpOptions.Monitor				= DBGetContactSettingByte (NULL,MODULNAME, "Monitor", SETTING_MONITOR_DEFAULT);
	//Transparency
	PopUpOptions.Enable9xTransparency	= DBGetContactSettingByte (NULL,MODULNAME, "EnableRegionTransparency", TRUE);
	PopUpOptions.UseTransparency		= DBGetContactSettingByte (NULL,MODULNAME, "UseTransparency", TRUE);
	PopUpOptions.Alpha					= DBGetContactSettingByte (NULL,MODULNAME, "Alpha", SETTING_ALPHA_DEFAULT);
	PopUpOptions.OpaqueOnHover			= DBGetContactSettingByte (NULL,MODULNAME, "OpaqueOnHover", TRUE);
	//Effects
	PopUpOptions.UseAnimations			= DBGetContactSettingByte (NULL,MODULNAME, "UseAnimations", TRUE);
	PopUpOptions.UseEffect				= DBGetContactSettingByte (NULL,MODULNAME, "Fade", TRUE);
	PopUpOptions.Effect					= (LPTSTR)DBGetContactSettingStringX(NULL,MODULNAME, "Effect", "", DBVT_TCHAR);
	PopUpOptions.FadeIn					= DBGetContactSettingDword(NULL,MODULNAME, "FadeInTime", SETTING_FADEINTIME_DEFAULT);
	PopUpOptions.FadeOut				= DBGetContactSettingDword(NULL,MODULNAME, "FadeOutTime",SETTING_FADEOUTTIME_DEFAULT);
	//other old stuff
	PopUpOptions.MaxPopups				= DBGetContactSettingWord (NULL,MODULNAME, "MaxPopups", 20);
}

INT_PTR CALLBACK DlgProcPopUpAdvOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	TCHAR tstr[64];
	static bool bDlgInit = false;	//some controls send WM_COMMAND before or during WM_INITDIALOG

	switch (msg) {
		case WM_INITDIALOG: {
			HWND hCtrl = NULL;

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
			CheckDlgButton(hwnd, IDC_ENABLE_HISTORY, PopUpOptions.EnableHistory);
			SetDlgItemInt (hwnd, IDC_HISTORYSIZE, PopUpOptions.HistorySize, FALSE);
			CheckDlgButton(hwnd, IDC_HPPLOG, PopUpOptions.UseHppHistoryLog);

			hCtrl = GetDlgItem(hwnd, IDC_SHOWHISTORY);
			SendMessage(hCtrl, BUTTONSETASFLATBTN, TRUE, 0);
			SendMessage(hCtrl, BUTTONADDTOOLTIP, (WPARAM)Translate("Popup History"), 0);
			SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(ICO_HISTORY,0));

			EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC1),	PopUpOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_HISTORYSIZE),		PopUpOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC2),	PopUpOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_SHOWHISTORY),		PopUpOptions.EnableHistory);
			EnableWindow(GetDlgItem(hwnd, IDC_HPPLOG),			PopUpOptions.EnableHistory && gbHppInstalled);
			}
			//Group: Avatars
			{
			//Borders
			CheckDlgButton(hwnd, IDC_AVT_BORDER, PopUpOptions.avatarBorders);
			CheckDlgButton(hwnd, IDC_AVT_PNGBORDER, PopUpOptions.avatarPNGBorders);
			EnableWindow(GetDlgItem(hwnd, IDC_AVT_PNGBORDER), PopUpOptions.avatarBorders);
			//Radius
			SetDlgItemInt(hwnd, IDC_AVT_RADIUS, PopUpOptions.avatarRadius, FALSE);
			SendDlgItemMessage(hwnd, IDC_AVT_RADIUS_SPIN,UDM_SETRANGE, 0, (LPARAM)MAKELONG((PopUpOptions.avatarSize / 2),0));
			//Size
			mir_subclassWindow(GetDlgItem(hwnd, IDC_AVT_SIZE_SLIDE),	AvatarTrackBarWndProc);

			SendDlgItemMessage(hwnd, IDC_AVT_SIZE_SLIDE, TBM_SETRANGE,FALSE,
				MAKELONG(SETTING_AVTSIZE_MIN, SETTING_AVTSIZE_MAX));
			SendDlgItemMessage(hwnd, IDC_AVT_SIZE_SLIDE, TBM_SETPOS, TRUE, 
				max(PopUpOptions.avatarSize, SETTING_AVTSIZE_MIN));
			SetDlgItemInt(hwnd, IDC_AVT_SIZE, PopUpOptions.avatarSize, FALSE);
			//Request avatars
			CheckDlgButton(hwnd, IDC_AVT_REQUEST, PopUpOptions.EnableAvatarUpdates);
			}
			//Group: Monitor
			{
			BOOL bMonitor = 0;

			bMonitor = GetSystemMetrics(SM_CMONITORS)>1;

			CheckDlgButton(hwnd, IDC_MIRANDAWND, bMonitor ? (PopUpOptions.Monitor == MN_MIRANDA) : TRUE);
			CheckDlgButton(hwnd, IDC_ACTIVEWND,  bMonitor ? (PopUpOptions.Monitor == MN_ACTIVE)  : FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_GRP_MULTIMONITOR),  bMonitor);
			EnableWindow(GetDlgItem(hwnd, IDC_MULTIMONITOR_DESC), bMonitor);
			EnableWindow(GetDlgItem(hwnd, IDC_MIRANDAWND), bMonitor);
			EnableWindow(GetDlgItem(hwnd, IDC_ACTIVEWND),  bMonitor);
			}
			//Group: Transparency
			{
			//9x/ME
			CheckDlgButton(hwnd, IDC_TRANS_9X, PopUpOptions.Enable9xTransparency);
			//EnableWindow(GetDlgItem(hwnd, IDC_TRANS_9X), !IsWinVer2000Plus());
			ShowWindow(GetDlgItem(hwnd, IDC_TRANS_9X), IsWinVer2000Plus() ? SW_HIDE : SW_SHOW);
			//win2k+
			CheckDlgButton(hwnd, IDC_TRANS, PopUpOptions.UseTransparency);
			SendDlgItemMessage(hwnd, IDC_TRANS_SLIDER, TBM_SETRANGE, FALSE, MAKELONG(1,255));
			SendDlgItemMessage(hwnd, IDC_TRANS_SLIDER, TBM_SETPOS, TRUE, PopUpOptions.Alpha);
			mir_subclassWindow(GetDlgItem(hwnd, IDC_TRANS_SLIDER),	AlphaTrackBarWndProc);
			wsprintf(tstr, _T("%d%%"), Byte2Percentile(PopUpOptions.Alpha));
			SetDlgItemText(hwnd, IDC_TRANS_PERCENT, tstr);
			CheckDlgButton(hwnd, IDC_TRANS_OPAQUEONHOVER, PopUpOptions.OpaqueOnHover);
			{

				BOOL how = TRUE;

				EnableWindow(GetDlgItem(hwnd, IDC_TRANS)				,how);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_TXT1)			,how && PopUpOptions.UseTransparency);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_SLIDER)			,how && PopUpOptions.UseTransparency);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_PERCENT)		,how && PopUpOptions.UseTransparency);
				EnableWindow(GetDlgItem(hwnd, IDC_TRANS_OPAQUEONHOVER)	,how && PopUpOptions.UseTransparency);
			}
			ShowWindow(GetDlgItem(hwnd, IDC_TRANS), IsWinVer2000Plus() ? SW_SHOW : SW_HIDE);
			}
			//Group: Effects
			{
			//Use Animations
			CheckDlgButton(hwnd, IDC_USEANIMATIONS, PopUpOptions.UseAnimations);
			//Fade
			SetDlgItemInt (hwnd, IDC_FADEIN, PopUpOptions.FadeIn, FALSE);
			SetDlgItemInt (hwnd, IDC_FADEOUT,PopUpOptions.FadeOut,FALSE);
			UDACCEL aAccels[] = {{0,50},{1,100},{3,500}};
 			SendDlgItemMessage(hwnd, IDC_FADEIN_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_FADEINTIME_MAX, SETTING_FADEINTIME_MIN));
			SendDlgItemMessage(hwnd, IDC_FADEIN_SPIN, UDM_SETACCEL, (WPARAM)SIZEOF(aAccels), (LPARAM)&aAccels);
 			SendDlgItemMessage(hwnd, IDC_FADEOUT_SPIN,UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_FADEOUTTIME_MAX,SETTING_FADEOUTTIME_MIN));
			SendDlgItemMessage(hwnd, IDC_FADEOUT_SPIN,UDM_SETACCEL, (WPARAM)SIZEOF(aAccels), (LPARAM)&aAccels);

			BOOL how = PopUpOptions.UseAnimations || PopUpOptions.UseEffect;
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

				hCtrl = GetDlgItem(hwnd, IDC_EFFECT);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("No effect"))	,-2);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Fade in/out"))		,-1);
				dwActiveItem = (DWORD)PopUpOptions.UseEffect;
				for (int i=0; i < g_lstPopupVfx.getCount(); ++i) {
					dwItem = ComboBox_AddString(hCtrl, TranslateTS(g_lstPopupVfx[i]));
					ComboBox_SetItemData(hCtrl, dwItem, i);
					if (PopUpOptions.UseEffect && !lstrcmp(g_lstPopupVfx[i], PopUpOptions.Effect))
						dwActiveItem = dwItem;
				}
				SendDlgItemMessage(hwnd, IDC_EFFECT, CB_SETCURSEL, dwActiveItem, 0);
			}
			}

			//later check stuff
			{
			SetDlgItemInt(hwnd, IDC_MAXPOPUPS, PopUpOptions.MaxPopups, FALSE);
			}

			TranslateDialogDefault(hwnd);	//do it on end of WM_INITDIALOG
			bDlgInit = true;
			}//end WM_INITDIALOG
			return TRUE;
		case WM_HSCROLL: {
				UINT idCtrl = GetDlgCtrlID((HWND)lParam);
				switch (idCtrl) {
					case IDC_AVT_SIZE_SLIDE:
						{
						PopUpOptions.avatarSize = SendDlgItemMessage(hwnd,IDC_AVT_SIZE_SLIDE, TBM_GETPOS,0,0);
						SetDlgItemInt(hwnd, IDC_AVT_SIZE ,PopUpOptions.avatarSize,FALSE);
						SendDlgItemMessage(hwnd, IDC_AVT_RADIUS_SPIN,UDM_SETRANGE, 0, (LPARAM)MAKELONG((PopUpOptions.avatarSize / 2),0));
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
						}
						break;
					case IDC_TRANS_SLIDER:
						{
						PopUpOptions.Alpha = (BYTE)SendDlgItemMessage(hwnd,IDC_TRANS_SLIDER, TBM_GETPOS, 0,0);
						wsprintf(tstr, TranslateT("%d%%"), Byte2Percentile(PopUpOptions.Alpha));
						SetDlgItemText(hwnd, IDC_TRANS_PERCENT, tstr);
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
						}
						break;
					default:
						break;
				}// end switch idCtrl
			}//end WM_HSCROLL
			break;
		case WM_COMMAND: {
				UINT idCtrl = LOWORD(wParam);
				switch (HIWORD(wParam)) {
					case BN_CLICKED:		//Button controls
						switch(idCtrl) {
							case IDC_ENABLE_HISTORY:
								{
								PopUpOptions.EnableHistory = !PopUpOptions.EnableHistory;
								EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC1),	PopUpOptions.EnableHistory);
								EnableWindow(GetDlgItem(hwnd, IDC_HISTORYSIZE),		PopUpOptions.EnableHistory);
								EnableWindow(GetDlgItem(hwnd, IDC_HISTORY_STATIC2),	PopUpOptions.EnableHistory);
								EnableWindow(GetDlgItem(hwnd, IDC_SHOWHISTORY),		PopUpOptions.EnableHistory);
								EnableWindow(GetDlgItem(hwnd, IDC_HPPLOG), PopUpOptions.EnableHistory && gbHppInstalled);
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_SHOWHISTORY:
								{
								PopupHistoryShow();
								}
								break;
							case IDC_HPPLOG:
								{
								PopUpOptions.UseHppHistoryLog = !PopUpOptions.UseHppHistoryLog;
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_AVT_BORDER:
								{
								PopUpOptions.avatarBorders = !PopUpOptions.avatarBorders;
								EnableWindow(GetDlgItem(hwnd, IDC_AVT_PNGBORDER),	PopUpOptions.avatarBorders);
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_AVT_PNGBORDER:
								{
								PopUpOptions.avatarPNGBorders = !PopUpOptions.avatarPNGBorders;
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_AVT_REQUEST:
								{
								PopUpOptions.EnableAvatarUpdates = !PopUpOptions.EnableAvatarUpdates;
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_MIRANDAWND:
								{
								PopUpOptions.Monitor = MN_MIRANDA;
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_ACTIVEWND:
								{
								PopUpOptions.Monitor = MN_ACTIVE;
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_TRANS_9X:
								{
								PopUpOptions.Enable9xTransparency = !PopUpOptions.Enable9xTransparency;
								SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
								}
								break;
							case IDC_TRANS:
								{
								PopUpOptions.UseTransparency = !PopUpOptions.UseTransparency;

								BOOL how = TRUE;

								EnableWindow(GetDlgItem(hwnd, IDC_TRANS_TXT1)			,how && PopUpOptions.UseTransparency);
								EnableWindow(GetDlgItem(hwnd, IDC_TRANS_SLIDER)			,how && PopUpOptions.UseTransparency);
								EnableWindow(GetDlgItem(hwnd, IDC_TRANS_PERCENT)		,how && PopUpOptions.UseTransparency);
								EnableWindow(GetDlgItem(hwnd, IDC_TRANS_OPAQUEONHOVER)	,how && PopUpOptions.UseTransparency);
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_TRANS_OPAQUEONHOVER:
								{
								PopUpOptions.OpaqueOnHover = !PopUpOptions.OpaqueOnHover;
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_USEANIMATIONS:
								{
								PopUpOptions.UseAnimations = !PopUpOptions.UseAnimations;
								BOOL enable = PopUpOptions.UseAnimations || PopUpOptions.UseEffect;
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
								{
								PopUpPreview();
								}
								break;
							default:
								break;
						}
						break;
					case CBN_SELCHANGE:		//ComboBox controls
						switch(idCtrl) {
							//lParam = Handle to the control
							case IDC_EFFECT:
								{
								int iEffect = ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
								PopUpOptions.UseEffect = (iEffect != -2) ? TRUE : FALSE;
								mir_free(PopUpOptions.Effect);
								PopUpOptions.Effect = mir_tstrdup((iEffect >= 0) ? g_lstPopupVfx[iEffect] : _T(""));

								BOOL enable = PopUpOptions.UseAnimations || PopUpOptions.UseEffect;
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
							default:
								break;
						}
						break;
					case EN_CHANGE:			//Edit controls change
						if (!bDlgInit) break;
						switch(idCtrl) {
							//lParam = Handle to the control
							case IDC_MAXPOPUPS:
								{
									int maxPop = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (maxPop > 0){
										PopUpOptions.MaxPopups = maxPop;
										SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
									}
								}
								break;
							case IDC_HISTORYSIZE:
								{
									int histSize = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (	histSize > 0 &&
										histSize <= SETTING_HISTORYSIZE_MAX){
										PopUpOptions.HistorySize = histSize;
										SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
									}
								}
								break;
							case IDC_AVT_RADIUS:
								{
									int avtRadius = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (	avtRadius <= SETTING_AVTSIZE_MAX / 2 ) {
											PopUpOptions.avatarRadius = avtRadius;
											SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
									}
								}
								break;
							case IDC_FADEIN:
								{
									int fadeIn = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (	fadeIn >= SETTING_FADEINTIME_MIN &&
										fadeIn <= SETTING_FADEINTIME_MAX ) {
											PopUpOptions.FadeIn = fadeIn;
											SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
									}
								}
								break;
							case IDC_FADEOUT:
								{
									int fadeOut =  GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (	fadeOut >= SETTING_FADEOUTTIME_MIN &&
										fadeOut <= SETTING_FADEOUTTIME_MAX){
											PopUpOptions.FadeOut = fadeOut;
											SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
									}
								}
								break;
							default:
								break;
						}//end switch(idCtrl)
						break;
					case EN_KILLFOCUS:		//Edit controls lost fokus
						switch(idCtrl) {
							//lParam = Handle to the control
							case IDC_MAXPOPUPS:
								{
									int maxPop = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (maxPop <= 0)
										PopUpOptions.MaxPopups = 20;
									if (maxPop != PopUpOptions.MaxPopups) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.MaxPopups, FALSE);
										//ErrorMSG(1);
										SetFocus((HWND)lParam);
									}
								}
								break;
							case IDC_HISTORYSIZE:
								{
									int histSize = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (histSize <= 0)
										PopUpOptions.HistorySize = SETTING_HISTORYSIZE_DEFAULT;
									else if (histSize > SETTING_HISTORYSIZE_MAX)
										PopUpOptions.HistorySize = SETTING_HISTORYSIZE_MAX;
									if (histSize != PopUpOptions.HistorySize) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.HistorySize, FALSE);
										ErrorMSG(1, SETTING_HISTORYSIZE_MAX);
										SetFocus((HWND)lParam);
									}
								}
								break;
							case IDC_AVT_RADIUS:
								{
									int avtRadius = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (avtRadius > SETTING_AVTSIZE_MAX / 2)
										PopUpOptions.avatarRadius = SETTING_AVTSIZE_MAX / 2;
									if (avtRadius != PopUpOptions.avatarRadius) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.avatarRadius, FALSE);
										ErrorMSG(0, SETTING_AVTSIZE_MAX / 2);
										SetFocus((HWND)lParam);
									}
								}
								break;
							case IDC_FADEIN:
								{
									int fade = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (fade < SETTING_FADEINTIME_MIN)
										PopUpOptions.FadeIn = SETTING_FADEINTIME_MIN;
									else if (fade > SETTING_FADEINTIME_MAX)										
										PopUpOptions.FadeIn = SETTING_FADEINTIME_MAX;
									if (fade != PopUpOptions.FadeIn) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.FadeIn, FALSE);
										ErrorMSG(SETTING_FADEINTIME_MIN, SETTING_FADEINTIME_MAX);
										SetFocus((HWND)lParam);
									}
								}
								break;
							case IDC_FADEOUT:
								{
									int fade = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (fade < SETTING_FADEOUTTIME_MIN)
										PopUpOptions.FadeOut = SETTING_FADEOUTTIME_MIN;
									else if (fade > SETTING_FADEOUTTIME_MAX)
										PopUpOptions.FadeOut = SETTING_FADEOUTTIME_MAX;
									if (fade != PopUpOptions.FadeOut) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.FadeOut, FALSE);
										ErrorMSG(SETTING_FADEOUTTIME_MIN, SETTING_FADEOUTTIME_MAX);
										SetFocus((HWND)lParam);
									}
								}
								break;
							default:
								break;
						}//end switch(idCtrl)
						break;
					default:
						break;
				}// end switch (HIWORD(wParam))
			}//end WM_COMMAND
			break;
		case WM_NOTIFY: {
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0: {
					switch (((LPNMHDR)lParam)->code) {
						case PSN_RESET:
							LoadOption_AdvOpts();
							return TRUE;
						case PSN_APPLY:
							{
							//History
							DBWriteContactSettingByte (NULL,MODULNAME, "EnableHistory", (BYTE)PopUpOptions.EnableHistory);
							DBWriteContactSettingWord (NULL,MODULNAME, "HistorySize", PopUpOptions.HistorySize);
							PopupHistoryResize();
							DBWriteContactSettingByte (NULL,MODULNAME, "UseHppHistoryLog", PopUpOptions.UseHppHistoryLog);
							//Avatars
							DBWriteContactSettingByte (NULL,MODULNAME, "AvatarBorders", PopUpOptions.avatarBorders);
							DBWriteContactSettingByte (NULL,MODULNAME, "AvatarPNGBorders", PopUpOptions.avatarPNGBorders);
							DBWriteContactSettingByte (NULL,MODULNAME, "AvatarRadius", PopUpOptions.avatarRadius);
							DBWriteContactSettingWord (NULL,MODULNAME, "AvatarSize", PopUpOptions.avatarSize);
							DBWriteContactSettingByte (NULL,MODULNAME, "EnableAvatarUpdates", PopUpOptions.EnableAvatarUpdates);
							//Monitor
							DBWriteContactSettingByte (NULL,MODULNAME, "Monitor", PopUpOptions.Monitor);
							//Transparency
							DBWriteContactSettingByte (NULL,MODULNAME, "EnableRegionTransparency", PopUpOptions.Enable9xTransparency);
							DBWriteContactSettingByte (NULL,MODULNAME, "UseTransparency", PopUpOptions.UseTransparency);
							DBWriteContactSettingByte (NULL,MODULNAME, "Alpha", PopUpOptions.Alpha);
							DBWriteContactSettingByte (NULL,MODULNAME, "OpaqueOnHover", PopUpOptions.OpaqueOnHover);

							//Effects
							DBWriteContactSettingByte (NULL,MODULNAME, "UseAnimations", PopUpOptions.UseAnimations);
							DBWriteContactSettingByte (NULL,MODULNAME, "Fade", PopUpOptions.UseEffect);
							DBWriteContactSettingTString(NULL, MODULNAME, "Effect", PopUpOptions.Effect);
							DBWriteContactSettingDword(NULL,MODULNAME, "FadeInTime", PopUpOptions.FadeIn);
							DBWriteContactSettingDword(NULL,MODULNAME, "FadeOutTime", PopUpOptions.FadeOut);
							//other old stuff
							DBWriteContactSettingWord (NULL,MODULNAME, "MaxPopups", (BYTE)PopUpOptions.MaxPopups);
							}
							return TRUE;
						default:
							break;
					}
					}
					break;
				default:
					break;
			}
			}//end WM_NOTIFY
			break;
		case WM_DESTROY:
			{
			bDlgInit = false;
			}//end WM_DESTROY
			break;
		default:
			return FALSE;
		}//end switch (msg)
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
				if (oldVal < 0) {
					SetWindowLongPtr(hwndBox, GWLP_USERDATA, 0);
				}
				RECT rc; GetWindowRect(hwnd, &rc);
				SetWindowPos(hwndBox, NULL,
					(rc.left+rc.right-newVal)/2, rc.bottom+2, newVal, newVal,
					SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);

				HRGN rgn = CreateRoundRectRgn(0, 0, newVal, newVal, 2 * PopUpOptions.avatarRadius, 2 * PopUpOptions.avatarRadius);
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
						(rc.left+rc.right-170)/2, rc.bottom+2, 170, 50,
						SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
					SetWindowRgn(hwndBox, NULL, TRUE);
				}
				SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetLayeredWindowAttributes(hwndBox, NULL, newVal, LWA_ALPHA);

				oldVal = newVal;
			}
			break;
		}
	case WM_MOUSELEAVE:
		SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwndBox, NULL, 255, LWA_ALPHA);

		ShowWindow(hwndBox, SW_HIDE);
		oldVal = -1;
		break;
	}
	return mir_callNextSubclass(hwnd, AlphaTrackBarWndProc, msg, wParam, lParam);
}

