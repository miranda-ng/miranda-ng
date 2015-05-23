#include "stdafx.h"

INT_PTR CToxProto::SetMyNickname(WPARAM wParam, LPARAM lParam)
{
	ptrT nickname((wParam & SMNN_UNICODE) ? mir_u2t((TCHAR*)lParam) : mir_a2t((char*)lParam));
	setTString("Nick", nickname);

	if (IsOnline())
	{
		T2Utf szNick8(nickname);
		TOX_ERR_SET_INFO error;
		if (!tox_self_set_name(tox, szNick8, mir_strlen(szNick8), &error))
			debugLogA(__FUNCTION__": failed to set nick name");
	}

	return 0;
}
