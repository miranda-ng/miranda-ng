/*
Bonsai plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#ifndef M_BONSAI_COMMONHEADERS_H
#define M_BONSAI_COMMONHEADERS_H

#define MIRANDA_VER 0x0600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <math.h>
#include <dbghelp.h>
#include <commctrl.h>

#ifndef MODULES_H_
typedef INT_PTR (*MIRANDASERVICE)(WPARAM,LPARAM);
#endif

#include "newpluginapi.h"
#include "m_database.h"
#include "m_contacts.h"
#include "m_langpack.h"
#include "m_options.h"
//#include "m_hotkeys.h"
//#include "sdk/m_hotkeysplus.h"
#include "m_popup.h"
#include "m_utils.h"
#include "sdk/m_updater.h"
#include "m_system.h"

#include "version.h"
#include "utils.h"
#include "mirandaMem.h"
#include "services.h"
#include "list.h"
#include "dlg_handlers.h"
#include "hooked_events.h"

#include "resource.h"

extern char ModuleName[];
extern HINSTANCE hInstance;
extern PLUGINLINK *pluginLink;
extern int bUseANSIStrings;

extern HBITMAP hbModified;
extern HBITMAP hbUnmodified;
extern HBITMAP hbNewlyModified;
extern HIMAGELIST imgList;

#define OLD_MIRANDAPLUGININFO_SUPPORT PLUGININFO oldPluginInfo = { \
	sizeof(PLUGININFO), \
	pluginInfo.shortName, \
	pluginInfo.version, \
	pluginInfo.description, \
	pluginInfo.author, \
	pluginInfo.authorEmail, \
	pluginInfo.copyright, \
	pluginInfo.homepage, \
	pluginInfo.flags, \
	pluginInfo.replacesDefaultModule \
}; \
\
extern "C" __declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion) \
{ \
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 6, 0, 0)) \
	{ \
		return NULL; \
	} \
	return &oldPluginInfo; \
}

#endif //M_WWI_COMMONHEADERS_H