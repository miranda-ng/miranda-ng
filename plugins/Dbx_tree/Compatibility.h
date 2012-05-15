/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#pragma once

#include "Interface.h"
#include "DataBase.h"
#include "Services.h"

bool CompatibilityRegister();
bool CompatibilityUnRegister();

INT_PTR CompGetContactCount(WPARAM wParam,LPARAM lParam);
INT_PTR CompFindFirstContact(WPARAM wParam,LPARAM lParam);
INT_PTR CompFindNextContact(WPARAM hContact,LPARAM lParam);
INT_PTR CompDeleteContact(WPARAM hContact,LPARAM lParam);
INT_PTR CompAddContact(WPARAM wParam,LPARAM lParam);
INT_PTR CompIsDbContact(WPARAM hContact,LPARAM lParam);

INT_PTR CompGetContactSetting(WPARAM hContact, LPARAM pSetting);
INT_PTR CompGetContactSettingStr(WPARAM hContact, LPARAM pSetting);
INT_PTR CompGetContactSettingStatic(WPARAM hContact, LPARAM pSetting);
INT_PTR CompFreeVariant(WPARAM wParam, LPARAM pSetting);
INT_PTR CompWriteContactSetting(WPARAM hContact, LPARAM pSetting);
INT_PTR CompDeleteContactSetting(WPARAM hContact, LPARAM pSetting);
INT_PTR CompEnumContactSettings(WPARAM hContact, LPARAM pEnum);

INT_PTR CompGetEventCount(WPARAM wParam, LPARAM lParam);
INT_PTR CompAddEvent(WPARAM hContact, LPARAM pEventInfo);
INT_PTR CompDeleteEvent(WPARAM hContact, LPARAM hEvent);
INT_PTR CompGetBlobSize(WPARAM hEvent, LPARAM lParam);
INT_PTR CompGetEvent(WPARAM hEvent, LPARAM pEventInfo);
INT_PTR CompMarkEventRead(WPARAM hContact, LPARAM hEvent);
INT_PTR CompGetEventContact(WPARAM hEvent, LPARAM lParam);
INT_PTR CompFindFirstEvent(WPARAM hContact, LPARAM lParam);
INT_PTR CompFindFirstUnreadEvent(WPARAM hContact, LPARAM lParam);
INT_PTR CompFindLastEvent(WPARAM hContact, LPARAM lParam);
INT_PTR CompFindNextEvent(WPARAM hEvent, LPARAM lParam);
INT_PTR CompFindPrevEvent(WPARAM hEvent, LPARAM lParam);


INT_PTR CompEncodeString(WPARAM wParam, LPARAM lParam);
INT_PTR CompDecodeString(WPARAM wParam, LPARAM lParam);

INT_PTR CompGetProfileName(WPARAM cbBytes, LPARAM pszName);
INT_PTR CompGetProfilePath(WPARAM cbBytes, LPARAM pszName);
