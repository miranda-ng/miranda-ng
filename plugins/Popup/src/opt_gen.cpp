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

#include "stdafx.h"

INT_PTR CALLBACK PositionBoxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper for Status Tree
static int CountStatusModes(uint32_t flags)
{
	int res = 0;
	if (flags & PF2_ONLINE) ++res;
	if (flags & PF2_INVISIBLE) ++res;
	if (flags & PF2_SHORTAWAY) ++res;
	if (flags & PF2_LONGAWAY) ++res;
	if (flags & PF2_LIGHTDND) ++res;
	if (flags & PF2_HEAVYDND) ++res;
	if (flags & PF2_FREECHAT) ++res;
	++res; //  Offline
	return res;
}

int AddStatusMode(OPTTREE_OPTION *options, int pos, LPTSTR prefix, uint32_t flag)
{
	if (!flag) return pos;
	options[pos].dwFlag = flag;
	options[pos].groupId = OPTTREE_CHECK;
	options[pos].pszOptionName = (LPTSTR)mir_alloc(sizeof(wchar_t) * mir_wstrlen(prefix) + 32);
	options[pos].pszSettingName = mir_wstrdup(prefix);
	options[pos].iconIndex = 0;

	mir_wstrcpy(options[pos].pszOptionName, prefix);
	mir_wstrcat(options[pos].pszOptionName, L"/");
	switch (flag)
	{
	case PF2_IDLE: mir_wstrcat(options[pos].pszOptionName, LPGENW("Offline")); break;
	case PF2_ONLINE: mir_wstrcat(options[pos].pszOptionName, LPGENW("Online")); break;
	case PF2_INVISIBLE: mir_wstrcat(options[pos].pszOptionName, LPGENW("Invisible")); break;
	case PF2_SHORTAWAY: mir_wstrcat(options[pos].pszOptionName, LPGENW("Away")); break;
	case PF2_LONGAWAY: mir_wstrcat(options[pos].pszOptionName, LPGENW("Not available")); break;
	case PF2_LIGHTDND: mir_wstrcat(options[pos].pszOptionName, LPGENW("Occupied")); break;
	case PF2_HEAVYDND: mir_wstrcat(options[pos].pszOptionName, LPGENW("Do not disturb")); break;
	case PF2_FREECHAT: mir_wstrcat(options[pos].pszOptionName, LPGENW("Free for chat")); break;
	}
	return pos + 1;
}

int AddStatusModes(OPTTREE_OPTION *options, int pos, LPTSTR prefix, uint32_t flags)
{
	pos = AddStatusMode(options, pos, prefix, PF2_IDLE);
	pos = AddStatusMode(options, pos, prefix, flags & PF2_ONLINE);
	pos = AddStatusMode(options, pos, prefix, flags & PF2_INVISIBLE);
	pos = AddStatusMode(options, pos, prefix, flags & PF2_SHORTAWAY);
	pos = AddStatusMode(options, pos, prefix, flags & PF2_LONGAWAY);
	pos = AddStatusMode(options, pos, prefix, flags & PF2_LIGHTDND);
	pos = AddStatusMode(options, pos, prefix, flags & PF2_HEAVYDND);
	pos = AddStatusMode(options, pos, prefix, flags & PF2_FREECHAT);
	return pos;
}


