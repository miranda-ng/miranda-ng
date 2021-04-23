/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::InitChat(WAUser *pUser, const WANode *pRoot)
{
	CMStringW wszId(Utf2T(pRoot->getAttr("jid"))), wszNick(Utf2T(pRoot->getAttr("name")));

	setWString(pUser->hContact, "Nick", wszNick);

	pUser->si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszId, wszNick);
	Chat_Control(m_szModuleName, wszId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, wszId, SESSION_ONLINE);
}
