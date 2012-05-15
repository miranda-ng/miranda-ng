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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/opt_gen.cpp $
Revision       : $Revision: 1628 $
Last change on : $Date: 2010-06-30 03:42:27 +0300 (Ð¡Ñ€, 30 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

INT_PTR CALLBACK PositionBoxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//Helper for Status Tree
static int CountStatusModes(DWORD flags)
{
	int res = 0;
	if (flags & PF2_ONLINE) ++res;
	if (flags & PF2_INVISIBLE) ++res;
	if (flags & PF2_SHORTAWAY) ++res;
	if (flags & PF2_LONGAWAY) ++res;
	if (flags & PF2_LIGHTDND) ++res;
	if (flags & PF2_HEAVYDND) ++res;
	if (flags & PF2_FREECHAT) ++res;
	if (flags & PF2_OUTTOLUNCH) ++res;
	if (flags & PF2_ONTHEPHONE) ++res;
	if (res) ++res; // Offline
	return res;
}

int AddStatusMode (OPTTREE_OPTION *options, int pos, LPTSTR prefix, DWORD flag)
{
	if (!flag) return pos;
	options[pos].dwFlag = flag;
	options[pos].groupId = OPTTREE_CHECK;
	options[pos].pszOptionName = (LPTSTR)mir_alloc(sizeof(TCHAR)*lstrlen(prefix)+32);
	options[pos].pszSettingName = mir_tstrdup(prefix);
	options[pos].iconIndex = 0;

	lstrcpy(options[pos].pszOptionName, prefix);
	switch (flag)
	{
		case PF2_IDLE:		lstrcat(options[pos].pszOptionName, _T("/Offline")); break;
		case PF2_ONLINE:	lstrcat(options[pos].pszOptionName, _T("/Online")); break;
		case PF2_INVISIBLE:	lstrcat(options[pos].pszOptionName, _T("/Invisible")); break;
		case PF2_SHORTAWAY:	lstrcat(options[pos].pszOptionName, _T("/Away")); break;
		case PF2_LONGAWAY:	lstrcat(options[pos].pszOptionName, _T("/NA")); break;
		case PF2_LIGHTDND:	lstrcat(options[pos].pszOptionName, _T("/Occupied")); break;
		case PF2_HEAVYDND:	lstrcat(options[pos].pszOptionName, _T("/DND")); break;
		case PF2_FREECHAT:	lstrcat(options[pos].pszOptionName, _T("/Free for chat")); break;
		case PF2_OUTTOLUNCH:lstrcat(options[pos].pszOptionName, _T("/Out to lunch")); break;
		case PF2_ONTHEPHONE:lstrcat(options[pos].pszOptionName, _T("/On the phone")); break;
	}
	return pos+1;
}

int AddStatusModes(OPTTREE_OPTION *options, int pos, LPTSTR prefix, DWORD flags)
{
	pos = AddStatusMode(options, pos, prefix, PF2_IDLE);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_ONLINE);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_INVISIBLE);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_SHORTAWAY);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_LONGAWAY);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_LIGHTDND);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_HEAVYDND);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_FREECHAT);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_OUTTOLUNCH);
	pos = AddStatusMode(options, pos, prefix, flags&PF2_ONTHEPHONE);
	return pos;
}


