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
	
	CConversation::Ref conversation;

	MCONTACT hContact;	

	OBJLIST<ChatMember> members;

	CSkypeProto *ppro;

	ChatRoom(const wchar_t *cid);

	inline static int CompareMembers(const ChatMember *p1, const ChatMember *p2) { return ChatMember::Compare(p1, p2); }

	void CreateChatSession(bool showWindow = false);

	bool IsMe(const ChatMember &item) const;
	bool IsSys(const ChatMember &item) const;
	void SendEvent(const ChatMember &item, int eventType, DWORD timestamp = time(NULL), DWORD flags = GCEF_ADDTOLOG, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL);
	
	void UpdateMember(const ChatMember &item, DWORD timestamp = time(NULL));
	void KickMember(const ChatMember &item, const ChatMember *author, DWORD timestamp = time(NULL));
	void RemoveMember(const ChatMember &item, DWORD timestamp = time(NULL));

public:
	ChatMember *me;
	ChatMember *sys;

	static wchar_t *Roles[];

	ChatRoom(const wchar_t *cid, const wchar_t *name, CSkypeProto *ppro);
	~ChatRoom();

	MCONTACT GetContactHandle() const;

	void SetTopic(const wchar_t *topic);
	wchar_t *GetUri();
	void ShowWindow();

	void Invite(const StringList &contacts);
	void Start(const ConversationRef &conversation, bool showWindow = false);
	//void Join(const wchar_t *joinBlob, bool showWindow = false);

	void SendMessage(const wchar_t *text);

	void LeaveChat();
	void LeaveChatAndDelete();

	void SendEvent(const wchar_t *sid, int eventType, DWORD timestamp = time(NULL), DWORD flags = GCEF_ADDTOLOG, DWORD itemData = 0, const wchar_t *status = NULL, const wchar_t *message = NULL);

	bool IsMe(const wchar_t *sid) const;
	bool IsSys(const wchar_t *sid) const;

	ChatMember *FindChatMember(const wchar_t *sid);

	void AddMember(const ChatMember &item, const ChatMember &author, DWORD timestamp = time(NULL));

	void UpdateMemberNick(ChatMember *member, const wchar_t *nick, DWORD timestamp = time(NULL));
	void UpdateMemberRole(ChatMember *member, int role, const ChatMember &author = NULL, DWORD timestamp = time(NULL));
	void UpdateMemberStatus(ChatMember *member, int status, DWORD timestamp = time(NULL));
	
	void UpdateMember(const wchar_t *sid, const wchar_t *nick, int role, int status, DWORD timestamp = time(NULL));

	//void GiveMember(const wchar_t *sid);
	void AddApplicant(const ChatMember *sid);

	void KickMember(const wchar_t *sid, const wchar_t *author, DWORD timestamp = time(NULL));
	void RemoveMember(const wchar_t *sid, DWORD timestamp = time(NULL));

	void OnEvent(const ConversationRef &conversation, const MessageRef &message);
	void OnChange(const ConversationRef &conversation, int prop);

	void OnParticipantChanged(const ParticipantRef &participant, int prop);

	static void Create(const ChatRoomParam *param, CSkypeProto *ppro);
	static void Join(const wchar_t *joinBlob, CSkypeProto *ppro);
};