// Main Dialog Proc
void LoadOption_General()
{
	// Seconds
	PopupOptions.InfiniteDelay = g_plugin.getByte("InfiniteDelay", FALSE);
	PopupOptions.Seconds =
		DBGetContactSettingRangedWord(0, MODULENAME, "Seconds", SETTING_LIFETIME_DEFAULT, SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
	PopupOptions.LeaveHovered = g_plugin.getByte("LeaveHovered", TRUE);

	// Dynamic Resize
	PopupOptions.DynamicResize = g_plugin.getByte("DynamicResize", FALSE);
	PopupOptions.UseMinimumWidth = g_plugin.getByte("UseMinimumWidth", TRUE);
	PopupOptions.MinimumWidth = g_plugin.getWord("MinimumWidth", 160);
	PopupOptions.UseMaximumWidth = g_plugin.getByte("UseMaximumWidth", TRUE);
	PopupOptions.MaximumWidth = g_plugin.getWord("MaximumWidth", 300);

	// Position
	PopupOptions.Position =
		DBGetContactSettingRangedByte(0, MODULENAME, "Position", POS_LOWERRIGHT, POS_MINVALUE, POS_MAXVALUE);

	// Configure popup area
	PopupOptions.gapTop = g_plugin.getWord("gapTop", 5);
	PopupOptions.gapBottom = g_plugin.getWord("gapBottom", 5);
	PopupOptions.gapLeft = g_plugin.getWord("gapLeft", 5);
	PopupOptions.gapRight = g_plugin.getWord("gapRight", 5);
	PopupOptions.spacing = g_plugin.getWord("spacing", 5);

	// Spreading
	PopupOptions.Spreading =
		DBGetContactSettingRangedByte(0, MODULENAME, "Spreading", SPREADING_VERTICAL, SPREADING_MINVALUE, SPREADING_MAXVALUE);

	// miscellaneous
	PopupOptions.ReorderPopups = g_plugin.getByte("ReorderPopups", TRUE);
	PopupOptions.ReorderPopupsWarning = g_plugin.getByte("ReorderPopupsWarning", TRUE);

	// disable When
	PopupOptions.DisableWhenFullscreen = g_plugin.getByte("DisableWhenFullscreen", TRUE);
}

INT_PTR CALLBACK DlgProcPopupGeneral(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bDlgInit = false;	// some controls send WM_COMMAND before or during WM_INITDIALOG

	static OPTTREE_OPTION *statusOptions = nullptr;
	static int statusOptionsCount = 0;
	if (statusOptions) {
		int index;
		if (OptTree_ProcessMessage(hwnd, msg, wParam, lParam, &index, IDC_STATUSES, statusOptions, statusOptionsCount))
			return TRUE;
	}

	switch (msg) {
	case WM_INITDIALOG:
		// Seconds of delay
		CheckDlgButton(hwnd, IDC_INFINITEDELAY, PopupOptions.InfiniteDelay ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_LEAVEHOVERED, PopupOptions.LeaveHovered ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwnd, IDC_SECONDS), !PopupOptions.InfiniteDelay);
		EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC1), !PopupOptions.InfiniteDelay);
		EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC2), !PopupOptions.InfiniteDelay);
		EnableWindow(GetDlgItem(hwnd, IDC_LEAVEHOVERED), !PopupOptions.InfiniteDelay);
		SetDlgItemInt(hwnd, IDC_SECONDS, PopupOptions.Seconds, FALSE);
		SendDlgItemMessage(hwnd, IDC_SECONDS_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_LIFETIME_MAX, SETTING_LIFETIME_MIN));

		// Dynamic Resize
		CheckDlgButton(hwnd, IDC_DYNAMICRESIZE, PopupOptions.DynamicResize ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hwnd, IDC_USEMAXIMUMWIDTH, PopupOptions.DynamicResize ? LPGENW("Maximum width") : LPGENW("Width"));
		// Minimum Width
		CheckDlgButton(hwnd, IDC_USEMINIMUMWIDTH, PopupOptions.UseMinimumWidth ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwnd, IDC_MINIMUMWIDTH_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_MAXIMUMWIDTH_MAX, SETTING_MINIMUMWIDTH_MIN));
		SetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, PopupOptions.MinimumWidth, FALSE);
		// Maximum Width
		PopupOptions.UseMaximumWidth = PopupOptions.DynamicResize ? PopupOptions.UseMaximumWidth : TRUE;
		CheckDlgButton(hwnd, IDC_USEMAXIMUMWIDTH, PopupOptions.UseMaximumWidth ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwnd, IDC_MAXIMUMWIDTH_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_MAXIMUMWIDTH_MAX, SETTING_MINIMUMWIDTH_MIN));
		SetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, PopupOptions.MaximumWidth, FALSE);
		// And finally let's enable/disable them.
		EnableWindow(GetDlgItem(hwnd, IDC_USEMINIMUMWIDTH), PopupOptions.DynamicResize);
		EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH), PopupOptions.DynamicResize && PopupOptions.UseMinimumWidth);
		EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH_SPIN), PopupOptions.DynamicResize && PopupOptions.UseMinimumWidth);
		EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH), PopupOptions.UseMaximumWidth);
		EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH_SPIN), PopupOptions.UseMaximumWidth);

		// Position combobox.
		{
			HWND hCtrl = GetDlgItem(hwnd, IDC_WHERE);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upper left corner")), POS_UPPERLEFT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Lower left corner")), POS_LOWERLEFT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Lower right corner")), POS_LOWERRIGHT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upper right corner")), POS_UPPERRIGHT);
			SendDlgItemMessage(hwnd, IDC_WHERE, CB_SETCURSEL, PopupOptions.Position, 0);
		}
		// Configure popup area
		{
			HWND hCtrl = GetDlgItem(hwnd, IDC_CUSTOMPOS);
			SendMessage(hCtrl, BUTTONSETASFLATBTN, TRUE, 0);
			SendMessage(hCtrl, BUTTONADDTOOLTIP, (WPARAM)L"Popup area", BATF_UNICODE);
			SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_RESIZE));
		}
		// Spreading combobox
		{
			HWND hCtrl = GetDlgItem(hwnd, IDC_LAYOUT);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Horizontal")), SPREADING_HORIZONTAL);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Vertical")), SPREADING_VERTICAL);
			SendDlgItemMessage(hwnd, IDC_LAYOUT, CB_SETCURSEL, PopupOptions.Spreading, 0);
		}
		// miscellaneous
		CheckDlgButton(hwnd, IDC_REORDERPOPUPS, PopupOptions.ReorderPopups ? BST_CHECKED : BST_UNCHECKED);

		// Popup enabled
		{
			bool bEnabled = Popup_Enabled();
			CheckDlgButton(hwnd, IDC_POPUPENABLED, bEnabled ? BST_UNCHECKED : BST_CHECKED);
			CheckDlgButton(hwnd, IDC_DISABLEINFS, PopupOptions.DisableWhenFullscreen ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwnd, IDC_DISABLEINFS), bEnabled);
			EnableWindow(GetDlgItem(hwnd, IDC_STATUSES), bEnabled);
		}

		// new status options
		{
			statusOptionsCount = 0;
			uint32_t globalFlags = 0;
			auto &accs = Accounts();
			for (auto &pa : accs) {
				if (!pa->bIsVirtual) {
					uint32_t protoFlags = CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
					globalFlags |= protoFlags;
					statusOptionsCount += CountStatusModes(protoFlags);
				}
			}
			statusOptionsCount += CountStatusModes(globalFlags);

			statusOptions = new OPTTREE_OPTION[statusOptionsCount];

			int pos = AddStatusModes(statusOptions, 0, LPGENW("Global Status"), globalFlags);
			for (auto &pa : accs) {
				if (!pa->bIsVirtual) {
					uint32_t protoFlags = CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
					if (!CountStatusModes(protoFlags))
						continue;

					wchar_t prefix[128];
					mir_snwprintf(prefix, LPGENW("Protocol Status")L"/%s", pa->tszAccountName);
					pos = AddStatusModes(statusOptions, pos, prefix, protoFlags);
				}
			}

			int index;
			OptTree_ProcessMessage(hwnd, msg, wParam, lParam, &index, IDC_STATUSES, statusOptions, statusOptionsCount);

			for (auto &pa : accs) {
				if (!pa->bIsVirtual) {
					uint32_t protoFlags = CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
					if (!CountStatusModes(protoFlags))
						continue;

					char prefix[128];
					mir_snprintf(prefix, "Protocol Status/%s", pa->szModuleName);

					wchar_t pszSettingName[256];
					mir_snwprintf(pszSettingName, LPGENW("Protocol Status")L"/%s", pa->tszAccountName);
					OptTree_SetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount, g_plugin.getDword(prefix, 0), pszSettingName);
				}
			}
			OptTree_SetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount, g_plugin.getDword("Global Status", 0), LPGENW("Global Status"));
		}

		TranslateDialogDefault(hwnd);	// do it on end of WM_INITDIALOG
		bDlgInit = true;
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:	// Button controls
			switch (LOWORD(wParam)) {
			case IDC_INFINITEDELAY:
				PopupOptions.InfiniteDelay = !PopupOptions.InfiniteDelay;
				EnableWindow(GetDlgItem(hwnd, IDC_SECONDS), !PopupOptions.InfiniteDelay);
				EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC1), !PopupOptions.InfiniteDelay);
				EnableWindow(GetDlgItem(hwnd, IDC_SECONDS_STATIC2), !PopupOptions.InfiniteDelay);
				EnableWindow(GetDlgItem(hwnd, IDC_LEAVEHOVERED), !PopupOptions.InfiniteDelay);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_LEAVEHOVERED:
				PopupOptions.LeaveHovered = !PopupOptions.LeaveHovered;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_DYNAMICRESIZE:
				PopupOptions.DynamicResize = !PopupOptions.DynamicResize;
				EnableWindow(GetDlgItem(hwnd, IDC_USEMINIMUMWIDTH), PopupOptions.DynamicResize);
				EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH), PopupOptions.DynamicResize && PopupOptions.UseMinimumWidth);
				EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH_SPIN), PopupOptions.DynamicResize && PopupOptions.UseMinimumWidth);
				SetDlgItemText(hwnd, IDC_USEMAXIMUMWIDTH, PopupOptions.DynamicResize ? TranslateT("Maximum width") : TranslateT("Width"));
				if (!PopupOptions.DynamicResize) {
					PopupOptions.UseMaximumWidth = TRUE;
					CheckDlgButton(hwnd, IDC_USEMAXIMUMWIDTH, BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd, IDC_USEMAXIMUMWIDTH), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH_SPIN), TRUE);
				}
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_USEMINIMUMWIDTH:
				PopupOptions.UseMinimumWidth = !PopupOptions.UseMinimumWidth;
				EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH), PopupOptions.UseMinimumWidth);
				EnableWindow(GetDlgItem(hwnd, IDC_MINIMUMWIDTH_SPIN), PopupOptions.UseMinimumWidth);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_USEMAXIMUMWIDTH:
				PopupOptions.UseMaximumWidth = Button_GetCheck((HWND)lParam);
				if (!PopupOptions.DynamicResize) { // ugly - set always on if DynamicResize = off
					CheckDlgButton(hwnd, LOWORD(wParam), BST_CHECKED);
					PopupOptions.UseMaximumWidth = TRUE;
				}
				EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH), PopupOptions.UseMaximumWidth);
				EnableWindow(GetDlgItem(hwnd, IDC_MAXIMUMWIDTH_SPIN), PopupOptions.UseMaximumWidth);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_CUSTOMPOS:
				{
					RECT rcButton, rcBox;
					HWND hwndBox = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_POSITION), nullptr, PositionBoxDlgProc);
					GetWindowRect((HWND)lParam, &rcButton);
					GetWindowRect(hwndBox, &rcBox);
					MoveWindow(hwndBox,
						rcButton.right - (rcBox.right - rcBox.left) + 15,
						rcButton.bottom + 3,
						rcBox.right - rcBox.left,
						rcBox.bottom - rcBox.top,
						FALSE);

					SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE) | WS_EX_LAYERED);
					SetLayeredWindowAttributes(hwndBox, NULL, 0, LWA_ALPHA);
					ShowWindow(hwndBox, SW_SHOW);
					for (int i = 0; i <= 255; i += 15) {
						SetLayeredWindowAttributes(hwndBox, NULL, i, LWA_ALPHA);
						UpdateWindow(hwndBox);
						Sleep(1);
					}
					SetWindowLongPtr(hwndBox, GWL_EXSTYLE, GetWindowLongPtr(hwndBox, GWL_EXSTYLE) & ~WS_EX_LAYERED);

					ShowWindow(hwndBox, SW_SHOW);
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_REORDERPOPUPS:
				PopupOptions.ReorderPopups = !PopupOptions.ReorderPopups;
				PopupOptions.ReorderPopupsWarning = PopupOptions.ReorderPopups ? g_plugin.getByte("ReorderPopupsWarning", TRUE) : TRUE;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_POPUPENABLED:
				{
					bool bEnabled = Popup_Enabled();
					int chk = IsDlgButtonChecked(hwnd, IDC_POPUPENABLED);
					if (bEnabled && chk || !bEnabled && !chk)
						svcEnableDisableMenuCommand(0, 0);
					EnableWindow(GetDlgItem(hwnd, IDC_STATUSES), !bEnabled);
					EnableWindow(GetDlgItem(hwnd, IDC_DISABLEINFS), !bEnabled);
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_DISABLEINFS:
				PopupOptions.DisableWhenFullscreen = !PopupOptions.DisableWhenFullscreen;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_PREVIEW:
				PopupPreview();
				break;
			}
			break;

		case CBN_SELCHANGE:		// ComboBox controls
			switch (LOWORD(wParam)) { // lParam = Handle to the control
			case IDC_WHERE:
				PopupOptions.Position = ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;
			case IDC_LAYOUT:
				PopupOptions.Spreading = ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;
			}
			break;

		case EN_CHANGE:			// Edit controls change
			if (!bDlgInit) break;
			switch (LOWORD(wParam)) { // lParam = Handle to the control
			case IDC_SECONDS:
				{
					int seconds = GetDlgItemInt(hwnd, LOWORD(wParam), nullptr, FALSE);
					if (seconds >= SETTING_LIFETIME_MIN &&
						seconds <= SETTING_LIFETIME_MAX &&
						seconds != PopupOptions.Seconds) {
						PopupOptions.Seconds = seconds;
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			
			case IDC_MINIMUMWIDTH:
				{
					int temp = GetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, nullptr, FALSE);
					if (temp >= SETTING_MINIMUMWIDTH_MIN &&
						temp <= SETTING_MAXIMUMWIDTH_MAX &&
						temp != PopupOptions.MinimumWidth) {
						PopupOptions.MinimumWidth = temp;
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			
			case IDC_MAXIMUMWIDTH:
				{
					int temp = GetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, nullptr, FALSE);
					if (temp >= SETTING_MINIMUMWIDTH_MIN &&
						temp <= SETTING_MAXIMUMWIDTH_MAX &&
						temp != PopupOptions.MaximumWidth) {
						PopupOptions.MaximumWidth = temp;
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
			}// end switch(idCtrl)
			break;

		case EN_KILLFOCUS:		// Edit controls lost fokus
			switch (LOWORD(wParam)) { // lParam = Handle to the control
			case IDC_SECONDS:
				{
					int seconds = GetDlgItemInt(hwnd, LOWORD(wParam), nullptr, FALSE);
					if (seconds > SETTING_LIFETIME_MAX)
						PopupOptions.Seconds = SETTING_LIFETIME_MAX;
					else if (seconds < SETTING_LIFETIME_MIN)
						PopupOptions.Seconds = SETTING_LIFETIME_MIN;
					if (seconds != PopupOptions.Seconds) {
						SetDlgItemInt(hwnd, LOWORD(wParam), PopupOptions.Seconds, FALSE);
						ErrorMSG(SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
						SetFocus((HWND)lParam);
					}
				}
				break;
			
			case IDC_MINIMUMWIDTH:
				{
					int temp = GetDlgItemInt(hwnd, LOWORD(wParam), nullptr, FALSE);
					if (temp < SETTING_MINIMUMWIDTH_MIN)
						PopupOptions.MinimumWidth = SETTING_MINIMUMWIDTH_MIN;
					else if (temp > SETTING_MAXIMUMWIDTH_MAX)
						PopupOptions.MinimumWidth = SETTING_MAXIMUMWIDTH_MAX;
					if (temp != PopupOptions.MinimumWidth) {
						SetDlgItemInt(hwnd, LOWORD(wParam), PopupOptions.MinimumWidth, FALSE);
						ErrorMSG(SETTING_MINIMUMWIDTH_MIN, SETTING_MAXIMUMWIDTH_MAX);
						SetFocus((HWND)lParam);
						break;
					}
					if (temp > PopupOptions.MaximumWidth) {
						PopupOptions.MaximumWidth = min(temp, SETTING_MAXIMUMWIDTH_MAX);
						SetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, PopupOptions.MaximumWidth, FALSE);
					}
				}
				break;
			
			case IDC_MAXIMUMWIDTH:
				{
					int temp = GetDlgItemInt(hwnd, LOWORD(wParam), nullptr, FALSE);
					if (temp >= SETTING_MAXIMUMWIDTH_MAX)
						PopupOptions.MaximumWidth = SETTING_MAXIMUMWIDTH_MAX;
					else if (temp < SETTING_MINIMUMWIDTH_MIN)
						PopupOptions.MaximumWidth = SETTING_MINIMUMWIDTH_MIN;
					if (temp != PopupOptions.MaximumWidth) {
						SetDlgItemInt(hwnd, LOWORD(wParam), PopupOptions.MaximumWidth, FALSE);
						ErrorMSG(SETTING_MINIMUMWIDTH_MIN, SETTING_MAXIMUMWIDTH_MAX);
						SetFocus((HWND)lParam);
						break;
					}
					if (temp < PopupOptions.MinimumWidth) {
						PopupOptions.MinimumWidth = max(temp, SETTING_MINIMUMWIDTH_MIN);
						SetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, PopupOptions.MinimumWidth, FALSE);
					}
				}
				break;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadOption_General();
				return TRUE;

			case PSN_APPLY:
				// Seconds
				g_plugin.setByte("InfiniteDelay", PopupOptions.InfiniteDelay);
				g_plugin.setWord("Seconds", (uint16_t)PopupOptions.Seconds);
				g_plugin.setByte("LeaveHovered", PopupOptions.LeaveHovered);

				// Dynamic Resize
				g_plugin.setByte("DynamicResize", PopupOptions.DynamicResize);
				g_plugin.setByte("UseMinimumWidth", PopupOptions.UseMinimumWidth);
				g_plugin.setWord("MinimumWidth", PopupOptions.MinimumWidth);
				g_plugin.setByte("UseMaximumWidth", PopupOptions.UseMaximumWidth);
				g_plugin.setWord("MaximumWidth", PopupOptions.MaximumWidth);

				// Position
				g_plugin.setByte("Position", (uint8_t)PopupOptions.Position);

				// Configure popup area
				g_plugin.setWord("gapTop", (uint16_t)PopupOptions.gapTop);
				g_plugin.setWord("gapBottom", (uint16_t)PopupOptions.gapBottom);
				g_plugin.setWord("gapLeft", (uint16_t)PopupOptions.gapLeft);
				g_plugin.setWord("gapRight", (uint16_t)PopupOptions.gapRight);
				g_plugin.setWord("spacing", (uint16_t)PopupOptions.spacing);

				// Spreading
				g_plugin.setByte("Spreading", (uint8_t)PopupOptions.Spreading);

				// miscellaneous
				Check_ReorderPopups(hwnd);	// this save also PopupOptions.ReorderPopups

				// disable When
				g_plugin.setByte("DisableWhenFullscreen", PopupOptions.DisableWhenFullscreen);

				// new status options
				for (auto &pa : Accounts())
					if (!pa->bIsVirtual) {
						char prefix[128];
						mir_snprintf(prefix, "Protocol Status/%s", pa->szModuleName);

						wchar_t pszSettingName[256];
						mir_snwprintf(pszSettingName, L"Protocol Status/%s", pa->tszAccountName);
						g_plugin.setDword(prefix, OptTree_GetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount, pszSettingName));
					}

				g_plugin.setDword("Global Status", OptTree_GetOptions(hwnd, IDC_STATUSES, statusOptions, statusOptionsCount, L"Global Status"));
				return TRUE;
			}
			break;

		case IDC_MINIMUMWIDTH_SPIN:
			{
				LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
				int temp = lpnmud->iPos + lpnmud->iDelta;
				if (temp > PopupOptions.MaximumWidth) {
					PopupOptions.MaximumWidth = min(temp, SETTING_MAXIMUMWIDTH_MAX);
					SetDlgItemInt(hwnd, IDC_MAXIMUMWIDTH, PopupOptions.MaximumWidth, FALSE);
				}
			}
			break;

		case IDC_MAXIMUMWIDTH_SPIN:
			{
				LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
				int temp = lpnmud->iPos + lpnmud->iDelta;
				if (temp < PopupOptions.MinimumWidth) {
					PopupOptions.MinimumWidth = max(temp, SETTING_MINIMUMWIDTH_MIN);
					SetDlgItemInt(hwnd, IDC_MINIMUMWIDTH, PopupOptions.MinimumWidth, FALSE);
				}
			}
		}
		break;

	case WM_DESTROY:
		if (statusOptions) {
			for (int i = 0; i < statusOptionsCount; ++i) {
				mir_free(statusOptions[i].pszOptionName);
				mir_free(statusOptions[i].pszSettingName);
			}
			delete[] statusOptions;
			statusOptions = nullptr;
			statusOptionsCount = 0;
			bDlgInit = false;
		}
		break;
	}
	return FALSE;
}

