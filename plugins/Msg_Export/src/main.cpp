
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

#include "stdafx.h"

CMPlugin g_plugin;

MWindowList hInternalWindowList = nullptr;

/////////////////////////////////////////////////////
// Remember to update the Version in the resource !!!
/////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {46102B07-C215-4162-9C83-D377881DA7CC}
	{ 0x46102b07, 0xc215, 0x4162, { 0x9c, 0x83, 0xd3, 0x77, 0x88, 0x1d, 0xa7, 0xcc } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////
// Member Function : ShowExportHistory
// Type            : Global
// Parameters      : wParam - (MCONTACT)hContact
//                   lParam - ?
// Returns         : static int
// Description     : Called when user selects my menu item "Open Exported History"
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

static INT_PTR ShowExportHistory(WPARAM wParam, LPARAM /*lParam*/)
{
	if (bUseInternalViewer()) {
		bShowFileViewer(wParam);
		return 0;
	}
	bOpenExternaly(wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : nSystemShutdown
// Type            : Global
// Parameters      : wparam - 0
//                   lparam - 0
// Returns         : int
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020428, 28 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int nSystemShutdown(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
	WindowList_Broadcast(hInternalWindowList, WM_CLOSE, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : MainInit
// Type            : Global
// Parameters      : wparam - ?
//                   lparam - ?
// Returns         : int
// Description     : Called when system modules has been loaded
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int MainInit(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
	bReadMirandaDirAndPath();
	UpdateFileToColWidth();

	HookEvent(ME_DB_EVENT_ADDED, nExportEvent);
	HookEvent(ME_DB_CONTACT_DELETED, nContactDeleted);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	if (!bReplaceHistory) {
		CMenuItem mi(&g_plugin);
		SET_UID(mi, 0x701c543, 0xd078, 0x41dd, 0x95, 0xe3, 0x96, 0x49, 0x8a, 0x72, 0xc7, 0x50);
		mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_EXPORT_MESSAGE));
		mi.position = 1000090100;
		mi.name.a = LPGEN("Open E&xported History");
		mi.pszService = MS_SHOW_EXPORT_HISTORY;
		Menu_AddContactMenuItem(&mi);
	}

	HookEvent(ME_SYSTEM_SHUTDOWN, nSystemShutdown);
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : Load
// Type            : Global
// Parameters      : link - ?
// Returns         : int
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);

	nMaxLineWidth = db_get_w(NULL, MODULENAME, "MaxLineWidth", nMaxLineWidth);
	if (nMaxLineWidth > 0 && nMaxLineWidth < 5)
		nMaxLineWidth = 5;

	sExportDir = _DBGetString(NULL, MODULENAME, "ExportDir", L"%dbpath%\\MsgExport\\");
	sDefaultFile = _DBGetString(NULL, MODULENAME, "DefaultFile", L"%nick%.txt");

	sTimeFormat = _DBGetString(NULL, MODULENAME, "TimeFormat", L"d s");

	sFileViewerPrg = _DBGetString(NULL, MODULENAME, "FileViewerPrg", L"");
	bUseInternalViewer(db_get_b(NULL, MODULENAME, "UseInternalViewer", bUseInternalViewer()) != 0);

	bReplaceHistory = db_get_b(NULL, MODULENAME, "ReplaceHistory", bReplaceHistory) != 0;
	bAppendNewLine = db_get_b(NULL, MODULENAME, "AppendNewLine", bAppendNewLine) != 0;
	bUseUtf8InNewFiles = db_get_b(NULL, MODULENAME, "UseUtf8InNewFiles", bUseUtf8InNewFiles) != 0;
	bUseLessAndGreaterInExport = db_get_b(NULL, MODULENAME, "UseLessAndGreaterInExport", bUseLessAndGreaterInExport) != 0;

	enRenameAction = (ENDialogAction)db_get_b(NULL, MODULENAME, "RenameAction", enRenameAction);
	enDeleteAction = (ENDialogAction)db_get_b(NULL, MODULENAME, "DeleteAction", enDeleteAction);

	HANDLE hServiceFunc = nullptr;
	if (bReplaceHistory)
		hServiceFunc = CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, ShowExportHistory); //this need new code

	if (!hServiceFunc)
		hServiceFunc = CreateServiceFunction(MS_SHOW_EXPORT_HISTORY, ShowExportHistory);

	hInternalWindowList = WindowList_Create();
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : Unload
// Type            : Global
// Parameters      : none
// Returns         : 
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	WindowList_Destroy(hInternalWindowList);
	bUseInternalViewer(false);
	return 0;
}
