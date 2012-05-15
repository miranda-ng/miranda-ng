/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "commonheaders.h"
#include "clc.h"

HANDLE hGroupChangeEvent;

static INT_PTR RenameGroup(WPARAM wParam, LPARAM lParam);
static INT_PTR MoveGroupBefore(WPARAM wParam, LPARAM lParam);

static int CountGroups(void)
{
	DBVARIANT dbv;
	int i;
	char str[33];

	for (i = 0;; i++) {
		_itoa(i, str, 10);
		if (DBGetContactSetting(NULL, "CListGroups", str, &dbv))
			break;
		DBFreeVariant(&dbv);
	}
	return i;
}

static int GroupNameExists(const TCHAR *name, int skipGroup)
{
	char idstr[33];
	DBVARIANT dbv;
	int i;

	for (i = 0;; i++) {
		if (i == skipGroup)
			continue;
		_itoa(i, idstr, 10);
		if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
			break;
		if (!_tcscmp(dbv.ptszVal + 1, name)) {
			DBFreeVariant(&dbv);
			return i+1;
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

static INT_PTR CreateGroup(WPARAM wParam, LPARAM lParam)
{
	int newId = CountGroups();
	TCHAR newBaseName[127], newName[128];
	char str[33];
	int i;
	DBVARIANT dbv;

	const TCHAR* grpName = lParam ? (TCHAR*)lParam : TranslateT("New Group");
	if (wParam) {
		_itoa(wParam - 1, str, 10);
		if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
			return 0;

		mir_sntprintf( newBaseName, SIZEOF(newBaseName), _T("%s\\%s"), dbv.ptszVal + 1, grpName );
		mir_free(dbv.pszVal);
	}
	else lstrcpyn( newBaseName, grpName, SIZEOF( newBaseName ));

	_itoa(newId, str, 10);
	lstrcpyn( newName + 1, newBaseName, SIZEOF(newName) - 1);
	if (lParam) {
		i = GroupNameExists(newBaseName, -1);
		if (i) newId = i - 1;
		i = !i;
	}
	else {
		i = 1;
		while (GroupNameExists(newName + 1, -1))
			mir_sntprintf( newName + 1, SIZEOF(newName) - 1, _T("%s (%d)"), newBaseName, ++i );
	}
	if (i) {
		const CLISTGROUPCHANGE grpChg = { sizeof(CLISTGROUPCHANGE), NULL, newName };

		newName[0] = 1 | GROUPF_EXPANDED;   //1 is required so we never get '\0'
		DBWriteContactSettingTString(NULL, "CListGroups", str, newName);
		CallService(MS_CLUI_GROUPADDED, newId + 1, 1);

		NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);
	}

	return newId + 1;
}

static INT_PTR GetGroupName2(WPARAM wParam, LPARAM lParam)
{
	char idstr[33];
	DBVARIANT dbv;
	static char name[128];

	_itoa(wParam - 1, idstr, 10);
	if (DBGetContactSettingString(NULL, "CListGroups", idstr, &dbv))
		return (INT_PTR) (char *) NULL;
	lstrcpynA(name, dbv.pszVal + 1, SIZEOF(name));
	if ((DWORD *) lParam != NULL)
		*(DWORD *) lParam = dbv.pszVal[0];
	DBFreeVariant(&dbv);
	return (INT_PTR) name;
}

TCHAR* fnGetGroupName( int idx, DWORD* pdwFlags )
{
	char idstr[33];
	DBVARIANT dbv;
	static TCHAR name[128];

	_itoa( idx-1, idstr, 10);
	if (DBGetContactSettingTString( NULL, "CListGroups", idstr, &dbv ))
		return NULL;

	lstrcpyn( name, dbv.ptszVal + 1, SIZEOF( name ));
	if ( pdwFlags != NULL )
		*pdwFlags = dbv.ptszVal[0];
	DBFreeVariant( &dbv );
	return name;
}

static INT_PTR GetGroupName(WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret;
	ret = GetGroupName2(wParam, lParam);
	if ((int *) lParam)
		*(int *) lParam = 0 != (*(int *) lParam & GROUPF_EXPANDED);
	return ret;
}

static INT_PTR DeleteGroup(WPARAM wParam, LPARAM)
{
	int i;
	char str[33];
	DBVARIANT dbv;
	HANDLE hContact;
	TCHAR name[256], szNewParent[256], *pszLastBackslash;

	//get the name
	_itoa(wParam - 1, str, 10);
	if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
		return 1;
	lstrcpyn(name, dbv.ptszVal + 1, SIZEOF(name));
	DBFreeVariant(&dbv);
	if (DBGetContactSettingByte(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT))
	{
		TCHAR szQuestion[256+100];
		mir_sntprintf( szQuestion, SIZEOF(szQuestion), TranslateT("Are you sure you want to delete group '%s'?  This operation can not be undone."), name );
		if (MessageBox(cli.hwndContactList, szQuestion, TranslateT("Delete Group"), MB_YESNO|MB_ICONQUESTION)==IDNO)
			return 1;
	}
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	//must remove setting from all child contacts too
	//children are demoted to the next group up, not deleted.
	lstrcpy(szNewParent, name);
	pszLastBackslash = _tcsrchr(szNewParent, '\\');
	if (pszLastBackslash)
		pszLastBackslash[0] = '\0';
	else
		szNewParent[0] = '\0';

	CLISTGROUPCHANGE grpChg = { sizeof(CLISTGROUPCHANGE), NULL, NULL };

	for (hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); 
		 hContact ; 
		 hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0))
	{
		if (DBGetContactSettingTString(hContact, "CList", "Group", &dbv))
			continue;

		if (_tcscmp(dbv.ptszVal, name)) 
		{
			DBFreeVariant(&dbv);
			continue;
		}
		DBFreeVariant(&dbv);

		if (szNewParent[0])
		{
			DBWriteContactSettingTString(hContact, "CList", "Group", szNewParent);
			grpChg.pszNewName = szNewParent;
		}
		else
		{
			DBDeleteContactSetting(hContact, "CList", "Group");
			grpChg.pszNewName = NULL;
		}
		NotifyEventHooks(hGroupChangeEvent, (WPARAM)hContact, (LPARAM)&grpChg);
	} 
	//shuffle list of groups up to fill gap
	for (i = wParam - 1;; i++) {
		_itoa(i + 1, str, 10);
		if (DBGetContactSettingStringUtf(NULL, "CListGroups", str, &dbv))
			break;
		_itoa(i, str, 10);
		DBWriteContactSettingStringUtf(NULL, "CListGroups", str, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	_itoa(i, str, 10);
	DBDeleteContactSetting(NULL, "CListGroups", str);
	//rename subgroups
	{
		TCHAR szNewName[256];
		int len;

		len = lstrlen(name);
		for (i = 0;; i++) {
			_itoa(i, str, 10);
			if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
				break;
			if (!_tcsncmp(dbv.ptszVal + 1, name, len) && dbv.pszVal[len + 1] == '\\' && _tcschr(dbv.ptszVal + len + 2, '\\') == NULL) {
				if (szNewParent[0])
					mir_sntprintf(szNewName, SIZEOF(szNewName), _T("%s\\%s"), szNewParent, dbv.ptszVal + len + 2);
				else
					lstrcpyn(szNewName, dbv.ptszVal + len + 2, SIZEOF(szNewName));
				cli.pfnRenameGroup(i + 1, szNewName);
			}
			DBFreeVariant(&dbv);
		}
	}
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	cli.pfnLoadContactTree();

	{
		const CLISTGROUPCHANGE grpChg = { sizeof(CLISTGROUPCHANGE), name, NULL };
		NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);
	}
	return 0;
}

static int RenameGroupWithMove(int groupId, const TCHAR *szName, int move)
{
	char idstr[33];
	TCHAR str[256], oldName[256];
	DBVARIANT dbv;
	HANDLE hContact;

	if (GroupNameExists(szName, groupId)) {
		MessageBox(NULL, TranslateT("You already have a group with that name. Please enter a unique name for the group."), TranslateT("Rename Group"), MB_OK);
		return 1;
	}

	//do the change
	_itoa(groupId, idstr, 10);
	if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
		return 1;
	str[0] = dbv.pszVal[0] & 0x7F;
	lstrcpyn(oldName, dbv.ptszVal + 1, SIZEOF(oldName));
	DBFreeVariant(&dbv);
	lstrcpyn(str + 1, szName, SIZEOF(str) - 1);
	DBWriteContactSettingTString(NULL, "CListGroups", idstr, str);

	//must rename setting in all child contacts too
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	do {
		ClcCacheEntryBase* cache = cli.pfnGetCacheEntry( hContact );
		if ( !lstrcmp(cache->group, oldName)) {
            DBWriteContactSettingTString(hContact, "CList", "Group", szName);
            mir_free(cache->group);
            cache->group = 0;
            cli.pfnCheckCacheItem(cache);
        }
	}
	while ((hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0)) != NULL);

	//rename subgroups
	{
		TCHAR szNewName[256];
		int len, i;

		len = lstrlen(oldName);
		for (i = 0;; i++) {
			if (i == groupId)
				continue;
			_itoa(i, idstr, 10);
			if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
				break;
			if ( !_tcsncmp(dbv.ptszVal + 1, oldName, len) && dbv.ptszVal[len + 1] == '\\' && _tcschr(dbv.ptszVal + len + 2, '\\') == NULL) {
				mir_sntprintf( szNewName, SIZEOF(szNewName), _T("%s\\%s"), szName, dbv.ptszVal + len + 2 );
				RenameGroupWithMove(i, szNewName, 0);   //luckily, child groups will never need reordering
			}
			DBFreeVariant(&dbv);
		}
	}

	//finally must make sure it's after any parent items
	if (move) {
		TCHAR *pszLastBackslash;
		int i;

		lstrcpyn(str, szName, SIZEOF(str));
		pszLastBackslash = _tcsrchr(str, '\\');
		if (pszLastBackslash != NULL) {
			*pszLastBackslash = '\0';
			for (i = 0;; i++) {
				_itoa(i, idstr, 10);
				if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
					break;
				if (!lstrcmp(dbv.ptszVal + 1, str)) {
					if (i < groupId)
						break;      //is OK
					MoveGroupBefore(groupId + 1, i + 2);
					break;
				}
				DBFreeVariant(&dbv);
			}
		}
	}
	{
		const CLISTGROUPCHANGE grpChg = { sizeof(CLISTGROUPCHANGE), oldName, (TCHAR*)szName };
		NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);
	}
	return 0;
}

