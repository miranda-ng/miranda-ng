
//This file is part of Msg_Export a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen ( http://sourceforge.net/projects/msg-export/ )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef MSG_EXP_GLOB_H
#define MSG_EXP_GLOB_H

#include <Windows.h>
#include <windowsx.h>
#include <Richedit.h>

#include <malloc.h>

using namespace std;
#include <Shlobj.h>
#include <list>
#include <string>
#include <map>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_metacontacts.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_icolib.h>
#include <m_history.h>
#include <m_userinfo.h>
#include <m_protosvc.h>
#include <m_timezones.h>
#include <m_skin.h>
#include <m_gui.h>
#include <m_json.h>

#include "utils.h"
#include "options.h"
#include "FileViewer.h"
#include "resource.h"
#include "version.h"

#define MODULENAME "Msg_Export"
#define MSG_BOX_TITEL TranslateT("Miranda NG (Message Export Plugin)")
#define MS_SHOW_EXPORT_HISTORY "History/ShowExportHistory"
#define MS_EXPORT_HISTORY "History/ExportHistory"
#define szFileViewDB "FileV_"
#define WM_RELOAD_FILE (WM_USER+10)

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern MWindowList hInternalWindowList;
extern wstring g_sDBPath, g_sMirandaPath;
extern IconItem iconList[];

///////////////////////////////////////////////////////////////////////////////

void __cdecl exportContactsMessages(struct ExportDialogData *data);
INT_PTR CALLBACK __stdcall DialogProc(HWND hwndDlg, UINT uMsg, WPARAM, LPARAM lParam);

struct ExportDialogData
{
	list<MCONTACT> contacts;
	HWND hDialog;

	void Run()
	{
		// Create progress dialog
		hDialog = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EXPORT_ALL_DLG), nullptr, DialogProc);
		ShowWindow(hDialog, SW_SHOWNORMAL);

		// Process the export in other thread
		mir_forkThread<ExportDialogData>(&exportContactsMessages, this);
	}
};

#endif