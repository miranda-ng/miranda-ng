#include "common.h"

int CToxProto::OnContactDeleted(MCONTACT hContact, LPARAM)
{
	if (!IsOnline())
	{
		return 1;
	}

	ToxBinAddress pubKey = ptrA(getStringA(hContact, TOX_SETTINGS_ID));
	if (!isChatRoom(hContact))
	{
		int32_t friendNumber = tox_get_friend_number(tox, pubKey);
		if (friendNumber == TOX_ERROR || tox_del_friend(tox, friendNumber) == TOX_ERROR)
		{
			return 1;
		}
	}
	else
	{
		OnLeaveChatRoom(hContact, 0);
		int groupNumber = 0; // ???
		if (groupNumber == TOX_ERROR || tox_del_groupchat(tox, groupNumber) == TOX_ERROR)
		{
			return 1;
		}
	}

	return 0;
}