//Main Dialog Proc
void LoadOption_General() {
	//Seconds
	PopUpOptions.InfiniteDelay		= DBGetContactSettingByte(NULL, MODULNAME, "InfiniteDelay", FALSE);
	PopUpOptions.Seconds			= DBGetContactSettingRangedWord(NULL, MODULNAME, "Seconds",
									  SETTING_LIFETIME_DEFAULT, SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
	PopUpOptions.LeaveHovered		= DBGetContactSettingByte(NULL, MODULNAME, "LeaveHovered", TRUE);
	//Dynamic Resize
	PopUpOptions.DynamicResize		= DBGetContactSettingByte(NULL, MODULNAME, "DynamicResize", FALSE);
	PopUpOptions.UseMinimumWidth	= DBGetContactSettingByte(NULL, MODULNAME, "UseMinimumWidth", TRUE);
	PopUpOptions.MinimumWidth		= DBGetContactSettingWord(NULL, MODULNAME, "MinimumWidth", 160);
	PopUpOptions.UseMaximumWidth	= DBGetContactSettingByte(NULL, MODULNAME, "UseMaximumWidth", TRUE);
	PopUpOptions.MaximumWidth		= DBGetContactSettingWord(NULL, MODULNAME, "MaximumWidth", 300);
	//Position
	PopUpOptions.Position			= DBGetContactSettingRangedByte(NULL, MODULNAME, "Position",
									  POS_LOWERRIGHT, POS_MINVALUE, POS_MAXVALUE);
	//Configure popup area
	PopUpOptions.gapTop				= DBGetContactSettingWord(NULL, MODULNAME, "gapTop", 5);
	PopUpOptions.gapBottom			= DBGetContactSettingWord(NULL, MODULNAME, "gapBottom", 5);
	PopUpOptions.gapLeft			= DBGetContactSettingWord(NULL, MODULNAME, "gapLeft", 5);
	PopUpOptions.gapRight			= DBGetContactSettingWord(NULL, MODULNAME, "gapRight", 5);
	PopUpOptions.spacing			= DBGetContactSettingWord(NULL, MODULNAME, "spacing", 5);
	//Spreading
	PopUpOptions.Spreading			= DBGetContactSettingRangedByte(NULL, MODULNAME, "Spreading",
									  SPREADING_VERTICAL, SPREADING_MINVALUE, SPREADING_MAXVALUE);
	//miscellaneous
	PopUpOptions.ReorderPopUps			= DBGetContactSettingByte(NULL, MODULNAME, "ReorderPopUps", TRUE);
	PopUpOptions.ReorderPopUpsWarning	= DBGetContactSettingByte(NULL, MODULNAME, "ReorderPopUpsWarning", TRUE);
	//disable When
	PopUpOptions.ModuleIsEnabled		= DBGetContactSettingByte(NULL, MODULNAME, "ModuleIsEnabled", TRUE);
	PopUpOptions.DisableWhenFullscreen	= DBGetContactSettingByte(NULL, MODULNAME, "DisableWhenFullscreen", TRUE);
	//new status options (done inside WM_INITDIALOG)
	//Debug (done inside LoadOptions())
}

INT_PTR CALLBACK DlgProcPopUpGeneral(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static bool bDlgInit = false;	//some controls send WM_COMMAND before or during WM_INITDIALOG

	static OPTTREE_OPTION *statusOptions = NULL;
	static int statusOptionsCount = 0;
	if (statusOptions) {
		int index;
		if (OptTree_ProcessMessage(hwnd, msg, wParam, lParam, &index, IDC_STATUSES, statusOptions, statusOptionsCount))
			return TRUE;
	}

	switch (msg) {
		case WM_INITDIALOG: {
			HWND hCtrl = NULL;

			//Seconds of delay
			{
			CheckDlgButton(hwnd, IDC_INFINITEDELAY, PopUpOptions.InfiniteDelay);
			CheckDlgButton(hwnd, IDC_LEAVEHOVERED, PopUpOptions.LeaveHovered);
			EnableWindow(GetDlgItem(hwnd, IDC_SECONDS), !PopUpOptions.InfiniteDelay);
			EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC1), !PopUpOptions.InfiniteDelay);
			EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC2), !PopUpOptions.InfiniteDelay);
			EnableWindow(GetDlgItem(hwnd, IDC_LEAVEHOVERED), !PopUpOptions.InfiniteDelay);
			SetDlgItemInt(hwnd, IDC_SECONDS, PopUpOptions.Seconds, FALSE);
			SendDlgItemMessage(hwnd, IDC_SECONDS_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_LIFETIME_MAX, SETTING_LIFETIME_MIN));
			}

			//Dynamic Resize
			{
			CheckDlgButton(hwnd, IDC_DYNAMICRESIZE, PopUpOptions.DynamicResize);
			SetDlgItemText(hwnd, IDC_USEMAXIMUMWIDTH, PopUpOptions.DynamicResize ? _T("Maximum width"): _T("Width"));
			//Minimum Width
			CheckDlgButton(hwnd, IDC_USEMINIMUMWIDTH, PopUpOptions.UseMinimumWidth);
			SendDlgItemMessage(hwnd, IDC_MINIMUMWIDTH_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(
				SETTING_MAXIMUMWIDTH_MAX,
				SETTING_MINIMUMWIDTH_MIN) );
			SetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, PopUpOptions.MinimumWidth, FALSE);
			//Maximum Width
			PopUpOptions.UseMaximumWidth = PopUpOptions.DynamicResize ? PopUpOptions.UseMaximumWidth : TRUE;
			CheckDlgButton(hwnd, IDC_USEMAXIMUMWIDTH, PopUpOptions.UseMaximumWidth);
			SendDlgItemMessage(hwnd, IDC_MAXIMUMWIDTH_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(
				SETTING_MAXIMUMWIDTH_MAX,
				SETTING_MINIMUMWIDTH_MIN) );
			SetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, PopUpOptions.MaximumWidth, FALSE);
			//And finally let's enable/disable them.
			EnableWindow(GetDlgItem(hwnd, IDC_USEMINIMUMWIDTH),		PopUpOptions.DynamicResize);
			EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH),		PopUpOptions.DynamicResize && PopUpOptions.UseMinimumWidth);
			EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH_SPIN),	PopUpOptions.DynamicResize && PopUpOptions.UseMinimumWidth);
			EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH),		PopUpOptions.UseMaximumWidth);
			EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH_SPIN),	PopUpOptions.UseMaximumWidth);
			}
			//Position combobox.
			{
			hCtrl = GetDlgItem(hwnd, IDC_WHERE);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("upper left corner"))	,POS_UPPERLEFT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("lower left corner"))	,POS_LOWERLEFT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("lower right corner"))	,POS_LOWERRIGHT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("upper right corner"))	,POS_UPPERRIGHT);
			SendDlgItemMessage(hwnd, IDC_WHERE, CB_SETCURSEL, (PopUpOptions.Position) , 0);
			}
			//Configure popup area
			{
			hCtrl = GetDlgItem(hwnd, IDC_CUSTOMPOS);
			SendMessage(hCtrl, BUTTONSETASFLATBTN, 0, 0);
			SendMessage(hCtrl, BUTTONADDTOOLTIP, (WPARAM)_T("Popup Area"), BATF_TCHAR);
			SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(ICO_OPT_RESIZE,0));
			}
			//Spreading combobox
			{
			hCtrl = GetDlgItem(hwnd, IDC_LAYOUT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("horizontal"))	,SPREADING_HORIZONTAL);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("vertical"))	,SPREADING_VERTICAL);
			SendDlgItemMessage(hwnd, IDC_LAYOUT, CB_SETCURSEL, (PopUpOptions.Spreading), 0);
			}
			//miscellaneous
			{
			CheckDlgButton(hwnd, IDC_REORDERPOPUPS, PopUpOptions.ReorderPopUps);
			}
			//Popup enabled
			{
			CheckDlgButton(hwnd, IDC_POPUPENABLED,		PopUpOptions.ModuleIsEnabled?BST_UNCHECKED:BST_CHECKED);
			CheckDlgButton(hwnd, IDC_DISABLEINFS,			PopUpOptions.DisableWhenFullscreen);
			EnableWindow(GetDlgItem(hwnd, IDC_DISABLEINFS),	PopUpOptions.ModuleIsEnabled);
			EnableWindow(GetDlgItem(hwnd, IDC_STATUSES),	PopUpOptions.ModuleIsEnabled);
			}
			//new status options
			{
			int protocolCount = 0;
			int i;
			PROTOCOLDESCRIPTOR **protocols;
			CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&protocolCount, (LPARAM)&protocols);
			DWORD globalFlags = 0;
			for (i = 0; i < protocolCount; ++i) {
				if (protocols[i]->type != PROTOTYPE_PROTOCOL)
					continue;
				DWORD protoFlags = CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0);
				globalFlags |= protoFlags;
				statusOptionsCount += CountStatusModes(protoFlags);
			}
			statusOptionsCount += CountStatusModes(globalFlags);

			statusOptions = new OPTTREE_OPTION[statusOptionsCount];

			int pos = 0;
			pos = AddStatusModes(statusOptions, pos, _T("Global Status"), globalFlags);
			for (i = 0; i < protocolCount; ++i) {
				if (protocols[i]->type != PROTOTYPE_PROTOCOL)
					continue;
				DWORD protoFlags = CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0);
				if (!CountStatusModes(protoFlags))
					continue;
				TCHAR prefix[128];
				wsprintf(prefix, _T("Protocol Status/%hs"), protocols[i]->szName);
				pos = AddStatusModes(statusOptions, pos, prefix, protoFlags);
			}

			int index;
			OptTree_ProcessMessage(hwnd, msg, wParam, lParam, &index, IDC_STATUSES, statusOptions, statusOptionsCount);

			char prefix[128];
			LPTSTR pszSettingName = NULL;
			for (i = 0; i < protocolCount; ++i) {
				if (protocols[i]->type != PROTOTYPE_PROTOCOL)
					continue;
				DWORD protoFlags = CallProtoService(protocols[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0);
				if (!CountStatusModes(protoFlags))
					continue;

				mir_snprintf(prefix, sizeof(prefix), "Protocol Status/%s", protocols[i]->szName);
				pszSettingName = mir_a2t(prefix);
				OptTree_SetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount,
					DBGetContactSettingDword(NULL, MODULNAME, prefix, 0),
					pszSettingName);
				mir_free(pszSettingName); pszSettingName = NULL;
			}
			OptTree_SetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount,
				DBGetContactSettingDword(NULL, MODULNAME, "Global Status", 0),
				_T("Global Status"));
			}
			//Debug
			{
			#if defined(_DEBUG)
				CheckDlgButton(hwnd, IDC_DEBUG, PopUpOptions.debug);
				ShowWindow(GetDlgItem(hwnd, IDC_DEBUG), SW_SHOW);
			#endif
			}

			TranslateDialogDefault(hwnd);	//do it on end of WM_INITDIALOG
			bDlgInit = true;
			}//end WM_INITDIALOG
			return TRUE;
		case WM_COMMAND: {
				UINT idCtrl = LOWORD(wParam);
				switch (HIWORD(wParam)) {
					case BN_CLICKED:		//Button controls
						switch(idCtrl) {
							case IDC_INFINITEDELAY:
								{
								PopUpOptions.InfiniteDelay = !PopUpOptions.InfiniteDelay;
								EnableWindow(GetDlgItem(hwnd, IDC_SECONDS),!PopUpOptions.InfiniteDelay);
								EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC1),	!PopUpOptions.InfiniteDelay);
								EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC2),	!PopUpOptions.InfiniteDelay);
								EnableWindow(GetDlgItem(hwnd, IDC_LEAVEHOVERED),	!PopUpOptions.InfiniteDelay);
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_LEAVEHOVERED:
								{
								PopUpOptions.LeaveHovered = !PopUpOptions.LeaveHovered;
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_DYNAMICRESIZE:
								{
								PopUpOptions.DynamicResize=!PopUpOptions.DynamicResize;
								EnableWindow(GetDlgItem(hwnd, IDC_USEMINIMUMWIDTH),		PopUpOptions.DynamicResize);
								EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH),		PopUpOptions.DynamicResize && PopUpOptions.UseMinimumWidth);
								EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH_SPIN),	PopUpOptions.DynamicResize && PopUpOptions.UseMinimumWidth);
								SetDlgItemText(hwnd, IDC_USEMAXIMUMWIDTH, PopUpOptions.DynamicResize ? TranslateT("Maximum width"): TranslateT("Width"));
								if(!PopUpOptions.DynamicResize) {
									PopUpOptions.UseMaximumWidth = TRUE;
									CheckDlgButton(hwnd, IDC_USEMAXIMUMWIDTH, BST_CHECKED);
									EnableWindow(GetDlgItem(hwnd, IDC_USEMAXIMUMWIDTH),		TRUE);
									EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH),		TRUE);
									EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH_SPIN),	TRUE);
								}
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_USEMINIMUMWIDTH:
								{
								PopUpOptions.UseMinimumWidth= !PopUpOptions.UseMinimumWidth;
								EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH),		PopUpOptions.UseMinimumWidth);
								EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH_SPIN),	PopUpOptions.UseMinimumWidth);
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_USEMAXIMUMWIDTH:
								{
								PopUpOptions.UseMaximumWidth= Button_GetCheck((HWND)lParam);
								if(!PopUpOptions.DynamicResize) { //ugly - set always on if DynamicResize = off
									CheckDlgButton(hwnd, idCtrl, BST_CHECKED);
									PopUpOptions.UseMaximumWidth = TRUE;
								}
								EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH), PopUpOptions.UseMaximumWidth);
								EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH_SPIN),	PopUpOptions.UseMaximumWidth);
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_CUSTOMPOS:
								{
								RECT rcButton, rcBox;
								HWND hwndBox = CreateDialog(hInst, MAKEINTRESOURCE(IDD_POSITION), NULL, PositionBoxDlgProc);
								GetWindowRect((HWND)lParam, &rcButton);
								GetWindowRect(hwndBox, &rcBox);
								MoveWindow(hwndBox,
									rcButton.right-(rcBox.right-rcBox.left) + 15,
									rcButton.bottom + 3,
									rcBox.right-rcBox.left,
									rcBox.bottom-rcBox.top,
									FALSE);
#if defined(_UNICODE)
								SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE)|WS_EX_LAYERED);
								SetLayeredWindowAttributes(hwndBox, NULL, 0, LWA_ALPHA);
								ShowWindow(hwndBox, SW_SHOW);
								for (int i = 0; i <= 255; i += 15) {
									SetLayeredWindowAttributes(hwndBox, NULL, i, LWA_ALPHA);
									UpdateWindow(hwndBox);
									Sleep(1);
								}
								SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE)&~WS_EX_LAYERED);
