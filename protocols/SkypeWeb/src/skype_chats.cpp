#include "common.h"

MCONTACT CSkypeProto::GetChat(const char *skypename)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA cSkypename(getStringA(hContact, "ChatID"));
		if (mir_strcmpi(skypename, cSkypename) == 0)
			break;
	}
	return hContact;
}

MCONTACT CSkypeProto::AddChatRoom(const char *chatname)
{
	MCONTACT hContact = GetChat(chatname);
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

void CSkypeProto::SetChatStatus(MCONTACT hContact, int iStatus)
{
	ptrT tszChatID(getTStringA(hContact, "ChatID"));
	if (tszChatID == NULL)
		return;

	GCDEST gcd = { m_szModuleName, tszChatID, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, (iStatus == ID_STATUS_OFFLINE) ? SESSION_OFFLINE : SESSION_ONLINE, (LPARAM)&gce);
}