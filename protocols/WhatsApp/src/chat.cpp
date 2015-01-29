#include "common.h"

// #TODO Remove, as we are not using the chat-module for groups anymore

INT_PTR WhatsAppProto::OnJoinChat(WPARAM, LPARAM)
{
	return 0;
}

INT_PTR WhatsAppProto::OnLeaveChat(WPARAM, LPARAM)
{
	return 0;
}

int WhatsAppProto::OnChatOutgoing(WPARAM wParam, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	char *text;

	if (strcmp(hook->pDest->pszModule, m_szModuleName))
		return 0;

	switch (hook->pDest->iType) {
	case GC_USER_MESSAGE:
		text = mir_t2a_cp(hook->ptszText, CP_UTF8);
		{
			std::string msg = text;

			char *id = mir_t2a_cp(hook->pDest->ptszID, CP_UTF8);
			std::string chat_id = id;

			mir_free(text);
			mir_free(id);

			if (isOnline()) {
				MCONTACT hContact = this->ContactIDToHContact(chat_id);
				if (hContact) {
					debugLogA("**Chat - Outgoing message: %s", text);
					this->SendMsg(hContact, IS_CHAT, msg.c_str());

					GCDEST gcd = { m_szModuleName, hook->pDest->ptszID, GC_EVENT_MESSAGE };
					GCEVENT gce = { sizeof(gce), &gcd };
					gce.dwFlags = GCEF_ADDTOLOG;
					gce.ptszNick = mir_a2t(m_szNick.c_str());
					gce.ptszUID = mir_a2t(m_szJid.c_str());
					gce.time = time(NULL);
					gce.ptszText = hook->ptszText;
					gce.bIsMe = TRUE;
					CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

					mir_free((void*)gce.ptszUID);
					mir_free((void*)gce.ptszNick);
				}
			}
		}
		break;

	case GC_USER_LEAVE:
	case GC_SESSION_TERMINATE:
		break;
	}

	return 0;
}