int fnRenameGroup( int groupID, TCHAR* newName )
{
	return -1 != RenameGroupWithMove( groupID-1, newName, 1);
}

static INT_PTR RenameGroup(WPARAM wParam, LPARAM lParam)
{
	#if defined( _UNICODE )
		WCHAR* temp = mir_a2u(( char* )lParam );
		int result = ( -1 != RenameGroupWithMove(wParam - 1, temp, 1));
		mir_free( temp );
		return result;
	#else
		return -1 != RenameGroupWithMove(wParam - 1, (TCHAR*) lParam, 1);
	#endif
}

static INT_PTR SetGroupExpandedState(WPARAM wParam, LPARAM lParam)
{
	char idstr[33];
	DBVARIANT dbv;

	_itoa(wParam - 1, idstr, 10);
	if (DBGetContactSettingStringUtf(NULL, "CListGroups", idstr, &dbv))
		return 1;
	if (lParam)
		dbv.pszVal[0] |= GROUPF_EXPANDED;
	else
		dbv.pszVal[0] = dbv.pszVal[0] & ~GROUPF_EXPANDED;
	DBWriteContactSettingStringUtf(NULL, "CListGroups", idstr, dbv.pszVal);
	DBFreeVariant(&dbv);
	return 0;
}

static INT_PTR SetGroupFlags(WPARAM wParam, LPARAM lParam)
{
	char idstr[33];
	DBVARIANT dbv;
	int flags, oldval, newval;

	_itoa(wParam - 1, idstr, 10);
	if (DBGetContactSettingStringUtf(NULL, "CListGroups", idstr, &dbv))
		return 1;
	flags = LOWORD(lParam) & HIWORD(lParam);
	oldval = dbv.pszVal[0];
	newval = dbv.pszVal[0] = ((oldval & ~HIWORD(lParam)) | flags) & 0x7f;
	DBWriteContactSettingStringUtf(NULL, "CListGroups", idstr, dbv.pszVal);
	DBFreeVariant(&dbv);
	if ((oldval & GROUPF_HIDEOFFLINE) != (newval & GROUPF_HIDEOFFLINE))
		cli.pfnLoadContactTree();
	return 0;
}

