/*

Miranda IM: the free IM client for Microsoft* Windows*
Copyright 2000-2009 Miranda ICQ/IM project, 

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

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

//---------------------------------------------------------------------------
#include "main.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////
//LIST_INTERFACE	li;
FI_INTERFACE	*FIP = 0;
HINSTANCE		hInst;			//!< Global reference to the application
MGLOBAL			myGlobals;
int hLangpack;


//Information gathered by Miranda, displayed in the plugin pane of the Option Dialog
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,		// altered here and on file listing, so as not to match original
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,		//doesn't replace anything built-in
	// {ED39AF7C-BECD-404e-9499-4D04F711B9CB}
	{ 0xed39af7c, 0xbecd, 0x404e, { 0x94, 0x99, 0x4d, 0x04, 0xf7, 0x11, 0xb9, 0xcb } }
};

//static char szSendSS[]=SZ_SENDSS;

HANDLE hsvc_SendScreenshot=0;
HANDLE hsvc_SendDesktop=0;
HANDLE hsvc_EditBitmap=0;
HANDLE hsvc_Send2ImageShack=0;

HANDLE hNetlibUser = 0;		//!< Netlib Register User
HANDLE hFolderScreenshot=0;

HANDLE hhook_ModulesLoad=0;
HANDLE hhook_SystemPShutdown=0;


// Functions ////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------
* DLL entry point - Required to store the instance handle
*/
extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	hInst = hinstDLL;
	// Freeing some unneeded resources
	DisableThreadLibraryCalls(GetModuleHandle(_T("sendss.dll")));

	return TRUE;
}

/*---------------------------------------------------------------------------
* Called by Miranda to get the information associated to this plugin.
* It only returns the PLUGININFO structure, without any test on the version
* @param mirandaVersion The version of the application calling this function
*/
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	myGlobals.mirandaVersion = mirandaVersion;
	return &pluginInfo;
}


