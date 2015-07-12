#include "stdafx.h"

static int lua_Paste(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	ptrT text(mir_utf8decodeT(luaL_checkstring(L, 2)));

	MessageWindowInputData mwid = { sizeof(MessageWindowInputData) };
	mwid.hContact = hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	MessageWindowData mwd = { sizeof(MessageWindowData) };

	INT_PTR res = ::CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd);
	lua_pushinteger(L, res);
	if (res)
		return 1;

	HWND hEdit = GetDlgItem(mwd.hwndWindow, 1002 /*IDC_MESSAGE*/);
	if (!hEdit) hEdit = GetDlgItem(mwd.hwndWindow, 1009 /*IDC_CHATMESSAGE*/);

	SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)text);

	return 1;
}

static int lua_Send(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	ptrT text(mir_utf8decodeT(luaL_checkstring(L, 2)));

	MessageWindowInputData mwid = { sizeof(MessageWindowInputData) };
	mwid.hContact = hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	MessageWindowData mwd = { sizeof(MessageWindowData) };

	INT_PTR res = ::CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd);
	lua_pushinteger(L, res);
	if (res)
		return 1;

	HWND hEdit = GetDlgItem(mwd.hwndWindow, 1002 /*IDC_MESSAGE*/);
	if (!hEdit) hEdit = GetDlgItem(mwd.hwndWindow, 1009 /*IDC_CHATMESSAGE*/);

	SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)text);
	SendMessage(mwd.hwndWindow, WM_COMMAND, IDOK, 0);

	return 1;
}

static luaL_Reg messageApi[] =
{
	{ "Paste", lua_Paste },
	{ "Send", lua_Send },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_message(lua_State *L)
{
	luaL_newlib(L, messageApi);

	return 1;
}
