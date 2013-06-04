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
		this->nick = NULL;
	}

	ChatMember(const wchar_t *sid)
	{
		this->sid = ::mir_wstrdup(sid);
		this->nick = NULL;
	}

	~ChatMember()
	{
		if (this->sid != NULL)
			::mir_free(this->sid);
		if (this->nick != NULL)
			::mir_free(this->nick);
	}

	bool operator==(const ChatMember &other) const
	{
		return ::lstrcmp(this->sid, other.sid) == 0;
	}
	
	bool operator!=(const ChatMember &other) const
	{
		return !(*this == other);
	}

	ChatMember& operator=(const ChatMember& right)
	{
        if (this == &right)
            return *this;

		::mir_free(this->sid);
		::mir_free(this->nick);
		this->sid = ::mir_wstrdup(right.sid);
		this->nick = ::mir_wstrdup(right.nick);
		this->rank = right.rank;
		this->status = right.status;
        return *this;
	}
};

class ChatRoom
{
	friend class ChatList;

private:
	wchar_t *cid;
	wchar_t *name;

	HANDLE hContact;

	ChatMember *me;

	LIST<ChatMember> members;

	CSkypeProto *ppro;

	static wchar_t *Roles[];

	ChatRoom(const wchar_t *cid);

	HANDLE AddChatRoom();

	inline static int CompareMembers(const ChatMember *p1, const ChatMember *p2) { return ::lstrcmpi(p1->sid, p2->sid); }

	void AddMember(ChatMember *member, DWORD timestamp, int flag);

public:
	ChatRoom(const wchar_t *cid, const wchar_t *name, CSkypeProto *ppro);
	~ChatRoom();	

	void Start(bool showWindow = false);
	void Start(const ParticipantRefs &participants, bool showWindow = false);

	void LeaveChat();

	void SendEvent(ChatMember *member, int eventType, DWORD timestamp = time(NULL), DWORD flags = GCEF_ADDTOLOG, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL);
	void SendEvent(const wchar_t *sid, int eventType, DWORD timestamp = time(NULL), DWORD flags = GCEF_ADDTOLOG, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL);

	bool IsMe(const wchar_t *sid) const;
	bool IsMe(ChatMember *member) const;

	ChatMember *FindChatMember(ChatMember *item);
	ChatMember *FindChatMember(const wchar_t *sid);

	void AddMember(ChatMember *member, DWORD timestamp);
	void AddMember(ChatMember *member);

	void UpdateMember(const wchar_t *sid, const wchar_t *nick, int rank, int status, DWORD timestamp = time(NULL), DWORD flags = GCEF_ADDTOLOG);

	void KickMember(ChatMember *member, const ChatMember *kicker, DWORD timestamp = time(NULL));
	void KickMember(const wchar_t *sid, const wchar_t *kicker, DWORD timestamp = time(NULL));

	void RemoveMember(ChatMember *member, DWORD timestamp = time(NULL));
	void RemoveMember(const wchar_t *sid, DWORD timestamp = time(NULL));

	void OnEvent(const ConversationRef &conversation, const MessageRef &message);

	static int __cdecl OnGCEventHook(WPARAM, LPARAM lParam);
	static int __cdecl OnGCMenuHook(WPARAM, LPARAM lParam);
};