/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team,
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
#include "database.h"

MDatabaseReadonly::MDatabaseReadonly()
{
}

BOOL MDatabaseReadonly::IsRelational(void)
{
	return FALSE;
}

void MDatabaseReadonly::SetCacheSafetyMode(BOOL)
{
}

BOOL MDatabaseReadonly::EnumModuleNames(DBMODULEENUMPROC, void*)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) MDatabaseReadonly::ReadCryptoKey(MBinBuffer&)
{
	return FALSE;
}

STDMETHODIMP_(BOOL) MDatabaseReadonly::StoreCryptoKey()
{
	return FALSE;
}

STDMETHODIMP_(CRYPTO_PROVIDER*) MDatabaseReadonly::ReadProvider()
{
	return nullptr;
}

STDMETHODIMP_(BOOL) MDatabaseReadonly::StoreProvider(CRYPTO_PROVIDER *)
{
	return FALSE;
}

STDMETHODIMP_(BOOL) MDatabaseReadonly::EnableEncryption(BOOL)
{
	return FALSE;
}

STDMETHODIMP_(BOOL) MDatabaseReadonly::ReadEncryption()
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT MDatabaseReadonly::AddContact(void)
{
	return 0;
}

int MDatabaseReadonly::DeleteContact(MCONTACT)
{
	return 1;
}

BOOL MDatabaseReadonly::IsDbContact(MCONTACT contactID)
{
	return contactID == 1;
}

int MDatabaseReadonly::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

MEVENT MDatabaseReadonly::AddEvent(MCONTACT, const DBEVENTINFO*)
{
	return 0;
}

BOOL MDatabaseReadonly::DeleteEvent(MEVENT)
{
	return 1;
}

BOOL MDatabaseReadonly::EditEvent(MCONTACT, MEVENT, const DBEVENTINFO*)
{
	return 1;
}

int MDatabaseReadonly::GetBlobSize(MEVENT)
{
	return 0;
}

MEVENT MDatabaseReadonly::FindFirstUnreadEvent(MCONTACT)
{
	return 0;
}

BOOL MDatabaseReadonly::MarkEventRead(MCONTACT, MEVENT)
{
	return 1;
}

MCONTACT MDatabaseReadonly::GetEventContact(MEVENT)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL MDatabaseReadonly::GetContactSettingWorker(MCONTACT, LPCSTR, LPCSTR, DBVARIANT*, int)
{
	return 1;
}

BOOL MDatabaseReadonly::WriteContactSettingWorker(MCONTACT, DBCONTACTWRITESETTING&)
{
	return 1;
}

BOOL MDatabaseReadonly::DeleteContactSetting(MCONTACT, LPCSTR, LPCSTR)
{
	return 1;
}

BOOL MDatabaseReadonly::EnumContactSettings(MCONTACT, DBSETTINGENUMPROC, const char*, void*)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL MDatabaseReadonly::MetaMergeHistory(DBCachedContact*, DBCachedContact*)
{
	return 1;
}

BOOL MDatabaseReadonly::MetaSplitHistory(DBCachedContact*, DBCachedContact*)
{
	return 1;
}

BOOL MDatabaseReadonly::MetaRemoveSubHistory(DBCachedContact*)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MEVENT MDatabaseReadonly::GetEventById(LPCSTR, LPCSTR)
{
	return 0;
}
