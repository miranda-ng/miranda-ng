/*
Copyright (C) 2006 Ricardo Pescuma Domenecci

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

#include "stdafx.h"

Timers g_timers;

// Prototypes ///////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	{ 0x1bfc449d, 0x8f6f, 0x4080, { 0x8f, 0x35, 0xf9, 0x40, 0xb3, 0xde, 0x12, 0x84 } } // {1BFC449D-8F6F-4080-8F35-F940B3DE1284}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULE_NAME, pluginInfoEx)
{
};

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

void CreateServices(void);

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int ProtoAck(WPARAM wParam, LPARAM lParam);

int CMPlugin::Load()
{
	CreateServices();

	// Hooks
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_PROTO_ACK, ProtoAck);
	return 0;
}

int CMPlugin::Unload()
{
	if (bk_brush != NULL)
		DeleteObject(bk_brush);

	return 0;
}
