// Copyright © 2010-19 sss
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

#include "stdafx.h"

int GpgOptInit(WPARAM, LPARAM);
int OnPreBuildContactMenu(WPARAM, LPARAM);

INT_PTR RecvMsgSvc(WPARAM, LPARAM);
INT_PTR SendMsgSvc(WPARAM, LPARAM);
INT_PTR onSendFile(WPARAM, LPARAM);

int HookSendMsg(WPARAM, LPARAM);
int GetJabberInterface(WPARAM, LPARAM);
int onProtoAck(WPARAM, LPARAM);
int onWindowEvent(WPARAM, LPARAM);
int onIconPressed(WPARAM, LPARAM);

void InitCheck();
void FirstRun();

// global variables
CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {4227c050-8d97-48d2-91ec-6a952b3dab94}
	{ 0x4227c050, 0x8d97, 0x48d2, { 0x91, 0xec, 0x6a, 0x95, 0x2b, 0x3d, 0xab, 0x94 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR LoadKey(WPARAM, LPARAM);
INT_PTR SendKey(WPARAM, LPARAM);
INT_PTR ExportGpGKeys(WPARAM, LPARAM);
INT_PTR ImportGpGKeys(WPARAM, LPARAM);
INT_PTR ToggleEncryption(WPARAM, LPARAM);

int onExtraImageApplying(WPARAM, LPARAM);
int onExtraImageListRebuilding(WPARAM, LPARAM);

void InitIconLib();

void init_vars()
{
	globals.bAppendTags = g_plugin.getByte("bAppendTags", 0) != 0;
	globals.bStripTags = g_plugin.getByte("bStripTags", 0) != 0;
	globals.inopentag = db_get_wsa(0, MODULENAME, "szInOpenTag", L"<GPGdec>");
	globals.inclosetag = db_get_wsa(0, MODULENAME, "szInCloseTag", L"</GPGdec>");
	globals.outopentag = db_get_wsa(0, MODULENAME, "szOutOpenTag", L"<GPGenc>");
	globals.outclosetag = db_get_wsa(0, MODULENAME, "szOutCloseTag", L"</GPGenc>");
	globals.bDebugLog = g_plugin.getByte("bDebugLog", 0) != 0;
	globals.bAutoExchange = g_plugin.getByte("bAutoExchange", 0) != 0;
	globals.bSameAction = g_plugin.getByte("bSameAction", 0) != 0;
	globals.password = db_get_wsa(0, MODULENAME, "szKeyPassword", L"");
	globals.debuglog.init();
	globals.bJabberAPI = g_plugin.getByte("bJabberAPI", true) != 0;
	globals.bPresenceSigning = g_plugin.getByte("bPresenceSigning", 0) != 0;
	globals.bFileTransfers = g_plugin.getByte("bFileTransfers", 0) != 0;
	globals.firstrun_rect.left = g_plugin.getDword("FirstrunWindowX", 0);
	globals.firstrun_rect.top = g_plugin.getDword("FirstrunWindowY", 0);
	globals.key_password_rect.left = g_plugin.getDword("PasswordWindowX", 0);
	globals.key_password_rect.top = g_plugin.getDword("PasswordWindowY", 0);
	globals.key_gen_rect.left = g_plugin.getDword("KeyGenWindowX", 0);
	globals.key_gen_rect.top = g_plugin.getDword("KeyGenWindowY", 0);
	globals.load_key_rect.left = g_plugin.getDword("LoadKeyWindowX", 0);
	globals.load_key_rect.top = g_plugin.getDword("LoadKeyWindowY", 0);
	globals.import_key_rect.left = g_plugin.getDword("ImportKeyWindowX", 0);
	globals.import_key_rect.top = g_plugin.getDword("ImportKeyWindowY", 0);
	globals.new_key_rect.left = g_plugin.getDword("NewKeyWindowX", 0);
	globals.new_key_rect.top = g_plugin.getDword("NewKeyWindowY", 0);
	globals.load_existing_key_rect.left = g_plugin.getDword("LoadExistingKeyWindowX", 0);
	globals.load_existing_key_rect.top = g_plugin.getDword("LoadExistingKeyWindowY", 0);
	globals.tabsrmm_used = isTabsrmmUsed();
	globals.bold_font = CreateFont(14, 0, 0, 0, 600, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Arial");
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	FirstRun();
	if (!g_plugin.getByte("FirstRun", 1))
		InitCheck();

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.flags = MBF_HIDDEN;
	sid.dwId = 0x00000001;
	sid.hIcon = IcoLib_GetIcon("secured");
	sid.szTooltip.a = LPGEN("GPG Turn off encryption");
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = 0x00000002;
	sid.hIcon = IcoLib_GetIcon("unsecured");
	sid.szTooltip.a = LPGEN("GPG Turn on encryption");
	Srmm_AddIcon(&sid, &g_plugin);

	if (globals.bJabberAPI) {
		GetJabberInterface(0, 0);
		HookEvent(ME_PROTO_ACCLISTCHANGED, GetJabberInterface);
	}

	HookEvent(ME_MSG_WINDOWEVENT, onWindowEvent);
	HookEvent(ME_MSG_ICONPRESSED, onIconPressed);

	Proto_RegisterModule(PROTOTYPE_ENCRYPTION, MODULENAME);

	CreateProtoServiceFunction(MODULENAME, PSR_MESSAGE, RecvMsgSvc);
	CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE, SendMsgSvc);
	CreateProtoServiceFunction(MODULENAME, PSS_FILE, onSendFile);
	clean_temp_dir();
	return 0;
}

int CMPlugin::Load()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildContactMenu);
	HookEvent(ME_DB_EVENT_FILTER_ADD, HookSendMsg);
	HookEvent(ME_OPT_INITIALISE, GpgOptInit);
	HookEvent(ME_PROTO_ACK, onProtoAck);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	init_vars();
	CreateServiceFunction("/LoadPubKey", LoadKey);
	CreateServiceFunction("/ToggleEncryption", ToggleEncryption);
	CreateServiceFunction("/SendKey", SendKey);
	CreateServiceFunction("/ExportGPGKeys", ExportGpGKeys);
	CreateServiceFunction("/ImportGPGKeys", ImportGpGKeys);

	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0xbd22e3f8, 0xc19c, 0x45a8, 0xb7, 0x37, 0x6b, 0x3b, 0x27, 0xf0, 0x8c, 0xbb);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.w = LPGENW("Load public GPG key");
	mi.pszService = "/LoadPubKey";
	globals.hLoadPubKey = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xc8008193, 0x56a9, 0x414a, 0x82, 0x98, 0x78, 0xe8, 0xa8, 0x84, 0x20, 0x67);
	mi.position = -0x7FFFFFFe;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.w = LPGENW("Toggle GPG encryption");
	mi.pszService = "/ToggleEncryption";
	globals.hToggleEncryption = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x42bb535f, 0xd58e, 0x4edb, 0xbf, 0x2c, 0xfa, 0x9a, 0xbf, 0x1e, 0xb8, 0x69);
	mi.position = -0x7FFFFFFd;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.w = LPGENW("Send public key");
	mi.pszService = "/SendKey";
	globals.hSendKey = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x33a204b2, 0xe3c0, 0x413b, 0xbf, 0xd8, 0x8b, 0x2e, 0x3d, 0xa0, 0xef, 0xa4);
	mi.position = -0x7FFFFFFe;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.w = LPGENW("Export GPG Public keys");
	mi.pszService = "/ExportGPGKeys";
	globals.hExportGpgKeys = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x627fcfc1, 0x4e60, 0x4428, 0xaf, 0x96, 0x11, 0x42, 0x24, 0xeb, 0x7, 0xea);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.w = LPGENW("Import GPG Public keys");
	mi.pszService = "/ImportGPGKeys";
	globals.hImportGpgKeys = Menu_AddMainMenuItem(&mi);

	InitIconLib();

	globals.g_hCLIcon = ExtraIcon_RegisterCallback(MODULENAME, Translate("GPG encryption status"), "secured", onExtraImageListRebuilding, onExtraImageApplying);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern list<wstring> transfers;
int CMPlugin::Unload()
{
	for (auto p : transfers)
		if (!p.empty())
			boost::filesystem::remove(p);

	mir_free(globals.inopentag);
	mir_free(globals.inclosetag);
	mir_free(globals.outopentag);
	mir_free(globals.outclosetag);
	if (globals.password)
		mir_free(globals.password);
	clean_temp_dir();
	return 0;
}
