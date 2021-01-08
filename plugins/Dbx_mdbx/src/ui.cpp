/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org)
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CompactMe(void* obj, WPARAM, LPARAM)
{
	CDbxMDBX *db = (CDbxMDBX*)obj;
	if (!db->Compact())
		MessageBox(0, TranslateT("Database was compacted successfully"), TranslateT("Database"), MB_OK | MB_ICONINFORMATION);
	else
		MessageBox(0, TranslateT("Database compaction failed"), TranslateT("Database"), MB_OK | MB_ICONERROR);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsDialog : public CDlgBase
{
	CCtrlCheck m_chkStandart;
	CCtrlCheck m_chkTotal;
	CDbxMDBX *m_db;

	bool OnInitDialog() override
	{
		m_chkStandart.SetState(!m_db->isEncrypted());
		m_chkTotal.SetState(m_db->isEncrypted());
		return true;
	}

	bool OnApply() override
	{
		SetCursor(LoadCursor(nullptr, IDC_WAIT));
		m_db->EnableEncryption(m_chkTotal.GetState() != 0);
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		m_chkStandart.SetState(!m_db->isEncrypted());
		m_chkTotal.SetState(m_db->isEncrypted());
		return true;
	}

public:
	COptionsDialog(CDbxMDBX *db) :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_chkStandart(this, IDC_STANDARD),
		m_chkTotal(this, IDC_TOTAL),
		m_db(db)
	{
	}
};

static int OnOptionsInit(PVOID obj, WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Database");
	odp.pDialog = new COptionsDialog((CDbxMDBX*)obj);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Compact"), "compact", IDI_COMPACT }
};

static int OnModulesLoaded(PVOID obj, WPARAM, LPARAM)
{
	g_plugin.registerIcon(LPGEN("Database"), iconList, "mdbx");

	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, obj);

	// main menu item
	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000, 0);

	SET_UID(mi, 0x98c0caf3, 0xBfe5, 0x4e31, 0xac, 0xf0, 0xab, 0x95, 0xb2, 0x9b, 0x9f, 0x73);
	mi.position++;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.a = LPGEN("Compact");
	mi.pszService = MS_DB_COMPACT;
	Menu_AddMainMenuItem(&mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMDBX::InitDialogs()
{
	hService[0] = CreateServiceFunctionObj(MS_DB_COMPACT, CompactMe, this);

	hHook = HookEventObj(ME_SYSTEM_MODULESLOADED, OnModulesLoaded, this);
}
