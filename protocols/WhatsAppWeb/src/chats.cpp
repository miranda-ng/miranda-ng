/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::InitChat(WAUser *pUser, const JSONNode &pRoot)
{
	CMStringW wszId(pRoot["jid"].as_mstring()), wszNick(pRoot["name"].as_mstring());

	setWString(pUser->hContact, "Nick", wszNick);

	pUser->si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, wszNick);
	Chat_Control(m_szModuleName, wszId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, wszId, SESSION_ONLINE);
}
