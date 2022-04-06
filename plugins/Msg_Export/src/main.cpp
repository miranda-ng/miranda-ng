
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

IconItem iconList[] =
{
	{ "Main icon", "main", IDI_EXPORT_MESSAGE },
};

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

static INT_PTR ShowExportHistory(WPARAM wParam, LPARAM)
{
	if (g_bUseIntViewer)
		bShowFileViewer(wParam);
	else
		bOpenExternaly(wParam);
	return 0;
}

static INT_PTR ExportContactHistory(WPARAM hContact, LPARAM)
{
	ExportDialogData* data = new ExportDialogData();
	data->contacts.push_back(hContact);
	data->Run();
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

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x4e889089, 0x2304, 0x425f, 0x8f, 0xaa, 0x4f, 0x8a, 0x7b, 0x26, 0x4d, 0x4d); // {4E889089-2304-425F-8FAA-4F8A7B264D4D}
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.position = 1000090101;
	mi.name.a = LPGEN("Export history");
	mi.pszService = MS_EXPORT_HISTORY;
	Menu_AddContactMenuItem(&mi);

	if (!g_bReplaceHistory) {
		SET_UID(mi, 0x701c543, 0xd078, 0x41dd, 0x95, 0xe3, 0x96, 0x49, 0x8a, 0x72, 0xc7, 0x50);
		mi.hIcolibItem = iconList[0].hIcolib;
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
	registerIcon(MODULENAME, iconList, MODULENAME);

	HookEvent(ME_DB_EVENT_ADDED, nExportEvent);
	HookEvent(ME_DB_EVENT_EDITED, nExportEvent);
	HookEvent(ME_DB_CONTACT_DELETED, nContactDeleted);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);

	nMaxLineWidth = getWord("MaxLineWidth", nMaxLineWidth);
	if (nMaxLineWidth > 0 && nMaxLineWidth < 5)
		nMaxLineWidth = 5;

	g_sExportDir = _DBGetStringW(0, MODULENAME, "ExportDir", L"%dbpath%\\MsgExport\\");
	g_sDefaultFile = _DBGetStringW(0, MODULENAME, "DefaultFile", L"%nick%.txt");

	g_sTimeFormat = _DBGetStringW(0, MODULENAME, "TimeFormat", L"d s");

	sFileViewerPrg = _DBGetStringW(0, MODULENAME, "FileViewerPrg", L"");
	g_bUseIntViewer = getBool("UseInternalViewer", true);

	g_bUseJson = getBool("UseJson", false);
	g_bAppendNewLine = getBool("AppendNewLine", true);
	g_bReplaceHistory = getBool("ReplaceHistory", false);
	g_bUseUtf8InNewFiles = getBool("UseUtf8InNewFiles", true);
	g_bUseLessAndGreaterInExport = getBool("UseLessAndGreaterInExport", false);

	g_enRenameAction = (ENDialogAction)getByte("RenameAction", eDAPromptUser);
	g_enDeleteAction = (ENDialogAction)getByte("DeleteAction", eDAPromptUser);

	HANDLE hServiceFunc = nullptr;
	if (g_bReplaceHistory)
		hServiceFunc = CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, ShowExportHistory); //this need new code

	if (!hServiceFunc)
		hServiceFunc = CreateServiceFunction(MS_SHOW_EXPORT_HISTORY, ShowExportHistory);

	CreateServiceFunction(MS_EXPORT_HISTORY, ExportContactHistory);

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
