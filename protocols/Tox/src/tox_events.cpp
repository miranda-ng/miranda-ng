#include "common.h"

int CToxProto::OnModulesLoaded(WPARAM, LPARAM)
{
	hProfileFolderPath = FoldersRegisterCustomPathT("Tox", Translate("Profiles folder"), MIRANDA_USERDATAT);

	return 0;
}

int CToxProto::OnContactDeleted(MCONTACT hContact, LPARAM)
{
	if (!IsOnline())
	{
		return -1;
	}

	if (!isChatRoom(hContact))
	{
		int32_t friendNumber = GetToxFriendNumber(hContact);
		TOX_ERR_FRIEND_DELETE error;
		if (!tox_friend_delete(tox, friendNumber, &error))
		{
			debugLogA(__FUNCTION__": failed to delete friend (%d)", error);
			return error;
		}
	}
	/*else
	{
		OnLeaveChatRoom(hContact, 0);
		int groupNumber = 0; // ???
		if (groupNumber == TOX_ERROR || tox_del_groupchat(tox, groupNumber) == TOX_ERROR)
		{
			return 1;
		}
	}*/

	return 0;
}