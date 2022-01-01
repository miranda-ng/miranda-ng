/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

/////////////////////////////////////////////////////////////////////////////////////////
// Common contact list window options

class CCluiOptsDlg : public CDlgBase
{
	CCtrlSpin spinMaxSize, spinHideTime;
	CCtrlCheck chkAutoHide, chkAutoSize, chkCaption, chkToolWnd, chkTransparent;
	CCtrlSlider m_active, m_inactive;

public:
	CCluiOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_CLUI),
		m_active(this, IDC_TRANSACTIVE, 255, 1),
		m_inactive(this, IDC_TRANSINACTIVE, 255, 1),
		chkCaption(this, IDC_SHOWCAPTION),
		chkToolWnd(this, IDC_TOOLWND),
		chkAutoHide(this, IDC_AUTOHIDE),
		chkAutoSize(this, IDC_AUTOSIZE),
		chkTransparent(this, IDC_TRANSPARENT),
		spinMaxSize(this, IDC_MAXSIZESPIN, 100),
		spinHideTime(this, IDC_HIDETIMESPIN, 900, 1)
	{
		m_active.OnChange = m_inactive.OnChange = Callback(this, &CCluiOptsDlg::onChange_Slider);

		chkCaption.OnChange = Callback(this, &CCluiOptsDlg::onChange_Caption);
		chkToolWnd.OnChange = Callback(this, &CCluiOptsDlg::onChange_ToolWnd);
		chkAutoHide.OnChange = Callback(this, &CCluiOptsDlg::onChange_AutoHide);
		chkAutoSize.OnChange = Callback(this, &CCluiOptsDlg::onChange_AutoSize);
		chkTransparent.OnChange = Callback(this, &CCluiOptsDlg::onChange_Transparent);
	}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_BRINGTOFRONT, g_plugin.getByte("BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ONTOP, g_plugin.getByte("OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_TOOLWND, g_plugin.getByte("ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_MIN2TRAY, g_plugin.getByte("Min2Tray", SETTING_MIN2TRAY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWMAINMENU, db_get_b(0, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CLIENTDRAG, db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_FADEINOUT, db_get_b(0, "CLUI", "FadeInOut", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DROPSHADOW, g_plugin.getByte("WindowShadow", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ONDESKTOP, g_plugin.getByte("OnDesktop", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_AUTOSIZEUPWARD, db_get_b(0, "CLUI", "AutoSizeUpward", 0) ? BST_CHECKED : BST_UNCHECKED);

		spinMaxSize.SetPosition(db_get_b(0, "CLUI", "MaxSizeHeight", 75));
		spinHideTime.SetPosition(g_plugin.getWord("HideTime", SETTING_HIDETIME_DEFAULT));

		chkCaption.SetState(db_get_b(0, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT));
		chkAutoSize.SetState(db_get_b(0, "CLUI", "AutoSize", 0));
		chkAutoHide.SetState(g_plugin.getByte("AutoHide", SETTING_AUTOHIDE_DEFAULT));
		chkTransparent.SetState(g_plugin.getByte("Transparent", SETTING_TRANSPARENT_DEFAULT));

		ptrW wszTitle(g_plugin.getWStringA("TitleText"));
		if (wszTitle)
			SetDlgItemTextW(m_hwnd, IDC_TITLETEXT, wszTitle);
		else
			SetDlgItemTextA(m_hwnd, IDC_TITLETEXT, MIRANDANAME);

		m_active.SetPosition(g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT));
		m_inactive.SetPosition(g_plugin.getByte("AutoAlpha", SETTING_AUTOALPHA_DEFAULT));
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("OnTop", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ONTOP));
		g_plugin.setByte("ToolWindow", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_TOOLWND));
		g_plugin.setByte("BringToFront", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_BRINGTOFRONT));
		db_set_b(0, "CLUI", "FadeInOut", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_FADEINOUT));
		db_set_b(0, "CLUI", "AutoSizeUpward", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_AUTOSIZEUPWARD));
		g_plugin.setByte("WindowShadow", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_DROPSHADOW));
		g_plugin.setByte("OnDesktop", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ONDESKTOP));
		db_set_b(0, "CLUI", "ShowCaption", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWCAPTION));
		db_set_b(0, "CLUI", "ShowMainMenu", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWMAINMENU));
		db_set_b(0, "CLUI", "ClientAreaDrag", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_CLIENTDRAG));
		g_plugin.setByte("Min2Tray", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_MIN2TRAY));

		g_plugin.setWord("HideTime", spinHideTime.GetPosition());
		db_set_b(0, "CLUI", "MaxSizeHeight", spinMaxSize.GetPosition());

		db_set_b(0, "CLUI", "AutoSize", chkAutoSize.GetState());
		g_plugin.setByte("AutoHide", chkAutoHide.GetState());
		g_plugin.setByte("Transparent", chkTransparent.GetState());

		g_plugin.setByte("Alpha", m_active.GetPosition());
		g_plugin.setByte("AutoAlpha", m_inactive.GetPosition());

		wchar_t title[256];
		GetDlgItemText(m_hwnd, IDC_TITLETEXT, title, _countof(title));
		g_plugin.setWString("TitleText", title);
		SetWindowText(g_clistApi.hwndContactList, title);

		g_clistApi.pfnLoadCluiGlobalOpts();
		SetWindowPos(g_clistApi.hwndContactList, IsDlgButtonChecked(m_hwnd, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE);
		if (IsDlgButtonChecked(m_hwnd, IDC_TOOLWND)) {
			// Window must be hidden to dynamically remove the taskbar button.
			// See http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/taskbar.asp
			WINDOWPLACEMENT p;
			p.length = sizeof(p);
			GetWindowPlacement(g_clistApi.hwndContactList, &p);
			ShowWindow(g_clistApi.hwndContactList, SW_HIDE);
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE,
				GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE);
			SetWindowPlacement(g_clistApi.hwndContactList, &p);
		}
		else SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & ~WS_EX_TOOLWINDOW);

		if (IsDlgButtonChecked(m_hwnd, IDC_ONDESKTOP)) {
			HWND hProgMan = FindWindowA("Progman", nullptr);
			if (IsWindow(hProgMan))
				::SetParent(g_clistApi.hwndContactList, hProgMan);
		}
		else ::SetParent(g_clistApi.hwndContactList, nullptr);

		if (IsDlgButtonChecked(m_hwnd, IDC_SHOWCAPTION))
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE,
				GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		else
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE,
				GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX));

		if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_SHOWMAINMENU))
			SetMenu(g_clistApi.hwndContactList, nullptr);
		else
			SetMenu(g_clistApi.hwndContactList, g_clistApi.hMenuMain);

		SetWindowPos(g_clistApi.hwndContactList, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);
		if (IsIconic(g_clistApi.hwndContactList) && BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_TOOLWND))
			ShowWindow(g_clistApi.hwndContactList, IsDlgButtonChecked(m_hwnd, IDC_MIN2TRAY) ? SW_HIDE : SW_SHOW);
		if (IsDlgButtonChecked(m_hwnd, IDC_TRANSPARENT)) {
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(g_clistApi.hwndContactList, RGB(0, 0, 0), (uint8_t)g_plugin.getByte("AutoAlpha", SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
		}
		else SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);

		SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);        //forces it to send a cln_listsizechanged
		return true;
	}

	void onChange_AutoHide(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_HIDETIME), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_HIDETIMESPIN), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC01), bEnabled);
	}

	void onChange_AutoSize(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC21), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC22), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_MAXSIZEHEIGHT), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_MAXSIZESPIN), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_AUTOSIZEUPWARD), bEnabled);
	}

	void onChange_Caption(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_TOOLWND), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_MIN2TRAY), !chkToolWnd.IsChecked() && bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TITLETEXT), bEnabled);
	}

	void onChange_Transparent(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC11), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC12), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TRANSACTIVE), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TRANSINACTIVE), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ACTIVEPERC), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_INACTIVEPERC), bEnabled);
	}

	void onChange_ToolWnd(CCtrlCheck *pCheck)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_MIN2TRAY), !pCheck->IsChecked());
	}

	void onChange_Slider(CCtrlSlider*)
	{
		char str[10];
		mir_snprintf(str, "%d%%", 100 * m_inactive.GetPosition() / 255);
		SetDlgItemTextA(m_hwnd, IDC_INACTIVEPERC, str);
		mir_snprintf(str, "%d%%", 100 * m_active.GetPosition() / 255);
		SetDlgItemTextA(m_hwnd, IDC_ACTIVEPERC, str);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Status bar options

class CStatusBarOpts : public CDlgBase
{
	CCtrlCheck chkShow;

public:
	CStatusBarOpts() :
		CDlgBase(g_plugin, IDD_OPT_SBAR),
		chkShow(this, IDC_SHOWSBAR)
	{
		chkShow.OnChange = Callback(this, &CStatusBarOpts::onChange_Show);
	}

	bool OnInitDialog() override
	{
		chkShow.SetState(db_get_b(0, "CLUI", "ShowSBar", 1));

		uint8_t showOpts = db_get_b(0, "CLUI", "SBarShow", 1);
		CheckDlgButton(m_hwnd, IDC_SHOWICON, showOpts & 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWPROTO, showOpts & 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWSTATUS, showOpts & 4 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_RIGHTSTATUS, db_get_b(0, "CLUI", "SBarRightClk", 0) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_RIGHTMIRANDA, IsDlgButtonChecked(m_hwnd, IDC_RIGHTSTATUS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_EQUALSECTIONS, db_get_b(0, "CLUI", "EqualSections", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SBPANELBEVEL, db_get_b(0, "CLUI", "SBarBevel", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWGRIP, db_get_b(0, "CLUI", "ShowGrip", 1) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		db_set_b(0, "CLUI", "ShowSBar", chkShow.GetState());
		db_set_b(0, "CLUI", "SBarShow",
			(uint8_t)((IsDlgButtonChecked(m_hwnd, IDC_SHOWICON) ? 1 : 0) |
				(IsDlgButtonChecked(m_hwnd, IDC_SHOWPROTO) ? 2 : 0) |
				(IsDlgButtonChecked(m_hwnd, IDC_SHOWSTATUS) ? 4 : 0)));
		db_set_b(0, "CLUI", "SBarRightClk", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_RIGHTMIRANDA));
		db_set_b(0, "CLUI", "EqualSections", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_EQUALSECTIONS));
		db_set_b(0, "CLUI", "SBarBevel", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SBPANELBEVEL));
		g_clistApi.pfnLoadCluiGlobalOpts();
		if (db_get_b(0, "CLUI", "ShowGrip", 1) != (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWGRIP)) {
			HWND parent = GetParent(g_clistApi.hwndStatus);
			int flags = WS_CHILD | CCS_BOTTOM;
			db_set_b(0, "CLUI", "ShowGrip", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWGRIP));
			ShowWindow(g_clistApi.hwndStatus, SW_HIDE);
			DestroyWindow(g_clistApi.hwndStatus);
			flags |= db_get_b(0, "CLUI", "ShowSBar", 1) ? WS_VISIBLE : 0;
			flags |= db_get_b(0, "CLUI", "ShowGrip", 1) ? SBARS_SIZEGRIP : 0;
			g_clistApi.hwndStatus = CreateWindow(STATUSCLASSNAME, nullptr, flags, 0, 0, 0, 0, parent, nullptr, g_plugin.getInst(), nullptr);
		}

		if (chkShow.GetState())
			ShowWindow(g_clistApi.hwndStatus, SW_SHOW);
		else
			ShowWindow(g_clistApi.hwndStatus, SW_HIDE);
		SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
		return true;
	}

	void onChange_Show(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWICON), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWPROTO), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWSTATUS), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_RIGHTSTATUS), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_RIGHTMIRANDA), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_EQUALSECTIONS), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SBPANELBEVEL), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWGRIP), bEnabled);
	}
};

/****************************************************************************************/

int CluiOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTitle.a = LPGEN("Window");
	odp.szGroup.a = LPGEN("Contact list");
	odp.pDialog = new CCluiOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTitle.a = LPGEN("Status bar");
	odp.pDialog = new CStatusBarOpts();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
