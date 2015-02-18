#include "common.h"

INT_PTR __cdecl CToxProto::SetMyNickname(WPARAM wParam, LPARAM lParam)
{
	TCHAR *nickname = (wParam & SMNN_UNICODE) ? mir_u2t((WCHAR*)lParam) : mir_a2t((char*)lParam);

	setTString("Nick", nickname);
	tox_set_name(tox, (uint8_t*)(char*)_T2A(nickname), mir_tstrlen(nickname));
	return 0;
}
