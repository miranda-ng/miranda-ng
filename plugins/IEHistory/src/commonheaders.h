/*
IEView history viewer plugin for Miranda IM

Copyright © 2005-2006 Cristian Libotean

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

#ifndef M_COMMON_HEADERS_H
#define M_COMMON_HEADERS_H


#define EVENTTYPE_STATUS 25368 //tabsrmm status events

#include "stdafx.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <uxtheme.h>

#include "resource.h"
#include "version.h"

#include "utils.h"
#include "mirandaMem.h"
#include "events.h"
#include "dlgHandlers.h"

#include "../../../include/newpluginapi.h"
#include "../../../include/m_database.h"
#include "../../../include/m_utils.h"
#include "../../../include/m_system.h"
#include "../../../include/m_skin.h"
#include "../../../include/m_options.h"
#include "../../../include/m_clist.h"
#include "../../../include/m_langpack.h"
#include "../../../include/m_history.h"
#include "../../../include/m_contacts.h"
//#include "../../../include/m_utils.h"
#include "../../../include/m_popup.h"
#include "../../../include/m_ieview.h"
#include "../../../include/m_updater.h"


extern HICON hIcon; //history icon
extern char ModuleName[];
extern HINSTANCE hInstance; //dll instance
extern HANDLE hOpenWindowsList;

extern PLUGININFOEX pluginInfo;

extern BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD);

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
	return &oldPluginInfo; \
}

#endif