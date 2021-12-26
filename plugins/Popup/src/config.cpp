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

// ===== General Plugin =====
HANDLE hSemaphore;
BOOL closing = FALSE;
HANDLE folderId;
BOOL gbPopupLoaded = FALSE;
BOOL gbHppInstalled = FALSE;

// ===== Brushes, Colours and Fonts =====
HBITMAP hbmNoAvatar;

// ===== Options =====
POPUPOPTIONS PopupOptions;

HRESULT(WINAPI *MyDwmEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);

// ====== Common Vars ========================

//  common funcs
void LoadOptions()
{
	memset(&PopupOptions, 0, sizeof(PopupOptions));
#if defined(_DEBUG)
	PopupOptions.debug = g_plugin.getByte("debug", FALSE);
#endif

	// Load Popup Options
	if (!OptionLoaded) {
		LoadOption_General();
		LoadOption_Skins();
		LoadOption_Actions();
		LoadOption_AdvOpts();
	}
	Check_ReorderPopups();
	OptionLoaded = true;
	return;
}

void PopupPreview()
{
	wchar_t *lptzTitle1Eng = TranslateT("The Jabberwocky");
	wchar_t *lptzText1Eng = TranslateT("`Twas brillig, and the slithy toves\r\nDid gyre and gimble in the wabe:\r\nAll mimsy were the borogoves,\r\nAnd the mome raths outgrabe.\r\n\t[b][i]Lewis Carroll, 1855[/i][/b]");

	wchar_t *lptzTitle2 = TranslateT("Test preview for the popup plugin settings. This is supposed to be long enough not to fit in one line...");
	wchar_t *lptzText2 = TranslateW(
		LPGENW("This is a special test preview for the popup plugin settings. The text and title are quite long so you can tweak your skin and plugin settings to best fit your needs :)")
		);

	POPUPDATA2 ppd = {};
	ppd.cbSize = sizeof(ppd);
	ppd.flags = PU2_UNICODE;

	ppd.szTitle.w = lptzTitle1Eng;
	ppd.szText.w = lptzText1Eng;
	ppd.lchIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	CallService(MS_POPUP_ADDPOPUP2, (WPARAM)&ppd, APF_NO_HISTORY);
	if (PopupOptions.UseAnimations || PopupOptions.UseEffect)
		Sleep((ANIM_TIME * 2) / 3); // Pause

	memset(&ppd, 0, sizeof(ppd));
	ppd.cbSize = sizeof(ppd);
	ppd.flags = PU2_UNICODE;
	ppd.szTitle.w = lptzTitle2;
	ppd.szText.w = lptzText2;
	ppd.lchIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	ppd.hbmAvatar = hbmNoAvatar;

	CallService(MS_POPUP_ADDPOPUP2, (WPARAM)&ppd, APF_NO_HISTORY);
	if (PopupOptions.UseAnimations || PopupOptions.UseEffect) Sleep((ANIM_TIME * 2) / 3); // Pause

	PUShowMessageW(TranslateT("This is a notification message"), (uint32_t)SM_NOTIFY | 0x80000000);
	if (PopupOptions.UseAnimations || PopupOptions.UseEffect) Sleep((ANIM_TIME * 2) / 3); // Pause

	PUShowMessageW(TranslateT("This is a warning message"), (uint32_t)SM_WARNING | 0x80000000);
	if (PopupOptions.UseAnimations || PopupOptions.UseEffect) Sleep((ANIM_TIME * 2) / 3); // Pause

	PUShowMessageW(TranslateT("This is an error message"), (uint32_t)SM_ERROR | 0x80000000);
}

//////////////////////////////////////////////////////////////////////////////////////////////

struct EnumProcParam
{
	LPCSTR szModule, szNewModule;
};

static int EnumProc(const char *szSetting, void *lParam)
{
	EnumProcParam* param = (EnumProcParam*)lParam;

	DBVARIANT dbv;
	if (!db_get(NULL, param->szModule, szSetting, &dbv)) {
		db_set(NULL, param->szNewModule, szSetting, &dbv);
		db_free(&dbv);
	}
	return 0;
}

static void CopyModule(const char *szModule, const char *szNewModule)
{
	EnumProcParam param = { szModule, szNewModule };
	db_enum_settings(NULL, EnumProc, szModule, &param);

	db_delete_module(0, szModule);
}

void UpgradeDb()
{
	if (db_get_b(0, "Compatibility", "Popup+ Opts", 0) == 1)
		return;

	CopyModule("PopUp", "Popup");
	CopyModule("PopUpCLASS", "PopupCLASS");
	CopyModule("PopUpActions", "PopupActions");
	CopyModule("PopUpNotifications", "PopupNotifications");

	db_set_b(0, "Compatibility", "Popup+ Opts", 1);
}
