/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "global.h"
#include <m_hotkeys.h>

// Prototypes ///////////////////////////////////////////////////////////////////////////
//LIST_INTERFACE	li;
FI_INTERFACE	*FIP = 0;
HINSTANCE		hInst;			//!< Global reference to the application
MGLOBAL			myGlobals;
int				hLangpack;


//Information gathered by Miranda, displayed in the plugin pane of the Option Dialog
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {ED39AF7C-BECD-404E-9499-4D04F711B9CB}
	{0xed39af7c, 0xbecd, 0x404e, {0x94, 0x99, 0x4d, 0x04, 0xf7, 0x11, 0xb9, 0xcb}}
};

HANDLE hsvc_SendScreenshot=0;
HANDLE hsvc_SendDesktop=0;
HANDLE hsvc_EditBitmap=0;
HANDLE hsvc_Send2ImageShack=0;

HANDLE hNetlibUser = 0;		//!< Netlib Register User
HANDLE hFolderScreenshot=0;

// Functions ////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------
* DLL entry point - Required to store the instance handle
*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

/*---------------------------------------------------------------------------
* Called by Miranda to get the information associated to this plugin.
* It only returns the PLUGININFO structure, without any test on the version
* @param mirandaVersion The version of the application calling this function
*/
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}


/*---------------------------------------------------------------------------
* Initializes the services provided and the link to those needed
* Called when the plugin is loaded into Miranda
*/
ATOM g_clsTargetHighlighter=0;
HANDLE g_hookModulesLoaded=0;
HANDLE g_hookSystemPreShutdown=0;
extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	INT_PTR result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&FIP);

	if(FIP == NULL || result != S_OK) {
		MessageBoxEx(NULL, TranslateT("Fatal error, image services not found. Send Screenshot will be disabled."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return 1;
	}

	// load icon library (use UserInfoEx icon Pack)
	IcoLib_LoadModule();

	g_hookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED, hook_ModulesLoaded);
	g_hookSystemPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN, hook_SystemPreShutdown);

	RegisterServices();
	AddMenuItems();

	//hotkey's
	HOTKEYDESC hkd={sizeof(hkd)};
	hkd.pszName="Open SendSS+";
	hkd.ptszDescription=LPGENT("Open SendSS+");
	hkd.ptszSection=_T("SendSS+");
	hkd.pszService=MS_SENDSS_OPENDIALOG;
	//hkd.DefHotKey=HOTKEYCODE(HOTKEYF_CONTROL, VK_F10) | HKF_MIRANDA_LOCAL;
	hkd.lParam=0xFFFF;
	hkd.dwFlags = HKD_TCHAR;
	Hotkey_Register(&hkd);

	HBRUSH brush=CreateSolidBrush(0x0000FF00);//owned by class
	WNDCLASS wndclass={CS_HREDRAW|CS_VREDRAW,DefWindowProc,0,0,hInst,NULL,NULL,brush,NULL,L"SendSSHighlighter"};
	g_clsTargetHighlighter=RegisterClass(&wndclass);
	return 0;
}

int hook_ModulesLoaded(WPARAM, LPARAM)
{
	myGlobals.PopupExist		= ServiceExists(MS_POPUP_ADDPOPUPT);
	myGlobals.PopupActionsExist	= ServiceExists(MS_POPUP_REGISTERACTIONS);
	myGlobals.PluginHTTPExist	= ServiceExists(MS_HTTP_ACCEPT_CONNECTIONS);
	myGlobals.PluginFTPExist	= ServiceExists(MS_FTPFILE_SHAREFILE);
	myGlobals.PluginDropboxExist	= ServiceExists(MS_DROPBOX_SEND_FILE);

	// Netlib register
	NetlibInit();

	// load my button class / or use UInfoEx
	CtrlButtonLoadModule();

	// Folders plugin support
	hFolderScreenshot = FoldersRegisterCustomPathT(LPGEN("SendSS"), LPGEN("Screenshots"),
		_T(PROFILE_PATH)_T("\\")_T(CURRENT_PROFILE)_T("\\Screenshots"));

	return 0;
}

