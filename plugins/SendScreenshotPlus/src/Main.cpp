/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original source code
(c) 2004-2006 Sérgio Vieira Rolanski (ported from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////

CMPlugin g_plugin;
HGENMENU g_hMenu1, g_hMenu2;

ATOM g_clsTargetHighlighter = 0;
MGLOBAL g_myGlobals;
HNETLIBUSER g_hNetlibUser;

IconItem ICONS[ICO_END_] =
{
	{ LPGEN("Main Icon"), "main", IDI_MAIN, 32 },
	{ LPGEN("Main Icon"), "mainxs", IDI_MAIN },
	{ LPGEN("Target Cursor"), "target", IDI_TARGET, 32 },
	{ LPGEN("Target Desktop"), "monitor", IDI_MONITOR, 32 },
};

IconItem ICONS_BTN[ICO_BTN_END_] =
{
	{ LPGEN("Help"), "help", IDI_HELP },
	{ LPGEN("Open Folder"), "folder", IDI_FOLDER },
	{ LPGEN("Description off"), "desc", IDI_DESC },
	{ LPGEN("Description on"), "descon", IDI_DESCON },
	{ LPGEN("Delete off"), "del", IDI_DEL },
	{ LPGEN("Delete on"), "delon", IDI_DELON },
	{ LPGEN("Prev"), "arrowl", IDI_ARROWL },
	{ LPGEN("Next"), "arrowr", IDI_ARROWR },
	{ LPGEN("Update"), "update", IDI_UPDATE },
	{ LPGEN("OK"), "ok", IDI_OK },
	{ LPGEN("Cancel"), "cancel", IDI_CANCEL },
	{ LPGEN("Edit"), "edit", IDI_EDIT },
	{ LPGEN("Edit on"), "editon", IDI_EDITON },
	{ LPGEN("Copy"), "copy", IDI_COPY },
	{ LPGEN("BBCode"), "bbc", IDI_BBC },
	{ LPGEN("BBCode link"), "bbclnk", IDI_BBC2 },
	{ LPGEN("Down arrow"), "downarrow", IDI_DOWNARROW },
};

static HANDLE m_hFolderScreenshot = nullptr;
wchar_t* GetCustomPath()
{
	wchar_t *pszPath = Utils_ReplaceVarsW(L"%miranda_userdata%\\Screenshots");
	if (m_hFolderScreenshot) {
		wchar_t szPath[1024] = { 0 };
		FoldersGetCustomPathW(m_hFolderScreenshot, szPath, 1024, pszPath);
		mir_free(pszPath);
		pszPath = mir_wstrdup(szPath);
	}
	if (!pszPath) {
		MessageBox(nullptr, L"Can not retrieve screenshot path.", L"SendSS", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return nullptr;
	}
	int result = CreateDirectoryTreeW(pszPath);
	if (result) {
		wchar_t szError[MAX_PATH];
		mir_snwprintf(szError, MAX_PATH, TranslateT("Could not create screenshot folder (error code: %d):\n%s\nDo you have write permissions?"), result, pszPath);
		MessageBox(nullptr, szError, L"SendSS", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		mir_free(pszPath);
		return nullptr;
	}
	return pszPath;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Callback function of service for contact menu and main menu
// wParam = contact handle
// lParam = 0 (or 0xFFFF to preselect window under cursor)

INT_PTR service_OpenCaptureDialog(WPARAM wParam, LPARAM lParam)
{
	TfrmMain *frmMain = new TfrmMain();
	if (!frmMain) {
		MessageBox(nullptr, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return -1;
	}
	wchar_t *pszPath = GetCustomPath();
	if (!pszPath) {
		delete frmMain;
		return -1;
	}
	frmMain->Init(pszPath, wParam);
	mir_free(pszPath);
	if (lParam == 0xFFFF) {
		frmMain->SetTargetWindow(nullptr);
	}
	frmMain->Show();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Callback function of service
// 1. Send a screenshot of the desktop to the selected contact
// wParam = contact handle
// lParam = 0
// 2. Open the capture dialog in take screenshot only mode (it will not be sent)
// wParam = 0
// lParam = anything but 0

INT_PTR service_SendDesktop(WPARAM wParam, LPARAM)
{
	TfrmMain *frmMain = new TfrmMain();
	if (!frmMain) {
		MessageBox(nullptr, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return -1;
	}
	wchar_t *pszPath = GetCustomPath();
	if (!pszPath) {
		delete frmMain;
		return -1;
	}

	MCONTACT hContact = (MCONTACT)wParam;
	frmMain->m_opt_chkTimed = false;
	frmMain->m_opt_tabCapture = 1;
	frmMain->m_opt_cboxDesktop = 0;
	frmMain->m_opt_chkEditor = false;
	frmMain->m_opt_cboxSendBy = Contact::IsGroupChat(hContact) ? SS_IMAGESHACK : SS_FILESEND;
	frmMain->Init(pszPath, hContact);		// this method create the window hidden.
	mir_free(pszPath);
	frmMain->btnCaptureClick();				// this method will call Close()
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Callback function of service for sending image to imageshack.us
// wParam = (char*)filename
// lParam = (HANDLE)contact (can be null)

INT_PTR service_Send2ImageShack(WPARAM wParam, LPARAM lParam)
{
	char *result = nullptr;
	CSendHost_ImageShack *cSend = new CSendHost_ImageShack(nullptr, lParam, false);
	cSend->m_bDeleteAfterSend = false;
	cSend->SetFile((char *)wParam);
	if (lParam != NULL) {
		if (cSend->Send()) delete cSend;
		return NULL;
	}
	cSend->SendSilent();
	if (cSend->GetURL()) {
		result = mir_strdup(cSend->GetURL());
	}
	else {
		result = mir_u2a(cSend->GetErrorMsg());
	}
	delete cSend;
	return (INT_PTR)result;
}

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {ED39AF7C-BECD-404E-9499-4D04F711B9CB}
	{ 0xed39af7c, 0xbecd, 0x404e, { 0x94, 0x99, 0x4d, 0x04, 0xf7, 0x11, 0xb9, 0xcb } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// hooks

int hook_ModulesLoaded(WPARAM, LPARAM)
{
	g_myGlobals.PluginHTTPExist = ServiceExists(MS_HTTP_ACCEPT_CONNECTIONS);
	g_myGlobals.PluginFTPExist = ServiceExists(MS_FTPFILE_UPLOAD);
	g_myGlobals.PluginCloudFileExist = ServiceExists(MS_CLOUDFILE_UPLOAD);

	// Netlib register
	NETLIBUSER nlu = {};
	nlu.szSettingsModule = __PLUGIN_NAME;
	nlu.szDescriptiveName.w = TranslateT("SendSS");
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;			//|NUF_NOHTTPSOPTION;
	g_hNetlibUser = Netlib_RegisterUser(&nlu);

	// load my button class / or use UInfoEx
	CtrlButtonLoadModule();

	// Folders plugin support
	m_hFolderScreenshot = FoldersRegisterCustomPathW(LPGEN("SendSS"), LPGEN("Screenshots"),
		PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\Screenshots");
	return 0;
}

int hook_SystemPreShutdown(WPARAM, LPARAM)
{
	TfrmMain::Unload();

	// Netlib unregister
	Netlib_CloseHandle(g_hNetlibUser);

	// uninitialize classes
	CtrlButtonUnloadModule();
	return 0;
}

int hook_PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	INT_PTR flags = CallProtoService(Proto_GetBaseAccountName(hContact), PS_GETCAPS, PFLAGNUM_1, 0);
	bool bEnabled = (flags != CALLSERVICE_NOTFOUND) && (flags & PF1_FILE) != 0;
	Menu_ShowItem(g_hMenu1, bEnabled);
	Menu_ShowItem(g_hMenu2, bEnabled);
	return 0;
}

static int TabsrmmButtonsInit(WPARAM, LPARAM)
{
	// SRMM toolbar button
	BBButton bbd = {};
	bbd.pszModuleName = MODULENAME;
	bbd.dwButtonID = 1;
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON;
	bbd.dwDefPos = 201;
	bbd.hIcon = GetIconHandle(ICO_MAINXS);
	Srmm_AddButton(&bbd, &g_plugin);
	return 0;
}

static int TabsrmmButtonPressed(WPARAM hContact, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (!mir_strcmp(cbcd->pszModule, MODULENAME) && cbcd->dwButtonId == 1)
		CallService(MS_SENDSS_OPENDIALOG, hContact, 0);

	return 0;
}

int CMPlugin::Load()
{
	// hook events
	HookEvent(ME_SYSTEM_MODULESLOADED, hook_ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, hook_SystemPreShutdown);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, hook_PrebuildContactMenu);

	HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonsInit);

	// icons
	g_plugin.registerIcon(MODULENAME, ICONS, MODULENAME);
	g_plugin.registerIcon(MODULENAME "/" LPGEN("Buttons"), ICONS_BTN, MODULENAME);

	// services
#define srv_reg(name) CreateServiceFunction(MODULENAME "/" #name, service_##name);
	srv_reg(OpenCaptureDialog);
	srv_reg(SendDesktop);
	srv_reg(Send2ImageShack);

	// menu items
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = GetIconHandle(ICO_MAINXS);

	SET_UID(mi, 0xa559a22e, 0xd0f9, 0x4553, 0x8e, 0x68, 0x55, 0xb3, 0xae, 0xc4, 0x5d, 0x93);
	mi.name.w = LPGENW("Take a screenshot");
	mi.pszService = MS_SENDSS_OPENDIALOG;
	mi.position = 1000001;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xfea0a84, 0x1767, 0x4605, 0x99, 0xf0, 0xa9, 0x48, 0x1a, 0xa6, 0x6f, 0xce);
	mi.name.w = LPGENW("Send screenshot");
	mi.pszService = MS_SENDSS_OPENDIALOG;
	mi.position = 1000000;
	g_hMenu1 = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x8d5b0d9a, 0x68d4, 0x4594, 0x9f, 0x41, 0x0, 0x64, 0x20, 0xe7, 0xf8, 0x9f);
	mi.name.w = LPGENW("Send desktop screenshot");
	mi.pszService = MS_SENDSS_SENDDESKTOP;
	mi.position = 1000001;
	g_hMenu2 = Menu_AddContactMenuItem(&mi);

	// hotkey's
	HOTKEYDESC hkd = {};
	hkd.pszName = "Open SendSS+";
	hkd.szDescription.w = LPGENW("Open SendSS+");
	hkd.szSection.w = L"SendSS+";
	hkd.pszService = MS_SENDSS_OPENDIALOG;
	hkd.lParam = 0xFFFF;
	hkd.dwFlags = HKD_UNICODE;
	g_plugin.addHotkey(&hkd);

	// register highlighter window class
	HBRUSH brush = CreateSolidBrush(0x0000FF00); // owned by class
	WNDCLASS wndclass = { CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0, 0, g_plugin.getInst(), nullptr, nullptr, brush, nullptr, L"SendSSHighlighter" };
	g_clsTargetHighlighter = RegisterClass(&wndclass);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Prepare the plugin to stop

int CMPlugin::Unload()
{
	if (g_clsTargetHighlighter)
		UnregisterClass((wchar_t *)g_clsTargetHighlighter, g_plugin.getInst()), g_clsTargetHighlighter = 0;
	return 0;
}
