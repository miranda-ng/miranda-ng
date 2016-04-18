/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "clc.h"

struct CGroupInternal
{
	CGroupInternal(int _id, const TCHAR *_name) :
		groupId(_id),
		groupName(mir_tstrdup(_name))
		{}

	~CGroupInternal()
	{	mir_free(groupName);
	}

	int    groupId;
	TCHAR *groupName;

	void save()
	{
		char idstr[33];
		itoa(groupId, idstr, 10);
		db_set_ts(NULL, "CListGroups", idstr, groupName);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareGrpByName(const CGroupInternal *p1, const CGroupInternal *p2)
{	return mir_tstrcmp(p1->groupName+1, p2->groupName+1);
}

static LIST<CGroupInternal> arByName(20, CompareGrpByName);

/////////////////////////////////////////////////////////////////////////////////////////

struct CGroupList : public LIST<CGroupInternal>
{
	CGroupList() :
		LIST<CGroupInternal>(20, NumericKeySortT)
		{}

	__inline CGroupInternal* find(int key)
	{	return LIST<CGroupInternal>::find((CGroupInternal*)&key);
	}
};

static CGroupList arByIds;

/////////////////////////////////////////////////////////////////////////////////////////

HANDLE hGroupChangeEvent;

static mir_cs csGroups;

/////////////////////////////////////////////////////////////////////////////////////////

static int GroupNameExists(const TCHAR *ptszGroupName, int skipGroup)
{
	if (ptszGroupName == 0)
		return 0;

	TCHAR str[256];
	_tcsncpy_s(str + 1, _countof(str) - 1, ptszGroupName, _TRUNCATE);

	CGroupInternal *tmp = (CGroupInternal*)_alloca(sizeof(CGroupInternal));
	tmp->groupName = (TCHAR*)str;
	if (tmp = arByName.find(tmp))
		return (skipGroup == tmp->groupId) ? 0 : tmp->groupId + 1;
	return 0;
}

MIR_APP_DLL(MGROUP) Clist_GroupExists(LPCTSTR ptszGroupName)
{
	return GroupNameExists(ptszGroupName, -1);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CreateGroupInternal(MGROUP hParent, const TCHAR *ptszName)
{
	TCHAR newBaseName[127], newName[128];

	const TCHAR *grpName = ptszName ? ptszName : TranslateT("New group");
	if (hParent) {
		CGroupInternal *tmp = arByIds.find(hParent-1);
		if (tmp == NULL)
			return NULL;

		mir_sntprintf(newBaseName, _T("%s\\%s"), tmp->groupName+1, grpName);
	}
	else _tcsncpy_s(newBaseName, grpName, _TRUNCATE);

	mir_tstrncpy(newName + 1, newBaseName, _countof(newName) - 1);
	if (ptszName) {
		int id = GroupNameExists(newBaseName, -1);
		if (id)
			return id;
	}
	else {
		for (int idCopy = 1; GroupNameExists(newName + 1, -1); idCopy++)
			mir_sntprintf(newName + 1, _countof(newName) - 1, _T("%s (%d)"), newBaseName, idCopy);
	}

	int newId = arByIds.getCount();
	newName[0] = 1 | GROUPF_EXPANDED;   // 1 is required so we never get '\0'
	CGroupInternal *pNew = new CGroupInternal(newId, newName);
	pNew->save();
	arByIds.insert(pNew);
	arByName.insert(pNew);

	CallService(MS_CLUI_GROUPADDED, newId + 1, 1);

	CLISTGROUPCHANGE grpChg = { sizeof(grpChg), NULL, newName };
	NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);

	return newId + 1;
}

MIR_APP_DLL(MGROUP) Clist_GroupCreate(MGROUP hParent, LPCTSTR ptszGroupName)
{
	// no name specified. just create a new group with a default name
	if (ptszGroupName == 0)
		return CreateGroupInternal(hParent, NULL);

	if (ptszGroupName == NULL || ptszGroupName[0] == '\0' || ptszGroupName[0] == '\\')
		return 0;

	TCHAR *tszName = NEWTSTR_ALLOCA(ptszGroupName);
	for (TCHAR *p = tszName; *p; p++) {
		if (*p == '\\') {
			*p = '\0';
			CreateGroupInternal(hParent, tszName);
			*p = '\\';
		}
	}
	return CreateGroupInternal(hParent, tszName);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(TCHAR*) Clist_GroupGetName(MGROUP hGroup, DWORD *pdwFlags)
{
	CGroupInternal *p = arByIds.find(hGroup-1);
	if (p == NULL)
		return NULL;

	if (pdwFlags != NULL)
		*pdwFlags = p->groupName[0] & ~1;
	return p->groupName+1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GroupDelete(MGROUP hGroup)
{
	// get the name
	CGroupInternal *pGroup = arByIds.find(hGroup-1);
	if (pGroup == NULL)
		return 1;

	if (db_get_b(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT)) {
		TCHAR szQuestion[256 + 100];
		mir_sntprintf(szQuestion, TranslateT("Are you sure you want to delete group '%s'? This operation cannot be undone."), pGroup->groupName+1);
		if (MessageBox(cli.hwndContactList, szQuestion, TranslateT("Delete group"), MB_YESNO | MB_ICONQUESTION) == IDNO)
			return 1;
	}

	SetCursor(LoadCursor(NULL, IDC_WAIT));
	
	// must remove setting from all child contacts too
	// children are demoted to the next group up, not deleted.
	TCHAR *szNewParent = NEWTSTR_ALLOCA(pGroup->groupName+1);
	{
		TCHAR *pszLastBackslash = _tcsrchr(szNewParent, '\\');
		if (pszLastBackslash)
			pszLastBackslash[0] = '\0';
		else
			szNewParent[0] = '\0';
	}

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ptrT tszGroupName(db_get_tsa(hContact, "CList", "Group"));
		if (mir_tstrcmp(tszGroupName, pGroup->groupName+1))
			continue;

		CLISTGROUPCHANGE grpChg = { sizeof(grpChg), NULL, NULL };
		grpChg.pszOldName = pGroup->groupName+1;
		if (szNewParent[0]) {
			db_set_ts(hContact, "CList", "Group", szNewParent);
			grpChg.pszNewName = szNewParent;
		}
		else {
			db_unset(hContact, "CList", "Group");
			grpChg.pszNewName = NULL;
		}

		NotifyEventHooks(hGroupChangeEvent, hContact, (LPARAM)&grpChg);
	}
	
	// shuffle list of groups up to fill gap
	arByIds.remove(pGroup);
	arByName.remove(pGroup);

	for (int i = hGroup-1; i < arByIds.getCount(); i++) {
		CGroupInternal *p = arByIds[i];
		p->groupId--;
		p->save();
	}

	char idstr[33];
	_itoa(arByIds.getCount(), idstr, 10);
	db_unset(NULL, "CListGroups", idstr);
	
	// rename subgroups
	TCHAR szNewName[256];
	size_t len = mir_tstrlen(pGroup->groupName+1);
	for (int i = 0; i < arByIds.getCount(); i++) {
		CGroupInternal *p = arByIds[i];
		
		if (!_tcsncmp(pGroup->groupName+1, p->groupName+1, len) && p->groupName[len+1] == '\\' && _tcschr(p->groupName + len + 2, '\\') == NULL) {
			if (szNewParent[0])
				mir_sntprintf(szNewName, _T("%s\\%s"), szNewParent, p->groupName + len + 2);
			else
				mir_tstrncpy(szNewName, p->groupName + len + 2, _countof(szNewName));
			Clist_GroupRename(i + 1, szNewName);
		}
	}

	SetCursor(LoadCursor(NULL, IDC_ARROW));
	cli.pfnLoadContactTree();

	const CLISTGROUPCHANGE grpChg = { sizeof(grpChg), pGroup->groupName+1, NULL };
	NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);

	delete(pGroup);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GroupMoveBefore(MGROUP hGroup, MGROUP hGroupBefore)
{
	if (hGroup == 0 || hGroup == hGroupBefore)
		return 0;

	CGroupInternal *pGroup = arByIds.find(hGroup - 1);
	if (pGroup == NULL)
		return 0;

	// shuffle list of groups up to fill gap
	int shuffleFrom, shuffleTo, shuffleStep;
	if (hGroupBefore == 0) {
		shuffleFrom = hGroup - 1;
		shuffleTo = -1;
		shuffleStep = 1;
	}
	else {
		CGroupInternal *pDest = arByIds.find(hGroupBefore - 1);
		if (pDest == NULL)
			return 0;

		if (hGroup < hGroupBefore) {
			shuffleFrom = hGroup - 1;
			shuffleTo = hGroupBefore - 2;
			shuffleStep = 1;
		}
		else {
			shuffleFrom = hGroup - 1;
			shuffleTo = hGroupBefore - 1;
			shuffleStep = -1;
		}
	}

	arByIds.remove(pGroup);

	for (int i = shuffleFrom; i != shuffleTo; i += shuffleStep) {
		CGroupInternal *p = arByIds[i + shuffleStep];
		p->groupId -= shuffleStep;
		p->save();
	}

	pGroup->groupId = shuffleTo; // reinsert group back
	pGroup->save();
	
	arByIds.insert(pGroup);
	return shuffleTo + 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int RenameGroupWithMove(int groupId, const TCHAR *szName, int move)
{
	if (GroupNameExists(szName, groupId)) {
		MessageBox(NULL, TranslateT("You already have a group with that name. Please enter a unique name for the group."), TranslateT("Rename group"), MB_ICONERROR | MB_OK);
		return 1;
	}

	CGroupInternal *pGroup = arByIds.find(groupId);
	if (pGroup == NULL)
		return 0;

	// do the change
	TCHAR *oldName = NEWTSTR_ALLOCA(pGroup->groupName+1);

	TCHAR str[256];
	str[0] = pGroup->groupName[0];
	mir_tstrncpy(str + 1, szName, _countof(str) - 1);

	pGroup->groupName = mir_tstrdup(str);
	pGroup->save();

	// must rename setting in all child contacts too
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ClcCacheEntry *cache = cli.pfnGetCacheEntry(hContact);
		if (!mir_tstrcmp(cache->tszGroup, oldName)) {
			db_set_ts(hContact, "CList", "Group", szName);
			replaceStrT(cache->tszGroup, szName);
		}
	}

	// rename subgroups
	size_t len = mir_tstrlen(oldName);
	for (int i = 0; i < arByIds.getCount(); i++) {
		if (i == groupId)
			continue;

		CGroupInternal *p = arByIds[i];
		if (!_tcsncmp(p->groupName+1, oldName, len) && p->groupName[len+1] == '\\' && _tcschr(p->groupName + len + 2, '\\') == NULL) {
			TCHAR szNewName[256];
			mir_sntprintf(szNewName, _T("%s\\%s"), szName, p->groupName + len + 2);
			RenameGroupWithMove(i, szNewName, 0); // luckily, child groups will never need reordering
		}
	}

	// finally must make sure it's after any parent items
	if (move) {
		_tcsncpy_s(str, szName, _TRUNCATE);
		TCHAR *pszLastBackslash = _tcsrchr(str, '\\');
		if (pszLastBackslash != NULL) {
			*pszLastBackslash = '\0';
			for (int i = 0; i < arByIds.getCount(); i++) {
				CGroupInternal *p = arByIds[i];
				if (!mir_tstrcmp(p->groupName+1, str)) {
					if (i >= groupId)
						Clist_GroupMoveBefore(groupId + 1, i + 2);
					break;
				}
			}
		}
		cli.pfnInitAutoRebuild(cli.hwndContactTree);
	}

	const CLISTGROUPCHANGE grpChg = { sizeof(grpChg), oldName, (TCHAR*)szName };
	NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);
	return 0;
}

MIR_APP_DLL(int) Clist_GroupRename(MGROUP hGroup, const TCHAR *ptszNewName)
{
	return 0 != RenameGroupWithMove(hGroup-1, ptszNewName, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GroupSetExpanded(MGROUP hGroup, int iNewState)
{
	CGroupInternal *pGroup = arByIds.find(hGroup-1);
	if (pGroup == NULL)
		return 1;

	if (iNewState)
		pGroup->groupName[0] |= GROUPF_EXPANDED;
	else
		pGroup->groupName[0] &= ~GROUPF_EXPANDED;
	pGroup->save();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GroupSetFlags(MGROUP hGroup, LPARAM iNewFlags)
{
	CGroupInternal *pGroup = arByIds.find(hGroup-1);
	if (pGroup == NULL)
		return 1;

	int flags = LOWORD(iNewFlags) & HIWORD(iNewFlags);
	int oldval = pGroup->groupName[0];
	pGroup->groupName[0] = ((oldval & ~HIWORD(iNewFlags)) | flags) & 0x7f;
	pGroup->save();

	if ((oldval & GROUPF_HIDEOFFLINE) != (pGroup->groupName[0] & GROUPF_HIDEOFFLINE))
		cli.pfnLoadContactTree();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HMENU) Clist_GroupBuildMenu()
{
	if (arByIds.getCount() == 0)
		return NULL;

	int nextMenuId = 100;

	HMENU hRootMenu = CreateMenu();
	for (int i = 0; i < arByIds.getCount(); i++) {
		const TCHAR *pNextField = arByIds[i]->groupName + 1;
		HMENU hThisMenu = hRootMenu;

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);

		TCHAR szThisField[128], szThisMenuItem[128];
		do {
			const TCHAR *pBackslash = _tcschr(pNextField, '\\');
			if (pBackslash == NULL) {
				mir_tstrncpy(szThisField, pNextField, _countof(szThisField));
				pNextField = NULL;
			}
			else {
				mir_tstrncpy(szThisField, pNextField, min(_countof(szThisField), pBackslash - pNextField + 1));
				pNextField = pBackslash + 1;
			}
			int compareResult = 1;
			int menuId, menuItemCount = GetMenuItemCount(hThisMenu);
			for (menuId = 0; menuId < menuItemCount; menuId++) {
				mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA;
				mii.cch = _countof(szThisMenuItem);
				mii.dwTypeData = szThisMenuItem;
				GetMenuItemInfo(hThisMenu, menuId, TRUE, &mii);
				compareResult = mir_tstrcmp(szThisField, szThisMenuItem);
				if (compareResult == 0) {
					if (pNextField == NULL) {
						mii.fMask = MIIM_DATA;
						mii.dwItemData = i + 1;
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
				if ((int)mii.dwItemData - 1 > i)
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
					mii.dwItemData = i + 1;
				}
				InsertMenuItem(hThisMenu, menuId, TRUE, &mii);
				if (pNextField)
					hThisMenu = mii.hSubMenu;
			}
		} while (pNextField);
	}
	return hRootMenu;
}

int InitGroupServices(void)
{
	for (int i = 0;; i++) {
		char str[32];
		_itoa(i, str, 10);
		ptrT tszGroup(db_get_tsa(NULL, "CListGroups", str));
		if (tszGroup == NULL)
			break;

		CGroupInternal *p = new CGroupInternal(i, tszGroup);
		arByIds.insert(p);
		arByName.insert(p);
	}

	hGroupChangeEvent = CreateHookableEvent(ME_CLIST_GROUPCHANGE);
	return 0;
}

void UninitGroupServices(void)
{
	for (int i = 0; i < arByIds.getCount(); i++)
		delete arByIds[i];

	arByIds.destroy();
	arByName.destroy();
}
