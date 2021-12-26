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

// Windows Header Files:
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>
#include "prsht.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <uxtheme.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_history.h>
#include <m_ieview.h>
#include <m_gui.h>
#include <m_langpack.h>
#include <m_metacontacts.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_utils.h>

#include "resource.h"
#include "version.h"

#include "utils.h"
#include "dlgHandlers.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMOption<uint8_t> bEnableRtl, bShowLastFirst, bUseWorker;
	CMOption<uint32_t> iLoadCount;

	CMPlugin();

	int Load() override;
	int Unload() override;
}; 

extern HICON hIcon; //history icon
extern MWindowList hOpenWindowsList;

extern BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, uint32_t);

#endif
