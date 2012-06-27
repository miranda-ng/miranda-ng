/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"
#include <m_plugins.h>

// other static variables
static BOOL bServiceMode = FALSE;
static DWORD  mainThreadId;
static HANDLE hMainThread;
static HANDLE hMissingService;

void ParseCommandLine();		// core: IDD_WAITRESTART
int LoadSystemModule(void);		// core: m_system.h services
int LoadNewPluginsModuleInfos(void); // core: preloading plugins
int LoadNewPluginsModule(void);	// core: N.O. plugins
int LoadSslModule(void);
int LoadNetlibModule(void);		// core: network
void NetlibInitSsl(void);
int LoadLangpackModule(void);	// core: translation
int LoadProtocolsModule(void);	// core: protocol manager
int LoadAccountsModule(void);    // core: account manager
int LoadIgnoreModule(void);		// protocol filter: ignore

int LoadSendRecvUrlModule(void);	//send/recv
int LoadSendRecvEMailModule(void);	//send/recv
int LoadSendRecvAuthModule(void);	//send/recv
int LoadSendRecvFileModule(void);	//send/recv

int LoadContactListModule(void);// ui: clist
int LoadOptionsModule(void);	// ui: options dialog
int LoadFindAddModule(void);	// ui: search/add users
int LoadSkinIcons(void);
int LoadSkinSounds(void);
int LoadSkinHotkeys(void);
int LoadHelpModule(void);		// ui: help stuff
int LoadUserInfoModule(void);	// ui: user info
int LoadHistoryModule(void);	// ui: history viewer
int LoadAwayMsgModule(void);	// ui: setting away messages
int LoadVisibilityModule(void);	// ui: visibility control
int LoadCLUIModule(void);		// ui: CList UI
int LoadPluginOptionsModule(void);	// ui: plugin viewer
int LoadAddContactModule(void);	// ui: authcontrol contacts
int LoadIdleModule(void);		// rnd: report idle information
int LoadAutoAwayModule(void);	// ui: away
int LoadUserOnlineModule(void);	// ui: online alert
int LoadUtilsModule(void);		// ui: utils (has a few window classes, like HyperLink)
int LoadCLCModule(void);		// window class: CLC control
int LoadButtonModule(void);		// window class: button class
int LoadContactsModule(void);    // random: contact
int LoadFontserviceModule(void); // ui: font manager
int LoadIcoLibModule(void);   // ui: icons manager
int LoadUpdateNotifyModule(void); // random: update notification
int LoadServiceModePlugin(void);
int LoadErrorsModule();

void UnloadUtilsModule(void);
void UnloadButtonModule(void);
void UnloadClcModule(void);
void UnloadContactListModule(void);
void UnloadEventsModule(void);
void UnloadIdleModule(void);
void UnloadSslModule(void);
void UnloadNetlibModule(void);
void UnloadNewPlugins(void);
void UnloadUpdateNotifyModule(void);
void UnloadIcoLibModule(void);
void UnloadSkinSounds(void);
void UnloadSkinHotkeys(void);
void UnloadProtocolsModule(void);
void UnloadAccountsModule(void);
void UnloadErrorsModule(void);

int LoadIcoTabsModule();
int LoadHeaderbarModule();
int LoadDescButtonModule();
int LoadDefaultModules(void)
{
    //load order is very important for these
	if ( LoadSystemModule()) return 1;
	if ( LoadLangpackModule()) return 1;		// langpack will be a system module in the new order so this is moved here
	ParseCommandLine();						// IDD_WAITRESTART need langpack  so this is moved here
	if ( LoadUtilsModule()) return 1;		//order not important for this, but no dependencies and no point in pluginising
	if ( LoadIcoTabsModule()) return 1;
	if ( LoadHeaderbarModule()) return 1;
	if ( LoadNewPluginsModuleInfos()) return 1;

	// database is available here
	if ( LoadButtonModule()) return 1;
	if ( LoadIcoLibModule()) return 1;
	if ( LoadSkinIcons()) return 1;

//	if ( LoadErrorsModule()) return 1;

	bServiceMode = LoadServiceModePlugin();
	switch (bServiceMode)
	{
		case 1:	return 0; // stop loading here
		case 0: break;
		default: return 1;
	}

	//this info will be available at LoadNewPluginsModule()
	INT_PTR *disableDefaultModule=(INT_PTR*)CallService(MS_PLUGINS_GETDISABLEDEFAULTARRAY, 0, 0);

	if ( LoadSkinSounds()) return 1;
	if ( LoadSkinHotkeys()) return 1;
	if ( LoadFontserviceModule()) return 1;

	if ( LoadDescButtonModule()) return 1;
	if ( LoadOptionsModule()) return 1;
	if ( LoadNetlibModule()) return 1;
	if ( LoadProtocolsModule()) return 1;
	    LoadDbAccounts();                    // retrieves the account array from a database
	if ( LoadContactsModule()) return 1;
	if ( LoadContactListModule()) return 1;
	if ( LoadAddContactModule()) return 1;
	if ( LoadNewPluginsModule()) return 1;    // will call Load() on everything, clist will load first

	LangPackDropUnusedItems();

	if ( !disableDefaultModule[DEFMOD_SSL]) if (LoadSslModule()) return 1;
	NetlibInitSsl();

	if (LoadAccountsModule()) return 1;

    //order becomes less important below here
	if ( !disableDefaultModule[DEFMOD_UIFINDADD]) if (LoadFindAddModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_UIUSERINFO]) if (LoadUserInfoModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_SRURL]) if (LoadSendRecvUrlModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_SREMAIL]) if (LoadSendRecvEMailModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_SRAUTH]) if (LoadSendRecvAuthModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_SRFILE]) if (LoadSendRecvFileModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_UIHELP]) if (LoadHelpModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_UIHISTORY]) if (LoadHistoryModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_RNDIDLE]) if (LoadIdleModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_RNDAUTOAWAY]) if (LoadAutoAwayModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_RNDUSERONLINE]) if (LoadUserOnlineModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_SRAWAY]) if (LoadAwayMsgModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_RNDIGNORE]) if (LoadIgnoreModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_UIVISIBILITY]) if (LoadVisibilityModule()) return 1;
	if ( !disableDefaultModule[DEFMOD_UPDATENOTIFY]) if (LoadUpdateNotifyModule()) return 1;
	return 0;
}

void UnloadDefaultModules(void)
{
	UnloadAccountsModule();
	UnloadNewPlugins();
	UnloadProtocolsModule();
	UnloadSkinSounds();
	UnloadSkinHotkeys();
//	UnloadErrorsModule();
	UnloadIcoLibModule();
	UnloadUtilsModule();
	UnloadButtonModule();
	UnloadClcModule();
	UnloadContactListModule();
	UnloadEventsModule();
	UnloadIdleModule();
	UnloadUpdateNotifyModule();
	UnloadNetlibModule();
	UnloadSslModule();
}
