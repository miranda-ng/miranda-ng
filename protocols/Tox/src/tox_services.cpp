#include "stdafx.h"

INT_PTR CToxProto::SetMyNickname(WPARAM wParam, LPARAM lParam)
{
	ptrT nickname((wParam & SMNN_UNICODE) ? mir_u2t((TCHAR*)lParam) : mir_a2t((char*)lParam));

	setTString("Nick", nickname);
	TOX_ERR_SET_INFO error;
	if (!tox_self_set_name(tox, (uint8_t*)(char*)ptrA(mir_utf8encodeT(nickname)), mir_tstrlen(nickname), &error))
	{
		debugLogA(__FUNCTION__": failed to set nick name");
		return 1;
	}

	return 0;
}
