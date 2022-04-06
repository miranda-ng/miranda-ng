// Copyright © 2010-22 sss
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
int onExtraIconPressed(WPARAM, LPARAM, LPARAM);

void InitCheck();
void FirstRun();
void RemoveHandlers();

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
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bDebugLog(MODULENAME, "bDebugLog", false),
	bJabberAPI(MODULENAME, "bJabberAPI", true),
	bStripTags(MODULENAME, "bStripTags", false),
	bAppendTags(MODULENAME, "bAppendTags", false),
	bSameAction(MODULENAME, "bSameAction", false),
	bAutoExchange(MODULENAME, "bAutoExchange", false),
	bFileTransfers(MODULENAME, "bFileTransfers", false),
	bPresenceSigning(MODULENAME, "bPresenceSigning", false),
	bSendErrorMessages(MODULENAME, "bSendErrorMessages", false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR LoadKey(WPARAM, LPARAM);
INT_PTR SendKey(WPARAM, LPARAM);
INT_PTR ExportGpGKeys(WPARAM, LPARAM);
INT_PTR ImportGpGKeys(WPARAM, LPARAM);
INT_PTR ToggleEncryption(WPARAM, LPARAM);

void InitIconLib();

void init_vars()
{
	globals.wszInopentag = g_plugin.getMStringW("szInOpenTag", L"<GPGdec>");
	globals.wszInclosetag = g_plugin.getMStringW("szInCloseTag", L"</GPGdec>");
	globals.wszOutopentag = g_plugin.getMStringW("szOutOpenTag", L"<GPGenc>");
	globals.wszOutclosetag = g_plugin.getMStringW("szOutCloseTag", L"</GPGenc>");
	globals.wszPassword = g_plugin.getMStringW("szKeyPassword");
	globals.bold_font = CreateFont(14, 0, 0, 0, 600, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Arial");

	globals.debuglog.init();
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

	if (g_plugin.bJabberAPI) {
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

static int OnShutdown(WPARAM, LPARAM)
{
	RemoveHandlers();
	return 0;
}

static INT_PTR EventGetIcon(WPARAM flags, LPARAM)
{
	HICON hIcon = g_plugin.getIcon(IDI_SECURED);
	return (INT_PTR)((flags & LR_SHARED) ? hIcon : CopyIcon(hIcon));
}

static INT_PTR GetEventText(WPARAM pEvent, LPARAM datatype)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)pEvent;
	ptrW wszText(mir_utf8decodeW((char *)dbei->pBlob));
	return (datatype != DBVT_WCHAR) ? (INT_PTR)mir_u2a(wszText) : (INT_PTR)wszText.detach();
}

int CMPlugin::Load()
{
	DBEVENTTYPEDESCR dbEventType = {};
	dbEventType.module = MODULENAME;
	dbEventType.descr = "GPG service event";
	dbEventType.iconService = MODULENAME "/GetEventIcon";
	dbEventType.textService = MODULENAME "/GetEventText";
	DbEvent_RegisterType(&dbEventType);

	CreateServiceFunction(dbEventType.iconService, &EventGetIcon);
	CreateServiceFunction(dbEventType.textService, &GetEventText);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPreBuildContactMenu);
	HookEvent(ME_DB_EVENT_FILTER_ADD, HookSendMsg);
	HookEvent(ME_OPT_INITIALISE, GpgOptInit);
	HookEvent(ME_PROTO_ACK, onProtoAck);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);

	InitIconLib();
	init_vars();

	////////////////////////////////////////////////////////////////////////////////////////
	// Comtact menu items

	CMenuItem mi(&g_plugin);
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SECURED);

	SET_UID(mi, 0xbd22e3f8, 0xc19c, 0x45a8, 0xb7, 0x37, 0x6b, 0x3b, 0x27, 0xf0, 0x8c, 0xbb);
	mi.position = -0x7FFFFFFF;
	mi.name.a = LPGEN("Load public GPG key");
	mi.pszService = "/LoadPubKey";
	Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, LoadKey);

	SET_UID(mi, 0xc8008193, 0x56a9, 0x414a, 0x82, 0x98, 0x78, 0xe8, 0xa8, 0x84, 0x20, 0x67);
	mi.position = -0x7FFFFFFe;
	mi.name.a = LPGEN("Toggle GPG encryption");
	mi.pszService = "/ToggleEncryption";
	g_plugin.hToggleEncryption = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, ToggleEncryption);

	SET_UID(mi, 0x42bb535f, 0xd58e, 0x4edb, 0xbf, 0x2c, 0xfa, 0x9a, 0xbf, 0x1e, 0xb8, 0x69);
	mi.position = -0x7FFFFFFd;
	mi.name.a = LPGEN("Send public key");
	mi.pszService = "/SendKey";
	g_plugin.hSendKey = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, SendKey);

	////////////////////////////////////////////////////////////////////////////////////////
	// Main menu items

	SET_UID(mi, 0x0bac023bb, 0xd2e, 0x46e0, 0x93, 0x13, 0x7c, 0xf9, 0xf6, 0xb5, 0x02, 0xd1);
	mi.position = -0x7FFFFFFe;
	mi.name.a = "GPG";
	mi.root = Menu_AddMainMenuItem(&mi);
	mi.flags = CMIF_UNMOVABLE;

	SET_UID(mi, 0x33a204b2, 0xe3c0, 0x413b, 0xbf, 0xd8, 0x8b, 0x2e, 0x3d, 0xa0, 0xef, 0xa4);
	mi.position = -0x7FFFFFFe;
	mi.name.a = LPGEN("Export GPG Public keys");
	mi.pszService = "/ExportGPGKeys";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, ExportGpGKeys);

	SET_UID(mi, 0x627fcfc1, 0x4e60, 0x4428, 0xaf, 0x96, 0x11, 0x42, 0x24, 0xeb, 0x07, 0xea);
	mi.position = -0x7FFFFFFF;
	mi.name.a = LPGEN("Import GPG Public keys");
	mi.pszService = "/ImportGPGKeys";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, ImportGpGKeys);

	////////////////////////////////////////////////////////////////////////////////////////
	// Extra icon

	hCLIcon = ExtraIcon_RegisterIcolib(MODULENAME, Translate("GPG encryption status"), "secured", &onExtraIconPressed);
	for (auto &cc : Contacts())
		if (isContactHaveKey(cc))
			setSrmmIcon(cc);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern list<wstring> transfers;

int CMPlugin::Unload()
{
	for (auto p : transfers)
		if (!p.empty())
			boost::filesystem::remove(p);

	clean_temp_dir();
	return 0;
}
