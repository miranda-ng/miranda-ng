/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
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

#include <m_db_int.h>

struct CDdxMmap : public MIDatabase
{
	CDdxMmap(const TCHAR* tszFileName);

	STDMETHODIMP_(void)   SetCacheSafetyMode(BOOL);

	STDMETHODIMP_(LONG)   GetContactCount(void);
	STDMETHODIMP_(HANDLE) FindFirstContact(const char* szProto = NULL);
	STDMETHODIMP_(HANDLE) FindNextContact(HANDLE hContact, const char* szProto = NULL);
	STDMETHODIMP_(LONG)   DeleteContact(HANDLE hContact);
	STDMETHODIMP_(HANDLE) AddContact(void);
	STDMETHODIMP_(BOOL)   IsDbContact(HANDLE hContact);

	STDMETHODIMP_(LONG)   GetEventCount(HANDLE hContact);
	STDMETHODIMP_(HANDLE) AddEvent(HANDLE hContact, DBEVENTINFO *dbei);
	STDMETHODIMP_(BOOL)   DeleteEvent(HANDLE hContact, HANDLE hDbEvent);
	STDMETHODIMP_(LONG)   GetBlobSize(HANDLE hDbEvent);
	STDMETHODIMP_(BOOL)   GetEvent(HANDLE hDbEvent, DBEVENTINFO *dbei);
	STDMETHODIMP_(BOOL)   MarkEventRead(HANDLE hContact, HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE) GetEventContact(HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE) FindFirstEvent(HANDLE hContact);
	STDMETHODIMP_(HANDLE) FindFirstUnreadEvent(HANDLE hContact);
	STDMETHODIMP_(HANDLE) FindLastEvent(HANDLE hContact);
	STDMETHODIMP_(HANDLE) FindNextEvent(HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE) FindPrevEvent(HANDLE hDbEvent);

	STDMETHODIMP_(BOOL)   EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam);

	STDMETHODIMP_(BOOL)   GetContactSetting(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   GetContactSettingStr(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   GetContactSettingStatic(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   FreeVariant(DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)   WriteContactSetting(HANDLE hContact, DBCONTACTWRITESETTING *dbcws);
	STDMETHODIMP_(BOOL)   DeleteContactSetting(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   EnumContactSettings(HANDLE hContact, DBCONTACTENUMSETTINGS* dbces);
	STDMETHODIMP_(BOOL)   SetSettingResident(BOOL bIsResident, const char *pszSettingName);
	STDMETHODIMP_(BOOL)   EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam);

private:
	int CheckProto(HANDLE hContact, const char *proto);

	BOOL safetyMode;
};
