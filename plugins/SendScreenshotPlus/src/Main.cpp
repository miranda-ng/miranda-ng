/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (с) 2012-15 Miranda NG project (http://miranda-ng.org),
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
#include "Main.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////
HINSTANCE		g_hSendSS;
MGLOBAL			g_myGlobals;
HANDLE			g_hNetlibUser=0;//!< Netlib Register User
FI_INTERFACE*	FIP=NULL;
int				hLangpack;//Miranda NG langpack used by translate functions, filled by mir_getLP()

IconItem ICONS[ICO_END_]={
	{LPGEN("Main Icon"),"main",IDI_MAIN,32},
	{LPGEN("Main Icon"),"mainxs",IDI_MAIN},
	{LPGEN("Target Cursor"),"target",IDI_TARGET,32},
	{LPGEN("Target Desktop"),"monitor",IDI_MONITOR,32},
};
IconItem ICONS_BTN[ICO_BTN_END_]={
	{LPGEN("Help"),"help",IDI_HELP},
	{LPGEN("Open Folder"),"folder",IDI_FOLDER},
	{LPGEN("Description off"),"desc",IDI_DESC},
	{LPGEN("Description on"),"descon",IDI_DESCON},
	{LPGEN("Delete off"),"del",IDI_DEL},
	{LPGEN("Delete on"),"delon",IDI_DELON},
	{LPGEN("Prev"),"arrowl",IDI_ARROWL},
	{LPGEN("Next"),"arrowr",IDI_ARROWR},
	{LPGEN("Update"),"update",IDI_UPDATE},
	{LPGEN("OK"),"ok",IDI_OK},
	{LPGEN("Cancel"),"cancel",IDI_CANCEL},
//		{LPGEN("Apply"),"apply",IDI_APPLY},
	{LPGEN("Edit"),"edit",IDI_EDIT},
	{LPGEN("Edit on"),"editon",IDI_EDITON},
	{LPGEN("Copy"),"copy",IDI_COPY},
	{LPGEN("BBC"),"bbc",IDI_BBC},
	{LPGEN("BBC link"),"bbclnk",IDI_BBC2},
	{LPGEN("Down arrow"),"downarrow",IDI_DOWNARROW},
};

static HANDLE m_hFolderScreenshot=0;
TCHAR* GetCustomPath() {
	TCHAR* pszPath = Utils_ReplaceVarsT(_T("%miranda_userdata%\\Screenshots"));
	if(m_hFolderScreenshot){
		TCHAR szPath[1024]={0};
		FoldersGetCustomPathT(m_hFolderScreenshot, szPath, 1024, pszPath);
		mir_free(pszPath);
		pszPath = mir_tstrdup(szPath);
	}
	if(!pszPath){
		MessageBox(NULL, _T("Can not retrieve screenshot path."), _T("SendSS"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return 0;
	}
	int result = CreateDirectoryTreeT(pszPath);
	if(result){
		TCHAR szError[MAX_PATH];
		mir_sntprintf(szError,MAX_PATH,TranslateT("Could not create screenshot folder (error code: %d):\n%s\nDo you have write permissions?"),result,pszPath);
		MessageBox(NULL, szError, _T("SendSS"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		mir_free(pszPath);
		return 0;
	}
	return pszPath;
}
/// services
static HANDLE m_hOpenCaptureDialog=0;
static HANDLE m_hSendDesktop=0;
static HANDLE m_hEditBitmap=0;
static HANDLE m_hSend2ImageShack=0;
//---------------------------------------------------------------------------
// Callback function of service for contact menu and main menu
// wParam = contact handle
// lParam = 0 (or 0xFFFF to preselect window under cursor)
INT_PTR service_OpenCaptureDialog(WPARAM wParam, LPARAM lParam){
	TfrmMain* frmMain=new TfrmMain();
	if(!frmMain) {
		MessageBox(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
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
// Callback function of service
// 1. Send a screenshot of the desktop to the selected contact
// wParam = contact handle
// lParam = 0
// 2. Open the capture dialog in take screenshot only mode (it will not be sent)
// wParam = 0
// lParam = anything but 0
INT_PTR service_SendDesktop(WPARAM wParam, LPARAM lParam) {
	TfrmMain* frmMain=new TfrmMain();
	if(!frmMain) {
		MessageBox(NULL, TranslateT("Could not create main dialog."), TranslateT("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
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

// Functions ////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hSendSS=hinstDLL;
	return TRUE;
}
static const PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {ED39AF7C-BECD-404E-9499-4D04F711B9CB}
	{0xed39af7c, 0xbecd, 0x404e, {0x94, 0x99, 0x4d, 0x04, 0xf7, 0x11, 0xb9, 0xcb}}
};
DLL_EXPORT PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return const_cast<PLUGININFOEX*>(&pluginInfo);
}
/// hooks
static HANDLE m_hookModulesLoaded=0;
static HANDLE m_hookSystemPreShutdown=0;
int hook_ModulesLoaded(WPARAM, LPARAM)
{
	g_myGlobals.PopupExist		= ServiceExists(MS_POPUP_ADDPOPUPT);
	g_myGlobals.PopupActionsExist	= ServiceExists(MS_POPUP_REGISTERACTIONS);
	g_myGlobals.PluginHTTPExist	= ServiceExists(MS_HTTP_ACCEPT_CONNECTIONS);
	g_myGlobals.PluginFTPExist	= ServiceExists(MS_FTPFILE_SHAREFILE);
	g_myGlobals.PluginDropboxExist	= ServiceExists(MS_DROPBOX_SEND_FILE);
	// Netlib register
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.szSettingsModule = __PLUGIN_NAME;
	nlu.ptszDescriptiveName = TranslateT("SendSS HTTP connections");
	nlu.flags = NUF_OUTGOING|NUF_HTTPCONNS|NUF_TCHAR;			//|NUF_NOHTTPSOPTION;
	g_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	// load my button class / or use UInfoEx
	CtrlButtonLoadModule();
	// Folders plugin support
	m_hFolderScreenshot = FoldersRegisterCustomPathT(LPGEN("SendSS"), LPGEN("Screenshots"),
		_T(PROFILE_PATH)_T("\\")_T(CURRENT_PROFILE)_T("\\Screenshots"));
	return 0;
}
int hook_SystemPreShutdown(WPARAM wParam, LPARAM lParam)
{
	TfrmAbout::Unload();//crashes if done from "Unload" because of dependencies
	TfrmMain::Unload();// "
	// Netlib unregister
	Netlib_CloseHandle(g_hNetlibUser);
	// uninitialize classes
	CtrlButtonUnloadModule();
	return 0;
}

ATOM g_clsTargetHighlighter=0;
DLL_EXPORT int Load(void)
{
	mir_getLP(&pluginInfo);
	INT_PTR result=CallService(MS_IMG_GETINTERFACE,FI_IF_VERSION,(LPARAM)&FIP);
	if(FIP==NULL || result!=S_OK) {
		MessageBox(NULL, TranslateT("Image services (AdvaImg) not found.\nSendSS disabled."), TranslateT("SendSS"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return 1;
	}
	/// hook events
	m_hookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,hook_ModulesLoaded);
	m_hookSystemPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN,hook_SystemPreShutdown);
	/// icons
	Icon_Register(g_hSendSS,SZ_SENDSS,ICONS,sizeof(ICONS)/sizeof(IconItem),SZ_SENDSS);
	Icon_Register(g_hSendSS,SZ_SENDSS "/" LPGEN("Buttons"),ICONS_BTN,sizeof(ICONS_BTN)/sizeof(IconItem),SZ_SENDSS);
	/// services
	#define srv_reg(name) do{\
		m_h##name=CreateServiceFunction(SZ_SENDSS "/" #name, service_##name);\
		if(!m_h##name) MessageBoxA(NULL,Translate("Could not register Miranda service."),SZ_SENDSS "/" #name,MB_OK|MB_ICONERROR|MB_APPLMODAL);\
	}while(0)
	srv_reg(OpenCaptureDialog);
	srv_reg(SendDesktop);
	srv_reg(EditBitmap);
	srv_reg(Send2ImageShack);
	/// menu items
	CLISTMENUITEM mi={sizeof(mi)};
	mi.flags=CMIF_ROOTHANDLE|CMIF_TCHAR/*|CMIF_ICONFROMICOLIB*/;
	mi.hParentMenu=HGENMENU_ROOT;
	mi.icolibItem=GetIconHandle(ICO_MAINXS);
	#define _Menu_AddMainMenuItemEx(name,srv,pos) do{mi.ptszName=name;mi.pszService=srv;mi.position=pos;Menu_AddMainMenuItem(&mi);}while(0)
	#define _Menu_AddContactMenuItemEx(name,srv,pos) do{mi.ptszName=name;mi.pszService=srv;mi.position=pos;Menu_AddContactMenuItem(&mi);}while(0)
	_Menu_AddMainMenuItemEx(LPGENT("Take a screenshot"),MS_SENDSS_OPENDIALOG,1000001);
	_Menu_AddContactMenuItemEx(LPGENT("Send screenshot"),MS_SENDSS_OPENDIALOG,1000000);
	_Menu_AddContactMenuItemEx(LPGENT("Send desktop screenshot"),MS_SENDSS_SENDDESKTOP,1000001);
	/// hotkey's
	HOTKEYDESC hkd={sizeof(hkd)};
	hkd.pszName="Open SendSS+";
	hkd.ptszDescription=LPGENT("Open SendSS+");
	hkd.ptszSection=_T("SendSS+");
	hkd.pszService=MS_SENDSS_OPENDIALOG;
	//hkd.DefHotKey=HOTKEYCODE(HOTKEYF_CONTROL, VK_F10) | HKF_MIRANDA_LOCAL;
	hkd.lParam=0xFFFF;
	hkd.dwFlags = HKD_TCHAR;
	Hotkey_Register(&hkd);
	/// register highlighter window class
	HBRUSH brush=CreateSolidBrush(0x0000FF00);//owned by class
	WNDCLASS wndclass={CS_HREDRAW|CS_VREDRAW,DefWindowProc,0,0,g_hSendSS,NULL,NULL,brush,NULL,L"SendSSHighlighter"};
	g_clsTargetHighlighter=RegisterClass(&wndclass);
	return 0;
}
/*---------------------------------------------------------------------------
* Prepare the plugin to stop
* Called by Miranda when it will exit or when the plugin gets deselected
*/
DLL_EXPORT int Unload(void)
{//as "ghazan" says, it's useless to unregister services or unhook events, let's still do it for now :P
	CallService(MS_HOTKEY_UNREGISTER,0,(LPARAM)"Open SendSS+");
	/// deregister services
	#define srv_dereg(name) do{ if(m_h##name) DestroyServiceFunction(m_h##name),m_h##name=0; }while(0)
	srv_dereg(OpenCaptureDialog);
	srv_dereg(SendDesktop);
	srv_dereg(EditBitmap);
	srv_dereg(Send2ImageShack);
	if(m_hookModulesLoaded) UnhookEvent(m_hookModulesLoaded),m_hookModulesLoaded=0;
	if(m_hookSystemPreShutdown) UnhookEvent(m_hookSystemPreShutdown),m_hookSystemPreShutdown=0;
	if(g_clsTargetHighlighter) UnregisterClass((TCHAR*)g_clsTargetHighlighter,g_hSendSS),g_clsTargetHighlighter=0;
	return 0;
}
