#pragma once

#include "skype.h"
#include <m_chat.h>

class ChatMember
{
public:
	wchar_t *sid;
	wchar_t *nick;
	int rank;
	WORD status;

	ChatMember()
	{
		this->sid = NULL;
	}

	ChatMember(const wchar_t *sid)
	{
		this->sid = ::mir_wstrdup(sid);
	}

	~ChatMember()
	{
		if (this->sid != NULL)
			::mir_free(this->sid);
	}

	bool operator==(const ChatMember &other) const
	{
		return ::lstrcmp(this->sid, other.sid) == 0;
	}
	
	bool operator!=(const ChatMember &other) const
	{
		return !(*this == other);
	}

	/*ChatMember& operator=(const ChatMember& right)
	{
        if (this == &right)
            return *this;

		::mir_free(this->sid);
		this->sid = ::mir_wstrdup(right.sid);
        return *this;
	}*/
};

class ChatRoom
{
public:
	wchar_t *cid;
	wchar_t *topic;

	ChatMember *me;
	LIST<ChatMember> members;

	CSkypeProto *ppro;

	static wchar_t *Roles[];

	ChatRoom(const wchar_t *cid);
	ChatRoom(ChatMember *me);

	void Start(bool showWindow = false);
	void LeaveChat();

	void SendChatEvent(const wchar_t *sid, int eventType, DWORD flags = GCEF_ADDTOLOG, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL, DWORD timestamp = time(NULL));

	void Add(ChatMember *member);
	void Add(const wchar_t *sid, int rank = 0, WORD status = ID_STATUS_OFFLINE);

private:
	static int SortMembers(const ChatMember *p1, const ChatMember *p2);

	int __cdecl OnGCEventHook(WPARAM, LPARAM lParam);
	int __cdecl OnGCMenuHook(WPARAM, LPARAM lParam);
};