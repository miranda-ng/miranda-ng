/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "resource.h"

static class CAboutDlg *pAboutDialog;

class CAboutDlg : public CDlgBase
{
	int m_iState = 0;

	CCtrlBase ctrlHeaderBar, ctrlDevelopers, ctrlCredits, ctrlWhiteRect;
	CCtrlButton btnLink;

public:
	CAboutDlg() :
		CDlgBase(g_plugin, IDD_ABOUT),
		btnLink(this, IDC_CONTRIBLINK),
		ctrlCredits(this, IDC_CREDITSFILE),
		ctrlHeaderBar(this, IDC_HEADERBAR),
		ctrlWhiteRect(this, IDC_WHITERECT),
		ctrlDevelopers(this, IDC_DEVS)
	{
		btnLink.OnClick = Callback(this, &CAboutDlg::onClick);

		ctrlCredits.UseSystemColors();
		ctrlWhiteRect.UseSystemColors();
		ctrlDevelopers.UseSystemColors();
	}

	bool OnInitDialog() override
	{
		ptrW wszCopyright(mir_utf8decodeW(LEGAL_COPYRIGHT));
		if (wszCopyright == nullptr)
			wszCopyright = mir_a2u(LEGAL_COPYRIGHT);
		ctrlDevelopers.SetText(wszCopyright);

		char productVersion[56];
		Miranda_GetVersionText(productVersion, _countof(productVersion));
		ctrlHeaderBar.SetText(CMStringW(FORMAT, L"Miranda NG\nv%S", productVersion));

		HRSRC hResInfo = FindResource(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CREDITS), L"TEXT");
		uint32_t ResSize = SizeofResource(g_plugin.getInst(), hResInfo);
		HGLOBAL hRes = LoadResource(g_plugin.getInst(), hResInfo);
		char *pszMsg = (char*)LockResource(hRes);
		if (pszMsg) {
			char *pszMsgt = (char*)alloca(ResSize + 1);
			memcpy(pszMsgt, pszMsg, ResSize); pszMsgt[ResSize] = 0;

			ptrW ptszMsg;
			if (ResSize >= 3 && pszMsgt[0] == '\xef' && pszMsgt[1] == '\xbb' && pszMsgt[2] == '\xbf')
				ptszMsg = mir_utf8decodeW(pszMsgt + 3);
			else
				ptszMsg = mir_a2u_cp(pszMsgt, 1252);
			ctrlCredits.SetText(ptszMsg);

			UnlockResource(pszMsg);
		}
		FreeResource(hRes);
		ctrlCredits.Hide();

		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_MIRANDA);
		return true;
	}

	void OnDestroy() override
	{
		pAboutDialog = nullptr;
		Window_FreeIcon_IcoLib(m_hwnd);
	}

	void onClick(CCtrlButton*)
	{
		if (m_iState) {
			btnLink.SetText(TranslateT("Credits >"));
			ctrlDevelopers.Show();
			ctrlCredits.Hide();
		}
		else {
			btnLink.SetText(TranslateT("< Copyright"));
			ctrlDevelopers.Hide();
			ctrlCredits.Show();
		}
		m_iState = !m_iState;
	}
};

static INT_PTR AboutCommand(WPARAM wParam, LPARAM)
{
	if (pAboutDialog) {
		SetForegroundWindow(pAboutDialog->GetHwnd());
		SetFocus(pAboutDialog->GetHwnd());
	}
	else {
		pAboutDialog = new CAboutDlg();
		pAboutDialog->SetParent((HWND)wParam);
		pAboutDialog->Show();
	}
	return 0;
}

static INT_PTR IndexCommand(WPARAM, LPARAM)
{
	Utils_OpenUrl("https://wiki.miranda-ng.org");
	return 0;
}

static INT_PTR WebsiteCommand(WPARAM, LPARAM)
{
	Utils_OpenUrl("https://miranda-ng.org");
	return 0;
}

static INT_PTR BugCommand(WPARAM, LPARAM)
{
	Utils_OpenUrl("https://github.com/miranda-ng/miranda-ng/issues/new");
	return 0;
}

int ShutdownHelpModule(WPARAM, LPARAM)
{
	if (pAboutDialog)
		pAboutDialog->Close();
	return 0;
}

int LoadHelpModule(void)
{
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownHelpModule);

	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("&Help"), 2000090000);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "8824ECA5-6942-46D7-9D07-1BA600E0D02E");

	SET_UID(mi, 0xf3ebf1fa, 0x587c, 0x494d, 0xbd, 0x33, 0x7f, 0x88, 0xb3, 0x61, 0x1e, 0xd3);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.position = 2000090000;
	mi.name.a = LPGEN("&About...");
	mi.pszService = "Help/AboutCommand";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, AboutCommand);

	SET_UID(mi, 0x495df66f, 0x844e, 0x479a, 0xaf, 0x21, 0x3e, 0x42, 0xc5, 0x14, 0x7c, 0x7e);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HELP);
	mi.position = -500050000;
	mi.name.a = LPGEN("&Support");
	mi.pszService = "Help/IndexCommand";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, IndexCommand);

	SET_UID(mi, 0x15e18b58, 0xec73, 0x45c2, 0xb9, 0xf4, 0x2a, 0xfe, 0xc2, 0xb7, 0xd3, 0x25);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDAWEB);
	mi.position = 2000050000;
	mi.name.a = LPGEN("&Miranda NG homepage");
	mi.pszService = "Help/WebsiteCommand";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, WebsiteCommand);

	SET_UID(mi, 0xe7d0fe8b, 0xfdeb, 0x45b3, 0xba, 0x83, 0x3, 0x1e, 0x15, 0xda, 0x7e, 0x52);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	mi.position = 2000040000;
	mi.name.a = LPGEN("&Report bug");
	mi.pszService = "Help/BugCommand";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, BugCommand);
	return 0;
}
