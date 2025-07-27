/*
Copyright © 2025 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

int CDeltaChatProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(GetMenuItem(PROTO_MENU_GRANT_AUTH), getByte(hContact, "Grant") > 0);
	return 0;
}

INT_PTR CDeltaChatProto::OnMenuHandleGrantAuth(WPARAM hContact, LPARAM)
{
	uint32_t chat_id = getDword(hContact, DB_KEY_CHATID);
	if (chat_id)
		dc_join_securejoin(m_context, getMStringA(hContact, "QR"));

	return 0;
}

INT_PTR CDeltaChatProto::OnMenuCopyQR(WPARAM, LPARAM)
{
	char *qr = dc_get_securejoin_qr(m_context, 0);
	Utils_ClipboardCopy(MClipAnsi(qr));
	dc_str_unref(qr);
	return 0;
}

INT_PTR CDeltaChatProto::OnMenuEnterQR(WPARAM, LPARAM)
{
	ENTER_STRING es = {};
	es.type = ESF_MULTILINE;
	es.caption = TranslateT("Enter QR code, received from another device");
	if (EnterString(&es)) {
		T2Utf qr(es.ptszResult);
		mir_free(es.ptszResult);

		auto *lot = dc_check_qr(m_context, qr);
		auto state = dc_lot_get_state(lot);
		dc_lot_unref(lot);

		bool bAddPrivate;
		if (state == DC_QR_ASK_VERIFYCONTACT)
			bAddPrivate = true;
		else if (state == DC_QR_ASK_VERIFYGROUP)
			bAddPrivate = false;
		else {
			MessageBoxW(0, TranslateT("Invalid or broken QR code"), _T(MODULENAME), MB_ICONERROR);
			return 0;
		}

		uint32_t chat_id = dc_join_securejoin(m_context, qr);
		if (chat_id) {
			if (bAddPrivate) {
				auto *pChat = dc_get_chat(m_context, chat_id);

				MCONTACT hContact = AddContact();				
				setDword(hContact, DB_KEY_CHATID, chat_id);
				setString(hContact, "QR", qr);

				if (auto *pContacts = dc_get_chat_contacts(m_context, chat_id)) {
					int contact_id = dc_array_get_id(pContacts, 0);
					if (contact_id)
						setDword(hContact, DB_KEY_DCID, contact_id);
					dc_array_unref(pContacts);
				}

				if (auto *pName = dc_chat_get_name(pChat)) {
					setUString(hContact, "Nick", pName);
					dc_str_unref(pName);
				}

				if (!dc_chat_can_send(pChat))
					Contact::Readonly(hContact, true);

				dc_chat_unref(pChat);

				setWord(hContact, "Status", ID_STATUS_ONLINE);
			}
		}
	}
	return 0;
}

void CDeltaChatProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	mi.pszService = "/ApplyQR";
	mi.name.a = LPGEN("Add chat using QR code");
	mi.position = 200001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CDeltaChatProto::OnMenuEnterQR);

	mi.pszService = "/CopyQR";
	mi.name.a = LPGEN("Copy QR to chat with me");
	mi.position++;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
	CreateProtoService(mi.pszService, &CDeltaChatProto::OnMenuCopyQR);
}