/*---------------------------------------------------------------------------
* Initializes the services provided and the link to those needed
* Called when the plugin is loaded into Miranda
*/
extern "C" int __declspec(dllexport) Load(void) {
	mir_getLP(&pluginInfo);
	INT_PTR result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&FIP);

	if(FIP == NULL || result != S_OK) {
		MessageBoxEx(NULL, TranslateT("Fatal error, image services not found. Send Screenshot will be disabled."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return 1;
	}

	// load icon library (use UserInfoEx icon Pack)
	IcoLib_LoadModule();

	hhook_ModulesLoad		= HookEvent(ME_SYSTEM_MODULESLOADED, hook_ModulesLoaded);
	//hhook_options_init	= HookEvent(ME_OPT_INITIALISE, hook_options_init);
	//hhook_OkToExit		= HookEvent(ME_SYSTEM_OKTOEXIT, hook_OkToExit);
	hhook_SystemPShutdown	= HookEvent(ME_SYSTEM_PRESHUTDOWN, hook_SystemPShutdown);

	AddMenuItems();
	RegisterServices();

	return 0;
}

int hook_ModulesLoaded(WPARAM, LPARAM) {

	myGlobals.PopUpExist		= ServiceExists(MS_POPUP_ADDPOPUP);
	myGlobals.PopUpActionsExist	= ServiceExists(MS_POPUP_REGISTERACTIONS);
	myGlobals.PluginHTTPExist	= ServiceExists(MS_HTTP_ACCEPT_CONNECTIONS);
	myGlobals.PluginFTPExist	= ServiceExists(MS_FTPFILE_SHAREFILE);
//	myGlobals.PluginUserinfoEx	= ServiceExists(MS_USERINFO_VCARD_EXPORT);

	// Netlib register
	if (!NetlibInit()){
		;
	}

	// load my button class
	if(!ServiceExists("UserInfo/vCard/Export")) {
		CtrlButtonLoadModule();
	}

	// Folders plugin support
	hFolderScreenshot = FoldersRegisterCustomPathT("SendSS", "Screenshots",
							_T(PROFILE_PATH)_T("\\")_T(CURRENT_PROFILE)_T("\\Screenshots"));

	return 0;
}

/*---------------------------------------------------------------------------
* Prepare the plugin to stop
* Called by Miranda when it will exit or when the plugin gets deselected
*/
extern "C" int __declspec(dllexport) Unload(void) {
	UnhookEvent(hhook_SystemPShutdown);

	DestroyServiceFunction(MS_SENDSS_OPENDIALOG);
	DestroyServiceFunction(MS_SENDSS_EDITBITMAP);
	DestroyServiceFunction(MS_SENDSS_SENDDESKTOP);
	DestroyServiceFunction(MS_SENDSS_SEND2IMAGESHACK);
	return 0;
}

int hook_SystemPShutdown(WPARAM wParam, LPARAM lParam) {
	UnhookEvent(hhook_ModulesLoad);

	// Netlib unregister
	NetlibClose();
	
	// uninitialize classes
	CtrlButtonUnloadModule();

	return 0;
}

//---------------------------------------------------------------------------
// Netlib
HANDLE NetlibInit(void) {
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.szSettingsModule = PLUGNAME;
	nlu.ptszDescriptiveName = TranslateT("SendSS HTTP connections");
	nlu.flags = NUF_OUTGOING|NUF_HTTPCONNS|NUF_TCHAR;			//|NUF_NOHTTPSOPTION;
	return hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

void NetlibClose(void) {
	Netlib_CloseHandle(hNetlibUser);
}


//---------------------------------------------------------------------------
// Callback function of service
// 1. Send a screenshot of the desktop to the selected contact
// wParam = contact handle
// lParam = 0
// 2. Open the capture dialog in take screenshot only mode (it will not be sent)
// wParam = 0
// lParam = anything but 0
INT_PTR service_CaptureAndSendDesktop(WPARAM wParam, LPARAM lParam) {
	TfrmMain *frmMain=new TfrmMain();
	if (!frmMain) {
		MessageBoxEx(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return -1;
	}
	LPTSTR pszPath = GetCustomPath();
	if(pszPath)
	{
		HANDLE hContact = (HANDLE) wParam;
		LPSTR  pszProto = GetContactProto(hContact);
		bool bChatRoom = db_get_b(hContact, pszProto, "ChatRoom", 0) != 0;
		frmMain->m_opt_chkTimed			= false;
		frmMain->m_opt_tabCapture		= 1;
		frmMain->m_opt_cboxDesktop		= 0;
		frmMain->m_opt_chkEditor		= false;
		frmMain->m_opt_cboxSendBy		= bChatRoom ? SS_IMAGESHACK:SS_FILESEND;
		frmMain->Init(pszPath, hContact);		// this method create the window hidden.
		frmMain->btnCaptureClick();					// this method will call Close()
		mir_free(pszPath);
	}
	return 0;
}

//---------------------------------------------------------------------------
// Callback function of service for contact menu and main menu
// wParam = contact handle
// lParam = 0
INT_PTR service_OpenCaptureDialog(WPARAM wParam, LPARAM lParam) {
	TfrmMain *frmMain=new TfrmMain();
	if (!frmMain) {
		MessageBoxEx(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return -1;
	}

	LPTSTR pszPath = GetCustomPath();
	if(pszPath)
	{
		frmMain->Init(pszPath, (HANDLE)wParam);
		mir_free(pszPath);
	}
	frmMain->Show();
	return 0;
}

//---------------------------------------------------------------------------
// Edit a in-memory bitmap on the edit window
// wParam = (SENDSSCB) callback function address to call when editing is done
// lParam = (HBITMAP) bitmap handle, a copy is made so the calling function can free this handle after the service function returns
// Returns:
INT_PTR service_EditBitmap(WPARAM wParam, LPARAM lParam) {
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

//---------------------------------------------------------------------------
// Callback function of service for sending image to imageshack.us
// wParam = (char*)filename
// lParam = (HANDLE)contact (can be null)
INT_PTR service_Send2ImageShack(WPARAM wParam, LPARAM lParam) {
	LPSTR result = NULL;
	CSendImageShack* cSend = new CSendImageShack(NULL, (HANDLE)lParam, false);
	cSend->m_pszFile = mir_a2t((char*)wParam);
	cSend->m_bDeleteAfterSend = FALSE;
	if (lParam != NULL) {
		cSend->Send();
		return 0;
	}
	cSend->SendSync(TRUE);
	cSend->Send();
	if (cSend->GetURL()) {
		result = mir_strdup(cSend->GetURL());
	}
	else {
		result = cSend->GetError();
	}
	delete cSend;
	return (INT_PTR)result;
}

//---------------------------------------------------------------------------
// Add SendSS menu item in contact menu
void AddMenuItems(void)
{
	// Common
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
	mi.hParentMenu = HGENMENU_ROOT;

	// Add item to contact menu
	mi.position		= 1000000;
	mi.ptszName		= LPGENT("Send Screenshot");
	mi.hIcon		= IcoLib_GetIcon(ICO_PLUG_SSWINDOW2);
	mi.pszService	= MS_SENDSS_OPENDIALOG;
	Menu_AddContactMenuItem(&mi);

	// Add item to contact menu
	mi.position		= 1000001;
	mi.ptszName		= LPGENT("Send desktop screenshot");
	mi.hIcon		= IcoLib_GetIcon(ICO_PLUG_SSWINDOW2);
	mi.pszService	= MS_SENDSS_SENDDESKTOP;
	Menu_AddContactMenuItem(&mi);

	// Add item to main menu
	mi.position		= 1000001;
	mi.ptszName		= LPGENT("Take a screenshot");
	mi.hIcon		= IcoLib_GetIcon(ICO_PLUG_SSWINDOW2);
	mi.pszService	= MS_SENDSS_OPENDIALOG;
	Menu_AddMainMenuItem(&mi);
}

//---------------------------------------------------------------------------
// Register Send screenshot services
int RegisterServices(void) {
	hsvc_SendScreenshot = CreateServiceFunction(MS_SENDSS_OPENDIALOG, service_OpenCaptureDialog);
	if (!hsvc_SendScreenshot)
		MessageBoxEx(NULL, TranslateT("Could not register miranda service."), _T("MS_SENDSS_OPENDIALOG"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	hsvc_SendDesktop = CreateServiceFunction(MS_SENDSS_SENDDESKTOP, service_CaptureAndSendDesktop);
	if (!hsvc_SendDesktop)
		MessageBoxEx(NULL, TranslateT("Could not register miranda service."), _T("MS_SENDSS_SENDDESKTOP"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	hsvc_EditBitmap = CreateServiceFunction(MS_SENDSS_EDITBITMAP, service_EditBitmap);
	if (!hsvc_EditBitmap)
		MessageBoxEx(NULL, TranslateT("Could not register miranda service."), _T("MS_SENDSS_EDITBITMAP"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	hsvc_Send2ImageShack = CreateServiceFunction(MS_SENDSS_SEND2IMAGESHACK, service_Send2ImageShack);
	if (!hsvc_Send2ImageShack)
		MessageBoxEx(NULL, TranslateT("Could not register miranda service."), _T("MS_SENDSS_SEND2IMAGESHACK"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	return 0;
}

//---------------------------------------------------------------------------
LPTSTR GetCustomPath() {
	LPTSTR pszPath = Utils_ReplaceVarsT(_T("%miranda_userdata%\\Screenshots"));
	if (hFolderScreenshot) {
		TCHAR szPath[1024] = {'\0'};
		FoldersGetCustomPathT(hFolderScreenshot, szPath, 1024, pszPath);
		mir_freeAndNil(pszPath);
		pszPath = mir_tstrdup(szPath);
	}
	if(pszPath == NULL)
	{
		MessageBox(NULL, _T("Can not retrieve Screenshot path."), _T("Send Screenshot"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return 0;
	}

	int result = CreateDirectoryTreeT(pszPath);
	if(result != NULL)
	{
		TCHAR szError[MAX_PATH];
		mir_sntprintf(szError,MAX_PATH,TranslateT("Could not create Screenshot folder (error code: %d):\n%s\nDo you have write permissions?"),result,pszPath);
		MessageBox(NULL, szError, _T("Send Screenshot"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		mir_free(pszPath);
		return 0;
	}
	return pszPath;
}
