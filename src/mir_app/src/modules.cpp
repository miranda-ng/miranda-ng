/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "stdafx.h"

#include "plugins.h"
#include "langpack.h"
#include "chat.h"

INT_PTR CheckRestart();		// core: IDD_WAITRESTART

int  LoadSystemModule(void);		// core: m_system.h services
int  LoadNewPluginsModuleInfos(void); // core: preloading plugins
int  LoadSendRecvAuthModule(void); // core: auth dialogs
int  LoadSendRecvFileModule(void); // code: file send/recv
int  LoadNewPluginsModule(void);	// core: N.O. plugins
int  LoadNetlibModule(void);		// core: network
int  LoadProtocolsModule(void);	// core: protocol manager
int  LoadAccountsModule(void);    // core: account manager
int  LoadIgnoreModule(void);		// protocol filter: ignore
int  LoadDbintfModule(void);
int  LoadSrmmModule(void);
int  LoadHelpModule(void);
int  LoadProtocolPlugins(void);

int  LoadContactsModule(void);
int  LoadDatabaseModule(void);
int  LoadMetacontacts(void);
int  LoadOptionsModule(void);	// ui: options dialog
int  LoadFindAddModule(void);	// ui: search/add users
int  LoadSkinIcons(void);
int  LoadSkinSounds(void);
int  LoadSkinHotkeys(void);
int  LoadUserInfoModule(void);	// ui: user info

int  LoadUtilsModule(void);		// ui: utils (has a few window classes, like HyperLink)
int  LoadCLCModule(void);		// window class: CLC control
int  LoadButtonModule(void);		// window class: button class
int  LoadFontserviceModule(void); // ui: font manager
int  LoadIcoLibModule(void);   // ui: icons manager

void UnloadAccountsModule(void);
void UnloadClcModule(void);
void UnloadContactListModule(void);
void UnloadDatabase(void);
void UnloadEventsModule(void);
void UnloadExtraIconsModule(void);
void UnloadIcoLibModule(void);
void UnloadMetacontacts(void);
void UnloadNetlibModule(void);
void UnloadNewPlugins(void);
void UnloadProtocolsModule(void);
void UnloadSkinSounds(void);
void UnloadSkinHotkeys(void);
void UnloadSrmmModule(void);

int LoadHeaderbarModule();
int LoadDescButtonModule();

int LoadDefaultModules(void)
{
	// load order is very important for these
	if (LoadSystemModule()) return 1;
	if (LoadLangPackModule()) return 1; // langpack will be a system module in the new order so this is moved here
	if (CheckRestart()) return 1;
	if (LoadUtilsModule()) return 1;
	if (LoadHeaderbarModule()) return 1;
	if (LoadDbintfModule()) return 1;

	// load database drivers & service plugins without executing their Load()
	if (LoadNewPluginsModuleInfos()) return 1;

	// the database will select which db plugin to use, or fail if no profile is selected
	if (LoadDatabaseModule()) return 1;
	if (LoadNetlibModule()) return 1;

	// database is available here
	if (LoadButtonModule()) return 1;
	if (LoadIcoLibModule()) return 1;
	if (LoadSkinIcons()) return 1;
	if (LoadContactsModule()) return 1;

	// check if a service plugin is scheduled to execution
	if (plugin_service != nullptr) {
		if (LoadProtocolPlugins()) return 1;

		switch (LaunchServicePlugin(plugin_service)) {
		case SERVICE_CONTINUE:  // continue loading Miranda normally
			break;
		case SERVICE_ONLYDB:    // load database and go to the message cycle
			return 0;
		case SERVICE_MONOPOLY:  // unload database and go to the message cycle
			UnloadDatabase();
			return 0;
		default:                // smth went wrong, terminating
			return 1;
		}

		plugin_service = nullptr;
	}

	InitIni();
	if (LoadSkinSounds()) return 1;
	if (LoadSkinHotkeys()) return 1;
	if (LoadFontserviceModule()) return 1;
	if (LoadSrmmModule()) return 1;
	if (LoadChatModule()) return 1;
	if (LoadSendRecvAuthModule()) return 1;
	if (LoadDescButtonModule()) return 1;
	if (LoadSendRecvFileModule()) return 1;
	if (LoadOptionsModule()) return 1;
	if (LoadProtocolsModule()) return 1;

	LoadDbAccounts();                    // retrieves the account array from a database
	if (LoadMetacontacts()) return 1;

	if (LoadNewPluginsModule()) return 1;    // will call Load(void) on everything, clist will load first
	if (LoadHelpModule()) return 1;
	if (LoadStdPlugins()) return 1;

	if (LoadAccountsModule()) return 1;

	// order becomes less important below here
	if (LoadFindAddModule()) return 1;
	if (LoadIgnoreModule()) return 1;
	if (LoadIdleModule()) return 1;
	return 0;
}

void UnloadDefaultModules(void)
{
	UnloadIdleModule();
	UnloadChatModule();
	UnloadAccountsModule();
	UnloadMetacontacts();
	UnloadNewPlugins();
	UnloadProtocolsModule();
	UnloadSkinSounds();
	UnloadSkinHotkeys();
	UnloadSrmmModule();
	UnloadIcoLibModule();
	UnloadExtraIconsModule();
	UnloadClcModule();
	UnloadContactListModule();
	UnloadEventsModule();
	UnloadNetlibModule();
}
