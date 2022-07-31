#include "../stdafx.h"

static int message_Paste(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	ptrW text(mir_utf8decodeW(luaL_checkstring(L, 2)));

	MessageWindowData mwd;
	INT_PTR res = Srmm_GetWindowData(hContact, mwd);
	lua_pushinteger(L, res);
	if (res)
		return 1;

	HWND hEdit = GetDlgItem(mwd.hwndWindow, 3012 /*IDC_MESSAGE*/);
	if (!hEdit) hEdit = GetDlgItem(mwd.hwndWindow, 1009 /*IDC_CHATMESSAGE*/);

	SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)text);

	return 1;
}

static int message_Send(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	const char *message = luaL_checkstring(L, 2);

	INT_PTR res = 1;

	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (Contact::IsGroupChat(hContact, szProto)) {
		ptrW wszChatRoom(db_get_wsa(hContact, szProto, "ChatRoomID"));
		ptrW wszMessage(mir_utf8decodeW(message));
		res = Chat_SendUserMessage(szProto, wszChatRoom, wszMessage);
		lua_pushinteger(L, res);
	}
	else if ((res = ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)message)) != ACKRESULT_FAILED) {
		DBEVENTINFO dbei = {};
		dbei.szModule = MODULENAME;
		dbei.timestamp = time(0);
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.cbBlob = (uint32_t)mir_strlen(message);
		dbei.pBlob = (uint8_t*)mir_strdup(message);
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

	{ nullptr, nullptr }
};

LUAMOD_API int luaopen_m_message(lua_State *L)
{
	luaL_newlib(L, messageApi);

	MT<MessageWindowEventData>(L, "MessageWindowEventData")
		.Field(&MessageWindowEventData::uType, "Type", LUA_TINTEGER)
		.Field(&MessageWindowEventData::hContact, "hContact", LUA_TINTEGER)
		.Field(&MessageWindowEventData::uFlags, "Flags", LUA_TINTEGER);

	return 1;
}
