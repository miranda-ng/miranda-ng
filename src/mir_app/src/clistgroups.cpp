/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#define MAX_GROUPNAME_LEN 256

struct CGroupInternal
{
	CGroupInternal(int _id, const wchar_t *_name) :
		groupId(_id),
		groupName(mir_wstrdup(_name))
	{}

	~CGroupInternal()
	{	mir_free(groupName);
	}

	int    groupId, oldId;
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
	if (ptszGroupName == nullptr)
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
	wchar_t newBaseName[MAX_GROUPNAME_LEN-1], newName[MAX_GROUPNAME_LEN];

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

	CLISTGROUPCHANGE grpChg = { nullptr, newName+1 };
	NotifyEventHooks(hGroupChangeEvent, 0, (LPARAM)&grpChg);

	return newId + 1;
}

MIR_APP_DLL(MGROUP) Clist_GroupCreate(MGROUP hParent, LPCTSTR ptszGroupName)
{
	// no name specified. just create a new group with a default name
	if (ptszGroupName == nullptr)
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

MIR_APP_DLL(wchar_t*) Clist_GroupGetName(MGROUP hGroup, uint32_t *pdwFlags)
{
	CGroupInternal *p = arByIds.find(hGroup-1);
	if (p == nullptr)
		return nullptr;

	if (pdwFlags != nullptr)
		*pdwFlags = p->groupName[0] & ~1;
	return p->groupName+1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isParentOf(const CMStringW &pParent, const wchar_t *p)
{
	if (mir_wstrncmp(pParent, p, pParent.GetLength()))
		return false;

	switch (p[pParent.GetLength()]) {
	case '\\':
	case 0:
		return true;
	default:
		return false;
	}
}

MIR_APP_DLL(int) Clist_GroupDelete(MGROUP hGroup)
{
	// get the name
	CGroupInternal *pGroup = arByIds.find(hGroup-1);
	if (pGroup == nullptr)
		return 1;

	if (Clist::ConfirmDelete) {
		wchar_t szQuestion[256 + 100];
		mir_snwprintf(szQuestion, TranslateT("Are you sure you want to delete group '%s'? This operation cannot be undone."), pGroup->groupName+1);
		if (MessageBox(g_clistApi.hwndContactList, szQuestion, TranslateT("Delete group"), MB_YESNO | MB_ICONQUESTION) == IDNO)
			return 1;
	}

	SetCursor(LoadCursor(nullptr, IDC_WAIT));
	
	// must remove setting from all child contacts too
	// children are demoted to the next group up, not deleted.
	CMStringW wszOldName(pGroup->groupName + 1), wszNewParent;
	{
		int idx = wszOldName.ReverseFind('\\');
		if (idx != -1)
			wszNewParent = wszOldName.Left(idx);
	}

	for (auto &hContact : Contacts()) {
		ptrW tszGroupName(Clist_GetGroup(hContact));
		if (!tszGroupName || !isParentOf(wszOldName, tszGroupName))
			continue;

		Clist_SetGroup(hContact, wszNewParent);

		CLISTGROUPCHANGE grpChg = { wszOldName, 0 };
		if (!wszNewParent.IsEmpty())
			grpChg.pszNewName = wszNewParent;
		NotifyEventHooks(hGroupChangeEvent, hContact, (LPARAM)&grpChg);
	}
	
	// shuffle list of groups up to fill gap
	for (auto &it : arByIds)
		it->oldId = it->groupId;

	int iGap = 0;
	for (auto &it : arByIds.rev_iter()) {
		if (!isParentOf(wszOldName, it->groupName + 1))
			continue;

		iGap++;
		arByName.remove(it);
		arByIds.removeItem(&it);
	}

	for (auto &it : arByIds) {
		it->groupId = arByIds.indexOf(&it);
		if (it->groupId != it->oldId)
			it->save();
	}

	for (int i = 0; i < iGap; i++) {
		char idstr[33];
		_itoa(arByIds.getCount()+i, idstr, 10);
		db_unset(0, "CListGroups", idstr);
	}

	SetCursor(LoadCursor(nullptr, IDC_ARROW));
	Clist_LoadContactTree();

	const CLISTGROUPCHANGE grpChg = { wszOldName, nullptr };
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
	for (auto &hContact : Contacts()) {
		ClcCacheEntry *cache = Clist_GetCacheEntry(hContact);
		if (!mir_wstrcmp(cache->tszGroup, oldName)) {
			Clist_SetGroup(hContact, szName);
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

	const CLISTGROUPCHANGE grpChg = { oldName, szName };
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
		Clist_LoadContactTree();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(wchar_t*) Clist_GetGroup(MCONTACT hContact)
{
	ptrW wszName(db_get_wsa(hContact, "CList", "Group"));
	return (mir_wstrlen(wszName)) ? wszName.detach() : nullptr;
}

MIR_APP_DLL(void) Clist_SetGroup(MCONTACT hContact, const wchar_t *pwszName)
{
	if (mir_wstrlen(pwszName))
		db_set_ws(hContact, "CList", "Group", pwszName);
	else
		db_unset(hContact, "CList", "Group");
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HMENU) Clist_GroupBuildMenu(int startId)
{
	if (arByIds.getCount() == 0)
		return nullptr;

	int nextMenuId = startId + 1; // to use it as MGROUP then

	HMENU hRootMenu = CreateMenu();
	for (int i = 0; i < arByIds.getCount(); i++) {
		const wchar_t *pNextField = arByIds[i]->groupName + 1;
		HMENU hThisMenu = hRootMenu;

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);

		wchar_t szThisField[MAX_GROUPNAME_LEN], szThisMenuItem[MAX_GROUPNAME_LEN];
		do {
			const wchar_t *pBackslash = wcschr(pNextField, '\\');
			if (pBackslash == nullptr) {
				mir_wstrncpy(szThisField, pNextField, _countof(szThisField));
				pNextField = nullptr;
			}
			else {
				mir_wstrncpy(szThisField, pNextField, min(_countof(szThisField), size_t(pBackslash - pNextField + 1)));
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
	for (auto &p : arByIds)
		delete p;

	arByIds.destroy();
	arByName.destroy();
}
