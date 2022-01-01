/*

Standard auto away module for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "stdafx.h"

int LoadAutoAwayModule(void);

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// 9f5ca736-1108-4872-bec3-19c84bc2143b
	{ 0x9f5ca736, 0x1108, 0x4872, {0xbe, 0xc3, 0x19, 0xc8, 0x4b, 0xc2, 0x14, 0x3b}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bIdleCheck(IDLENAME, "UserIdleCheck", 0),
	bIdleMethod(IDLENAME, "IdleMethod", 0),
	bIdleOnSaver(IDLENAME, "IdleOnSaver", 0),
	bIdleOnFullScr(IDLENAME, "IdleOnFullScr", 0),
	bIdleOnLock(IDLENAME, "IdleOnLock", 0),
	bIdlePrivate(IDLENAME, "IdlePrivate", 0),
	bIdleSoundsOff(IDLENAME, "IdleSoundsOff", 1),
	bIdleOnTerminal(IDLENAME, "IdleOnTerminalDisconnect", 0),
	bIdleStatusLock(IDLENAME, "IdleStatusLock", 0),
	bAAEnable(IDLENAME, "AAEnable", 0),
	bAAStatus(IDLENAME, "AAStatus", 0),
	iIdleTime1st(IDLENAME, "IdleTime1st", 10)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_AUTOAWAY, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	LoadIdleModule();
	LoadAutoAwayModule();
	return 0;
}

int CMPlugin::Unload()
{
	UnloadIdleModule();
	return 0;
}
