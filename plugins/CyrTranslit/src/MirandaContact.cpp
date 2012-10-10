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

#include "plugin.h"
#include "MirandaContact.h"
#include "TransliterationProtocol.h"

namespace CyrTranslit
{

const std::string MirandaContact::SETTINGS_MODULE = "CyrTranslit";

const std::string MirandaContact::SETTING_SHOULD_TRANSLITERATE
    = "translit";

char *MirandaContact::MENU_ITEM_TEXT = "&Transliterate (ÔÛÂÀ->FYVA)";

char *MirandaContact::MENU_COMMAND_CALLBACK_SERVICE
    = "CyrTranslit/ContactMenuCmd";
    
HANDLE MirandaContact::hTransliterateCmdMenuItem = 0;

//------------------------------------------------------------------------------

MirandaContact::MirandaContact()
    :   handle(0),
        transliterateOutgoingMessages(false)
{
}

//------------------------------------------------------------------------------

MirandaContact::~MirandaContact()
{
}

//------------------------------------------------------------------------------

void MirandaContact::initialize()
{
    CreateServiceFunction(MENU_COMMAND_CALLBACK_SERVICE,onMenuCommandTransliterate);
    generateMenuItemsForAllContacts();
    
    HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPreBuildContactMenu);
    
    TransliterationProtocol::initialize();
    activateTransliterationProtocolForSubscribedContacts();
}

//------------------------------------------------------------------------------

MirandaContact MirandaContact::getContact(HANDLE hContact)
{
    int b = DBGetContactSettingByte(
        hContact,
        SETTINGS_MODULE.c_str(),
        SETTING_SHOULD_TRANSLITERATE.c_str(),
        0);
        
    MirandaContact ret;
    ret.handle = hContact;
    ret.transliterateOutgoingMessages = (b != 0);
    
    return ret;
}

//------------------------------------------------------------------------------

void MirandaContact::save() const
{
    DBWriteContactSettingByte(
        handle,
        SETTINGS_MODULE.c_str(),
        SETTING_SHOULD_TRANSLITERATE.c_str(),
        transliterateOutgoingMessages? 1 : 0);
}

//------------------------------------------------------------------------------

bool MirandaContact::shouldTransliterateOutgoingMessages() const
{
    return transliterateOutgoingMessages;
}

//------------------------------------------------------------------------------

void MirandaContact::fillInMenuItem(CLISTMENUITEM &mi)
{
    mi.cbSize = sizeof(CLISTMENUITEM);
    mi.pszName = MENU_ITEM_TEXT;
    mi.flags = 0;
    mi.position = 65535;
    mi.hIcon = NULL;
    mi.pszService = MENU_COMMAND_CALLBACK_SERVICE;
    mi.pszPopupName = NULL;
    mi.popupPosition = 0;
    mi.hotKey = 0;
    mi.pszContactOwner = NULL;
}

//------------------------------------------------------------------------------

void MirandaContact::generateMenuItemsForAllContacts()
{
    CLISTMENUITEM mi;
    fillInMenuItem(mi);
    hTransliterateCmdMenuItem = Menu_AddContactMenuItem(&mi);
}

//------------------------------------------------------------------------------

void MirandaContact::activateTransliterationProtocolForSubscribedContacts()
{
    HANDLE hContact = db_find_first();
    if(!hContact)
		 return;

    do
    {
        MirandaContact mc = getContact(reinterpret_cast<HANDLE>(hContact));
        
        if(mc.shouldTransliterateOutgoingMessages())
        {
            TransliterationProtocol::activateForContact(mc.handle);
        }
    } while(hContact = db_find_next(hContact));
}

//------------------------------------------------------------------------------

INT_PTR MirandaContact::onMenuCommandTransliterate(WPARAM wParam, LPARAM lParam)
{
    HANDLE hContact = reinterpret_cast<HANDLE>(wParam);
    if(!CallService(MS_DB_CONTACT_IS, wParam, 0)) return 0;

    MirandaContact mc = getContact(hContact);
    mc.transliterateOutgoingMessages = !mc.transliterateOutgoingMessages;
    mc.save();
    
    if(mc.shouldTransliterateOutgoingMessages())
    {
        TransliterationProtocol::activateForContact(mc.handle);
    }
    else
    {
        TransliterationProtocol::deactivateForContact(mc.handle);
    }
    
    return 0;
}

//------------------------------------------------------------------------------

int MirandaContact::onPreBuildContactMenu(WPARAM wParam, LPARAM lParam)
{
    if(!hTransliterateCmdMenuItem) return 0;
    HANDLE hContact = reinterpret_cast<HANDLE>(wParam);
    if(!CallService(MS_DB_CONTACT_IS, wParam, 0)) return 0;
    
    MirandaContact mc = getContact(hContact);
    
    CLISTMENUITEM mi;
    fillInMenuItem(mi);
    
    DWORD toSet = mc.shouldTransliterateOutgoingMessages()? CMIF_CHECKED : 0;
    mi.flags = CMIM_FLAGS | toSet;
    
    CallService(
        MS_CLIST_MODIFYMENUITEM,
        reinterpret_cast<WPARAM>(hTransliterateCmdMenuItem),
        reinterpret_cast<LPARAM>(&mi));
    
    return 0;
}

}