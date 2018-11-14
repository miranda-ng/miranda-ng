/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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

HICON hiMailIcon = nullptr;
HWND hEmailsDlg = nullptr;

CMPlugin g_plugin;

CExchangeServer exchangeServer;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	__VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
  {0x2fd0df15, 0x7098, 0x41ce, {0xaa, 0x92, 0xff, 0x62, 0x18, 0x06, 0xe3, 0x8b}} //{2fd0df15-7098-41ce-aa92-ff621806e38b}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("ExchangeNotify", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	hiMailIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_MAIL));
	InitServices();
	HookEvents();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	DestroyServices();
	UnhookEvents();
	return 0;
}