#else
								if (MySetLayeredWindowAttributes) {
									SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE)|WS_EX_LAYERED);
									MySetLayeredWindowAttributes(hwndBox, NULL, 0, LWA_ALPHA);
									ShowWindow(hwndBox, SW_SHOW);
									for (int i = 0; i <= 255; i += 15) {
										MySetLayeredWindowAttributes(hwndBox, NULL, i, LWA_ALPHA);
										UpdateWindow(hwndBox);
										Sleep(1);
									}
									SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE)&~WS_EX_LAYERED);
								}
#endif
								ShowWindow(hwndBox, SW_SHOW);
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_REORDERPOPUPS:
								{
								PopUpOptions.ReorderPopUps = !PopUpOptions.ReorderPopUps;
								PopUpOptions.ReorderPopUpsWarning = PopUpOptions.ReorderPopUps ? 
									DBGetContactSettingByte(NULL, MODULNAME, "ReorderPopUpsWarning", TRUE) : TRUE;
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							case IDC_POPUPENABLED:
								{//PopUpOptions.ModuleIsEnabled is set and store by EnableDisable menu service !!!!!!
								int chk = IsDlgButtonChecked(hwnd, IDC_POPUPENABLED);
								if (PopUpOptions.ModuleIsEnabled&&chk || !PopUpOptions.ModuleIsEnabled&&!chk)
									svcEnableDisableMenuCommand(0,0);
								EnableWindow(GetDlgItem(hwnd, IDC_STATUSES),	PopUpOptions.ModuleIsEnabled);
								EnableWindow(GetDlgItem(hwnd, IDC_DISABLEINFS),	PopUpOptions.ModuleIsEnabled);
								}
								break;
							case IDC_DISABLEINFS:
								{
								PopUpOptions.DisableWhenFullscreen = !PopUpOptions.DisableWhenFullscreen;
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								}
								break;
							#if defined(_DEBUG)		//DEBUG
							case IDC_DEBUG:
								PopUpOptions.debug = (BYTE)Button_GetCheck((HWND)lParam);
								//The following line is dangerous, but useful for my debug purposes.
								//MySetLayeredWindowAttributes = (BOOL)!MySetLayeredWindowAttributes;
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								break;
							#endif					//DEBUG
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
							case IDC_WHERE:
								PopUpOptions.Position = ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								break;
							case IDC_LAYOUT:
								PopUpOptions.Spreading = ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
								SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
								break;
							default:
								break;
						}
						break;
					case EN_CHANGE:			//Edit controls change
						if(!bDlgInit) break;
						switch(idCtrl) {
							//lParam = Handle to the control
							case IDC_SECONDS:
								{
									int seconds = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (	seconds >= SETTING_LIFETIME_MIN && 
											seconds <= SETTING_LIFETIME_MAX &&
											seconds != PopUpOptions.Seconds) {
										PopUpOptions.Seconds = seconds;
										SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
									}
								}
								break;
							case IDC_MINIMUMWIDTH:
								{
									int temp = GetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, NULL, FALSE);
									if (	temp >= SETTING_MINIMUMWIDTH_MIN && 
											temp <= SETTING_MAXIMUMWIDTH_MAX &&
											temp != PopUpOptions.MinimumWidth) {
										PopUpOptions.MinimumWidth = temp;
										SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
									}
								}
								break;
							case IDC_MAXIMUMWIDTH:
								{
									int temp = GetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, NULL, FALSE);
									if (	temp >= SETTING_MINIMUMWIDTH_MIN && 
											temp <= SETTING_MAXIMUMWIDTH_MAX &&
											temp != PopUpOptions.MaximumWidth) {
										PopUpOptions.MaximumWidth = temp;
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
							case IDC_SECONDS:
								{
									int seconds = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (seconds > SETTING_LIFETIME_MAX)
										PopUpOptions.Seconds = SETTING_LIFETIME_MAX;
									else if (seconds < SETTING_LIFETIME_MIN)
										PopUpOptions.Seconds = SETTING_LIFETIME_MIN;
									if(seconds != PopUpOptions.Seconds) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.Seconds, FALSE);
										ErrorMSG(SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
										SetFocus((HWND)lParam);
									}
								}
								break;
							case IDC_MINIMUMWIDTH:
								{
									int temp = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (temp < SETTING_MINIMUMWIDTH_MIN)
										PopUpOptions.MinimumWidth = SETTING_MINIMUMWIDTH_MIN;
									else if(temp > SETTING_MAXIMUMWIDTH_MAX)
										PopUpOptions.MinimumWidth = SETTING_MAXIMUMWIDTH_MAX;
									if(temp != PopUpOptions.MinimumWidth) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.MinimumWidth, FALSE);
										ErrorMSG(SETTING_MINIMUMWIDTH_MIN, SETTING_MAXIMUMWIDTH_MAX);
										SetFocus((HWND)lParam);
										break;
									}
									if (temp > PopUpOptions.MaximumWidth) {
										PopUpOptions.MaximumWidth = min(temp, SETTING_MAXIMUMWIDTH_MAX);
										SetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, PopUpOptions.MaximumWidth, FALSE);
									}
								}
								break;
							case IDC_MAXIMUMWIDTH:
								{
									int temp = GetDlgItemInt(hwnd, idCtrl, NULL, FALSE);
									if (temp >= SETTING_MAXIMUMWIDTH_MAX)
										PopUpOptions.MaximumWidth = SETTING_MAXIMUMWIDTH_MAX;
									else if(temp < SETTING_MINIMUMWIDTH_MIN)
										PopUpOptions.MaximumWidth = SETTING_MINIMUMWIDTH_MIN;
									if(temp != PopUpOptions.MaximumWidth) {
										SetDlgItemInt(hwnd, idCtrl, PopUpOptions.MaximumWidth, FALSE);
										ErrorMSG(SETTING_MINIMUMWIDTH_MIN, SETTING_MAXIMUMWIDTH_MAX);
										SetFocus((HWND)lParam);
										break;
									}
									if (temp < PopUpOptions.MinimumWidth) {
										PopUpOptions.MinimumWidth = max(temp, SETTING_MINIMUMWIDTH_MIN);
										SetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, PopUpOptions.MinimumWidth, FALSE);
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
							LoadOption_General();
							return TRUE;
						case PSN_APPLY: {
							//Seconds
							DBWriteContactSettingByte(NULL, MODULNAME, "InfiniteDelay", PopUpOptions.InfiniteDelay);
							DBWriteContactSettingWord(NULL, MODULNAME, "Seconds", (WORD)PopUpOptions.Seconds);
							DBWriteContactSettingByte(NULL, MODULNAME, "LeaveHovered", PopUpOptions.LeaveHovered);
							//Dynamic Resize
							DBWriteContactSettingByte(NULL, MODULNAME, "DynamicResize", PopUpOptions.DynamicResize);
							DBWriteContactSettingByte(NULL, MODULNAME, "UseMinimumWidth", PopUpOptions.UseMinimumWidth);
							DBWriteContactSettingWord(NULL, MODULNAME, "MinimumWidth", PopUpOptions.MinimumWidth);
							DBWriteContactSettingByte(NULL, MODULNAME, "UseMaximumWidth", PopUpOptions.UseMaximumWidth);
							DBWriteContactSettingWord(NULL, MODULNAME, "MaximumWidth", PopUpOptions.MaximumWidth);
							//Position
							DBWriteContactSettingByte(NULL, MODULNAME, "Position", (BYTE)PopUpOptions.Position);
							//Configure popup area
							DBWriteContactSettingWord(NULL, MODULNAME, "gapTop",	(WORD)PopUpOptions.gapTop);
							DBWriteContactSettingWord(NULL, MODULNAME, "gapBottom",	(WORD)PopUpOptions.gapBottom);
							DBWriteContactSettingWord(NULL, MODULNAME, "gapLeft",	(WORD)PopUpOptions.gapLeft);
							DBWriteContactSettingWord(NULL, MODULNAME, "gapRight",	(WORD)PopUpOptions.gapRight);
							DBWriteContactSettingWord(NULL, MODULNAME, "spacing",	(WORD)PopUpOptions.spacing);
							//Spreading
							DBWriteContactSettingByte(NULL, MODULNAME, "Spreading", (BYTE)PopUpOptions.Spreading);
							//miscellaneous
							//DBWriteContactSettingByte(NULL, MODULNAME, "ReorderPopUps", PopUpOptions.ReorderPopUps);
							Check_ReorderPopUps(hwnd);	//this save also PopUpOptions.ReorderPopUps
							//disable When
							DBWriteContactSettingByte(NULL, MODULNAME, "DisableWhenFullscreen", PopUpOptions.DisableWhenFullscreen);
							//new status options
							{
								int protocolCount;
								PROTOCOLDESCRIPTOR **protocols;
								CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&protocolCount, (LPARAM)&protocols);

								char prefix[128];
								LPTSTR pszSettingName = NULL;
								for (int i = 0; i < protocolCount; ++i)
								{
									if (protocols[i]->type != PROTOTYPE_PROTOCOL)
										continue;

									mir_snprintf(prefix, sizeof(prefix), "Protocol Status/%s", protocols[i]->szName);
									pszSettingName = mir_a2t(prefix);
									DBWriteContactSettingDword(NULL, MODULNAME, prefix,
										OptTree_GetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount, pszSettingName));
									mir_free(pszSettingName); pszSettingName = NULL;
								}
								DBWriteContactSettingDword(NULL, MODULNAME, "Global Status",
									OptTree_GetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount, _T("Global Status")));
							}
							//Debug
							#if defined(_DEBUG)
								DBWriteContactSettingByte(NULL, MODULNAME, "debug", PopUpOptions.debug);
							#endif

							}//end PSN_APPLY:
							return TRUE;
						default:
							break;
					}
					}
					break;
				case IDC_MINIMUMWIDTH_SPIN:
					{
						LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;
						int temp = lpnmud->iPos + lpnmud->iDelta;
						if (temp > PopUpOptions.MaximumWidth) {
							PopUpOptions.MaximumWidth = min(temp, SETTING_MAXIMUMWIDTH_MAX);
							SetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, PopUpOptions.MaximumWidth, FALSE);
						}
					}
					break;
				case IDC_MAXIMUMWIDTH_SPIN:
					{
						LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;
						int temp = lpnmud->iPos + lpnmud->iDelta;
						if (temp < PopUpOptions.MinimumWidth) {
							PopUpOptions.MinimumWidth = max(temp, SETTING_MINIMUMWIDTH_MIN);
							SetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, PopUpOptions.MinimumWidth, FALSE);
						}
					}
					break;
				default:
					break;
			}
			}//end WM_NOTIFY
			break;
		case WM_DESTROY: {
			if (statusOptions) {
				for (int i = 0; i < statusOptionsCount; ++i) {
					mir_free(statusOptions[i].pszOptionName);
					mir_free(statusOptions[i].pszSettingName);
				}
				delete [] statusOptions;
				statusOptions = NULL;
				statusOptionsCount = 0;
				bDlgInit = false;
			}
			}//end WM_DESTROY
			break;
		default:
			return FALSE;
		}//end switch (msg)
	return FALSE;
}

