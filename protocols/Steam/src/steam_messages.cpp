#include "stdafx.h"

int CSteamProto::UserIsTyping(MCONTACT hContact, int type)
{
	// NOTE: Steam doesn't support sending "user stopped typing" so we're sending only positive info
	if (type == PROTOTYPE_SELFTYPING_OFF)
		return 0;

	ptrA steamId(getStringA(hContact, DBKEY_STEAM_ID));
	//SendRequest(new SendTypingRequest(m_szAccessToken, m_szUmqId, steamId));
	return 0;
}
