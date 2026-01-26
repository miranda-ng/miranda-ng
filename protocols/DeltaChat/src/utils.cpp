/*
Copyright © 2025-26 Miranda NG team

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

/////////////////////////////////////////////////////////////////////////////////////////

void dc_set_config(dc_context_t *context, const char *key, const wchar_t *value)
{
	dc_set_config(context, key, T2Utf(value).get());
}

void dc_set_config(dc_context_t *context, const char *key, int value)
{
	char buf[100];
	itoa(value, buf, 10);
	dc_set_config(context, key, buf);
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CDeltaChatProto::AddContact()
{
	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	if (mir_wstrlen(m_defaultGroup))
		Clist_SetGroup(hContact, m_defaultGroup);

	return hContact;
}

MCONTACT CDeltaChatProto::FindChat(uint32_t chat_id)
{
	for (auto &cc : AccContacts())
		if (getDword(cc, DB_KEY_DCID) == chat_id)
			return cc;

	return 0;
}

MCONTACT CDeltaChatProto::FindContact(uint32_t chat_id)
{
	for (auto &cc : AccContacts())
		if (getDword(cc, DB_KEY_CHATID) == chat_id)
			return cc;

	return 0;
}
