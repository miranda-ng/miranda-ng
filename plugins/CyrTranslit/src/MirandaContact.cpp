/**
 * CyrTranslit: the Cyrillic transliteration plug-in for Miranda IM.
 * Copyright 2005 Ivan Krechetov. 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "stdafx.h"

namespace CyrTranslit
{

const char* MirandaContact::SETTINGS_MODULE = "CyrTranslit";

const char* MirandaContact::SETTING_SHOULD_TRANSLITERATE = "translit";

char *MirandaContact::MENU_ITEM_TEXT = LPGEN("&Transliterate");

char *MirandaContact::MENU_COMMAND_CALLBACK_SERVICE
	= "CyrTranslit/ContactMenuCmd";

HGENMENU MirandaContact::hTransliterateCmdMenuItem = 0;

//------------------------------------------------------------------------------

void MirandaContact::initialize()
{
	CreateServiceFunction(MENU_COMMAND_CALLBACK_SERVICE,onMenuCommandTransliterate);
	addMenuItem();

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPreBuildContactMenu);

	TransliterationProtocol::initialize();
}

//------------------------------------------------------------------------------

bool MirandaContact::bIsActive(MCONTACT hContact)
{
	return db_get_b(hContact, SETTINGS_MODULE, SETTING_SHOULD_TRANSLITERATE, 0) != 0;
}

//------------------------------------------------------------------------------

void MirandaContact::save(MCONTACT hContact, bool bValue)
{
	db_set_b(hContact, SETTINGS_MODULE, SETTING_SHOULD_TRANSLITERATE, bValue);
}

//------------------------------------------------------------------------------

void MirandaContact::addMenuItem()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszName = MENU_ITEM_TEXT;
	mi.flags = 0;
	mi.position = 65535;
	mi.hIcon = NULL;
	mi.pszService = MENU_COMMAND_CALLBACK_SERVICE;
	mi.pszPopupName = NULL;
	mi.popupPosition = 0;
	mi.hotKey = 0;
	mi.pszContactOwner = NULL;
	hTransliterateCmdMenuItem = Menu_AddContactMenuItem(&mi);
}

//------------------------------------------------------------------------------

INT_PTR MirandaContact::onMenuCommandTransliterate(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = MCONTACT(wParam);
	if (!CallService(MS_DB_CONTACT_IS, wParam, 0))
		return 0;

	save(hContact, !bIsActive(hContact));
	return 0;
}

//------------------------------------------------------------------------------

int MirandaContact::onPreBuildContactMenu(WPARAM wParam, LPARAM)
{
	if (!hTransliterateCmdMenuItem) return 0;
	MCONTACT hContact = MCONTACT(wParam);
	if (!CallService(MS_DB_CONTACT_IS, wParam, 0)) return 0;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS;
	if ( bIsActive(hContact))
		mi.flags |= CMIF_CHECKED;

	Menu_ModifyItem(hTransliterateCmdMenuItem, &mi);
	return 0;
}

}