static INT_PTR MoveGroupBefore(WPARAM wParam, LPARAM lParam)
{
	int i, shuffleFrom, shuffleTo, shuffleDir;
	char str[33];
	TCHAR *szMoveName;
	DBVARIANT dbv;

	if (wParam == 0 || (LPARAM) wParam == lParam)
		return 0;
	_itoa(wParam - 1, str, 10);
	if (DBGetContactSettingTString(NULL, "CListGroups", str, &dbv))
		return 0;
	szMoveName = dbv.ptszVal;
	//shuffle list of groups up to fill gap
	if (lParam == 0) {
		shuffleFrom = wParam - 1;
		shuffleTo = -1;
		shuffleDir = -1;
	}
	else {
		if ((LPARAM) wParam < lParam) {
			shuffleFrom = wParam - 1;
			shuffleTo = lParam - 2;
			shuffleDir = -1;
		}
		else {
			shuffleFrom = wParam - 1;
			shuffleTo = lParam - 1;
			shuffleDir = 1;
		}
	}
	if (shuffleDir == -1) {
		for (i = shuffleFrom; i != shuffleTo; i++) {
			_itoa(i + 1, str, 10);
			if (DBGetContactSettingStringUtf(NULL, "CListGroups", str, &dbv)) {
				shuffleTo = i;
				break;
			}
			_itoa(i, str, 10);
			DBWriteContactSettingStringUtf(NULL, "CListGroups", str, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	else {
		for (i = shuffleFrom; i != shuffleTo; i--) {
			_itoa(i - 1, str, 10);
			if (DBGetContactSettingStringUtf(NULL, "CListGroups", str, &dbv)) {
				mir_free(szMoveName);
				return 1;
			}                   //never happens
			_itoa(i, str, 10);
			DBWriteContactSettingStringUtf(NULL, "CListGroups", str, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	_itoa(shuffleTo, str, 10);
	DBWriteContactSettingTString(NULL, "CListGroups", str, szMoveName);
	mir_free(szMoveName);
	return shuffleTo + 1;
}

static INT_PTR BuildGroupMenu(WPARAM, LPARAM)
{
	char idstr[33];
	DBVARIANT dbv;
	int groupId;
	HMENU hRootMenu, hThisMenu;
	int nextMenuId = 100;
	TCHAR *pBackslash, *pNextField, szThisField[128], szThisMenuItem[128];
	int menuId, compareResult, menuItemCount;
	MENUITEMINFO mii = { 0 };

	if (DBGetContactSettingStringUtf(NULL, "CListGroups", "0", &dbv))
		return (INT_PTR) (HMENU) NULL;
	DBFreeVariant(&dbv);
	hRootMenu = CreateMenu();
	for (groupId = 0;; groupId++) {
		_itoa(groupId, idstr, 10);
		if (DBGetContactSettingTString(NULL, "CListGroups", idstr, &dbv))
			break;

		pNextField = dbv.ptszVal + 1;
		hThisMenu = hRootMenu;
		mii.cbSize = MENUITEMINFO_V4_SIZE;
		do {
			pBackslash = _tcschr(pNextField, '\\');
			if (pBackslash == NULL) {
				lstrcpyn(szThisField, pNextField, SIZEOF(szThisField));
				pNextField = NULL;
			}
			else {
				lstrcpyn(szThisField, pNextField, min( SIZEOF(szThisField), pBackslash - pNextField + 1));
				pNextField = pBackslash + 1;
			}
			compareResult = 1;
			menuItemCount = GetMenuItemCount(hThisMenu);
			for (menuId = 0; menuId < menuItemCount; menuId++) {
				mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA;
				mii.cch = SIZEOF(szThisMenuItem);
				mii.dwTypeData = szThisMenuItem;
				GetMenuItemInfo(hThisMenu, menuId, TRUE, &mii);
				compareResult = lstrcmp(szThisField, szThisMenuItem);
				if (compareResult == 0) {
					if (pNextField == NULL) {
						mii.fMask = MIIM_DATA;
						mii.dwItemData = groupId + 1;
						SetMenuItemInfo(hThisMenu, menuId, TRUE, &mii);
					}
					else {
						if (mii.hSubMenu == NULL) {
							mii.fMask = MIIM_SUBMENU;
							mii.hSubMenu = CreateMenu();
							SetMenuItemInfo(hThisMenu, menuId, TRUE, &mii);
							mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_ID;
							//dwItemData doesn't change
							mii.fType = MFT_STRING;
							mii.dwTypeData = TranslateT("This group");
							mii.wID = nextMenuId++;
							InsertMenuItem(mii.hSubMenu, 0, TRUE, &mii);
							mii.fMask = MIIM_TYPE;
							mii.fType = MFT_SEPARATOR;
							InsertMenuItem(mii.hSubMenu, 1, TRUE, &mii);
						}
						hThisMenu = mii.hSubMenu;
					}
					break;
				}
				if ((int) mii.dwItemData - 1 > groupId)
					break;
			}
			if (compareResult) {
				mii.fMask = MIIM_TYPE | MIIM_ID;
				mii.wID = nextMenuId++;
				mii.dwTypeData = szThisField;
				mii.fType = MFT_STRING;
				if (pNextField) {
					mii.fMask |= MIIM_SUBMENU;
					mii.hSubMenu = CreateMenu();
				}
				else {
					mii.fMask |= MIIM_DATA;
					mii.dwItemData = groupId + 1;
				}
				InsertMenuItem(hThisMenu, menuId, TRUE, &mii);
				if (pNextField) {
					hThisMenu = mii.hSubMenu;
				}
			}
		} while (pNextField);

		DBFreeVariant(&dbv);
	}
	return (INT_PTR) hRootMenu;
}

int InitGroupServices(void)
{
	for (int i = 0; ; i++) 
	{
		char str[32];
		_itoa(i, str, 10);

		DBVARIANT dbv;
		if (DBGetContactSettingStringUtf(NULL, "CListGroups", str, &dbv))
			break;
		if (dbv.pszVal[0] & 0x80)
		{
			dbv.pszVal[0] &= 0x7f;
			DBWriteContactSettingStringUtf(NULL, "CListGroups", str, dbv.pszVal);
		}
		DBFreeVariant(&dbv);
	}

	CreateServiceFunction(MS_CLIST_GROUPCREATE, CreateGroup);
	CreateServiceFunction(MS_CLIST_GROUPDELETE, DeleteGroup);
	CreateServiceFunction(MS_CLIST_GROUPRENAME, RenameGroup);
	CreateServiceFunction(MS_CLIST_GROUPGETNAME, GetGroupName);
	CreateServiceFunction(MS_CLIST_GROUPGETNAME2, GetGroupName2);
	CreateServiceFunction(MS_CLIST_GROUPSETEXPANDED, SetGroupExpandedState);
	CreateServiceFunction(MS_CLIST_GROUPSETFLAGS, SetGroupFlags);
	CreateServiceFunction(MS_CLIST_GROUPMOVEBEFORE, MoveGroupBefore);
	CreateServiceFunction(MS_CLIST_GROUPBUILDMENU, BuildGroupMenu);

	hGroupChangeEvent = CreateHookableEvent( ME_CLIST_GROUPCHANGE );

	return 0;
}
