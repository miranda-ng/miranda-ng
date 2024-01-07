/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

INT_PTR CIcqProto::UploadGroups(WPARAM, LPARAM)
{
	for (auto &it : AccContacts()) {
		if (isChatRoom(it))
			continue;

		ptrW wszIcqGroup(getWStringA(it, "IcqGroup"));
		if (wszIcqGroup == nullptr)
			continue;

		ptrW wszMirGroup(Clist_GetGroup(it));
		if (!wszMirGroup)
			wszMirGroup = mir_wstrdup(L"General");
		if (mir_wstrcmp(wszIcqGroup, wszMirGroup))
			MoveContactToGroup(it, wszIcqGroup, wszMirGroup);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CGroupEditDlg : public CIcqDlgBase
{
	CCtrlListView groups;

public:

	static CGroupEditDlg *pDlg;

	CGroupEditDlg(CIcqProto *ppro) :
		CIcqDlgBase(ppro, IDD_EDITGROUPS),
		groups(this, IDC_GROUPS)
	{
		groups.OnBuildMenu = Callback(this, &CGroupEditDlg::onMenu);
	}

	void RefreshGroups()
	{
		groups.DeleteAllItems();

		for (auto &it : m_proto->m_arGroups)
			groups.AddItem(it->wszName, 0, (LPARAM)it);
	}

	bool OnInitDialog() override
	{
		pDlg = this;
		groups.AddColumn(0, TranslateT("Name"), 300);
		RefreshGroups();
		return true;
	}

	void OnDestroy() override
	{
		pDlg = nullptr;
	}

	void onMenu(void *)
	{
		int cur = groups.GetSelectionMark();
		if (cur == -1)
			return;

		IcqGroup *pGroup = (IcqGroup *)groups.GetItemData(cur);

		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, 1, TranslateT("Rename"));
		AppendMenu(hMenu, MF_STRING, 2, TranslateT("Delete"));

		POINT pt;
		GetCursorPos(&pt);
		int cmd = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
		DestroyMenu(hMenu);

		if (cmd == 1) { // rename
			ENTER_STRING es = {};
			es.szModuleName = m_proto->m_szModuleName;
			es.caption = TranslateT("Enter new group name");
			if (!EnterString(&es))
				return;

			m_proto->Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "/buddylist/renameGroup")
				<< AIMSID(m_proto) << WCHAR_PARAM("oldGroup", pGroup->wszSrvName) << GROUP_PARAM("newGroup", es.ptszResult));

			mir_free(es.ptszResult);
		}
		else if (cmd == 2) { // delete
			m_proto->Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "/buddylist/removeGroup")
				<< AIMSID(m_proto) << WCHAR_PARAM("group", pGroup->wszSrvName));
		}
	}
};

CGroupEditDlg *CGroupEditDlg::pDlg = nullptr;

INT_PTR CIcqProto::EditGroups(WPARAM, LPARAM)
{
	(new CGroupEditDlg(this))->Show();
	return 0;
}

void RefreshGroups(void)
{
	if (CGroupEditDlg::pDlg != nullptr)
		CGroupEditDlg::pDlg->RefreshGroups();
}
