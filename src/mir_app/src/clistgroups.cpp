/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
	CGroupInternal(int _id, const wchar_t *_name) :
		groupId(_id),
		groupName(mir_wstrdup(_name))
		{}

	~CGroupInternal()
	{	mir_free(groupName);
	}

	int    groupId;
	wchar_t *groupName;

	void save()
	{
		char idstr[33];
		itoa(groupId, idstr, 10);
		db_set_ws(0, "CListGroups", idstr, groupName);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareGrpByName(const CGroupInternal *p1, const CGroupInternal *p2)
{	return mir_wstrcmp(p1->groupName+1, p2->groupName+1);
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
bool g_bGroupsLocked = false;

static mir_cs csGroups;

/////////////////////////////////////////////////////////////////////////////////////////

static int GroupNameExists(const wchar_t *ptszGroupName, int skipGroup)
{
	if (ptszGroupName == 0)
		return 0;

	wchar_t str[256];
	wcsncpy_s(str + 1, _countof(str) - 1, ptszGroupName, _TRUNCATE);

	CGroupInternal *tmp = (CGroupInternal*)_alloca(sizeof(CGroupInternal));
	tmp->groupName = (wchar_t*)str;
	if (tmp = arByName.find(tmp))
		return (skipGroup == tmp->groupId) ? 0 : tmp->groupId + 1;
	return 0;
}

MIR_APP_DLL(MGROUP) Clist_GroupExists(LPCTSTR ptszGroupName)
{
	return GroupNameExists(ptszGroupName, -1);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CreateGroupInternal(MGROUP hParent, const wchar_t *ptszName)
{
	wchar_t newBaseName[127], newName[128];

	const wchar_t *grpName = ptszName ? ptszName : TranslateT("New group");
	if (hParent) {
		CGroupInternal *tmp = arByIds.find(hParent-1);
		if (tmp == nullptr)
			return 0;

		mir_snwprintf(newBaseName, L"%s\\%s", tmp->groupName+1, grpName);
	}
	else wcsncpy_s(newBaseName, grpName, _TRUNCATE);

	mir_wstrncpy(newName + 1, newBaseName, _countof(newName) - 1);
	if (ptszName) {
		int id = GroupNameExists(newBaseName, -1);
		if (id)
			return id;
	}
	else {
		for (int idCopy = 1; GroupNameExists(newName + 1, -1); idCopy++)
			mir_snwprintf(newName + 1, _countof(newName) - 1, L"%s (%d)", newBaseName, idCopy);
	}

	int newId = arByIds.getCount();
	newName[0] = 1 | GROUPF_EXPANDED;   // 1 is required so we never get '\0'
	CGroupInternal *pNew = new CGroupInternal(newId, newName);
	arByIds.insert(pNew);
	arByName.insert(pNew);
	pNew->save();

	Clist_GroupAdded(newId + 1);

	CLISTGROUPCHANGE grpChg = { sizeof(grpChg), nullptr, newName };
	NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);

	return newId + 1;
}

MIR_APP_DLL(MGROUP) Clist_GroupCreate(MGROUP hParent, LPCTSTR ptszGroupName)
{
	// no name specified. just create a new group with a default name
	if (ptszGroupName == 0)
		return CreateGroupInternal(hParent, nullptr);

	if (ptszGroupName == nullptr || ptszGroupName[0] == '\0' || ptszGroupName[0] == '\\')
		return 0;

	wchar_t *tszName = NEWWSTR_ALLOCA(ptszGroupName);
	for (wchar_t *p = tszName; *p; p++) {
		if (*p == '\\') {
			*p = '\0';
			CreateGroupInternal(hParent, tszName);
			*p = '\\';
		}
	}
	return CreateGroupInternal(hParent, tszName);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(wchar_t*) Clist_GroupGetName(MGROUP hGroup, DWORD *pdwFlags)
{
	CGroupInternal *p = arByIds.find(hGroup-1);
	if (p == nullptr)
		return nullptr;

	if (pdwFlags != nullptr)
		*pdwFlags = p->groupName[0] & ~1;
	return p->groupName+1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GroupDelete(MGROUP hGroup)
{
	// get the name
	CGroupInternal *pGroup = arByIds.find(hGroup-1);
	if (pGroup == nullptr)
		return 1;

	if (db_get_b(0, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT)) {
		wchar_t szQuestion[256 + 100];
		mir_snwprintf(szQuestion, TranslateT("Are you sure you want to delete group '%s'? This operation cannot be undone."), pGroup->groupName+1);
		if (MessageBox(cli.hwndContactList, szQuestion, TranslateT("Delete group"), MB_YESNO | MB_ICONQUESTION) == IDNO)
			return 1;
	}

	SetCursor(LoadCursor(nullptr, IDC_WAIT));
	
	// must remove setting from all child contacts too
	// children are demoted to the next group up, not deleted.
	wchar_t *szNewParent = NEWWSTR_ALLOCA(pGroup->groupName+1);
	{
		wchar_t *pszLastBackslash = wcsrchr(szNewParent, '\\');
		if (pszLastBackslash)
			pszLastBackslash[0] = '\0';
		else
			szNewParent[0] = '\0';
	}

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ptrW tszGroupName(db_get_wsa(hContact, "CList", "Group"));
		if (mir_wstrcmp(tszGroupName, pGroup->groupName+1))
			continue;

		CLISTGROUPCHANGE grpChg = { sizeof(grpChg), nullptr, nullptr };
		grpChg.pszOldName = pGroup->groupName+1;
		if (szNewParent[0]) {
			db_set_ws(hContact, "CList", "Group", szNewParent);
			grpChg.pszNewName = szNewParent;
		}
		else {
			db_unset(hContact, "CList", "Group");
			grpChg.pszNewName = nullptr;
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
	db_unset(0, "CListGroups", idstr);
	
	// rename subgroups
	wchar_t szNewName[256];
	size_t len = mir_wstrlen(pGroup->groupName+1);
	for (int i = 0; i < arByIds.getCount(); i++) {
		CGroupInternal *p = arByIds[i];
		
		if (!wcsncmp(pGroup->groupName+1, p->groupName+1, len) && p->groupName[len+1] == '\\' && wcschr(p->groupName + len + 2, '\\') == nullptr) {
			if (szNewParent[0])
				mir_snwprintf(szNewName, L"%s\\%s", szNewParent, p->groupName + len + 2);
			else
				mir_wstrncpy(szNewName, p->groupName + len + 2, _countof(szNewName));
			Clist_GroupRename(i + 1, szNewName);
		}
	}

	SetCursor(LoadCursor(nullptr, IDC_ARROW));
	cli.pfnLoadContactTree();

	const CLISTGROUPCHANGE grpChg = { sizeof(grpChg), pGroup->groupName+1, nullptr };
	NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);

	delete(pGroup);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GroupMoveBefore(MGROUP hGroup, MGROUP hGroupBefore)
{
	if (hGroup == 0 || hGroup == hGroupBefore)
		return 0;

	hGroup--;
	CGroupInternal *pGroup = arByIds.find(hGroup);
	if (pGroup == nullptr)
		return 0;

	// shuffle list of groups up to fill gap
	int shuffleFrom, shuffleTo, shuffleStep;
	if (hGroupBefore == 0) {
		shuffleFrom = 0;
		shuffleTo = hGroup;
		shuffleStep = 1;
	}
	else {
		hGroupBefore--;
		CGroupInternal *pDest = arByIds.find(hGroupBefore);
		if (pDest == nullptr)
			return 0;

		if (hGroup < hGroupBefore) {
			shuffleFrom = hGroup;
			shuffleTo = hGroupBefore;
			shuffleStep = 1;
		}
		else {
			shuffleFrom = hGroupBefore;
			shuffleTo = hGroup;
			shuffleStep = -1;
		}
	}

	g_bGroupsLocked = true;
	arByIds.remove(pGroup);

	for (int i = shuffleFrom; i < shuffleTo; i++) {
		CGroupInternal *p = arByIds[i];
		p->groupId -= shuffleStep;
		p->save();
	}

	pGroup->groupId = hGroupBefore; // reinsert group back
	pGroup->save();
	arByIds.insert(pGroup);

	g_bGroupsLocked = false;
	Clist_BroadcastAsync(CLM_AUTOREBUILD, 0, 0);
	return hGroupBefore + 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int RenameGroupWithMove(int groupId, const wchar_t *szName, int move)
{
	if (GroupNameExists(szName, groupId)) {
		MessageBox(nullptr, TranslateT("You already have a group with that name. Please enter a unique name for the group."), TranslateT("Rename group"), MB_ICONERROR | MB_OK);
		return 1;
	}

	CGroupInternal *pGroup = arByIds.find(groupId);
	if (pGroup == nullptr)
		return 0;

	// do the change
	wchar_t *oldName = NEWWSTR_ALLOCA(pGroup->groupName+1);
	arByName.remove(pGroup);

	wchar_t str[256];
	str[0] = pGroup->groupName[0];
	mir_wstrncpy(str + 1, szName, _countof(str) - 1);

	replaceStrW(pGroup->groupName, str);
	pGroup->save();
	arByName.insert(pGroup);

	// must rename setting in all child contacts too
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ClcCacheEntry *cache = cli.pfnGetCacheEntry(hContact);
		if (!mir_wstrcmp(cache->tszGroup, oldName)) {
			db_set_ws(hContact, "CList", "Group", szName);
			replaceStrW(cache->tszGroup, szName);
		}
	}

	// rename subgroups
	size_t len = mir_wstrlen(oldName);
	for (int i = 0; i < arByIds.getCount(); i++) {
		if (i == groupId)
			continue;

		CGroupInternal *p = arByIds[i];
		if (!wcsncmp(p->groupName+1, oldName, len) && p->groupName[len+1] == '\\' && wcschr(p->groupName + len + 2, '\\') == nullptr) {
			wchar_t szNewName[256];
			mir_snwprintf(szNewName, L"%s\\%s", szName, p->groupName + len + 2);
			RenameGroupWithMove(i, szNewName, 0); // luckily, child groups will never need reordering
		}
	}

	// finally must make sure it's after any parent items
	if (move) {
		wcsncpy_s(str, szName, _TRUNCATE);
		wchar_t *pszLastBackslash = wcsrchr(str, '\\');
		if (pszLastBackslash != nullptr) {
			*pszLastBackslash = '\0';
			for (int i = 0; i < arByIds.getCount(); i++) {
				CGroupInternal *p = arByIds[i];
				if (!mir_wstrcmp(p->groupName+1, str)) {
					if (i >= groupId)
						Clist_GroupMoveBefore(groupId + 1, i + 2);
					break;
				}
			}
		}
	}

	const CLISTGROUPCHANGE grpChg = { sizeof(grpChg), oldName, (wchar_t*)szName };
	NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);
	return 0;
}

MIR_APP_DLL(int) Clist_GroupRename(MGROUP hGroup, const wchar_t *ptszNewName)
{
	return 0 != RenameGroupWithMove(hGroup-1, ptszNewName, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_GroupSetExpanded(MGROUP hGroup, int iNewState)
{
	CGroupInternal *pGroup = arByIds.find(hGroup-1);
	if (pGroup == nullptr)
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
	if (pGroup == nullptr)
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
		return nullptr;

	int nextMenuId = 100;

	HMENU hRootMenu = CreateMenu();
	for (int i = 0; i < arByIds.getCount(); i++) {
		const wchar_t *pNextField = arByIds[i]->groupName + 1;
		HMENU hThisMenu = hRootMenu;

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);

		wchar_t szThisField[128], szThisMenuItem[128];
		do {
			const wchar_t *pBackslash = wcschr(pNextField, '\\');
			if (pBackslash == nullptr) {
				mir_wstrncpy(szThisField, pNextField, _countof(szThisField));
				pNextField = nullptr;
			}
			else {
				mir_wstrncpy(szThisField, pNextField, min(_countof(szThisField), pBackslash - pNextField + 1));
				pNextField = pBackslash + 1;
			}
			int compareResult = 1;
			int menuId, menuItemCount = GetMenuItemCount(hThisMenu);
			for (menuId = 0; menuId < menuItemCount; menuId++) {
				mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA;
				mii.cch = _countof(szThisMenuItem);
				mii.dwTypeData = szThisMenuItem;
				GetMenuItemInfo(hThisMenu, menuId, TRUE, &mii);
				compareResult = mir_wstrcmp(szThisField, szThisMenuItem);
				if (compareResult == 0) {
					if (pNextField == nullptr) {
						mii.fMask = MIIM_DATA;
						mii.dwItemData = i + 1;
						SetMenuItemInfo(hThisMenu, menuId, TRUE, &mii);
					}
					else {
						if (mii.hSubMenu == nullptr) {
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
		ptrW tszGroup(db_get_wsa(0, "CListGroups", str));
		if (tszGroup == nullptr)
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
