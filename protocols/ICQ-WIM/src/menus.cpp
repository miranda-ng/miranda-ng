/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

#define MenuExecService "/NSExecMenu"

int CIcqProto::OnPrebuildMenu(WPARAM hContact, LPARAM lParam)
{
	if (!Proto_IsProtoOnContact(hContact, m_szModuleName)) {
		Menu_ShowItem(hmiForward, false);
		Menu_ShowItem(hmiConvert, false);
	}
	else {
		auto *dbei = (DB::EventInfo *)lParam;
		Menu_ShowItem(hmiForward, dbei->eventType == EVENTTYPE_MESSAGE || dbei->eventType == EVENTTYPE_FILE);

		ptrW wszText(DbEvent_GetTextW(dbei));
		Menu_ShowItem(hmiConvert, fileText2url(wszText.get()));
	}
	return 0;
}

void CIcqProto::InitMenus()
{
	if (!HookProtoEvent(ME_NS_PREBUILDMENU, &CIcqProto::OnPrebuildMenu))
		return;

	CreateProtoService(MenuExecService, &CIcqProto::SvcExecMenu);

	CMStringA szServiceName(FORMAT, "%s%s", m_szModuleName, MenuExecService);
	CMenuItem mi(&g_plugin);
	mi.pszService = szServiceName;

	mi.position = 1000000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_FORWARD);
	mi.name.a = LPGEN("Forward");
	hmiForward = Menu_AddNewStoryMenuItem(&mi, 1);

	mi.position++;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	mi.name.a = LPGEN("Add to favorites");
	hmiConvert = Menu_AddNewStoryMenuItem(&mi, 2);

	mi.position++;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_FILE);
	mi.name.a = LPGEN("Convert a message into a file transfer");
	hmiConvert = Menu_AddNewStoryMenuItem(&mi, 3);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Dialog for message forwarding

class CForwardDlg : public CIcqDlgBase
{
	CCtrlClc m_clist;
	MEVENT m_hEvent;

	void FilterList(CCtrlClc *)
	{
		for (auto &hContact : Contacts())
			if (!Proto_IsProtoOnContact(hContact, m_proto->m_szModuleName))
				if (HANDLE hItem = m_clist.FindContact(hContact))
					m_clist.DeleteItem(hItem);
	}

	void ResetListOptions(CCtrlClc *)
	{
		m_clist.SetHideEmptyGroups(true);
		m_clist.SetHideOfflineRoot(true);
	}

public:
	CForwardDlg(CIcqProto *ppro, MEVENT hEvent) :
		CIcqDlgBase(ppro, IDD_FORWARD),
		m_hEvent(hEvent),
		m_clist(this, IDC_CLIST)
	{
		m_clist.OnNewContact =
			m_clist.OnListRebuilt = Callback(this, &CForwardDlg::FilterList);
		m_clist.OnOptionsChanged = Callback(this, &CForwardDlg::ResetListOptions);
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_HIDEOFFLINE | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		m_clist.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);
		ResetListOptions(&m_clist);
		FilterList(&m_clist);
		return true;
	}

	bool OnApply() override
	{
		for (auto &hContact : m_proto->AccContacts())
			if (HANDLE hItem = m_clist.FindContact(hContact))
				if (m_clist.GetCheck(hItem))
					m_proto->ForwardMessage(m_hEvent, hContact);

		return true;
	}
};

void CIcqProto::ForwardMessage(MEVENT hEvent, MCONTACT to)
{
	DB::EventInfo dbei(hEvent);
	if (!dbei || !dbei.szId || mir_strcmp(dbei.szModule, m_szModuleName))
		return;

	CMStringW wszId(GetUserId(dbei.hContact));
	ptrW wszText(DbEvent_GetTextW(&dbei));

	JSONNode parts(JSON_ARRAY);
	JSONNode msgText; msgText << CHAR_PARAM("mediaType", "forward") << WCHAR_PARAM("sn", wszId) << INT_PARAM("time", dbei.timestamp)
		<< CHAR_PARAM("msgId", dbei.szId) << WCHAR_PARAM("text", wszText);
	parts.push_back(msgText);

	SendMessageParts(to, parts);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CIcqProto::SvcExecMenu(WPARAM iCommand, LPARAM pHandle)
{
	MEVENT hEvent = NS_GetCurrent(HANDLE(pHandle));
	if (!hEvent)
		return 0;

	switch (iCommand) {
	case 1: // forward message
		CForwardDlg(this, hEvent).DoModal();
		break;

	case 2: // Add to favorites
		ForwardMessage(hEvent, m_hFavContact);

	case 3: // convert a message into a file transfer
		DB::EventInfo dbei(hEvent);
		if (!dbei)
			return 0;

		IcqFileInfo *pFileInfo = nullptr;
		CMStringW wszText(ptrW(DbEvent_GetTextW(&dbei)));
		if (CheckFile(dbei.hContact, wszText, pFileInfo)) {
			if (!pFileInfo || pFileInfo->bIsSticker) {
				// sticker is a simple text message prcoessed by SmileyAdd
				T2Utf szBody(wszText);
				mir_free(dbei.pBlob);
				dbei.cbBlob = (int)mir_strlen(szBody.get());
				dbei.pBlob = szBody.detach();
			}
			else {
				// create the offline file event
				dbei.eventType = EVENTTYPE_FILE;

				DB::FILE_BLOB blob(pFileInfo->wszDescr, wszText);
				blob.setUrl(pFileInfo->szOrigUrl);
				blob.setSize(pFileInfo->dwFileSize);
				blob.write(dbei);
			}
			db_event_edit(hEvent, &dbei);
		}
	}
	return 0;
}
