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
bool bAppendTags = false, bDebugLog = false, bJabberAPI = false, bPresenceSigning = false, bIsMiranda09 = false, bMetaContacts = false, bFileTransfers = false, bAutoExchange = false, bStripTags = false, tabsrmm_used = false;
TCHAR *inopentag = NULL, *inclosetag = NULL, *outopentag = NULL, *outclosetag = NULL, *password = NULL;

list <JabberAccount*> Accounts;

HINSTANCE hInst;
HANDLE hLoadPubKey = NULL, hToggleEncryption = NULL, hOnPreBuildContactMenu = NULL, hSendKey = NULL, g_hCLIcon = NULL, hExportGpgKeys = NULL, hImportGpgKeys = NULL;
RECT key_from_keyserver_rect = {0}, firstrun_rect = {0}, new_key_rect = {0}, key_gen_rect = {0}, load_key_rect = {0}, import_key_rect = {0}, key_password_rect = {0}, load_existing_key_rect = {0};
XML_API xi = {0};
int hLangpack = 0;
logtofile debuglog;
bool gpg_valid = false, gpg_keyexist = false;
std::map<HANDLE, contact_data> hcontact_data;


#define MIID_GPG { 0x4227c050, 0x8d97, 0x48d2, { 0x91, 0xec, 0x6a, 0x95, 0x2b, 0x3d, 0xab, 0x94 } }

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"GPG",
	PLUGIN_MAKE_VERSION(0,0,0,11),
	"New GPG encryption support plugin, based on code from old GPG plugin and SecureIM.",
	"sss",
	"sss123next@list.ru",
	"© 2010-2012 sss",
	"http://sss.chaoslab.ru/tracker/mim_plugs/",
	1,		//unicode
	MIID_GPG
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_GPG, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

INT_PTR LoadKey(WPARAM w, LPARAM l);
INT_PTR ToggleEncryption(WPARAM w, LPARAM l);
INT_PTR SendKey(WPARAM w, LPARAM l);
INT_PTR ExportGpGKeys(WPARAM w, LPARAM l);
INT_PTR ImportGpGKeys(WPARAM w, LPARAM l);

void init_vars()
{
	bAppendTags = DBGetContactSettingByte(NULL, szGPGModuleName, "bAppendTags", 0);
	bStripTags = DBGetContactSettingByte(NULL, szGPGModuleName, "bStripTags", 0);
	inopentag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szInOpenTag", _T("<GPGdec>"));
	inclosetag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szInCloseTag", _T("</GPGdec>"));
	outopentag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutOpenTag", _T("<GPGenc>"));
	outclosetag = UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutCloseTag", _T("</GPGenc>"));
	bDebugLog = DBGetContactSettingByte(NULL, szGPGModuleName, "bDebugLog", 0);
	bAutoExchange = DBGetContactSettingByte(NULL, szGPGModuleName, "bAutoExchange", 0);
	password = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", _T(""));
	debuglog.init();
	bJabberAPI = DBGetContactSettingByte(NULL, szGPGModuleName, "bJabberAPI", bIsMiranda09?1:0);
	bPresenceSigning = DBGetContactSettingByte(NULL, szGPGModuleName, "bPresenceSigning", 0);
	bFileTransfers = DBGetContactSettingByte(NULL, szGPGModuleName, "bFileTransfers", 0);
	firstrun_rect.left = DBGetContactSettingDword(NULL, szGPGModuleName, "FirstrunWindowX", 0);
	firstrun_rect.top = DBGetContactSettingDword(NULL, szGPGModuleName, "FirstrunWindowY", 0);
	key_password_rect.left = DBGetContactSettingDword(NULL, szGPGModuleName, "PasswordWindowX", 0);
	key_password_rect.top = DBGetContactSettingDword(NULL, szGPGModuleName, "PasswordWindowY", 0);
	key_gen_rect.left = DBGetContactSettingDword(NULL, szGPGModuleName, "KeyGenWindowX", 0);
	key_gen_rect.top = DBGetContactSettingDword(NULL, szGPGModuleName, "KeyGenWindowY", 0);
	load_key_rect.left = DBGetContactSettingDword(NULL, szGPGModuleName, "LoadKeyWindowX", 0);
	load_key_rect.top = DBGetContactSettingDword(NULL, szGPGModuleName, "LoadKeyWindowY", 0);
	import_key_rect.left = DBGetContactSettingDword(NULL, szGPGModuleName, "ImportKeyWindowX", 0);
	import_key_rect.top = DBGetContactSettingDword(NULL, szGPGModuleName, "ImportKeyWindowY", 0);
	new_key_rect.left = DBGetContactSettingDword(NULL, szGPGModuleName, "NewKeyWindowX", 0);
	new_key_rect.top = DBGetContactSettingDword(NULL, szGPGModuleName, "NewKeyWindowY", 0);
	load_existing_key_rect.left = DBGetContactSettingDword(NULL, szGPGModuleName, "LoadExistingKeyWindowX", 0);
	load_existing_key_rect.top = DBGetContactSettingDword(NULL, szGPGModuleName, "LoadExistingKeyWindowY", 0);
	tabsrmm_used = isTabsrmmUsed();
}