void ErrorMSG(int minValue, int maxValue) {
		TCHAR str[128];
		wsprintf(str, TranslateT("You cannot specify a value lower than %d and higher than %d."), minValue, maxValue);
		MSGERROR(str);
}

void Check_ReorderPopUps(HWND hwnd) {
	if (!PopUpOptions.ReorderPopUps && PopUpOptions.ReorderPopUpsWarning)
	{
		int res = MessageBox(hwnd,
			TranslateTS(
				_T("'Reorder Popups' option is currently diabled.\r\n")
				_T("This may cause misaligned popups when used with\r\n")
				_T("avatars and text replacement (mainly NewStatusNotify).\r\n")
				_T("\r\n")
				_T("Do you want to enable popup reordering now?\r\n")
				),
			TranslateT("Popup Plus Warning"), MB_ICONEXCLAMATION|MB_YESNOCANCEL);

		switch (res)
		{
			case IDYES:
				PopUpOptions.ReorderPopUps = TRUE;
				//Reset warning for next option change !!!
				PopUpOptions.ReorderPopUpsWarning = TRUE;
				break;
			case IDNO:
				PopUpOptions.ReorderPopUps = FALSE;
				PopUpOptions.ReorderPopUpsWarning = FALSE;
				break;
			default:
				return;
		}
	}
	DBWriteContactSettingByte(NULL, MODULNAME, "ReorderPopUps", PopUpOptions.ReorderPopUps);
	DBWriteContactSettingByte(NULL, MODULNAME, "ReorderPopUpsWarning", PopUpOptions.ReorderPopUpsWarning);
	if(hwnd) CheckDlgButton(hwnd, IDC_REORDERPOPUPS, PopUpOptions.ReorderPopUps);
}

