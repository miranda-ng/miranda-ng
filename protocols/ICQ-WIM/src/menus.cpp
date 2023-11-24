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

void CIcqProto::InitMenus()
{
	if (!HookProtoEvent(ME_NS_PREBUILDMENU, &CIcqProto::OnPrebuildMenu))
		return;

	CreateProtoService(MenuExecService, &CIcqProto::SvcExecMenu);

	CMStringA szServiceName(FORMAT, "%s%s", m_szModuleName, MenuExecService);
	CMenuItem mi(&g_plugin);
	mi.pszService = szServiceName;

	mi.position = 1000000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_FILE);
	mi.name.a = LPGEN("Convert a message into a file transfer");
	hmiConvert = Menu_AddNewStoryMenuItem(&mi, 1);
}

INT_PTR CIcqProto::SvcExecMenu(WPARAM iCommand, LPARAM pHandle)
{
	// convert a message into a file transfer
	if (iCommand == 1) {
		if (MEVENT hEvent = NS_GetCurrent(HANDLE(pHandle))) {
			DB::EventInfo dbei(hEvent);
			if (!dbei)
				return 0;

			IcqFileInfo *pFileInfo = nullptr;
			CMStringW wszText(ptrW(DbEvent_GetTextW(&dbei, CP_UTF8)));
			if (CheckFile(db_event_getContact(hEvent), wszText, pFileInfo)) {
				if (pFileInfo->bIsSticker) {
					// sticker is a simple text message prcoessed by SmileyAdd
					T2Utf szBody(wszText);
					mir_free(dbei.pBlob);
					dbei.pBlob = (uint8_t*)szBody.get();
					dbei.cbBlob = (int)mir_strlen(szBody.get());
				}
				else {
					// create the offline file event
					dbei.eventType = EVENTTYPE_FILE;

					DB::FILE_BLOB blob(pFileInfo->wszDescr, L"");
					blob.setUrl(pFileInfo->szOrigUrl);
					blob.setSize(pFileInfo->dwFileSize);
					blob.write(dbei);
				}
				db_event_edit(hEvent, &dbei);
			}
		}
	}
	return 0;
}

int CIcqProto::OnPrebuildMenu(WPARAM, LPARAM lParam)
{
	auto *dbei = (DB::EventInfo *)lParam;
	ptrW wszText(DbEvent_GetTextW(dbei, CP_UTF8));
	Menu_ShowItem(hmiConvert, 0 == mir_wstrncmp(wszText, L"https://files.icq.net/get/", 26));
	return 0;
}
