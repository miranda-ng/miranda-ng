#include "common.h"

MCONTACT CSkypeProto::AddChatRoom(const char *chatname)
{
	MCONTACT hContact = GetContact(chatname);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, "ChatID", chatname);

		TCHAR title[MAX_PATH];
		mir_sntprintf(title, SIZEOF(title), _T("%s #%d"), TranslateT("Groupchat"), chatname);
		setTString(hContact, "Nick", title);

		DBVARIANT dbv;
		if (!db_get_s(NULL, "Chat", "AddToGroup", &dbv, DBVT_TCHAR))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}

		setByte(hContact, "ChatRoom", 1);
	}
	return hContact;
}