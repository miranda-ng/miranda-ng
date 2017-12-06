#include "stdafx.h"

INT_PTR CToxProto::SetMyNickname(WPARAM wParam, LPARAM lParam)
{
	ptrW nickname((wParam & SMNN_UNICODE) ? mir_wstrdup((wchar_t*)lParam) : mir_a2u((char*)lParam));
	setWString("Nick", nickname);

	if (IsOnline()) {
		T2Utf szNick8(nickname);
		TOX_ERR_SET_INFO error;
		if (!tox_self_set_name(toxThread->Tox(), szNick8, mir_strlen(szNick8), &error))
			debugLogA(__FUNCTION__": failed to set nick name");
	}

	return 0;
}
