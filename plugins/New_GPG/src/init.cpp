// Copyright © 2010-2012 sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "commonheaders.h"

//global variables
bool bAppendTags = false, bDebugLog = false, bJabberAPI = false, bPresenceSigning = false, bIsMiranda09 = false, bFileTransfers = false, bSameAction = false, bAutoExchange = false, bStripTags = false, tabsrmm_used = false;
TCHAR *inopentag = NULL, *inclosetag = NULL, *outopentag = NULL, *outclosetag = NULL, *password = NULL;

list <JabberAccount*> Accounts;

HINSTANCE hInst;
HFONT bold_font = NULL;
HANDLE hLoadPubKey = NULL, g_hCLIcon = NULL, hExportGpgKeys = NULL, hImportGpgKeys = NULL;
HGENMENU hSendKey = NULL, hToggleEncryption = NULL;
RECT key_from_keyserver_rect = {0}, firstrun_rect = {0}, new_key_rect = {0}, key_gen_rect = {0}, load_key_rect = {0}, import_key_rect = {0}, key_password_rect = {0}, load_existing_key_rect = {0};
XML_API xi = {0};
int hLangpack = 0;
logtofile debuglog;
bool gpg_valid = false, gpg_keyexist = false;
std::map<MCONTACT, contact_data> hcontact_data;

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
	// {4227c050-8d97-48d2-91ec-6a952b3dab94}
	{ 0x4227c050, 0x8d97, 0x48d2, { 0x91, 0xec, 0x6a, 0x95, 0x2b, 0x3d, 0xab, 0x94 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}


INT_PTR LoadKey(WPARAM w, LPARAM l);
INT_PTR ToggleEncryption(WPARAM w, LPARAM l);
INT_PTR SendKey(WPARAM w, LPARAM l);
INT_PTR ExportGpGKeys(WPARAM w, LPARAM l);
INT_PTR ImportGpGKeys(WPARAM w, LPARAM l);

void init_vars()
{
	bAppendTags = db_get_b(NULL, szGPGModuleName, "bAppendTags", 0) != 0;
	bStripTags = db_get_b(NULL, szGPGModuleName, "bStripTags", 0) != 0;
	inopentag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szInOpenTag", _T("<GPGdec>"));
	inclosetag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szInCloseTag", _T("</GPGdec>"));
	outopentag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutOpenTag", _T("<GPGenc>"));
	outclosetag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutCloseTag", _T("</GPGenc>"));
	bDebugLog = db_get_b(NULL, szGPGModuleName, "bDebugLog", 0) != 0;
	bAutoExchange = db_get_b(NULL, szGPGModuleName, "bAutoExchange", 0) != 0;
	bSameAction = db_get_b(NULL, szGPGModuleName, "bSameAction", 0) != 0;
	password = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", _T(""));
	debuglog.init();
	bIsMiranda09 = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0) >= 0x00090001;
	bJabberAPI = db_get_b(NULL, szGPGModuleName, "bJabberAPI", bIsMiranda09) != 0;
	bPresenceSigning = db_get_b(NULL, szGPGModuleName, "bPresenceSigning", 0) != 0;
	bFileTransfers = db_get_b(NULL, szGPGModuleName, "bFileTransfers", 0) != 0;
	firstrun_rect.left = db_get_dw(NULL, szGPGModuleName, "FirstrunWindowX", 0);
	firstrun_rect.top = db_get_dw(NULL, szGPGModuleName, "FirstrunWindowY", 0);
	key_password_rect.left = db_get_dw(NULL, szGPGModuleName, "PasswordWindowX", 0);
	key_password_rect.top = db_get_dw(NULL, szGPGModuleName, "PasswordWindowY", 0);
	key_gen_rect.left = db_get_dw(NULL, szGPGModuleName, "KeyGenWindowX", 0);
	key_gen_rect.top = db_get_dw(NULL, szGPGModuleName, "KeyGenWindowY", 0);
	load_key_rect.left = db_get_dw(NULL, szGPGModuleName, "LoadKeyWindowX", 0);
	load_key_rect.top = db_get_dw(NULL, szGPGModuleName, "LoadKeyWindowY", 0);
	import_key_rect.left = db_get_dw(NULL, szGPGModuleName, "ImportKeyWindowX", 0);
	import_key_rect.top = db_get_dw(NULL, szGPGModuleName, "ImportKeyWindowY", 0);
	new_key_rect.left = db_get_dw(NULL, szGPGModuleName, "NewKeyWindowX", 0);
	new_key_rect.top = db_get_dw(NULL, szGPGModuleName, "NewKeyWindowY", 0);
	load_existing_key_rect.left = db_get_dw(NULL, szGPGModuleName, "LoadExistingKeyWindowX", 0);
	load_existing_key_rect.top = db_get_dw(NULL, szGPGModuleName, "LoadExistingKeyWindowY", 0);
	tabsrmm_used = isTabsrmmUsed();
	bold_font = CreateFont(14, 0, 0, 0, 600, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
}

static int OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	int GpgOptInit(WPARAM wParam,LPARAM lParam);
	int OnPreBuildContactMenu(WPARAM w, LPARAM l);
	INT_PTR RecvMsgSvc(WPARAM w, LPARAM l);
	INT_PTR SendMsgSvc(WPARAM w, LPARAM l);
	int HookSendMsg(WPARAM w, LPARAM l);
//	int TestHook(WPARAM w, LPARAM l);
	int GetJabberInterface(WPARAM w, LPARAM l);
	int onWindowEvent(WPARAM wParam, LPARAM lParam);
	int onIconPressed(WPARAM wParam, LPARAM lParam);
	int onExtraImageListRebuilding(WPARAM, LPARAM);
	int onExtraImageApplying(WPARAM wParam, LPARAM);
	int onProtoAck(WPARAM, LPARAM);
	INT_PTR onSendFile(WPARAM, LPARAM);
	void InitIconLib();

	void InitCheck();
	void FirstRun();
	FirstRun();
	if(!db_get_b(NULL, szGPGModuleName, "FirstRun", 1))
		InitCheck();
	InitIconLib();

	HICON IconLibGetIcon(const char* ident);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = szGPGModuleName;
	sid.flags = MBF_HIDDEN;
	sid.dwId = 0x00000001;
	sid.hIcon = IconLibGetIcon("secured");
	sid.szTooltip = LPGEN("GPG Turn off encryption");
	Srmm_AddIcon(&sid);

	sid.dwId = 0x00000002;
	sid.hIcon = IconLibGetIcon("unsecured");
	sid.szTooltip = LPGEN("GPG Turn on encryption");
	Srmm_AddIcon(&sid);

	if(bJabberAPI && bIsMiranda09)
		GetJabberInterface(0,0);

	HookEvent(ME_OPT_INITIALISE, GpgOptInit);
	HookEvent(ME_DB_EVENT_FILTER_ADD, HookSendMsg);
	if(bJabberAPI && bIsMiranda09)
		HookEvent(ME_PROTO_ACCLISTCHANGED, GetJabberInterface);

	HookEvent(ME_PROTO_ACK, onProtoAck);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildContactMenu);

	HookEvent(ME_MSG_WINDOWEVENT, onWindowEvent);
	HookEvent(ME_MSG_ICONPRESSED, onIconPressed);

	g_hCLIcon = ExtraIcon_Register(szGPGModuleName, Translate("GPG encryption status"), "secured", (MIRANDAHOOK)onExtraImageListRebuilding, (MIRANDAHOOK)onExtraImageApplying);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = szGPGModuleName;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);
	
	CreateProtoServiceFunction(szGPGModuleName, PSR_MESSAGE, RecvMsgSvc);
	CreateProtoServiceFunction(szGPGModuleName, PSS_MESSAGE, SendMsgSvc);
	CreateProtoServiceFunction(szGPGModuleName, PSS_FILE,    onSendFile);
	clean_temp_dir();
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	mir_getXI(&xi);	//TODO: check if we have access to api
	mir_getLP(&pluginInfo);
	init_vars();
	CreateServiceFunction("/LoadPubKey",LoadKey);
	CreateServiceFunction("/ToggleEncryption",ToggleEncryption);
	CreateServiceFunction("/SendKey",SendKey);
	CreateServiceFunction("/ExportGPGKeys",ExportGpGKeys);
	CreateServiceFunction("/ImportGPGKeys",ImportGpGKeys);

   // !!!!!!!! check it later
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position=-0x7FFFFFFF;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Load GPG public key");
	mi.pszService="/LoadPubKey";
	hLoadPubKey = Menu_AddContactMenuItem(&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFe;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Toggle GPG encryption");
	mi.pszService="/ToggleEncryption";
	hToggleEncryption = Menu_AddContactMenuItem(&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFd;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Send public key");
	mi.pszService="/SendKey";
	hSendKey = Menu_AddContactMenuItem(&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFe;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Export GPG Public keys");
	mi.pszService="/ExportGPGKeys";
	hExportGpgKeys = Menu_AddMainMenuItem(&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Import GPG Public keys");
	mi.pszService="/ImportGPGKeys";
	hImportGpgKeys = Menu_AddMainMenuItem(&mi);

	return 0;
}

extern list<wstring> transfers;
extern "C" int __declspec(dllexport) Unload(void)
{
	if(!transfers.empty())
	{
		for(list<wstring>::iterator p = transfers.begin(); p != transfers.end(); p++)
			if(!(*p).empty())
				boost::filesystem::remove((*p));
	}
	mir_free(inopentag);
	mir_free(inclosetag);
	mir_free(outopentag);
	mir_free(outclosetag);
	if(password)
		mir_free(password);
	clean_temp_dir();
	return 0;
}
