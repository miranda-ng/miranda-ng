/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (с) 2012-17 Miranda NG project (https://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 S�rgio Vieira Rolanski (portet from Borland C++)

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
#include "Main.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////
CHAT_MANAGER    *pci;
CLIST_INTERFACE *pcli;
HINSTANCE g_hSendSS;
MGLOBAL g_myGlobals;
HNETLIBUSER g_hNetlibUser=0;//!< Netlib Register User
FI_INTERFACE *FIP=NULL;
int hLangpack;//Miranda NG langpack used by translate functions, filled by mir_getLP()

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
	//		{LPGEN("Apply"),"apply",IDI_APPLY},
	{ LPGEN("Edit"), "edit", IDI_EDIT },
	{ LPGEN("Edit on"), "editon", IDI_EDITON },
	{ LPGEN("Copy"), "copy", IDI_COPY },
	{ LPGEN("BBC"), "bbc", IDI_BBC },
	{ LPGEN("BBC link"), "bbclnk", IDI_BBC2 },
	{ LPGEN("Down arrow"), "downarrow", IDI_DOWNARROW },
};

static HANDLE m_hFolderScreenshot = 0;
wchar_t* GetCustomPath()
{
	wchar_t* pszPath = Utils_ReplaceVarsW(L"%miranda_userdata%\\Screenshots");
	if (m_hFolderScreenshot) {
		wchar_t szPath[1024] = { 0 };
		FoldersGetCustomPathT(m_hFolderScreenshot, szPath, 1024, pszPath);
		mir_free(pszPath);
		pszPath = mir_wstrdup(szPath);
	}
	if (!pszPath) {
		MessageBox(NULL, L"Can not retrieve screenshot path.", L"SendSS", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return 0;
	}
	int result = CreateDirectoryTreeW(pszPath);
	if (result) {
		wchar_t szError[MAX_PATH];
		mir_snwprintf(szError, MAX_PATH, TranslateT("Could not create screenshot folder (error code: %d):\n%s\nDo you have write permissions?"), result, pszPath);
		MessageBox(NULL, szError, L"SendSS", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		mir_free(pszPath);
		return 0;
	}
	return pszPath;
}
/// services
static HANDLE m_hOpenCaptureDialog = 0;
static HANDLE m_hSendDesktop = 0;
static HANDLE m_hEditBitmap = 0;
static HANDLE m_hSend2ImageShack = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// Callback function of service for contact menu and main menu
// wParam = contact handle
// lParam = 0 (or 0xFFFF to preselect window under cursor)

INT_PTR service_OpenCaptureDialog(WPARAM wParam, LPARAM lParam)
{
	TfrmMain* frmMain = new TfrmMain();
	if (!frmMain) {
		MessageBox(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return -1;
	}
	wchar_t* pszPath = GetCustomPath();
	if (!pszPath) {
		delete frmMain;
		return -1;
	}
	frmMain->Init(pszPath, wParam);
	mir_free(pszPath);
	if (lParam == 0xFFFF) {
		frmMain->SetTargetWindow(NULL);
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
	TfrmMain* frmMain = new TfrmMain();
	if (!frmMain) {
		MessageBox(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return -1;
	}
	wchar_t* pszPath = GetCustomPath();
	if (!pszPath) {
		delete frmMain;
		return -1;
	}
	MCONTACT hContact = (MCONTACT)wParam;
	char*  pszProto = GetContactProto(hContact);
	bool bChatRoom = db_get_b(hContact, pszProto, "ChatRoom", 0) != 0;
	frmMain->m_opt_chkTimed = false;
	frmMain->m_opt_tabCapture = 1;
	frmMain->m_opt_cboxDesktop = 0;
	frmMain->m_opt_chkEditor = false;
	frmMain->m_opt_cboxSendBy = bChatRoom ? SS_IMAGESHACK : SS_FILESEND;
	frmMain->Init(pszPath, hContact);		// this method create the window hidden.
	mir_free(pszPath);
	frmMain->btnCaptureClick();				// this method will call Close()
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Edit a in-memory bitmap on the edit window
// wParam = (SENDSSCB) callback function address to call when editing is done
// lParam = (HBITMAP) bitmap handle, a copy is made so the calling function can free this handle after the service function returns
// Returns:

INT_PTR service_EditBitmap(WPARAM, LPARAM)
{
	/*	TfrmEdit *frmEdit=new TfrmEdit(NULL);
		if (!frmEdit)
		return -1;

		Graphics::TBitmap *bitmap=new Graphics::TBitmap();
		if (!bitmap)
		return -2;

		bitmap->Handle = (void*)lParam;
		frmEdit->InitEditor(bitmap); // a copy of the bitmap is made inside this function
		frmEdit->Show();
		delete bitmap;
		*/
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Callback function of service for sending image to imageshack.us
// wParam = (char*)filename
// lParam = (HANDLE)contact (can be null)

INT_PTR service_Send2ImageShack(WPARAM wParam, LPARAM lParam)
{
	char* result = NULL;
	CSendHost_ImageShack* cSend = new CSendHost_ImageShack(NULL, lParam, false);
	cSend->m_bDeleteAfterSend = false;
	cSend->SetFile((char*)wParam);
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

// Functions ////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	g_hSendSS = hinstDLL;
	return TRUE;
}
static const PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR, __AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {ED39AF7C-BECD-404E-9499-4D04F711B9CB}
	{ 0xed39af7c, 0xbecd, 0x404e, { 0x94, 0x99, 0x4d, 0x04, 0xf7, 0x11, 0xb9, 0xcb } }
};
DLL_EXPORT PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return const_cast<PLUGININFOEX*>(&pluginInfo);
}
/// hooks
int hook_ModulesLoaded(WPARAM, LPARAM)
{
	g_myGlobals.PopupExist = ServiceExists(MS_POPUP_ADDPOPUPT);
	g_myGlobals.PopupActionsExist = ServiceExists(MS_POPUP_REGISTERACTIONS);
	g_myGlobals.PluginHTTPExist = ServiceExists(MS_HTTP_ACCEPT_CONNECTIONS);
	g_myGlobals.PluginFTPExist = ServiceExists(MS_FTPFILE_UPLOAD);
	g_myGlobals.PluginDropboxExist = ServiceExists(MS_DROPBOX_UPLOAD);
	// Netlib register
	NETLIBUSER nlu = {};
	nlu.szSettingsModule = __PLUGIN_NAME;
	nlu.szDescriptiveName.w = TranslateT("SendSS HTTP connections");
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;			//|NUF_NOHTTPSOPTION;
	g_hNetlibUser = Netlib_RegisterUser(&nlu);
	// load my button class / or use UInfoEx
	CtrlButtonLoadModule();
	// Folders plugin support
	m_hFolderScreenshot = FoldersRegisterCustomPathT(LPGEN("SendSS"), LPGEN("Screenshots"),
		PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\Screenshots");
	return 0;
}
int hook_SystemPreShutdown(WPARAM, LPARAM)
{
	TfrmAbout::Unload();//crashes if done from "Unload" because of dependencies
	TfrmMain::Unload();// "
	// Netlib unregister
	Netlib_CloseHandle(g_hNetlibUser);
	// uninitialize classes
	CtrlButtonUnloadModule();
	return 0;
}

ATOM g_clsTargetHighlighter = 0;
DLL_EXPORT int Load(void)
{
	mir_getLP(&pluginInfo);
	pci = Chat_GetInterface();
	pcli = Clist_GetInterface();

	INT_PTR result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&FIP);
	if (FIP == NULL || result != S_OK) {
		MessageBox(NULL, TranslateT("Image services (AdvaImg plugin) not found.\nSendSS disabled."), TranslateT("SendSS"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return 1;
	}
	/// hook events
	HookEvent(ME_SYSTEM_MODULESLOADED, hook_ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, hook_SystemPreShutdown);
	
	/// icons
	Icon_Register(g_hSendSS, SZ_SENDSS, ICONS, sizeof(ICONS) / sizeof(IconItem), SZ_SENDSS);
	Icon_Register(g_hSendSS, SZ_SENDSS "/" LPGEN("Buttons"), ICONS_BTN, sizeof(ICONS_BTN) / sizeof(IconItem), SZ_SENDSS);
	
	/// services
#define srv_reg(name) do{\
		m_h##name=CreateServiceFunction(SZ_SENDSS "/" #name, service_##name);\
		if(!m_h##name) MessageBoxA(NULL,Translate("Could not register Miranda service."),SZ_SENDSS "/" #name,MB_OK|MB_ICONERROR|MB_APPLMODAL);\
		}while(0)
	srv_reg(OpenCaptureDialog);
	srv_reg(SendDesktop);
	srv_reg(EditBitmap);
	srv_reg(Send2ImageShack);

	// menu items
	CMenuItem mi;
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
	Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x8d5b0d9a, 0x68d4, 0x4594, 0x9f, 0x41, 0x0, 0x64, 0x20, 0xe7, 0xf8, 0x9f);
	mi.name.w = LPGENW("Send desktop screenshot");
	mi.pszService = MS_SENDSS_SENDDESKTOP;
	mi.position = 1000001;
	Menu_AddContactMenuItem(&mi);

	/// hotkey's
	HOTKEYDESC hkd = {};
	hkd.pszName = "Open SendSS+";
	hkd.szDescription.w = LPGENW("Open SendSS+");
	hkd.szSection.w = L"SendSS+";
	hkd.pszService = MS_SENDSS_OPENDIALOG;
	hkd.lParam = 0xFFFF;
	hkd.dwFlags = HKD_UNICODE;
	Hotkey_Register(&hkd);

	/// register highlighter window class
	HBRUSH brush = CreateSolidBrush(0x0000FF00);//owned by class
	WNDCLASS wndclass = { CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0, 0, g_hSendSS, NULL, NULL, brush, NULL, L"SendSSHighlighter" };
	g_clsTargetHighlighter = RegisterClass(&wndclass);
	return 0;
}
/*---------------------------------------------------------------------------
* Prepare the plugin to stop
* Called by Miranda when it will exit or when the plugin gets deselected
*/
DLL_EXPORT int Unload(void)
{
	if (g_clsTargetHighlighter)
		UnregisterClass((wchar_t*)g_clsTargetHighlighter, g_hSendSS), g_clsTargetHighlighter = 0;
	return 0;
}
