//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "stdafx.h"
#include "db.h"
#include "resources.h"
#include "options.h"
#include "inbox.h"

#define MS_GTALKEXT_OPENMAILBOX SHORT_PLUGIN_NAME "/OpenMailbox"

extern HICON g_hPopupIcon;

INT_PTR OpenMailboxMenuHandler(WPARAM, LPARAM, LPARAM param)
{
	OpenContactInbox((LPCSTR)param);
	return 0;
}

int InitMenus(WPARAM wParam, LPARAM lParam)
{
	GoogleTalkAcc* pga = isGoogle(lParam);
	if (pga != NULL) {
		LPCSTR szModuleName = pga->m_japi->GetModuleName();
		char szServiceName[100];
		mir_snprintf(szServiceName, SIZEOF(szServiceName), "%s/%s", szModuleName, MS_GTALKEXT_OPENMAILBOX);
		CreateServiceFunctionParam(szServiceName, OpenMailboxMenuHandler, (LPARAM)szModuleName);

		CLISTMENUITEM cmi = { sizeof(cmi) };
		cmi.flags = CMIF_CHILDPOPUP;
		cmi.hParentMenu = HGENMENU(wParam);
		cmi.hIcon = g_hPopupIcon;
		cmi.position = 200101;
		cmi.pszName = LPGEN("Open mailbox");
		cmi.pszService = szServiceName;
		Menu_AddProtoMenuItem(&cmi);
	}
	return 0;
}
