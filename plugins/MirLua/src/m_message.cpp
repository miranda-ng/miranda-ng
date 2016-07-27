#include "stdafx.h"

static int message_Paste(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	ptrW text(mir_utf8decodeW(luaL_checkstring(L, 2)));

	MessageWindowInputData mwid = { sizeof(MessageWindowInputData) };
	mwid.hContact = hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	MessageWindowData mwd = { sizeof(MessageWindowData) };

	INT_PTR res = CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd);
	lua_pushinteger(L, res);
	if (res)
		return 1;

	HWND hEdit = GetDlgItem(mwd.hwndWindow, 1002 /*IDC_MESSAGE*/);
	if (!hEdit) hEdit = GetDlgItem(mwd.hwndWindow, 1009 /*IDC_CHATMESSAGE*/);

	SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)text);

	return 1;
}

static int message_Send(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	const char *message = luaL_checkstring(L, 2);

	INT_PTR res = 1;

	const char *szProto = GetContactProto(hContact);
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == TRUE)
	{
		ptrW tszChatRoom(db_get_tsa(hContact, szProto, "ChatRoomID"));
		GCDEST gcd = { szProto, tszChatRoom, GC_EVENT_SENDMESSAGE };
		GCEVENT gce = { sizeof(gce), &gcd };
		gce.bIsMe = TRUE;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.ptszText = mir_utf8decodeW(message);
		gce.time = time(NULL);

		res = CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
		lua_pushinteger(L, res);

		mir_free((void*)gce.ptszText);
	}
	else if ((res = ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)message)) != ACKRESULT_FAILED)
	{
		DBEVENTINFO dbei;
		dbei.cbSize = sizeof(dbei);
		dbei.szModule = MODULE;
		dbei.timestamp = time(NULL);
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.cbBlob = mir_strlen(message);
		dbei.pBlob = (PBYTE)mir_strdup(message);
		dbei.flags = DBEF_UTF | DBEF_SENT;
		db_event_add(hContact, &dbei);

		lua_pushinteger(L, res);
		return 1;
	}

	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg messageApi[] =
{
	{ "Paste", message_Paste },
	{ "Send", message_Send },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_message(lua_State *L)
{
	luaL_newlib(L, messageApi);

	MT<MessageWindowEventData>(L, "MessageWindowEventData")
		.Field(&MessageWindowEventData::szModule, "Module", LUA_TSTRINGA)
		.Field(&MessageWindowEventData::uType, "Type", LUA_TINTEGER)
		.Field(&MessageWindowEventData::hContact, "hContact", LUA_TINTEGER)
		.Field(&MessageWindowEventData::uFlags, "Flags", LUA_TINTEGER);

	return 1;
}
