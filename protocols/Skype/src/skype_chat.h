#pragma once

#include "skype.h"
#include <m_chat.h>

class ChatMember
{
private:
	wchar_t *sid;
	wchar_t *nick;
	int rank;
	WORD status;

public:
	CParticipant::Ref participant;

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

	ChatMember(const ChatMember &other)
	{
		this->sid = NULL;
		this->nick = NULL;
		this->operator=(other);
	}

	~ChatMember()
	{
		if (this->sid != NULL)
			::mir_free(this->sid);
		if (this->nick != NULL)
			::mir_free(this->nick);
	}

	void SetNick(const wchar_t *nick)
	{
		if (this->nick != NULL)
			::mir_free(this->nick);
		this->nick = ::mir_wstrdup(nick);
	}

	wchar_t *GetSid() const
	{
		return this->sid;
	}

	wchar_t *GetNick() const
	{
		if (this->nick == NULL)
			return this->sid;

		return this->nick;
	}

	void SetRank(int rank)
	{
		this->rank = rank;
	}

	int GetRank() const
	{
		return this->rank;
	}

	void SetStatus(int status)
	{
		this->status = status;
	}

	int GetStatus() const
	{
		return this->status;
	}

	void SetPaticipant(const ParticipantRef &participant)
	{
		this->participant = participant;
		//this->participant.fetch();
	}

	static int Compare(const ChatMember *p1, const ChatMember *p2)
	{
		return ::lstrcmpi(p1->sid, p2->sid);
	}

	bool operator==(const ChatMember &other) const
	{
		return ::lstrcmp(this->sid, other.sid) == 0;
	}
	
	bool operator!=(const ChatMember &other) const
	{
		return !(*this == other);
	}

	ChatMember& operator=(const ChatMember &other)
	{
        if (this == &other)
            return *this;

		if (this->sid != NULL)
			::mir_free(this->sid);
		this->sid = ::mir_wstrdup(other.sid);

		if (this->nick != NULL)
			::mir_free(this->nick);
		this->nick = ::mir_wstrdup(other.nick);

		this->rank = other.rank;
		this->status = other.status;
		this->participant = other.participant;
        return *this;
	}
};

class ChatRoom
{
private:
	wchar_t *cid;
	wchar_t *name;

	HANDLE hContact;

	

	LIST<ChatMember> members;

	CSkypeProto *ppro;

	static wchar_t *Roles[];

	ChatRoom(const wchar_t *cid);

	HANDLE AddChatRoom();

	inline static int CompareMembers(const ChatMember *p1, const ChatMember *p2) { return ChatMember::Compare(p1, p2); }

	bool IsMe(const ChatMember &item) const;
	void SendEvent(const ChatMember &item, int eventType, DWORD timestamp = time(NULL), DWORD flags = GCEF_ADDTOLOG, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL);
	
	void UpdateMember(const ChatMember &item, DWORD timestamp = time(NULL));
	void KickMember(const ChatMember &item, const ChatMember *author, DWORD timestamp = time(NULL));
	void RemoveMember(const ChatMember &item, DWORD timestamp = time(NULL));

public:
	ChatMember *me;
	CConversation::Ref conversation;

	ChatRoom(const wchar_t *cid, const wchar_t *name, CSkypeProto *ppro);
	~ChatRoom();	

	void Start(bool showWindow = false);
	void Start(const ParticipantRefs &participants, bool showWindow = false);

	void LeaveChat();

	void SendEvent(const wchar_t *sid, int eventType, DWORD timestamp = time(NULL), DWORD flags = GCEF_ADDTOLOG, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL);

	bool IsMe(const wchar_t *sid) const;

	//
	ChatMember *FindChatMember(const wchar_t *sid);

	void AddMember(const ChatMember &item, const ChatMember *author, DWORD timestamp = time(NULL));

	void UpdateMember(const wchar_t *sid, const wchar_t *nick, int rank, int status, DWORD timestamp = time(NULL));
	void KickMember(const wchar_t *sid, const wchar_t *author, DWORD timestamp = time(NULL));
	void RemoveMember(const wchar_t *sid, DWORD timestamp = time(NULL));

	void OnEvent(const ConversationRef &conversation, const MessageRef &message);

	static int __cdecl OnGCEventHook(WPARAM, LPARAM lParam);
	static int __cdecl OnGCMenuHook(WPARAM, LPARAM lParam);
};