static int OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	int GpgOptInit(WPARAM wParam,LPARAM lParam);
	int OnPreBuildContactMenu(WPARAM w, LPARAM l);
	int RecvMsgSvc(WPARAM w, LPARAM l);
	int SendMsgSvc(WPARAM w, LPARAM l);
	int HookSendMsg(WPARAM w, LPARAM l);
//	int TestHook(WPARAM w, LPARAM l);
	int GetJabberInterface(WPARAM w, LPARAM l);
	int onWindowEvent(WPARAM wParam, LPARAM lParam);
	int onIconPressed(WPARAM wParam, LPARAM lParam);
	int onExtraImageListRebuilding(WPARAM, LPARAM);
	int onExtraImageApplying(WPARAM wParam, LPARAM);
	int onProtoAck(WPARAM, LPARAM);
	int onSendFile(WPARAM, LPARAM);
	void InitIconLib();

	void InitCheck();
	void FirstRun();
	bIsMiranda09 = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0) >= 0x00090001?true:false;
	FirstRun();
	InitCheck();
	InitIconLib();
	if(ServiceExists(MS_MSG_ADDICON)) 
	{
		HICON IconLibGetIcon(const char* ident);
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = szGPGModuleName;
		sid.flags = MBF_HIDDEN;
		sid.dwId = 0x00000001;
		sid.hIcon = IconLibGetIcon("secured");
		sid.szTooltip = Translate("GPG Turn off encryption");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
		ZeroMemory(&sid, sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.szModule = szGPGModuleName;
		sid.flags = MBF_HIDDEN;
		sid.dwId = 0x00000002;
		sid.hIcon = IconLibGetIcon("unsecured");
		sid.szTooltip = Translate("GPG Turn on encryption");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
	}


	bMetaContacts = ServiceExists(MS_MC_GETMETACONTACT);
	
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
	
	CreateProtoServiceFunction(szGPGModuleName, PSR_MESSAGE, (MIRANDASERVICE)RecvMsgSvc);
	CreateProtoServiceFunction(szGPGModuleName, PSS_MESSAGE, (MIRANDASERVICE)SendMsgSvc);
	CreateProtoServiceFunction(szGPGModuleName, PSR_MESSAGE"W", (MIRANDASERVICE)RecvMsgSvc);
	CreateProtoServiceFunction(szGPGModuleName, PSS_MESSAGE"W", (MIRANDASERVICE)SendMsgSvc);

	CreateProtoServiceFunction(szGPGModuleName, PSS_FILE, (MIRANDASERVICE)onSendFile);
	CreateProtoServiceFunction(szGPGModuleName, PSS_FILE"W", (MIRANDASERVICE)onSendFile);
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

	CLISTMENUITEM mi = {0};
	mi.cbSize=sizeof(mi);
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
	mi.position=-0x7FFFFFFe;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Send public key");
	mi.pszService="/SendKey";
	hSendKey = Menu_AddContactMenuItem(&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Export GPG Public keys from all users");
	mi.pszService="/ExportGPGKeys";
	hExportGpgKeys = Menu_AddMainMenuItem(&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName=LPGENT("Import GPG Public keys from all users");
	mi.pszService="/ImportGPGKeys";
	hImportGpgKeys = Menu_AddMainMenuItem(&mi);

	return 0;
}

extern list<wstring> transfers;
extern "C" int __declspec(dllexport) Unload(void)
{
//	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
//		DBDeleteContactSetting(hContact, szGPGModuleName, "KeyID_Prescense");
	if(!transfers.empty())
	{
		for(list<wstring>::iterator p = transfers.begin(); p != transfers.end(); p++)
			if(!(*p).empty())
				DeleteFile((*p).c_str());
	}
	mir_free(inopentag);
	mir_free(inclosetag);
	mir_free(outopentag);
	mir_free(outclosetag);
	if(password)
		mir_free(password);
	return 0;
}
