/* 
Copyright (C) 2006-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "commons.h"


// Prototypes ///////////////////////////////////////////////////////////////////////////

// Service called by the main menu
#define MS_LISTENINGTO_MAINMENU				"ListeningTo/MainMenu"

// Service called by toptoolbar
#define MS_LISTENINGTO_TTB					"ListeningTo/TopToolBar"

// Services called by hotkeys
#define MS_LISTENINGTO_HOTKEYS_ENABLE		"ListeningTo/HotkeysEnable"
#define MS_LISTENINGTO_HOTKEYS_DISABLE		"ListeningTo/HotkeysDisable"
#define MS_LISTENINGTO_HOTKEYS_TOGGLE		"ListeningTo/HotkeysToggle"

int hLangpack; 

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"ListeningTo",
	PLUGIN_MAKE_VERSION(0,3,0,0),
	"Handle listening information to/for contacts",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2006-2009 Ricardo Pescuma Domenecci",
	"http://miranda-ng.org/",
	UNICODE_AWARE,		//doesn't replace anything built-in
	{ 0xf981f3f5, 0x35a, 0x444f, { 0x98, 0x92, 0xca, 0x72, 0x2c, 0x19, 0x5a, 0xda } } // {F981F3F5-035A-444f-9892-CA722C195ADA}
};


HINSTANCE hInst;

static HANDLE hEnableStateChangedEvent;
HANDLE hExtraIcon, hIcon1, hIcon2;
static HGENMENU hMainMenuGroup = NULL;
static HANDLE hListeningInfoChangedEvent = NULL;

static HANDLE hTTB = NULL;
static char *metacontacts_proto = NULL;
BOOL loaded = FALSE;
static UINT hTimer = 0;
static HANDLE hExtraImage = NULL;
static DWORD lastInfoSetTime = 0;

std::vector<ProtocolInfo> proto_itens;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int TopToolBarLoaded(WPARAM wParam, LPARAM lParam);
int ClistExtraListRebuild(WPARAM wParam, LPARAM lParam);
int SettingChanged(WPARAM wParam,LPARAM lParam);

INT_PTR MainMenuClicked(WPARAM wParam, LPARAM lParam);
BOOL    ListeningToEnabled(char *proto, BOOL ignoreGlobal = FALSE);
INT_PTR ListeningToEnabled(WPARAM wParam, LPARAM lParam);
INT_PTR EnableListeningTo(WPARAM wParam,LPARAM lParam);
INT_PTR GetTextFormat(WPARAM wParam,LPARAM lParam);
INT_PTR GetParsedFormat(WPARAM wParam,LPARAM lParam);
INT_PTR GetOverrideContactOption(WPARAM wParam,LPARAM lParam);
INT_PTR GetUnknownText(WPARAM wParam,LPARAM lParam);
INT_PTR SetNewSong(WPARAM wParam,LPARAM lParam);
void    SetExtraIcon(HANDLE hContact, BOOL set);
void    SetListeningInfos(LISTENINGTOINFO *lti);
INT_PTR HotkeysEnable(WPARAM wParam,LPARAM lParam);
INT_PTR HotkeysDisable(WPARAM wParam,LPARAM lParam);
INT_PTR HotkeysToggle(WPARAM wParam,LPARAM lParam);

TCHAR* VariablesParseInfo(ARGUMENTSINFO *ai);
TCHAR* VariablesParseType(ARGUMENTSINFO *ai);
TCHAR* VariablesParseArtist(ARGUMENTSINFO *ai);
TCHAR* VariablesParseAlbum(ARGUMENTSINFO *ai);
TCHAR* VariablesParseTitle(ARGUMENTSINFO *ai);
TCHAR* VariablesParseTrack(ARGUMENTSINFO *ai);
TCHAR* VariablesParseYear(ARGUMENTSINFO *ai);
TCHAR* VariablesParseGenre(ARGUMENTSINFO *ai);
TCHAR* VariablesParseLength(ARGUMENTSINFO *ai);
TCHAR* VariablesParsePlayer(ARGUMENTSINFO *ai);


#define XSTATUS_MUSIC 11

#define UNKNOWN(_X_) ( _X_ == NULL || _X_[0] == _T('\0') ? opts.unknown : _X_ )



// Functions ////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_LISTENINGTO, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void) 
{

	mir_getLP(&pluginInfo);

	CHECK_VERSION("Listening To")

	CoInitialize(NULL);

	// Services
	CreateServiceFunction(MS_LISTENINGTO_ENABLED, ListeningToEnabled);
	CreateServiceFunction(MS_LISTENINGTO_ENABLE, EnableListeningTo);
	CreateServiceFunction(MS_LISTENINGTO_GETTEXTFORMAT, GetTextFormat);
	CreateServiceFunction(MS_LISTENINGTO_GETPARSEDTEXT, GetParsedFormat);
	CreateServiceFunction(MS_LISTENINGTO_OVERRIDECONTACTOPTION, GetOverrideContactOption);
	CreateServiceFunction(MS_LISTENINGTO_GETUNKNOWNTEXT, GetUnknownText);
	CreateServiceFunction(MS_LISTENINGTO_MAINMENU, MainMenuClicked);
	CreateServiceFunction(MS_LISTENINGTO_SET_NEW_SONG, SetNewSong);
	CreateServiceFunction(MS_LISTENINGTO_HOTKEYS_ENABLE, HotkeysEnable);
	CreateServiceFunction(MS_LISTENINGTO_HOTKEYS_DISABLE, HotkeysDisable);
	CreateServiceFunction(MS_LISTENINGTO_HOTKEYS_TOGGLE, HotkeysToggle);
	
	// Hooks
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	hEnableStateChangedEvent = CreateHookableEvent(ME_LISTENINGTO_ENABLE_STATE_CHANGED);
	hListeningInfoChangedEvent = CreateHookableEvent(ME_LISTENINGTO_LISTENING_INFO_CHANGED);

	InitMusic();
	InitOptions();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{
	CoUninitialize();

	return 0;
}


int ProtoServiceExists(const char *szModule, const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}

void UpdateGlobalStatusMenus()
{
	BOOL enabled = ListeningToEnabled(NULL, TRUE);

	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(clmi);
	clmi.flags = CMIM_FLAGS 
			| (enabled ? CMIF_CHECKED : 0)
			| (opts.enable_sending ? 0 : CMIF_GRAYED);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto_itens[0].hMenu, (LPARAM) &clmi);

	if (hTTB != NULL)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM) hTTB, (LPARAM) (enabled ? TTBST_PUSHED : TTBST_RELEASED));
}


struct compareFunc : std::binary_function<const ProtocolInfo, const ProtocolInfo, bool>
{
	bool operator()(const ProtocolInfo &one, const ProtocolInfo &two) const
	{
		return lstrcmp(one.account, two.account) < 0;
	}
};


void RebuildMenu()
{
	std::sort(proto_itens.begin(), proto_itens.end(), compareFunc());

	for (unsigned int i = 1; i < proto_itens.size(); i++)
	{
		ProtocolInfo *info = &proto_itens[i];

		if (info->hMenu != NULL)
			CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM) info->hMenu, 0);

		TCHAR text[512];
		mir_sntprintf(text, MAX_REGS(text), TranslateT("Send to %s"), info->account);

		CLISTMENUITEM mi = {0};
		mi.cbSize = sizeof(mi);
		mi.position = 100000 + i;
		mi.pszPopupName = (char *) hMainMenuGroup;
		mi.popupPosition = 500080000 + i;
		mi.pszService = MS_LISTENINGTO_MAINMENU;
		mi.ptszName = text;
		mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR
				| (ListeningToEnabled(info->proto, TRUE) ? CMIF_CHECKED : 0)
				| (opts.enable_sending ? 0 : CMIF_GRAYED);

		info->hMenu = Menu_AddMainMenuItem(&mi);
	}

	UpdateGlobalStatusMenus();
}

void RegisterProtocol(char *proto, TCHAR *account)
{		
	if (!ProtoServiceExists(proto, PS_SET_LISTENINGTO) &&
		!ProtoServiceExists(proto, PS_ICQ_SETCUSTOMSTATUSEX))
		return;

	size_t id = proto_itens.size();
	proto_itens.resize(id+1);

	strncpy(proto_itens[id].proto, proto, MAX_REGS(proto_itens[id].proto));
	proto_itens[id].proto[MAX_REGS(proto_itens[id].proto)-1] = 0;

	lstrcpyn(proto_itens[id].account, account, MAX_REGS(proto_itens[id].account));

	proto_itens[id].hMenu = NULL;
	proto_itens[id].old_xstatus = 0;
	proto_itens[id].old_xstatus_name[0] = _T('\0');
	proto_itens[id].old_xstatus_message[0] = _T('\0');
}


int AccListChanged(WPARAM wParam, LPARAM lParam) 
{
	PROTOACCOUNT *proto = (PROTOACCOUNT *) lParam;
	if (proto == NULL || proto->type != PROTOTYPE_PROTOCOL)
		return 0;

	ProtocolInfo *info = GetProtoInfo(proto->szModuleName);
	if (info != NULL)
	{
		if (wParam == PRAC_UPGRADED || wParam == PRAC_CHANGED)
		{
			lstrcpyn(info->account, proto->tszAccountName, MAX_REGS(info->account));

			TCHAR text[512];
			mir_sntprintf(text, MAX_REGS(text), TranslateT("Send to %s"), info->account);

			CLISTMENUITEM clmi = {0};
			clmi.cbSize = sizeof(clmi);
			clmi.flags = CMIM_NAME | CMIF_TCHAR;
			clmi.ptszName = text;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) info->hMenu, (LPARAM) &clmi);
		} 
		else if (wParam == PRAC_REMOVED || (wParam == PRAC_CHECKED && !proto->bIsEnabled))
		{
			CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM) info->hMenu, 0);

			for(std::vector<ProtocolInfo>::iterator it = proto_itens.begin(); it != proto_itens.end(); ++it)
			{
				if (&(*it) == info)
				{
					proto_itens.erase(it);
					break;
				}
			}

			RebuildMenu();
		}
	}
	else
	{
		if (wParam == PRAC_ADDED || (wParam == PRAC_CHECKED && proto->bIsEnabled))
		{
			RegisterProtocol(proto->szModuleName, proto->tszAccountName);
			RebuildMenu();
		}
	}

	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	EnableDisablePlayers();

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

	hIcon1 = IcoLib_Register("listening_to_icon", "ListeningTo", "Listening to (enabled)", IDI_LISTENINGTO);
	hIcon2 = IcoLib_Register("listening_off_icon", "ListeningTo", "Listening to (disabled)", IDI_LISTENINGOFF);

	// Extra icon support
	hExtraIcon = ExtraIcon_Register(MODULE_NAME, "Listening to music", "listening_to_icon");
	if (hExtraIcon != NULL)
	{
		HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact != NULL)
		{
			char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
			if (proto != NULL)
			{
				DBVARIANT dbv = {0};
				if (!DBGetContactSettingTString(hContact, proto, "ListeningTo", &dbv))
				{
					if (dbv.ptszVal != NULL && dbv.ptszVal[0] != 0)
						SetExtraIcon(hContact, TRUE);
					
					DBFreeVariant(&dbv);
				}
			}

			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	}
	else if (hExtraIcon == NULL && ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
	{
		HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, ClistExtraListRebuild);
	}

	{
		CLISTMENUITEM mi = {0};
		mi.cbSize = sizeof(mi);

		// Add main menu item
		mi.position = 500080000;
		mi.ptszName = LPGENT("Listening to");
		mi.flags = CMIF_ROOTPOPUP | CMIF_ICONFROMICOLIB | CMIF_TCHAR;
		mi.icolibItem = hIcon1;
		hMainMenuGroup = Menu_AddMainMenuItem(&mi);

		mi.hParentMenu = hMainMenuGroup;
		mi.popupPosition = 500080000;
		mi.position = 0;
		mi.pszService = MS_LISTENINGTO_MAINMENU;
		mi.hIcon = NULL;

		// Add all protos
		mi.ptszName = LPGENT("Send to all protocols");
		mi.flags = CMIF_CHILDPOPUP  | CMIF_TCHAR
				| (ListeningToEnabled(NULL, TRUE) ? CMIF_CHECKED : 0)
				| (opts.enable_sending ? 0 : CMIF_GRAYED);
		proto_itens.resize(1);
		proto_itens[0].hMenu = Menu_AddMainMenuItem(&mi);
		proto_itens[0].proto[0] = 0;
		proto_itens[0].account[0] = 0;
		proto_itens[0].old_xstatus = 0;
		proto_itens[0].old_xstatus_name[0] = _T('\0');
		proto_itens[0].old_xstatus_message[0] = _T('\0');
	}

	// Add each proto

	if (ServiceExists(MS_PROTO_ENUMACCOUNTS))
	{
		PROTOACCOUNT **protos;
		int count;
		ProtoEnumAccounts(&count,&protos);

		for (int i = 0; i < count; i++)
		{
			if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;
			if (!protos[i]->bIsEnabled)
				continue;

			RegisterProtocol(protos[i]->szModuleName, protos[i]->tszAccountName);
		}

		HookEvent(ME_PROTO_ACCLISTCHANGED, AccListChanged);
	}

	RebuildMenu();

	HookEvent(ME_TTB_MODULELOADED, TopToolBarLoaded);

	// Variables support
	if (ServiceExists(MS_VARS_REGISTERTOKEN))
	{
		TOKENREGISTER tr = {0};
		tr.cbSize = sizeof(TOKENREGISTER);
		tr.memType = TR_MEM_MIRANDA;
		tr.flags = TRF_FREEMEM | TRF_PARSEFUNC | TRF_FIELD | TRF_TCHAR;

		tr.tszTokenString = _T("listening_info");
		tr.parseFunctionT = VariablesParseInfo;
		tr.szHelpText = "Listening info\tListening info as set in the options";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_type");
		tr.parseFunctionT = VariablesParseType;
		tr.szHelpText = "Listening info\tMedia type: Music, Video, etc";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_artist");
		tr.parseFunctionT = VariablesParseArtist;
		tr.szHelpText = "Listening info\tArtist name";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_album");
		tr.parseFunctionT = VariablesParseAlbum;
		tr.szHelpText = "Listening info\tAlbum name";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_title");
		tr.parseFunctionT = VariablesParseTitle;
		tr.szHelpText = "Listening info\tSong name";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_track");
		tr.parseFunctionT = VariablesParseTrack;
		tr.szHelpText = "Listening info\tTrack number";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_year");
		tr.parseFunctionT = VariablesParseYear;
		tr.szHelpText = "Listening info\tSong year";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_genre");
		tr.parseFunctionT = VariablesParseGenre;
		tr.szHelpText = "Listening info\tSong genre";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_length");
		tr.parseFunctionT = VariablesParseLength;
		tr.szHelpText = "Listening info\tSong length";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);

		tr.tszTokenString = _T("listening_player");
		tr.parseFunctionT = VariablesParsePlayer;
		tr.szHelpText = "Listening info\tPlayer name";
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);
	}

	// Hotkeys support
	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.pszSection = LPGEN("Listening to");

	hkd.pszService = MS_LISTENINGTO_HOTKEYS_ENABLE;
	hkd.pszName = "ListeningTo/EnableAll";
	hkd.pszDescription = LPGEN("Send to all protocols");
	Hotkey_Register(&hkd);

	hkd.pszService = MS_LISTENINGTO_HOTKEYS_DISABLE;
	hkd.pszName = "ListeningTo/DisableAll";
	hkd.pszDescription = LPGEN("Don't send to any protocols");
	Hotkey_Register(&hkd);

	hkd.pszService = MS_LISTENINGTO_HOTKEYS_TOGGLE;
	hkd.pszName = "ListeningTo/ToggleAll";
	hkd.pszDescription = LPGEN("Toggle send to all protocols");
	Hotkey_Register(&hkd);

	//
	SetListeningInfos(NULL);
	StartTimer();

	loaded = TRUE;

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	loaded = FALSE;

	if (hTimer != NULL)
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	DeInitOptions();

	DestroyHookableEvent(hEnableStateChangedEvent);
	DestroyHookableEvent(hListeningInfoChangedEvent);

	FreeMusic();
	return 0;
}

static INT_PTR TopToolBarClick(WPARAM wParam, LPARAM lParam)
{
	EnableListeningTo(NULL, !ListeningToEnabled(NULL, TRUE));
	return 0;
}

// Toptoolbar hook to put an icon in the toolbar
int TopToolBarLoaded(WPARAM wParam, LPARAM lParam) 
{
	BOOL enabled = ListeningToEnabled(NULL, TRUE);

	CreateServiceFunction(MS_LISTENINGTO_TTB, TopToolBarClick);

	TTBButton ttb = {0};
	ttb.cbSize = sizeof(ttb);
	ttb.hIconHandleDn = hIcon1;
	ttb.hIconHandleUp = hIcon2;
	ttb.pszService = MS_LISTENINGTO_TTB;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON | (enabled ? TTBBF_PUSHED : 0);
	ttb.name = LPGEN("Enable/Disable sending Listening To info (to all protocols)");
	ttb.pszTooltipDn = LPGEN("Disable ListeningTo (to all protocols)");
	ttb.pszTooltipUp = LPGEN("Enable ListeningTo (to all protocols)");
	hTTB = TopToolbar_AddButton(&ttb);

	return 0;
}

INT_PTR MainMenuClicked(WPARAM wParam, LPARAM lParam)
{
	if (!loaded)
		return -1;

	int pos = wParam == 0 ? 0 : wParam - 500080000;

	if (pos >= proto_itens.size() || pos < 0)
		return 0;

	EnableListeningTo((WPARAM) proto_itens[pos].proto, (LPARAM) !ListeningToEnabled(proto_itens[pos].proto, TRUE));
	return 0;
}

BOOL ListeningToEnabled(char *proto, BOOL ignoreGlobal) 
{
	if (!ignoreGlobal && !opts.enable_sending)
		return FALSE;

	if (proto == NULL || proto[0] == 0)
	{
		// Check all protocols
		BOOL enabled = TRUE;

		for (unsigned int i = 1; i < proto_itens.size(); ++i)
		{
			if (!ListeningToEnabled(proto_itens[i].proto, TRUE))
			{
				enabled = FALSE;
				break;
			}
		}

		return enabled;
	}
	else
	{
		char setting[256];
		mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
		return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, FALSE);
	}
}

INT_PTR ListeningToEnabled(WPARAM wParam, LPARAM lParam) 
{
	if (!loaded)
		return -1;

	return ListeningToEnabled((char *)wParam) ;
}

ProtocolInfo *GetProtoInfo(char *proto)
{
	for (unsigned int i = 1; i < proto_itens.size(); i++)
		if (strcmp(proto, proto_itens[i].proto) == 0)
			return &proto_itens[i];

	return NULL;
}

void SetListeningInfo(char *proto, LISTENINGTOINFO *lti)
{
	if (proto == NULL)
		return;

	if (!ListeningToEnabled(proto))
		lti = NULL;

	if (ProtoServiceExists(proto, PS_SET_LISTENINGTO))
		CallProtoService(proto, PS_SET_LISTENINGTO, 0, (LPARAM) lti);

	else if (ProtoServiceExists(proto, PS_ICQ_SETCUSTOMSTATUSEX)) {
		if (opts.xstatus_set == IGNORE_XSTATUS)
			return;

		int status;
		ICQ_CUSTOM_STATUS ics = {0};
		ics.cbSize = sizeof(ICQ_CUSTOM_STATUS);
		ics.status = &status;

		// Set or reset?
		if (lti == NULL)
		{
			// Reset -> only if is still in music xstatus
			ics.flags = CSSF_MASK_STATUS;
			if (CallProtoService(proto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM) &ics) || status != XSTATUS_MUSIC)
			{
				if (opts.xstatus_set == SET_XSTATUS)
				{
					ProtocolInfo *pi = GetProtoInfo(proto);
					if (pi != NULL) 
					{
						pi->old_xstatus = 0;
						pi->old_xstatus_name[0] = _T('\0');
						pi->old_xstatus_message[0] = _T('\0');
					}
				}
				return;
			}

			if (opts.xstatus_set == CHECK_XSTATUS_MUSIC)
			{
				// Set text to nothing
				TCHAR *fr[] = { 
					_T("listening"), opts.nothing
				};

				Buffer<TCHAR> name;
				ReplaceTemplate(&name, NULL, opts.xstatus_name, fr, MAX_REGS(fr));
				Buffer<TCHAR> msg;
				ReplaceTemplate(&msg, NULL, opts.xstatus_message, fr, MAX_REGS(fr));

				ics.flags = CSSF_TCHAR | CSSF_MASK_STATUS |	CSSF_MASK_NAME | CSSF_MASK_MESSAGE;
				ics.ptszName = name.str;
				ics.ptszMessage = msg.str;

				CallProtoService(proto, PS_ICQ_SETCUSTOMSTATUSEX, 0, (LPARAM) &ics);
			}
			else if (opts.xstatus_set == CHECK_XSTATUS)
			{
				status = 0;
				ics.flags = CSSF_MASK_STATUS;

				CallProtoService(proto, PS_ICQ_SETCUSTOMSTATUSEX, 0, (LPARAM) &ics);
			}
			else
			{
				// Set to old text
				ProtocolInfo *pi = GetProtoInfo(proto);
				if (pi != NULL) 
				{
					ics.flags = CSSF_TCHAR | CSSF_MASK_STATUS |	CSSF_MASK_NAME | CSSF_MASK_MESSAGE;
					ics.status = &pi->old_xstatus;
					ics.ptszName = pi->old_xstatus_name;
					ics.ptszMessage = pi->old_xstatus_message;
				}
				else
				{
					status = 0;
					ics.flags = CSSF_MASK_STATUS;
				}

				CallProtoService(proto, PS_ICQ_SETCUSTOMSTATUSEX, 0, (LPARAM) &ics);

				if (pi != NULL) 
				{
					pi->old_xstatus = 0;
					pi->old_xstatus_name[0] = _T('\0');
					pi->old_xstatus_message[0] = _T('\0');
				}
			}
		}
		else
		{
			// Set it
			if (opts.xstatus_set == CHECK_XSTATUS_MUSIC)
			{
				ics.flags = CSSF_MASK_STATUS;
				if (CallProtoService(proto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM) &ics) || status != XSTATUS_MUSIC)
					return;
			}
			else if (opts.xstatus_set == CHECK_XSTATUS)
			{
				ics.flags = CSSF_MASK_STATUS;
				if (!CallProtoService(proto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM) &ics) && status != XSTATUS_MUSIC && status != 0)
					return;
			}
			else
			{
				// Store old data
				ics.flags = CSSF_MASK_STATUS;
				if (!CallProtoService(proto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM) &ics) && status != XSTATUS_MUSIC)
				{
					ProtocolInfo *pi = GetProtoInfo(proto);
					if (pi != NULL)
					{
						ics.flags = CSSF_TCHAR | CSSF_MASK_STATUS |	CSSF_MASK_NAME | CSSF_MASK_MESSAGE;
						ics.status = &pi->old_xstatus;
						ics.ptszName = pi->old_xstatus_name;
						ics.ptszMessage = pi->old_xstatus_message;

						CallProtoService(proto, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM) &ics);
					}
				}
			}

			TCHAR *fr[] = { 
				_T("listening"), (TCHAR *) GetParsedFormat(0, (WPARAM) lti),
				_T("artist"), UNKNOWN(lti->ptszArtist),
				_T("album"), UNKNOWN(lti->ptszAlbum),
				_T("title"), UNKNOWN(lti->ptszTitle),
				_T("track"), UNKNOWN(lti->ptszTrack),
				_T("year"), UNKNOWN(lti->ptszYear),
				_T("genre"), UNKNOWN(lti->ptszGenre),
				_T("length"), UNKNOWN(lti->ptszLength),
				_T("player"), UNKNOWN(lti->ptszPlayer),
				_T("type"), UNKNOWN(lti->ptszType)
			};

			Buffer<TCHAR> name;
			ReplaceTemplate(&name, NULL, opts.xstatus_name, fr, MAX_REGS(fr));
			Buffer<TCHAR> msg;
			ReplaceTemplate(&msg, NULL, opts.xstatus_message, fr, MAX_REGS(fr));

			status = XSTATUS_MUSIC;
			ics.flags = CSSF_TCHAR | CSSF_MASK_STATUS |	CSSF_MASK_NAME | CSSF_MASK_MESSAGE;
			ics.status = &status;
			ics.ptszName = name.str;
			ics.ptszMessage = msg.str;

			CallProtoService(proto, PS_ICQ_SETCUSTOMSTATUSEX, 0, (LPARAM) &ics);

			mir_free(fr[1]);
		}
	} 
	else if (ProtoServiceExists(proto, PS_SETAWAYMSG)) 
	{
		if (lti == NULL)
		{
			int status = CallProtoService(proto, PS_GETSTATUS, 0, 0);
			CallProtoService(proto, PS_SETAWAYMSG, (WPARAM) status, 0);
		}
		else 
		{
			TCHAR *fr = (TCHAR *)GetParsedFormat(0, (WPARAM) lti);
			char *info = mir_t2a(fr);
			int status = CallProtoService(proto, PS_GETSTATUS, 0, 0);
			CallProtoService(proto, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)info);

			mir_free(fr);
			mir_free(info);
		}
	}
}

INT_PTR EnableListeningTo(WPARAM wParam,LPARAM lParam) 
{
	if (!loaded)
		return -1;

	char *proto = (char *)wParam;

	if (proto == NULL || proto[0] == 0)
	{
		// For all protocols
		for (unsigned int i = 1; i < proto_itens.size(); ++i)
		{
			EnableListeningTo((WPARAM) proto_itens[i].proto, lParam);
		}
	}
	else
	{
		if (!ProtoServiceExists(proto, PS_SET_LISTENINGTO) &&
			!ProtoServiceExists(proto, PS_ICQ_SETCUSTOMSTATUSEX) &&
			!ProtoServiceExists(proto, PS_SETAWAYMSG)) // by yaho
			return 0;

		char setting[256];
		mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
		DBWriteContactSettingByte(NULL, MODULE_NAME, setting, (BOOL) lParam);

		// Modify menu info
		ProtocolInfo *info = GetProtoInfo(proto);
		if (info != NULL)
		{
			CLISTMENUITEM clmi = {0};
			clmi.cbSize = sizeof(clmi);
			clmi.flags = CMIM_FLAGS 
					| (lParam ? CMIF_CHECKED : 0)
					| (opts.enable_sending ? 0 : CMIF_GRAYED);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) info->hMenu, (LPARAM) &clmi);

			if (!opts.enable_sending || !lParam)
				SetListeningInfo(proto, NULL);
			else
				SetListeningInfo(proto, GetListeningInfo());
		}

		// Set all protos info
		UpdateGlobalStatusMenus();
	}

	StartTimer();

	NotifyEventHooks(hEnableStateChangedEvent, wParam, lParam);

	return 0;
}

INT_PTR HotkeysEnable(WPARAM wParam,LPARAM lParam) 
{
	return EnableListeningTo(lParam, TRUE);
}

INT_PTR HotkeysDisable(WPARAM wParam,LPARAM lParam) 
{
	return EnableListeningTo(lParam, FALSE);
}

INT_PTR HotkeysToggle(WPARAM wParam,LPARAM lParam) 
{
	return EnableListeningTo(lParam, !ListeningToEnabled((char *)lParam, TRUE));
}

INT_PTR GetTextFormat(WPARAM wParam,LPARAM lParam) 
{
	if (!loaded)
		return NULL;

	return ( INT_PTR )mir_tstrdup(opts.templ);
}

INT_PTR GetParsedFormat(WPARAM wParam,LPARAM lParam) 
{
	if (!loaded)
		return NULL;

	LISTENINGTOINFO *lti = (LISTENINGTOINFO *) lParam;

	if (lti == NULL)
		return NULL;

	TCHAR *fr[] = { 
		_T("artist"), UNKNOWN(lti->ptszArtist),
		_T("album"), UNKNOWN(lti->ptszAlbum),
		_T("title"), UNKNOWN(lti->ptszTitle),
		_T("track"), UNKNOWN(lti->ptszTrack),
		_T("year"), UNKNOWN(lti->ptszYear),
		_T("genre"), UNKNOWN(lti->ptszGenre),
		_T("length"), UNKNOWN(lti->ptszLength),
		_T("player"), UNKNOWN(lti->ptszPlayer),
		_T("type"), UNKNOWN(lti->ptszType)
	};

	Buffer<TCHAR> ret;
	ReplaceTemplate(&ret, NULL, opts.templ, fr, MAX_REGS(fr));
	return (int) ret.detach();
}

INT_PTR GetOverrideContactOption(WPARAM wParam,LPARAM lParam) 
{
	return ( INT_PTR )opts.override_contact_template;
}


INT_PTR GetUnknownText(WPARAM wParam,LPARAM lParam) 
{
	return ( INT_PTR )opts.unknown;
}

void SetListeningInfos(LISTENINGTOINFO *lti)
{
	for (unsigned int i = 1; i < proto_itens.size(); ++i)
		SetListeningInfo(proto_itens[i].proto, lti);

	TCHAR *fr = NULL; 
	char *info = NULL;

	if (lti) {
		fr = (TCHAR *)GetParsedFormat(0, (WPARAM) lti);
		if (fr) info = mir_t2a(fr);
	}

	NotifyEventHooks(hListeningInfoChangedEvent, (WPARAM)info, (LPARAM)NULL);

	if (lti) {
		if (fr) mir_free(fr);
		if (info) mir_free(info);
	}
}

static void CALLBACK GetInfoTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (hTimer != NULL) {
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	// Check if we can set it now...
	DWORD now = GetTickCount();
	if (now < lastInfoSetTime + MIN_TIME_BEETWEEN_SETS) {
		hTimer = SetTimer(NULL, NULL, lastInfoSetTime + MIN_TIME_BEETWEEN_SETS - now, GetInfoTimer);
		return;
	}
	lastInfoSetTime = GetTickCount(); // TODO Move this to inside the if that really sets

	if (!opts.enable_sending) {
		SetListeningInfos(NULL);
		return;
	}

	// Set it
	int changed = ChangedListeningInfo();
	if (changed < 0)
		SetListeningInfos(NULL);
	else if (changed > 0)
		SetListeningInfos(GetListeningInfo());

	StartTimer();
}

void StartTimer()
{
	// See if any protocol want Listening info
	BOOL want = FALSE;

	if (opts.enable_sending)
	{
		if (!players[WATRACK]->enabled)
		{
			// See if any player needs it
			BOOL needPoll = FALSE;
			int i;
			for (i = FIRST_PLAYER; i < NUM_PLAYERS; i++)
			{
				if (players[i]->needPoll)
				{
					needPoll = TRUE;
					break;
				}
			}

			if (needPoll)
			{
				// Now see protocols
				for (unsigned int i = 1; i < proto_itens.size(); ++i)
				{
					if (ListeningToEnabled(proto_itens[i].proto))
					{
						want = TRUE;
						break;
					}
				}
			}
		}
	}

	if (want)
	{
		if (hTimer == NULL)
			hTimer = SetTimer(NULL, NULL, opts.time_to_pool * 1000, GetInfoTimer);
	}
	else
	{
		if (hTimer != NULL)
		{
			KillTimer(NULL, hTimer);
			hTimer = NULL;

			// To be sure that no one was left behind
//			m_log(_T("StartTimer"), _T("To be sure that no one was left behind"));
			SetListeningInfos(NULL);
		}
	}
}

void HasNewListeningInfo()
{
	if (hTimer != NULL)
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	hTimer = SetTimer(NULL, NULL, 100, GetInfoTimer);
}


int ClistExtraListRebuild(WPARAM wParam, LPARAM lParam)
{
	HICON hIcon = Skin_GetIconByHandle(hIcon1);

	hExtraImage = (HANDLE) CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM) hIcon, 0);

	Skin_ReleaseIcon(hIcon);

	return 0;
}

void SetExtraIcon(HANDLE hContact, BOOL set)
{
	if (hExtraIcon != NULL)
	{
		ExtraIcon_SetIcon(hExtraIcon, hContact, set ? "listening_to_icon" : NULL);
	}
	else if (opts.show_adv_icon && hExtraImage != NULL)
	{
		IconExtraColumn iec;
		iec.cbSize = sizeof(iec);
		iec.hImage = set ? hExtraImage : (HANDLE)-1;
		if (opts.adv_icon_slot < 2)
		{
			iec.ColumnType = opts.adv_icon_slot + EXTRA_ICON_ADV1;
		}
		else 
		{
			int first = CallService(MS_CLUI_GETCAPS, 0, CLUIF2_USEREXTRASTART);
			iec.ColumnType = opts.adv_icon_slot - 2 + first;
		}

		CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)hContact, (LPARAM)&iec);
	}
}

int SettingChanged(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (strcmp(cws->szSetting, "ListeningTo") != 0)
		return 0;

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL)
		return 0;

	if (strcmp(cws->szModule, proto) != 0)
		return 0;

	if (cws->value.type == DBVT_DELETED || cws->value.ptszVal == NULL || cws->value.ptszVal[0] == 0)
		SetExtraIcon(hContact, FALSE);
	else
		SetExtraIcon(hContact, TRUE);

	return 0;
}

INT_PTR SetNewSong(WPARAM wParam,LPARAM lParam)
{
	if (lParam == NULL)
		return -1;

	if (lParam == LISTENINGTO_ANSI) {
		CharToWchar data((char *) wParam);
		((GenericPlayer *) players[GENERIC])->NewData(data, wcslen(data));
	}
	else {
		WCHAR *data = (WCHAR *) wParam;
		((GenericPlayer *) players[GENERIC])->NewData(data, wcslen(data));
	}

	return 0;
}

TCHAR* VariablesParseInfo(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	LISTENINGTOINFO *lti = GetListeningInfo();
	if (lti == NULL)
	{
		ai->flags = AIF_FALSE;
		return mir_tstrdup(_T(""));
	}

	TCHAR *fr[] = { 
		_T("artist"), UNKNOWN(lti->ptszArtist),
		_T("album"), UNKNOWN(lti->ptszAlbum),
		_T("title"), UNKNOWN(lti->ptszTitle),
		_T("track"), UNKNOWN(lti->ptszTrack),
		_T("year"), UNKNOWN(lti->ptszYear),
		_T("genre"), UNKNOWN(lti->ptszGenre),
		_T("length"), UNKNOWN(lti->ptszLength),
		_T("player"), UNKNOWN(lti->ptszPlayer),
		_T("type"), UNKNOWN(lti->ptszType)
	};

	Buffer<TCHAR> ret;
	ReplaceTemplate(&ret, NULL, opts.templ, fr, MAX_REGS(fr));
	return ret.detach();
}

#define VARIABLES_PARSE_BODY(__field__) \
	if (ai->cbSize < sizeof(ARGUMENTSINFO)) \
		return NULL; \
	\
	LISTENINGTOINFO *lti = GetListeningInfo(); \
	if (lti == NULL) \
	{ \
		ai->flags = AIF_FALSE; \
		return mir_tstrdup(_T("")); \
	} \
	else if (IsEmpty(lti->__field__))  \
	{ \
		ai->flags = AIF_FALSE; \
		return mir_tstrdup(opts.unknown); \
	} \
	else \
	{ \
		ai->flags = AIF_DONTPARSE; \
		TCHAR *ret = mir_tstrdup(lti->__field__); \
		return ret; \
	}


TCHAR* VariablesParseType(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszType);
}

TCHAR* VariablesParseArtist(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszArtist);
}

TCHAR* VariablesParseAlbum(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszAlbum);
}

TCHAR* VariablesParseTitle(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszTitle);
}

TCHAR* VariablesParseTrack(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszTrack);
}

TCHAR* VariablesParseYear(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszYear);
}

TCHAR* VariablesParseGenre(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszGenre);
}

TCHAR* VariablesParseLength(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszLength);
}

TCHAR* VariablesParsePlayer(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszPlayer);
}
