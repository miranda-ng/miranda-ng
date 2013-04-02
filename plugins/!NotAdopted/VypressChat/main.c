/*
 * Miranda-IM Vypress Chat/quickChat plugins
 * Copyright (C) Saulius Menkevicius
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: main.c,v 1.11 2005/03/09 14:44:22 bobas Exp $
 */

#include "miranda.h"
#include "main.h"
#include "service.h"
#include "user.h"
#include "userlist.h"
#include "chatroom.h"
#include "msghandler.h"
#include "options.h"
#include "skin.h"

/* forward references
 */

/* global data
 */
HINSTANCE	g_hDllInstance;	/* plugin dll instance */
HANDLE		g_hMainThread;

PLUGINLINK	* pluginLink;

/* exported routines
 */
BOOL WINAPI
DllMain(HINSTANCE hInstanceDLL, DWORD fwReason, LPVOID lpvReserved)
{
	g_hDllInstance = hInstanceDLL;
	return TRUE;
}

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	VQCHAT_PROTO_NAME " Protocol",
	PLUGIN_MAKE_VERSION(VQCHAT_VER_MAJ0, VQCHAT_VER_MAJ1, VQCHAT_VER_MIN0, VQCHAT_VER_MIN1),
	"Adds support for " VQCHAT_PROTO_NAME " networks.",
	"Saulius Menkevicius",
	"bobas@sourceforge.net",
	"(C) 2005 Saulius Menkevicius",
	"http://sourceforge.net/projects/miranda-vqchat/",
	0,		// NOT transient
	0		// doesn't replace anything builtin
};

__declspec(dllexport) PLUGININFO *
MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

__declspec(dllexport) int
Load(PLUGINLINK * link)
{
	PROTOCOLDESCRIPTOR pd;
	INITCOMMONCONTROLSEX iccs;

	/* setup static data */
	pluginLink = link;

	/* get main thread handle */
	DuplicateHandle(
		GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &g_hMainThread,
		THREAD_SET_CONTEXT, FALSE, 0);
	
	/* init common controls library (for the IP adress entries to work)
	 */
	memset(&iccs, 0, sizeof(iccs));
	iccs.dwSize = sizeof(iccs);
	iccs.dwICC = ICC_INTERNET_CLASSES;
	InitCommonControlsEx(&iccs);

	/* init vqp_link module */
	vqp_init(NULL, NULL);

	/* register this module with miranda */
	memset(&pd, 0, sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = VQCHAT_PROTO;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	/* init our modules */
	options_init();
	msghandler_init();
	skin_init();
	user_init();
	userlist_init();
	chatroom_init();

	/* register protocol services */
	service_register_services();
	service_hook_all();

	return 0;
}

__declspec(dllexport) int
Unload(void)
{
	service_uninit();
	user_uninit();
	userlist_uninit();
	chatroom_uninit();
	msghandler_uninit();
	options_uninit();
	skin_uninit();

	vqp_uninit();

	return 0;
}