/*---------------------------------------------------------------------------
* Prepare the plugin to stop
* Called by Miranda when it will exit or when the plugin gets deselected
*/

extern "C" __declspec(dllexport) int Unload(void)
{//as "ghazan" says, it's useless to unregister services or unhook events, let's still do it for now :P
	CallService(MS_HOTKEY_UNREGISTER,0,(LPARAM)"Open SendSS+");
	UnRegisterServices();
	if(g_hookModulesLoaded) UnhookEvent(g_hookModulesLoaded),g_hookModulesLoaded=0;
	if(g_hookSystemPreShutdown) UnhookEvent(g_hookSystemPreShutdown),g_hookSystemPreShutdown=0;
	if(g_clsTargetHighlighter) UnregisterClass((TCHAR*)g_clsTargetHighlighter,hInst),g_clsTargetHighlighter=0;
	return 0;
}

int hook_SystemPreShutdown(WPARAM wParam, LPARAM lParam)
{
	TfrmAbout::Unload();//crashes if done from "Unload" because of dependencies
	TfrmMain::Unload();// "

	// Netlib unregister
	NetlibClose();

	// uninitialize classes
	CtrlButtonUnloadModule();
	return 0;
}

//---------------------------------------------------------------------------
// Netlib
HANDLE NetlibInit(void)
{
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.szSettingsModule = __PLUGIN_NAME;
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
	TfrmMain* frmMain=new TfrmMain();
	if(!frmMain) {
		MessageBoxEx(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return -1;
	}
	TCHAR* pszPath=GetCustomPath();
	if(!pszPath){
		delete frmMain;
		return -1;
	}
	MCONTACT hContact = (MCONTACT) wParam;
	char*  pszProto = GetContactProto(hContact);
	bool bChatRoom = db_get_b(hContact, pszProto, "ChatRoom", 0) != 0;
	frmMain->m_opt_chkTimed			= false;
	frmMain->m_opt_tabCapture		= 1;
	frmMain->m_opt_cboxDesktop		= 0;
	frmMain->m_opt_chkEditor		= false;
	frmMain->m_opt_cboxSendBy		= bChatRoom ? SS_IMAGESHACK:SS_FILESEND;
	frmMain->Init(pszPath,hContact);		// this method create the window hidden.
	mir_free(pszPath);
	frmMain->btnCaptureClick();				// this method will call Close()
	return 0;
}

//---------------------------------------------------------------------------
// Callback function of service for contact menu and main menu
// wParam = contact handle
// lParam = 0
INT_PTR service_OpenCaptureDialog(WPARAM wParam, LPARAM lParam){
	TfrmMain* frmMain=new TfrmMain();
	if(!frmMain) {
		MessageBoxEx(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return -1;
	}
	TCHAR* pszPath=GetCustomPath();
	if(!pszPath){
		delete frmMain;
		return -1;
	}
	frmMain->Init(pszPath, wParam);
	mir_free(pszPath);
	if(lParam==0xFFFF){
		frmMain->SetTargetWindow(NULL);
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
	char* result = NULL;
	CSendHost_ImageShack* cSend = new CSendHost_ImageShack(NULL, lParam, false);
	cSend->m_bDeleteAfterSend = false;
	cSend->SetFile((char*)wParam);
	if (lParam != NULL) {
		if(cSend->Send()) delete cSend;
		return NULL;
	}
	cSend->SendSilent();
	if (cSend->GetURL()) {
		result=mir_strdup(cSend->GetURL());
	}else{
		result=mir_t2a(cSend->GetErrorMsg());
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
	mi.hIcon		= Skin_GetIcon(ICO_COMMON_SSWINDOW2);
	mi.pszService	= MS_SENDSS_OPENDIALOG;
	Menu_AddContactMenuItem(&mi);

	// Add item to contact menu
	mi.position		= 1000001;
	mi.ptszName		= LPGENT("Send desktop screenshot");
//	mi.hIcon		= Skin_GetIcon(ICO_COMMON_SSWINDOW2);
	mi.pszService	= MS_SENDSS_SENDDESKTOP;
	Menu_AddContactMenuItem(&mi);

	// Add item to main menu
	mi.position		= 1000001;
	mi.ptszName		= LPGENT("Take a screenshot");
//	mi.hIcon		= Skin_GetIcon(ICO_COMMON_SSWINDOW2);
	mi.pszService	= MS_SENDSS_OPENDIALOG;
	Menu_AddMainMenuItem(&mi);
}

//---------------------------------------------------------------------------
// Register Send screenshot services
int RegisterServices(){
	hsvc_SendScreenshot = CreateServiceFunction(MS_SENDSS_OPENDIALOG, service_OpenCaptureDialog);
	if (!hsvc_SendScreenshot)
		MessageBoxEx(NULL, TranslateT("Could not register Miranda service."), _T("MS_SENDSS_OPENDIALOG"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	hsvc_SendDesktop = CreateServiceFunction(MS_SENDSS_SENDDESKTOP, service_CaptureAndSendDesktop);
	if (!hsvc_SendDesktop)
		MessageBoxEx(NULL, TranslateT("Could not register Miranda service."), _T("MS_SENDSS_SENDDESKTOP"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	hsvc_EditBitmap = CreateServiceFunction(MS_SENDSS_EDITBITMAP, service_EditBitmap);
	if (!hsvc_EditBitmap)
		MessageBoxEx(NULL, TranslateT("Could not register Miranda service."), _T("MS_SENDSS_EDITBITMAP"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	hsvc_Send2ImageShack = CreateServiceFunction(MS_SENDSS_SEND2IMAGESHACK, service_Send2ImageShack);
	if (!hsvc_Send2ImageShack)
		MessageBoxEx(NULL, TranslateT("Could not register Miranda service."), _T("MS_SENDSS_SEND2IMAGESHACK"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);

	return 0;
}
//---------------------------------------------------------------------------
// UnRegister Send screenshot services
int UnRegisterServices(){
	if(hsvc_SendScreenshot) DestroyServiceFunction(hsvc_SendScreenshot),hsvc_SendScreenshot=0;
	if(hsvc_SendDesktop) DestroyServiceFunction(hsvc_SendDesktop),hsvc_SendDesktop=0;
	if(hsvc_EditBitmap) DestroyServiceFunction(hsvc_EditBitmap),hsvc_EditBitmap=0;
	if(hsvc_Send2ImageShack) DestroyServiceFunction(hsvc_Send2ImageShack),hsvc_Send2ImageShack=0;
	return 0;
}

//---------------------------------------------------------------------------
TCHAR* GetCustomPath() {
	TCHAR* pszPath = Utils_ReplaceVarsT(_T("%miranda_userdata%\\Screenshots"));
	if(hFolderScreenshot){
		TCHAR szPath[1024]={0};
		FoldersGetCustomPathT(hFolderScreenshot, szPath, 1024, pszPath);
		mir_free(pszPath);
		pszPath = mir_tstrdup(szPath);
	}
	if(!pszPath){
		MessageBox(NULL, _T("Can not retrieve Screenshot path."), _T("Send Screenshot"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return 0;
	}

	int result = CreateDirectoryTreeT(pszPath);
	if(result){
		TCHAR szError[MAX_PATH];
		mir_sntprintf(szError,MAX_PATH,TranslateT("Could not create Screenshot folder (error code: %d):\n%s\nDo you have write permissions?"),result,pszPath);
		MessageBox(NULL, szError, _T("Send Screenshot"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		mir_free(pszPath);
		return 0;
	}
	return pszPath;
}