void ErrorMSG(int minValue, int maxValue)
{
	wchar_t str[128];
	mir_snwprintf(str, TranslateT("You cannot specify a value lower than %d and higher than %d."), minValue, maxValue);
	MSGERROR(str);
}

void Check_ReorderPopups(HWND hwnd)
{
	if (!PopupOptions.ReorderPopups && PopupOptions.ReorderPopupsWarning) {
		int res = MessageBox(hwnd,
			TranslateT("'Reorder popups' option is currently disabled.\r\nThis may cause misaligned popups when used with\r\navatars and text replacement (mainly NewXstatusNotify).\r\n\r\nDo you want to enable popup reordering now?\r\n"),
			TranslateT("Popup plus warning"), MB_ICONEXCLAMATION | MB_YESNOCANCEL);

		switch (res) {
		case IDYES:
			PopupOptions.ReorderPopups = TRUE;
			// Reset warning for next option change !!!
			PopupOptions.ReorderPopupsWarning = TRUE;
			break;
		case IDNO:
			PopupOptions.ReorderPopups = FALSE;
			PopupOptions.ReorderPopupsWarning = FALSE;
			break;
		default:
			return;
		}
	}
	g_plugin.setByte("ReorderPopups", PopupOptions.ReorderPopups);
	g_plugin.setByte("ReorderPopupsWarning", PopupOptions.ReorderPopupsWarning);
	if (hwnd) CheckDlgButton(hwnd, IDC_REORDERPOPUPS, PopupOptions.ReorderPopups ? BST_CHECKED : BST_UNCHECKED);
}