void ThemeDialogBackground(HWND hwnd) {
	if (IsWinVerXPPlus()) {
		static HMODULE hThemeAPI = NULL;
		if (!hThemeAPI) hThemeAPI = GetModuleHandleA("uxtheme");
		if (hThemeAPI) {
			HRESULT (STDAPICALLTYPE *MyEnableThemeDialogTexture)(HWND,DWORD) = (HRESULT (STDAPICALLTYPE*)(HWND,DWORD))GetProcAddress(hThemeAPI,"EnableThemeDialogTexture");
			if (MyEnableThemeDialogTexture)
				MyEnableThemeDialogTexture(hwnd,0x00000002|0x00000004); //0x00000002|0x00000004=ETDT_ENABLETAB
		}
	}
}

INT_PTR CALLBACK PositionBoxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hFontTitle = 0;
	switch (msg)
	{
	case WM_INITDIALOG:
		ThemeDialogBackground(hwndDlg);

		LOGFONT lf;
		GetObject((HFONT)SendMessage(GetDlgItem(hwndDlg, IDC_TITLE), WM_GETFONT, 0, 0), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfHeight *= 1.2;
		hFontTitle = CreateFontIndirect(&lf);
		SendMessage(GetDlgItem(hwndDlg, IDC_TITLE), WM_SETFONT, (WPARAM)hFontTitle, TRUE);

		SendMessage(GetDlgItem(hwndDlg, IDOK), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg, IDOK), BUTTONADDTOOLTIP, (WPARAM)_T("OK"), BATF_TCHAR);
		SendMessage(GetDlgItem(hwndDlg, IDOK), BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(ICO_OPT_OK,0));

		SendMessage(GetDlgItem(hwndDlg, IDCANCEL), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg, IDCANCEL), BUTTONADDTOOLTIP, (WPARAM)_T("Cancel"), BATF_TCHAR);
		SendMessage(GetDlgItem(hwndDlg, IDCANCEL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(ICO_OPT_CANCEL,0));

		SetDlgItemInt(hwndDlg, IDC_TXT_TOP, PopUpOptions.gapTop, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_BOTTOM, PopUpOptions.gapBottom, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_LEFT, PopUpOptions.gapLeft, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_RIGHT, PopUpOptions.gapRight, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_SPACING, PopUpOptions.spacing, FALSE);

		TranslateDialogDefault(hwndDlg);
		break;
	case WM_COMMAND:
		if ((LOWORD(wParam)) == IDOK)
		{
			PopUpOptions.gapTop = GetDlgItemInt(hwndDlg, IDC_TXT_TOP, NULL, FALSE);
			PopUpOptions.gapBottom = GetDlgItemInt(hwndDlg, IDC_TXT_BOTTOM, NULL, FALSE);
			PopUpOptions.gapLeft = GetDlgItemInt(hwndDlg, IDC_TXT_LEFT, NULL, FALSE);
			PopUpOptions.gapRight = GetDlgItemInt(hwndDlg, IDC_TXT_RIGHT, NULL, FALSE);
			PopUpOptions.spacing = GetDlgItemInt(hwndDlg, IDC_TXT_SPACING, NULL, FALSE);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		} else
		if ((LOWORD(wParam)) == IDCANCEL)
		{
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
		{
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	case WM_DESTROY:
		DeleteObject(hFontTitle);
		break;
	}
	return FALSE;
}