INT_PTR CALLBACK PositionBoxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	LOGFONT lf;
	static HFONT hFontTitle = nullptr;

	switch (msg) {
	case WM_INITDIALOG:
		EnableThemeDialogTexture(hwndDlg, ETDT_ENABLETAB);

		GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_GETFONT, 0, 0), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfHeight *= 1.2;
		hFontTitle = CreateFontIndirect(&lf);
		SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

		SendDlgItemMessage(hwndDlg, IDOK, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDOK, BUTTONADDTOOLTIP, (WPARAM)L"OK", BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_ACT_OK));

		SendDlgItemMessage(hwndDlg, IDCANCEL, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDCANCEL, BUTTONADDTOOLTIP, (WPARAM)L"Cancel", BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDCANCEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_ACT_CLOSE));

		SetDlgItemInt(hwndDlg, IDC_TXT_TOP, PopupOptions.gapTop, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_BOTTOM, PopupOptions.gapBottom, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_LEFT, PopupOptions.gapLeft, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_RIGHT, PopupOptions.gapRight, FALSE);
		SetDlgItemInt(hwndDlg, IDC_TXT_SPACING, PopupOptions.spacing, FALSE);

		TranslateDialogDefault(hwndDlg);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam)) == IDOK) {
			PopupOptions.gapTop = GetDlgItemInt(hwndDlg, IDC_TXT_TOP, nullptr, FALSE);
			PopupOptions.gapBottom = GetDlgItemInt(hwndDlg, IDC_TXT_BOTTOM, nullptr, FALSE);
			PopupOptions.gapLeft = GetDlgItemInt(hwndDlg, IDC_TXT_LEFT, nullptr, FALSE);
			PopupOptions.gapRight = GetDlgItemInt(hwndDlg, IDC_TXT_RIGHT, nullptr, FALSE);
			PopupOptions.spacing = GetDlgItemInt(hwndDlg, IDC_TXT_SPACING, nullptr, FALSE);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		else if ((LOWORD(wParam)) == IDCANCEL)
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		break;

	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
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
