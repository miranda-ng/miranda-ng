#include "skype.h"
#include "skype_chat.h"

enum CHAT_LIST_MENU
{
	ICM_CANCEL,

	ICM_DETAILS,
	ICM_AUTH_REQUEST,
	ICM_CONF_INVITE,
	ICM_ROLE, ICM_ROLE_ADMIN, ICM_ROLE_SPEAKER, ICM_ROLE_WRITER, ICM_ROLE_SPECTATOR,
	ICM_ADD, ICM_KICK, ICM_BAN,
	ICM_COPY_SID, ICM_COPY_URI
};

static struct gc_item crListItems[] =
{
	{ LPGENT("&User details"),        ICM_DETAILS,            MENU_ITEM      }, 
	{ LPGENT("&Request auth"),        ICM_AUTH_REQUEST,       MENU_ITEM      },
	{ NULL,                           0,                      MENU_SEPARATOR },
	{ LPGENT("Invite to conference"), ICM_CONF_INVITE,        MENU_ITEM      },
	{ NULL,                           0,                      MENU_SEPARATOR },
	{ LPGENT("Set &role"),            ICM_ROLE,               MENU_NEWPOPUP  },
	{ LPGENT("&Master"),              ICM_ROLE_ADMIN,         MENU_POPUPITEM },
	{ LPGENT("&Helper"),              ICM_ROLE_SPEAKER,       MENU_POPUPITEM },
	{ LPGENT("&User"),                ICM_ROLE_WRITER,        MENU_POPUPITEM },
	{ LPGENT("&Listener"),            ICM_ROLE_SPECTATOR,     MENU_POPUPITEM },
	{ NULL,                           0,                      MENU_SEPARATOR },
	{ LPGENT("&Add"),                 ICM_ADD,                MENU_ITEM      },
	{ LPGENT("&Kick"),                ICM_KICK,               MENU_ITEM      },
	{ LPGENT("Outlaw (&ban)"),        ICM_BAN,                MENU_ITEM      },	
	{ NULL,                           0,                      MENU_SEPARATOR },
	{ LPGENT("Copy &Skype name"),      ICM_COPY_SID,           MENU_ITEM      },
	{ LPGENT("Copy room &URI"),       ICM_COPY_URI,           MENU_ITEM      }
};

static void CheckChatMenuItem(CHAT_LIST_MENU checkedId)
{
	for (int i = 0; i < SIZEOF(crListItems); i++)
	{
		if (crListItems[i].dwID == checkedId)
		{
			if (crListItems[i].uType == MENU_ITEM)
				crListItems[i].uType = MENU_CHECK;
			else if (crListItems[i].uType == MENU_POPUPITEM)
				crListItems[i].uType = MENU_POPUPCHECK;
			break;
		}
	}
}

static void DisableChatMenuItem(CHAT_LIST_MENU disabledId)
{
	for (int i = 0; i < SIZEOF(crListItems); i++)
	{
		if (crListItems[i].dwID == disabledId)
		{
			crListItems[i].bDisabled = TRUE;
			break;
		}
	}
}

static void DisableChatMenuItems(CHAT_LIST_MENU disabledIds[])
{
	for (int i = 0; i < SIZEOF(disabledIds); i++)
		DisableChatMenuItem(disabledIds[i]);
}

static void ResetChatMenuItem()
{
	for (int i = 0; i < SIZEOF(crListItems); i++)
	{
		crListItems[i].bDisabled = FALSE;
		if (crListItems[i].uType == MENU_CHECK)
			crListItems[i].uType = MENU_ITEM;
		else if (crListItems[i].uType == MENU_POPUPCHECK)
			crListItems[i].uType = MENU_POPUPITEM;
	}
}

void CSkypeProto::InitChatModule()
{
	GCREGISTER gcr = {0};
	gcr.cbSize = sizeof(gcr);
	gcr.iMaxText = 0;
	gcr.ptszDispName = this->m_tszUserName;
	gcr.pszModule = this->m_szModuleName;
	::CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	this->HookProtoEvent(ME_GC_EVENT, &CSkypeProto::OnGCEventHook);
	this->HookProtoEvent(ME_GC_BUILDMENU, &CSkypeProto::OnGCMenuHook);
}

///

TCHAR *ChatRoom::Roles[] = 
{ 
	_T(""),			// ---
	LPGENT("Creator"),		// CREATOR	= 1
	LPGENT("Master"),		// ADMIN	= 2
	LPGENT("Helper"),		// SPEAKER	= 3
	LPGENT("User"),		// WRITER	= 4
	LPGENT("Listener"),	// SPECTATOR= 5
	LPGENT("Applicant"),	// APPLICANT= 6
	LPGENT("Retried"),		// RETIRED	= 7
	LPGENT("Outlaw"),		// OUTLAW	= 8
};

ChatRoom::ChatRoom(const wchar_t *cid) : members(1, CompareMembers) 
{
	this->cid = ::mir_wstrdup(cid);
	this->name = NULL;
	this->me = NULL;
}

ChatRoom::ChatRoom(const wchar_t *cid, const wchar_t *name, CSkypeProto *ppro) : members(1, CompareMembers) 
{
	this->cid = ::mir_wstrdup(cid);
	this->name = ::mir_wstrdup(name);
	this->ppro = ppro;
	//
	this->me = new ChatMember(ppro->login);
	this->me->SetNick(::TranslateT("me"));
	this->me->SetRank(0);
	this->me->SetStatus(ID_STATUS_OFFLINE);
	//
	this->sys = new ChatMember(L"sys");
	this->sys->SetNick(L"System");
	this->sys->SetRank(0);
	this->sys->SetStatus(ID_STATUS_OFFLINE);
}

ChatRoom::~ChatRoom()
{
	::mir_free(this->cid);
	::mir_free(this->name);
	delete this->me;
	delete this->sys;
	this->members.destroy();
}

MCONTACT ChatRoom::GetContactHandle() const
{
	return this->hContact;
}

void ChatRoom::CreateChatSession(bool showWindow)
{
	SEString data;

	// start chat session
	GCSESSION gcw = {0};
	gcw.cbSize = sizeof(gcw);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = ppro->m_szModuleName;
	gcw.ptszName = this->name;
	gcw.ptszID = this->cid;
	gcw.dwItemData = (DWORD)this;
	::CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	// load chat roles
	GCDEST gcd = { ppro->m_szModuleName, this->cid, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	for (int i = 1; i < SIZEOF(ChatRoom::Roles) - 2; i++) {
		gce.ptszStatus = ::TranslateTS(ChatRoom::Roles[i]);
		::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}

	// init [and show window]
	gcd.iType = GC_EVENT_CONTROL;
	gce.ptszStatus = NULL;
	::CallServiceSync(MS_GC_EVENT, showWindow ? SESSION_INITDONE : WINDOW_HIDDEN, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

	this->ppro->debugLogW(L"Created new chat session %s", this->cid);
}

void ChatRoom::SetTopic(const wchar_t *topic)
{
	if (this->conversation)
		this->conversation->SetTopic((char *)ptrA(::mir_utf8encodeW(topic)));
}

wchar_t *ChatRoom::GetUri()
{
	SEString data;
	this->conversation->GetJoinBlob(data);
	ptrW blob( ::mir_utf8decodeW(data));
				
	wchar_t *uri = (wchar_t *)::mir_alloc(sizeof(wchar_t) * MAX_PATH);
	::mir_sntprintf(uri, SIZEOF(uri), L"skype:?chat&blob=%s", blob);

	return uri;
}

void ChatRoom::ShowWindow()
{
	// show window
	GCDEST gcd = { this->ppro->m_szModuleName, this->cid, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	::CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
}

void ChatRoom::Invite(const StringList &contacts)
{
	SEStringList needToAdd;
	for (size_t i = 0; i < contacts.size(); i++)
	{
		if ( !contacts.contains(contacts[i]))
			needToAdd.append((char *)_T2A(contacts[i]));
	}
	this->conversation->AddConsumers(needToAdd);
}

void ChatRoom::Create(const ChatRoomParam *param, CSkypeProto *ppro)
{
	SEString data;
	ChatRoom *room = NULL;

	ConversationRef conversation;
	if (ppro->CreateConference(conversation))
	{
		conversation->SetOption(Conversation::P_OPT_JOINING_ENABLED, param->enableJoining);
		conversation->SetOption(Conversation::P_OPT_ENTRY_LEVEL_RANK, (Participant::RANK)param->joinRank);
		conversation->SetOption(Conversation::P_OPT_DISCLOSE_HISTORY, true);
		if (::wcslen(param->topic) > 0)
			conversation->SetTopic((char *)ptrA(::mir_utf8encodeW(param->topic)));
		if (::wcslen(param->guidline) > 0)
			conversation->SetGuidelines((char *)ptrA(::mir_utf8encodeW(param->guidline)));
		if (param->passwordProtection && ::wcslen(param->password) > 0)
		{
			conversation->SetPassword(
				(char *)ptrA(::mir_utf8encodeW(param->password)),
				(char *)ptrA(::mir_utf8encodeW(param->hint)));
		}
		
		SEStringList consumers;
		for (size_t i = 0; i < param->invitedContacts.size(); i++)
		{
			data = ::mir_utf8encodeW(param->invitedContacts[i]);
			consumers.append(data);
		}
		conversation->AddConsumers(consumers);
	}
}

void ChatRoom::Start(const ConversationRef &conversation, bool showWindow)
{
	SEString data;

	this->CreateChatSession(showWindow);

	this->conversation = conversation;
	this->conversation.fetch();
	this->conversation->SetChatRoom(this);

	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYID | GCF_HCONTACT;
	gci.pszModule = ppro->m_szModuleName;
	gci.pszID = this->cid;

	if ( !::CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci)) {
		this->hContact = gci.hContact;
		ptrW cid( ::db_get_wsa(gci.hContact, ppro->m_szModuleName, SKYPE_SETTINGS_SID));
		if (cid == NULL) {
			this->conversation->GetPropIdentity(data);
			cid = ::mir_utf8decodeW(data);
			::db_set_ws(gci.hContact, ppro->m_szModuleName, SKYPE_SETTINGS_SID, cid);
		}
	}

	ParticipantRefs participants;
	this->conversation->GetParticipants(participants, Conversation::CONSUMERS_AND_APPLICANTS);
	for (uint i = 0; i < participants.size(); i++)
	{
		auto participant = participants[i];

		participant->GetPropIdentity(data);
		ptrW sid( ::mir_utf8decodeW(data));

		ChatMember member(sid);
		member.SetRank(participant->GetUintProp(Participant::P_RANK));
				
		Contact::Ref contact;
		this->ppro->GetContact(data, contact);

		Contact::AVAILABILITY status;
		contact->GetPropAvailability(status);
		member.SetStatus(CSkypeProto::SkypeToMirandaStatus(status));

		contact->GetPropFullname(data);
		if (data.length() != 0)
		{
			ptrW nick( ::mir_utf8decodeW(data));
			member.SetNick(nick);
		}

		member.SetPaticipant(participant);
		/*member.participant.fetch();
		member.participant->SetOnChangedCallback(&ChatRoom::OnParticipantChanged, this);*/

		this->AddMember(member, NULL, NULL);
	}
}

void ChatRoom::Join(const wchar_t *joinBlob, CSkypeProto *ppro)
{
	SEString data;
	ConversationRef conversation;
	if (ppro->GetConversationByBlob(data, conversation))
	{
		conversation->GetPropIdentity(data);
		ptrW cid(::mir_utf8decodeW(data));

		conversation->GetPropDisplayname(data);
		ptrW name(::mir_utf8decodeW(data));

		CSkypeProto::ReplaceSpecialChars(cid);
		ChatRoom *room = new ChatRoom(cid, name, ppro);
		ppro->chatRooms.insert(room);
		room->Start(conversation, true);
	}
}

void ChatRoom::SendMessage(const wchar_t *text)
{
	this->ppro->debugLogW(L"Sending chat message %s", this->cid);

	CMessage::Ref message;
		if (this->conversation->PostText((char *)ptrA(::mir_utf8encodeW(text)), message))
			this->ppro->debugLogW(L"Chat message sent %s", this->cid);
}

void ChatRoom::LeaveChat()
{
	this->ppro->debugLogW(L"Leaving chat session %s", this->cid);

	if (this->conversation->RetireFrom())
		this->ppro->debugLogW(L"Retired from conversation %s", this->cid);

	GCDEST gcd = { ppro->m_szModuleName, this->cid, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	::CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
}

void ChatRoom::LeaveChatAndDelete()
{
	this->ppro->debugLogW(L"Leaving chat session %s", this->cid);

	if (this->conversation->RetireFrom())
		this->ppro->debugLogW(L"Retired from conversation %s", this->cid);

	if (this->conversation->Delete())
		this->ppro->debugLogW(L"Delete conversation %s", this->cid);

	GCDEST gcd = { ppro->m_szModuleName, this->cid, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	::CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
}

void ChatRoom::SendEvent(const ChatMember &item, int eventType, DWORD timestamp, DWORD flags, DWORD itemData, const wchar_t *status, const wchar_t *message)
{
	bool isMe = this->IsMe(item);

	GCDEST gcd = { ppro->m_szModuleName, this->cid, eventType };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = flags;
	gce.ptszUID = item.GetSid();
	gce.ptszNick = !isMe ? item.GetNick() : ::TranslateT("me");
	gce.bIsMe = isMe;
	gce.dwItemData = itemData;
	gce.ptszStatus = status;
	gce.ptszText = message;
	gce.time = timestamp;

	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
}

void ChatRoom::SendEvent(const wchar_t *sid, int eventType, DWORD timestamp, DWORD flags, DWORD itemData, const wchar_t *status, const wchar_t *message)
{
	if (this->IsMe(sid))
		this->SendEvent(*this->me, eventType, timestamp, flags, itemData, status, message);
	else if (this->IsSys(sid))
		this->SendEvent(*this->sys, eventType, timestamp, flags, itemData, status, message);
	else
	{
		ChatMember search(sid);
		ChatMember *member = this->members.find(&search);
		if (member != NULL)
			this->SendEvent(*member, eventType, timestamp, flags, itemData, status, message);
	}
}

bool ChatRoom::IsMe(const wchar_t *sid) const
{
	return ::lstrcmpi(this->ppro->login, sid) == 0;
}

bool ChatRoom::IsMe(const ChatMember &item) const
{
	return ::lstrcmpi(this->ppro->login, item.GetSid()) == 0;
}

bool ChatRoom::IsSys(const wchar_t *sid) const
{
	return ::lstrcmpi(L"sys", sid) == 0;
}

bool ChatRoom::IsSys(const ChatMember &item) const
{
	return ::lstrcmpi(L"sys", item.GetSid()) == 0;
}

ChatMember *ChatRoom::FindChatMember(const wchar_t *sid)
{
	if ( !IsMe(sid))
	{
		ChatMember search(sid);
		return this->members.find(&search);
	}
	else
		return this->me;
}

void ChatRoom::AddMember(const ChatMember &item, const ChatMember &author, DWORD timestamp)
{
	if ( !this->IsMe(item))
	{
		ChatMember *member = this->FindChatMember(item.GetSid());
		if (member != NULL)
		{
			this->UpdateMember(item, timestamp);
		}
		else
		{
			ChatMember *newMember = new ChatMember(item);
			newMember->participant.fetch();
			newMember->participant->SetChatRoom(this);
			//newMember->participant->SetOnChangedCallback(&ChatRoom::OnParticipantChanged, this);
			this->members.insert(newMember);

			if (newMember->GetRank() == CParticipant::APPLICANT)
				this->SendEvent(
					*this->sys, 
					GC_EVENT_INFORMATION, 
					time(NULL),
					GCEF_ADDTOLOG, 
					0, 
					NULL, 
					::TranslateT("waits to join"));
			else
				this->SendEvent(item, GC_EVENT_JOIN, timestamp, GCEF_ADDTOLOG, 0, ::TranslateW(ChatRoom::Roles[item.GetRank()]));

			int status = item.GetStatus();
			if (status == ID_STATUS_AWAY || status == ID_STATUS_DND)
				this->SendEvent(item, GC_EVENT_SETSTATUSEX, timestamp, 0, GC_SSE_ONLYLISTED | GC_SSE_ONLINE, (const wchar_t*)0, item.GetSid());
			else if (status == ID_STATUS_OFFLINE)
				this->SendEvent(item, GC_EVENT_SETSTATUSEX, timestamp, 0, GC_SSE_ONLYLISTED | GC_SSE_OFFLINE, (const wchar_t*)0, item.GetSid());
			else
				this->SendEvent(item, GC_EVENT_SETSTATUSEX, timestamp, 0, GC_SSE_ONLYLISTED, (const wchar_t*)0, item.GetSid());

			this->SendEvent(item, GC_EVENT_SETCONTACTSTATUS, timestamp, 0, status);
		}
	}
	else
	{
		if (!this->me->participant)
		{
			this->me->participant = item.participant;
			this->me->participant.fetch();
			this->me->participant->SetChatRoom(this);
			//this->me->participant->SetOnChangedCallback(&ChatRoom::OnParticipantChanged, this);
		}
		if (this->me->GetRank() != item.GetRank())
		{
			this->SendEvent(*this->me, GC_EVENT_REMOVESTATUS, timestamp, 0, 0, ::TranslateW(ChatRoom::Roles[this->me->GetRank()]));
			this->SendEvent(*this->me, GC_EVENT_ADDSTATUS, timestamp, !this->me->GetRank() ? 0 : GCEF_ADDTOLOG, 0, ::TranslateW(ChatRoom::Roles[item.GetRank()]), author == NULL ? this->sys->GetNick() : author.GetNick());
			this->me->SetRank(item.GetRank());
		}
	}
}

void ChatRoom::UpdateMemberNick(ChatMember *member, const wchar_t *nick, DWORD timestamp)
{
	if (::lstrcmp(member->GetNick(), nick) != 0)
	{
		this->SendEvent(*member, GC_EVENT_NICK, timestamp, GCEF_ADDTOLOG, 0, nick);
		member->SetNick(nick);
	}
}

void ChatRoom::UpdateMemberRole(ChatMember *member, int role, const ChatMember &author, DWORD timestamp)
{
	if (member->GetRank() != role)
	{
		this->SendEvent(*member, GC_EVENT_REMOVESTATUS, timestamp, 0, 0, ::TranslateW(ChatRoom::Roles[member->GetRank()]));
		this->SendEvent(*member, GC_EVENT_ADDSTATUS, timestamp, GCEF_ADDTOLOG, 0, ::TranslateW(ChatRoom::Roles[role]), author == NULL ? this->sys->GetNick() : author.GetNick());
		member->SetRank(role);
	}
}

void ChatRoom::UpdateMemberStatus(ChatMember *member, int status, DWORD timestamp)
{
	if (member->GetStatus() != status)
	{
		if (status == ID_STATUS_AWAY || status == ID_STATUS_DND)
			this->SendEvent(*member, GC_EVENT_SETSTATUSEX, timestamp, 0, GC_SSE_ONLYLISTED | GC_SSE_ONLINE, (const wchar_t*)0, member->GetSid());
		else if (status == ID_STATUS_OFFLINE)
			this->SendEvent(*member, GC_EVENT_SETSTATUSEX, timestamp, 0, GC_SSE_ONLYLISTED | GC_SSE_OFFLINE, (const wchar_t*)0, member->GetSid());
		else
			this->SendEvent(*member, GC_EVENT_SETSTATUSEX, timestamp, 0, GC_SSE_ONLYLISTED, (const wchar_t*)0, member->GetSid());

		this->SendEvent(*member, GC_EVENT_SETCONTACTSTATUS, timestamp, 0, status);
		member->SetStatus(status);
	}
}

void ChatRoom::UpdateMember(const wchar_t *sid, const wchar_t *nick, int role, int status, DWORD timestamp)
{
	ChatMember search(sid);
	ChatMember *member = this->members.find(&search);
	if (member != NULL)
	{
		this->UpdateMemberNick(member, nick, timestamp);
		this->UpdateMemberRole(member, role, NULL, timestamp);
		this->UpdateMemberStatus(member, status, timestamp);
	}
}

void ChatRoom::UpdateMember(const ChatMember &item, DWORD timestamp)
{
	ChatMember *member = this->FindChatMember(item.GetSid());
	if (member != NULL)
	{
		ptrW nick(item.GetNick());
		if (::lstrcmp(member->GetNick(), nick) != 0)
		{
			this->SendEvent(*member, GC_EVENT_NICK, timestamp, GCEF_ADDTOLOG, 0, nick);
			member->SetNick(nick);
		}
		if (member->GetRank() != item.GetRank())
		{
			this->SendEvent(*member, GC_EVENT_REMOVESTATUS, timestamp, 0, 0, ::TranslateW(ChatRoom::Roles[member->GetRank()]));
			this->SendEvent(*member, GC_EVENT_ADDSTATUS, timestamp, GCEF_ADDTOLOG, 0, ::TranslateW(ChatRoom::Roles[item.GetRank()]));
			member->SetRank(item.GetRank());
		}
		if (member->GetStatus() != item.GetStatus())
		{
			this->SendEvent(*member, GC_EVENT_SETCONTACTSTATUS, timestamp, 0, item.GetStatus());
			member->SetStatus(item.GetStatus());
		}
	}
}

void ChatRoom::AddApplicant(const ChatMember *member)
{
	SEStringList consumers;
	consumers.append((char *)ptrA(::mir_utf8encodeW(member->GetSid())));
	this->conversation->AddConsumers(consumers);
}

void ChatRoom::KickMember(const ChatMember &item, const ChatMember *author, DWORD timestamp)
{
	if ( !this->IsMe(item))
	{
		ChatMember *member = this->FindChatMember(item.GetSid());
		if (member != NULL)
		{
			this->SendEvent(*member, GC_EVENT_KICK, timestamp, GCEF_ADDTOLOG, 0, author->GetNick());
			this->members.remove(member);
			delete member;
		}
	}
	else
	{
		this->SendEvent(*this->me, GC_EVENT_KICK, timestamp, GCEF_ADDTOLOG, 0, author->GetNick());
		this->me->SetRank(/*RETIRED= */7);
	}
}

void ChatRoom::KickMember(const wchar_t *sid, const wchar_t *author, DWORD timestamp)
{
	ChatMember member(sid);
	this->KickMember(member, this->FindChatMember(author), timestamp);
}

void ChatRoom::RemoveMember(const ChatMember &item, DWORD timestamp)
{
	if ( !this->IsMe(item))
	{
		ChatMember *member = this->FindChatMember(item.GetSid());
		if (member != NULL)
		{
			this->SendEvent(*member, GC_EVENT_PART, timestamp);
			this->members.remove(member);
			delete member;
		}
	}
	else
		this->LeaveChat();
}

void ChatRoom::RemoveMember(const wchar_t *sid, DWORD timestamp)
{
	ChatMember member(sid);
	this->RemoveMember(member, timestamp);
}

void ChatRoom::OnEvent(const ConversationRef &conversation, const MessageRef &message)
{
	if ( !this->conversation)
		this->conversation = conversation;

	if ( this->conversation != conversation)
		return;

	uint messageType;
	messageType = message->GetUintProp(Message::P_TYPE);

	switch (messageType)
	{
	case CMessage::POSTED_EMOTE:
	case CMessage::POSTED_TEXT:
		{
			SEString data;

			message->GetPropAuthor(data);
			ptrW sid( ::mir_utf8decodeW(data));

			message->GetPropBodyXml(data);
			ptrW text( ::mir_utf8decodeW( ptrA(CSkypeProto::RemoveHtml(data))));

			uint timestamp;
			message->GetPropTimestamp(timestamp);
			
			this->SendEvent(
				sid, 
				messageType == CMessage::POSTED_TEXT ? GC_EVENT_MESSAGE : GC_EVENT_ACTION,
				timestamp,
				GCEF_ADDTOLOG,
				0,
				NULL,
				text);
		}
		break;

	case Message::ADDED_CONSUMERS:
	case Message::ADDED_APPLICANTS:
		{
			SEString data;

			Message::CONSUMPTION_STATUS status;
			message->GetPropConsumptionStatus(status);
			if (status != Message::CONSUMED)
			{
				uint timestamp;
				message->GetPropTimestamp(timestamp);

				message->GetPropAuthor(data);
				ChatMember *author = this->FindChatMember((wchar_t *)ptrW(::mir_utf8decodeW(data)));

				ParticipantRefs participants;
				conversation->GetParticipants(participants);
				for (size_t i = 0; i < participants.size(); i++)
				{					
					participants[i]->GetPropIdentity(data);
					ptrW sid(::mir_utf8decodeW(data));
					if (this->FindChatMember(sid) == NULL)
					{
						ChatMember member(sid);
						member.SetRank(participants[i]->GetUintProp(Participant::P_RANK));

						Contact::Ref contact;
						this->ppro->GetContact(data, contact);

						Contact::AVAILABILITY status;
						contact->GetPropAvailability(status);
						member.SetStatus(CSkypeProto::SkypeToMirandaStatus(status));

						contact->GetPropFullname(data);
						ptrW nick(::mir_utf8decodeW(data));
						if (data.length() != 0)
						{
							ptrW nick( ::mir_utf8decodeW(data));
							member.SetNick(nick);
						}
						
						member.participant = participants[i];
						/*member.participant.fetch();
						member.participant->SetOnChangedCallback(&ChatRoom::OnParticipantChanged, this);*/

						this->AddMember(member, *author, timestamp);
					}
				}

				// do not remove
				//message->GetPropIdentities(data);
				//char *identities = ::mir_strdup(data);
				//if (identities)
				//{
				//	char *identity = ::strtok(identities, " ");
				//	if (identity != NULL)
				//	{
				//		do
				//		{
				//			Contact::Ref contact;
				//			this->ppro->GetContact(identity, contact);

				//			contact->GetIdentity(data);
				//			ptrW sid = ::mir_utf8decodeW(data);

				//			ChatMember *member = new ChatMember(sid);
				//			//todo: fix rank
				//			
				//			member->rank = 
				//				messageType == Message::ADDED_APPLICANTS ? 
				//				Participant::APPLICANT : 
				//				Participant::SPEAKER;
				//				//conversation->GetUintProp(Conversation::P_OPT_ENTRY_LEVEL_RANK);
				//				//participants[i]->GetUintProp(Participant::P_RANK);

				//			Contact::AVAILABILITY status;
				//			contact->GetPropAvailability(status);
				//			member->status = CSkypeProto::SkypeToMirandaStatus(status);

				//			contact->GetPropFullname(data);
				//			member->nick = ::mir_utf8decodeW(data);

				//			this->AddMember(member, timestamp);

				//			identity = ::strtok(NULL, " ");
				//		}
				//		while (identity != NULL);
				//	}
				//	::mir_free(identities);
				//}
			}
		}
		break;

	case Message::RETIRED_OTHERS:
		{
			SEString data;

			Message::CONSUMPTION_STATUS status;
			message->GetPropConsumptionStatus(status);
			if (status != Message::CONSUMED)
			{
				this->ppro->debugLogW(L"Retired other event for conversation %s", this->cid);

				uint timestamp;
				message->GetPropTimestamp(timestamp);

				message->GetPropAuthor(data);
				ptrW author( ::mir_utf8decodeW(data));

				message->GetPropIdentities(data);
				char *identities = ::mir_strdup(data);
				if (identities)
				{
					char *identity = ::strtok(identities, " ");
					if (identity != NULL)
					{
						do
						{
							ptrW sid(::mir_utf8decodeW(identity));
							this->KickMember(sid, author, timestamp);

							identity = ::strtok(NULL, " ");
						}
						while (identity != NULL);
					}
					::mir_free(identities);
				}
			}
		}
		break;

	case Message::RETIRED:
		{
			SEString data;

			Message::CONSUMPTION_STATUS status;
			message->GetPropConsumptionStatus(status);
			if (status != Message::CONSUMED)
			{
				this->ppro->debugLogW(L"Retired event for conversation %s", this->cid);

				uint timestamp;
				message->GetPropTimestamp(timestamp);

				message->GetPropAuthor(data);
				ptrW sid( ::mir_utf8decodeW(data));

				this->RemoveMember(sid, timestamp);
			}
		}
		break;

	case Message::SET_RANK:
		{
			SEString data;
		}
		break;
	//		message->GetPropBodyXml(data);
	//		ptrA text = ::mir_strdup(data);
	//		int i = 0;

	//		/*Message::CONSUMPTION_STATUS status;
	//		message->GetPropConsumptionStatus(status);
	//		if (status == Message::UNCONSUMED_NORMAL)*/
	//		{
	//			message->GetPropAuthor(data);	
	//			ptrW sid = ::mir_utf8decodeW(data);

	//			ChatMember search(sid);
	//			ChatMember *member = this->FindChatMember(sid);
	//			if (member != NULL)
	//			{
	//				uint timestamp;
	//				message->GetPropTimestamp(timestamp);

	//				message->GetPropBodyXml(data);	
	//				ptrW rank = ::mir_utf8decodeW(data);

	//				member->SetRank(0);
	//			}
	//		}
	//	}
	//	break;

	/*case CMessage::STARTED_LIVESESSION:
		{
			SEString data;

			Message::CONSUMPTION_STATUS status;
			message->GetPropConsumptionStatus(status);
			if (status != Message::UNCONSUMED_NORMAL)
				break;

			message->GetPropAuthor(data);	
			ptrW sid = ::mir_utf8decodeW(data);

			uint timestamp;
			message->GetPropTimestamp(timestamp);
			
			this->SendEvent(
				sid, 
				GC_EVENT_INFORMATION, 
				timestamp,
				GCEF_ADDTOLOG, 
				0, 
				NULL, 
				::TranslateT("Incoming group call received"));
		}
		break;*/

	case CMessage::ENDED_LIVESESSION:
		{
			SEString data;

			//Message::CONSUMPTION_STATUS status;
			//message->GetPropConsumptionStatus(status);
			//if (status != Message::UNCONSUMED_NORMAL)
			//	break;

			message->GetPropAuthor(data);	
			ptrW sid( ::mir_utf8decodeW(data));

			uint timestamp;
			message->GetPropTimestamp(timestamp);
			
			this->SendEvent(
				*this->sys, 
				GC_EVENT_INFORMATION, 
				timestamp,
				GCEF_ADDTOLOG, 
				0, 
				NULL, 
				::TranslateT("Incoming group call finished"));
		}
		break;
	}
}

void ChatRoom::OnChange(const ConversationRef &conversation, int prop)
{
	if ( !this->conversation)
		this->conversation = conversation;

	if ( this->conversation != conversation)
		return;

	switch (prop)
	{
	case Conversation::P_MY_STATUS:
		{
			Conversation::MY_STATUS status;
			conversation->GetPropMyStatus(status);
			if (status == Conversation::INVALID_ACCESS_TOKEN)
			{
				PasswordRequestBoxParam param(this->name, false);
				if (this->ppro->RequestPassword(param))
				{
					if ( !this->conversation->EnterPassword(param.password))
						this->SendEvent(
							*this->sys, 
							GC_EVENT_INFORMATION, 
							time(NULL),
							GCEF_ADDTOLOG, 
							0, 
							NULL, 
							::TranslateT("The password is incorrect"));
				}
			}
			else if (status == Conversation::APPLICATION_DENIED)
			{
				this->SendEvent(
					*this->sys, 
					GC_EVENT_INFORMATION, 
					time(NULL),
					GCEF_ADDTOLOG, 
					0, 
					NULL, 
					::TranslateT("Your application to join the conference was denied"));
			}
		}
		break;

	case Conversation::P_LOCAL_LIVESTATUS:
		{
			Conversation::LOCAL_LIVESTATUS liveStatus;
			conversation->GetPropLocalLivestatus(liveStatus);
			if (liveStatus == Conversation::RINGING_FOR_ME)
			{
				SEString data;

				/*Message::CONSUMPTION_STATUS status;
				message->GetPropConsumptionStatus(status);
				if (status != Message::UNCONSUMED_NORMAL)
					break;*/

				/*message->GetPropAuthor(data);	
				ptrW sid = ::mir_utf8decodeW(data);*/

				/*uint timestamp;
				message->GetPropTimestamp(timestamp);*/
			
				this->SendEvent(
					*this->sys, 
					GC_EVENT_INFORMATION, 
					time(NULL),
					GCEF_ADDTOLOG, 
					0, 
					NULL, 
					::TranslateT("Incoming group call started"));
				}
		}
		break;
	}
}

void ChatRoom::OnParticipantChanged(const ParticipantRef &participant, int prop)
{
	if (prop == Participant::P_RANK)
	{
		Participant::RANK rank;
		participant->GetPropRank(rank);
	
		SEString identity;
		participant->GetPropIdentity(identity);

		ptrW sid(::mir_utf8decodeW(identity));
		ChatMember *member = this->FindChatMember(sid);
		if (member != NULL)
			this->UpdateMemberRole(member, rank);
	}
}

///

void CSkypeProto::ChatValidateContact(MCONTACT hItem, HWND hwndList, const StringList &contacts)
{
	if (this->IsProtoContact(hItem) && !this->isChatRoom(hItem))
	{
		ptrW sid( ::db_get_wsa(hItem, this->m_szModuleName, SKYPE_SETTINGS_SID));
		if (sid == NULL || contacts.contains(sid))
			::SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
	}
	else
		::SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

void CSkypeProto::ChatPrepare(MCONTACT hItem, HWND hwndList, const StringList &contacts)
{
	if (hItem == NULL)
		hItem = (MCONTACT)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		MCONTACT hItemN = (MCONTACT)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				this->ChatPrepare(hItemT, hwndList, contacts);
		}
		else if (IsHContactContact(hItem))
			this->ChatValidateContact(hItem, hwndList, contacts);

		hItem = hItemN;
	}
}

void CSkypeProto::GetInvitedContacts(MCONTACT hItem, HWND hwndList, StringList &chatTargets)
{
	if (hItem == NULL)
		hItem = (MCONTACT)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				this->GetInvitedContacts(hItemT, hwndList, chatTargets);
		}
		else {
			int chk = SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
			if (chk) {
				if (IsHContactInfo(hItem)) {
					TCHAR buf[128] = _T("");
					SendMessage(hwndList, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)buf);
					if (buf[0])
						chatTargets.insert(buf);
				}
				else {
					ptrW login( ::db_get_wsa(hItem, this->m_szModuleName, SKYPE_SETTINGS_SID));
					chatTargets.insert(login);
				}
			}
		}
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
	}
}

INT_PTR CALLBACK CSkypeProto::ChatRoomProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ChatRoomParam *param = (ChatRoomParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		param = (ChatRoomParam *)lParam;
		{
			HWND hwndClist = GetDlgItem(hwndDlg, IDC_CCLIST);
			SetWindowLongPtr(hwndClist, GWL_STYLE, GetWindowLongPtr(hwndClist, GWL_STYLE) & ~CLS_HIDEOFFLINE);

			if ( !param->ppro->IsOnline())
			{
				::EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				::EnableWindow(GetDlgItem(hwndDlg, IDC_ADDSCR), FALSE);
				::EnableWindow(GetDlgItem(hwndDlg, IDC_CCLIST), FALSE);
			}

			SendDlgItemMessage(hwndDlg, IDC_CHAT_JOINING, BM_SETCHECK, param->enableJoining, 0);
			for (int i = 1; i < SIZEOF(ChatRoom::Roles) - 4; i++) {
				int nItem = ::SendMessage(::GetDlgItem(hwndDlg, IDC_CHAT_ROLES), CB_ADDSTRING, 0, (LPARAM)::TranslateW(ChatRoom::Roles[i]));

				if (i == Participant::WRITER)
					::SendMessage(::GetDlgItem(hwndDlg, IDC_CHAT_ROLES), CB_SETCURSEL, nItem, 0);
			}

			SendDlgItemMessage(hwndDlg, IDC_CHAT_SECURED, BM_SETCHECK, param->passwordProtection, 0);
		}
		break;

	case WM_CLOSE:
		::EndDialog(hwndDlg, 0);
		break;

	case WM_NOTIFY:
		{
			NMCLISTCONTROL *nmc = (NMCLISTCONTROL *)lParam;
			if (nmc->hdr.idFrom == IDC_CCLIST) {
				switch (nmc->hdr.code) {
				case CLN_NEWCONTACT:
					if (param && (nmc->flags & (CLNF_ISGROUP | CLNF_ISINFO)) == 0)
						param->ppro->ChatValidateContact((MCONTACT)nmc->hItem, nmc->hdr.hwndFrom, param->invitedContacts);
					break;

				case CLN_LISTREBUILT:
					if (param)
						param->ppro->ChatPrepare(NULL, nmc->hdr.hwndFrom, param->invitedContacts);
					break;
				}
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADDSCR:
			if (param->ppro->IsOnline())
			{
				wchar_t sid[SKYPE_SID_LIMIT];
				::GetDlgItemText(hwndDlg, IDC_EDITSCR, sid, SIZEOF(sid));

				CLCINFOITEM cii = {0};
				cii.cbSize = sizeof(cii);
				cii.flags = CLCIIF_CHECKBOX | CLCIIF_BELOWCONTACTS;
				cii.pszText = ::wcslwr(sid);

				HANDLE hItem = (HANDLE)::SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
				::SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_SETCHECKMARK, (LPARAM)hItem, 1);
			}
			break;

		case IDC_CHAT_SECURED:
			{
				BOOL enable = (BOOL)::IsDlgButtonChecked(hwndDlg, IDC_CHAT_SECURED);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHAT_PASSWORD), enable);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHAT_CONFIRMATION), enable);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHAT_HINT), enable);
			}
			break;

		case IDC_CHAT_PASSWORD:
		case IDC_CHAT_CONFIRMATION:
			{
				wchar_t pwd[32], cfn[32];
				GetDlgItemText(hwndDlg, IDC_CHAT_PASSWORD, pwd, SIZEOF(pwd));
				GetDlgItemText(hwndDlg, IDC_CHAT_CONFIRMATION, cfn, SIZEOF(cfn));

				BOOL secured = ::IsDlgButtonChecked(hwndDlg, IDC_CHAT_SECURED);
				::EnableWindow(::GetDlgItem(hwndDlg, IDOK), secured && ::wcscmp(pwd, cfn) == 0);
			}
			break;

		case IDOK:
			{
				HWND hwndList = ::GetDlgItem(hwndDlg, IDC_CCLIST);

				param->invitedContacts.clear();
				param->ppro->GetInvitedContacts(NULL, hwndList, param->invitedContacts);

				if ( !param->invitedContacts.empty())
				{
					SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
					::EndDialog(hwndDlg, IDOK);
				}
				else
					param->ppro->ShowNotification(::TranslateT("You did not select any contact"));

				GetDlgItemText(hwndDlg, IDC_CHAT_TOPIC, param->topic, SIZEOF(param->topic));
				GetDlgItemText(hwndDlg, IDC_CHAT_GUIDLINE, param->guidline, SIZEOF(param->guidline));

				param->enableJoining = ::IsDlgButtonChecked(hwndDlg, IDC_CHAT_JOINING) != 0;
				param->joinRank = ::SendMessage(::GetDlgItem(hwndDlg, IDC_CHAT_ROLES), CB_GETCURSEL, 0, 0) + 1;

				param->passwordProtection = ::IsDlgButtonChecked(hwndDlg, IDC_CHAT_SECURED) != 0;
				if (param->passwordProtection)
				{
					GetDlgItemText(hwndDlg, IDC_CHAT_PASSWORD, param->password, SIZEOF(param->password));
					GetDlgItemText(hwndDlg, IDC_CHAT_CONFIRMATION, param->confirmation, SIZEOF(param->confirmation));
					GetDlgItemText(hwndDlg, IDC_CHAT_HINT, param->hint, SIZEOF(param->hint));
				}
			}
			break;

		case IDCANCEL:
			::EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CSkypeProto::CreateChatRoomCommand(WPARAM, LPARAM)
{
	ChatRoomParam *param = new ChatRoomParam(NULL, NULL, this);

	if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CHATROOM_CREATE), NULL, CSkypeProto::ChatRoomProc, (LPARAM)param) == IDOK && param->invitedContacts.size() > 0)
		ChatRoom::Create(param, this);

	delete param;

	return 0;
}

void CSkypeProto::ChatRoomInvite(MCONTACT hContact)
{
	ptrT chat_id(::db_get_tsa(hContact, this->m_szModuleName, "ChatRoomID"));

	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYID | GCF_USERS | GCF_DATA;
	gci.pszModule = this->m_szModuleName;
	gci.pszID = chat_id;
	if ( !::CallService(MS_GC_GETINFO, 0, (LPARAM)&gci)) {
		ChatRoom *room = (ChatRoom *)gci.dwItemData;
		if (room != NULL && gci.pszUsers != NULL) {
			StringList invitedContacts(_A2T(gci.pszUsers));
			ChatRoomParam *param = new ChatRoomParam(NULL, invitedContacts, this);

			if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, CSkypeProto::ChatRoomProc, (LPARAM)param) == IDOK && param->invitedContacts.size() > 0)
				room->Invite(param->invitedContacts);

			delete param;
		}
	}	
}

void CSkypeProto::CloseAllChatSessions()
{
	GC_INFO gci = {0};
	gci.Flags = GCF_BYINDEX | GCF_ID | GCF_DATA;
	gci.pszModule = this->m_szModuleName;

	int count = ::CallServiceSync(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)this->m_szModuleName);
	for (int i = 0; i < count ; i++)
	{
		gci.iItem = i;
		if ( !::CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci))
		{
			GCDEST gcd = { this->m_szModuleName, gci.pszID, GC_EVENT_CONTROL };
			GCEVENT gce = { sizeof(gce), &gcd };			
			::CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
			::CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
		}
	}
}

ChatRoom *CSkypeProto::FindChatRoom(const wchar_t *cid)
{
	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYID | GCF_DATA;
	gci.pszModule = this->m_szModuleName;
	gci.pszID = (wchar_t*)cid;

	if ( !::CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci))
		return (ChatRoom *)gci.dwItemData;

	return NULL;
}

int __cdecl CSkypeProto::OnGCEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	if (!gch) return 1;

	if (::strcmp(gch->pDest->pszModule, this->m_szModuleName) != 0)
		return 0;

	ChatRoom *room = this->FindChatRoom(gch->pDest->ptszID);
	if (room == NULL)
		return 0;

	switch (gch->pDest->iType)
	{
	case GC_USER_MESSAGE:
		if (gch->ptszText && gch->ptszText[0])
			room->SendMessage(gch->ptszText);
		break;

	/*case GC_USER_CHANMGR:
		if (this->GetConversationByIdentity(::mir_utf8encodeW(cid), conversation, false))
		{
			StringList invitedContacts(this->GetChatUsers(cid));
			this->InviteConactsToChat(conversation, invitedContacts); 
		}
		break;*/

	case GC_USER_PRIVMESS:
			::CallService(MS_MSG_SENDMESSAGE, (WPARAM)this->GetContactBySid(gch->ptszUID), 0);
		break;

	case GC_USER_LOGMENU:
	case GC_USER_NICKLISTMENU:
		switch (gch->dwData)
		{
		case CHAT_LIST_MENU::ICM_ROLE_ADMIN:
		case CHAT_LIST_MENU::ICM_ROLE_SPEAKER:
		case CHAT_LIST_MENU::ICM_ROLE_WRITER:
		case CHAT_LIST_MENU::ICM_ROLE_SPECTATOR:
			{
				ChatMember *member = room->FindChatMember(gch->ptszUID);
				if (member != NULL)
				{
					Participant::RANK rank;
					switch (gch->dwData)
					{
						case CHAT_LIST_MENU::ICM_ROLE_ADMIN:
							rank = Participant::ADMIN;
							break;

						case CHAT_LIST_MENU::ICM_ROLE_SPEAKER:
							rank = Participant::SPEAKER;
							break;

						case CHAT_LIST_MENU::ICM_ROLE_WRITER:
							rank = Participant::WRITER;
							break;

						case CHAT_LIST_MENU::ICM_ROLE_SPECTATOR:
							rank = Participant::SPECTATOR;
							break;
					}
					if (member->participant && member->participant->SetRankTo(rank))
						room->UpdateMemberRole(member, rank, *room->me);
				}
			}
			break;

		case CHAT_LIST_MENU::ICM_ADD:
			{
				ChatMember *member = room->FindChatMember(gch->ptszUID);
				if (member != NULL)
					room->AddApplicant(member);
			}
			break;

		case CHAT_LIST_MENU::ICM_KICK:
			{
				ChatMember *member = room->FindChatMember(gch->ptszUID);
				if (member != NULL)
				{
					if (member->participant && member->participant->Retire())
						room->KickMember(gch->ptszUID, room->me->GetSid());
				}
			}
			break;

		case CHAT_LIST_MENU::ICM_BAN:
			{
				ChatMember *member = room->FindChatMember(gch->ptszUID);
				if (member != NULL && member->participant)
				{
					member->participant->SetRankTo(Participant::OUTLAW);
					if (member->participant->Retire())
						room->KickMember(gch->ptszUID, room->me->GetSid());
				}
			}
			break;

		case CHAT_LIST_MENU::ICM_CONF_INVITE:
			{
				GC_INFO gci = { 0 };
				gci.Flags = GCF_BYID | GCF_USERS;
				gci.pszModule = this->m_szModuleName;
				gci.pszID = gch->pDest->ptszID;
				if ( !::CallService(MS_GC_GETINFO, 0, (LPARAM)&gci) && gci.pszUsers != NULL)
				{
					StringList invitedContacts(_A2T(gci.pszUsers));
					ChatRoomParam *param = new ChatRoomParam(NULL, invitedContacts, this);

					if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, CSkypeProto::ChatRoomProc, (LPARAM)param) == IDOK && param->invitedContacts.size() > 0)
						room->Invite(param->invitedContacts);

					delete param;
				}
			}
			break;

		case CHAT_LIST_MENU::ICM_AUTH_REQUEST:
			{
				CContact::Ref contact;
				SEString sid((char *)ptrA(::mir_utf8encodeW(gch->ptszUID)));
				if (this->GetContact(sid, contact))
				{
					this->AuthRequest(
						this->AddContact(contact),
						LPGENT("Hi! I'd like to add you to my contact list"));
				}
			}
			break;

		case CHAT_LIST_MENU::ICM_DETAILS:
			::CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)this->GetContactBySid(gch->ptszUID), 0);
			break;

		case CHAT_LIST_MENU::ICM_COPY_SID:
			{
				MCONTACT hContact = this->GetContactBySid(gch->ptszUID);
				if (!hContact)
				{
					ptrW sid( ::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));
					if (sid != NULL)
						CSkypeProto::CopyToClipboard(sid);
				}
			}
			break;

		case CHAT_LIST_MENU::ICM_COPY_URI:
				CSkypeProto::CopyToClipboard(ptrW(room->GetUri()));
			break;
		}
		break;

	//case GC_USER_TYPNOTIFY:
		//break;
	}
	return 0;
}

int __cdecl CSkypeProto::OnGCMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*) lParam;

	if (::stricmp(gcmi->pszModule, this->m_szModuleName) != 0)
		return 0;

	ChatRoom *room = this->FindChatRoom(gcmi->pszID);
	if (room == NULL)
		return 0;

	ResetChatMenuItem();

	if (room->me->GetRank() > Participant::ADMIN || room->me->GetRank() == 0)
	{
		DisableChatMenuItem(ICM_ROLE);
		DisableChatMenuItem(ICM_ADD);
		DisableChatMenuItem(ICM_KICK);
		DisableChatMenuItem(ICM_BAN);
	}

	//todo: add other case
	if (room->me->GetRank() >= Participant::APPLICANT)
	{
		DisableChatMenuItem(ICM_CONF_INVITE);
	}	

	ChatMember *member = room->FindChatMember(gcmi->pszUID);
	if (member != NULL)
	{
		if (member->GetRank() == Participant::CREATOR)
		{
			DisableChatMenuItem(ICM_ROLE);
			DisableChatMenuItem(ICM_ADD);
			DisableChatMenuItem(ICM_KICK);
			DisableChatMenuItem(ICM_BAN);
		}
		
		if (member->GetRank() <= Participant::SPECTATOR)
		{
			CHAT_LIST_MENU type = (CHAT_LIST_MENU)(ICM_ROLE + member->GetRank() - 1);
			CheckChatMenuItem(type);
			DisableChatMenuItem(type);

			DisableChatMenuItem(ICM_ADD);
		}
		
		if (member->GetRank() > Participant::SPECTATOR)
			DisableChatMenuItem(ICM_ROLE);

		MCONTACT hContact = this->GetContactBySid(gcmi->pszUID);
		if (hContact == NULL)
			DisableChatMenuItem(ICM_DETAILS);
		else if(this->getByte(hContact, "Auth", 0) == 0)
			DisableChatMenuItem(ICM_AUTH_REQUEST);
	}
	else
	{
		DisableChatMenuItem(ICM_DETAILS);
		DisableChatMenuItem(ICM_AUTH_REQUEST);
		DisableChatMenuItem(ICM_ROLE);
		DisableChatMenuItem(ICM_ADD);
		DisableChatMenuItem(ICM_KICK);
		DisableChatMenuItem(ICM_BAN);
		DisableChatMenuItem(ICM_COPY_SID);
	}

	gcmi->nItems = SIZEOF(crListItems);
	gcmi->Item = crListItems;

	return 0;
}

void CSkypeProto::UpdateChatUserStatus(const ContactRef &contact)
{
	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);

	SEString identity;
	contact->GetIdentity(identity);
	ptrW sid(::mir_utf8decodeW(identity));

	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYINDEX | GCF_DATA;
	gci.pszModule = this->m_szModuleName;

	int count = ::CallServiceSync(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)this->m_szModuleName);
	for (int i = 0; i < count ; i++)
	{
		gci.iItem = i;
		::CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci);

		ChatRoom *room = (ChatRoom *)gci.dwItemData;
		if (room != NULL)
		{
			ChatMember *member = room->FindChatMember(sid);
			if (member != NULL)
				room->UpdateMemberStatus(member, CSkypeProto::SkypeToMirandaStatus(availability));
		}
	}
}

void CSkypeProto::UpdateChatUserNick(const ContactRef &contact)
{
	SEString data;

	contact->GetIdentity(data);
	ptrW sid(::mir_utf8decodeW(data));

	ptrW nick(::mir_utf8decodeW(((CContact::Ref)contact)->GetNick()));

	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYINDEX | GCF_DATA;
	gci.pszModule = this->m_szModuleName;

	int count = ::CallServiceSync(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)this->m_szModuleName);
	for (int i = 0; i < count ; i++)
	{
		gci.iItem = i;
		::CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci);

		ChatRoom *room = (ChatRoom *)gci.dwItemData;
		if (room != NULL)
		{
			ChatMember *member = room->FindChatMember(sid);
			if (member != NULL)
				room->UpdateMemberNick(member, nick);
		}
	}
}

INT_PTR __cdecl CSkypeProto::OnJoinChat(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact)
	{
		ptrW cid(::db_get_wsa(hContact, this->m_szModuleName, "ChatRoomID"));

		ChatRoom *room = this->FindChatRoom(cid);
		if ( !room)
		{
			ConversationRef conversation;
			if (this->GetConversationByIdentity(
				(char *)_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)), 
				conversation))
			{
				SEString data;
				conversation->GetJoinBlob(data);
				ptrW joinBlob(::mir_utf8decodeW(data));
				ChatRoom::Join(joinBlob, this);
			}
		}
		else
			room->ShowWindow();
	}
	
	return 0;
}

INT_PTR __cdecl CSkypeProto::OnLeaveChat(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact) {
		ptrW cid(::db_get_wsa(hContact, this->m_szModuleName, "ChatRoomID"));
		
		ChatRoom *room = this->FindChatRoom(cid);
		if (room != NULL)
			room->LeaveChat();
	}

	return 0;
}

///

void __cdecl CSkypeProto::LoadChatList(void*)
{
	this->debugLogW(L"Updating group chats list");
	CConversation::Refs conversations;
	this->GetConversationList(conversations);

	SEString data;
	for (uint i = 0; i < conversations.size(); i++)
	{
		auto conversation = conversations[i];

		uint convoType = conversation->GetUintProp(Conversation::P_TYPE);
		if (convoType == CConversation::CONFERENCE)
		{
			CConversation::MY_STATUS status;
			conversation->GetPropMyStatus(status);
			if (status == Conversation::APPLICANT || status == Conversation::CONSUMER )
			{
				conversation->GetPropIdentity(data);
				ptrW cid( ::mir_utf8decodeW(data));
				CSkypeProto::ReplaceSpecialChars(cid);

				conversation->GetPropDisplayname(data);
				ptrW name( ::mir_utf8decodeW(data));

				ChatRoom *room = new ChatRoom(cid, name, this);
				chatRooms.insert(room);
				room->Start(conversation);
			}
		}
	}
}

///

void CSkypeProto::OnChatEvent(const ConversationRef &conversation, const MessageRef &message)
{
	uint messageType;
	messageType = message->GetUintProp(Message::P_TYPE);

	SEString data;
	conversation->GetPropIdentity(data);
	ptrW cid( ::mir_utf8decodeW(data));
	CSkypeProto::ReplaceSpecialChars(cid);

	ChatRoom *room = this->FindChatRoom(cid);
	if (room != NULL)
	{
		room->OnEvent(conversation, message);
	}
	else
	{
		Conversation::MY_STATUS status;
		conversation->GetPropMyStatus(status);
		if (status != Conversation::RETIRED_FORCEFULLY || status != Conversation::RETIRED_FORCEFULLY)
		{
			SEString data;

			conversation->GetPropDisplayname(data);
			ptrW name( ::mir_utf8decodeW(data));

			ChatRoom *room = new ChatRoom(cid, name, this);
			chatRooms.insert(room);
			room->Start(conversation, true);
		}
	}
}

void CSkypeProto::OnConversationListChange(
		const ConversationRef& conversation,
		const Conversation::LIST_TYPE& type,
		const bool& added)
{
	uint convoType = conversation->GetUintProp(Conversation::P_TYPE);
	if (convoType == Conversation::CONFERENCE && type == Conversation::INBOX_CONVERSATIONS  && added)
	{
		SEString data;

		conversation->GetPropIdentity(data);
		ptrW cid( ::mir_utf8decodeW(data));
		CSkypeProto::ReplaceSpecialChars(cid);

		if ( !this->FindChatRoom(cid))
		{
			conversation->GetPropDisplayname(data);
			ptrW name( ::mir_utf8decodeW(data));

			ChatRoom *room = new ChatRoom(cid, name, this);
			chatRooms.insert(room);
			room->Start(conversation, true);
		}
	}
}

void CSkypeProto::ChatRoomParseUriComands(const wchar_t *commands)
{
}

static void appendString(bool bIsTipper, const TCHAR *tszTitle, const TCHAR *tszValue, TCHAR* buf, size_t bufSize)
{
	if (*buf) {
		const TCHAR *szSeparator = bIsTipper ? _T("\n") : _T("\r\n");
 		_tcsncat(buf, szSeparator, bufSize);
	}

	size_t len = _tcslen(buf);
	buf += len;
	bufSize -= len;

	if (bIsTipper)
		mir_sntprintf(buf, bufSize, _T("%s%s%s%s"), _T("<b>"), TranslateTS(tszTitle), _T("</b>\t"), tszValue);
	else {
		TCHAR* p = TranslateTS(tszTitle);
		mir_sntprintf(buf, bufSize, _T("%s%s\t%s"), p, _tcslen(p)<=7 ? _T("\t") : _T(""), tszValue);
	}
}

INT_PTR __cdecl CSkypeProto::SkypeGCGetToolTipText(WPARAM wParam, LPARAM lParam)
{
	if ( !wParam || !lParam)
		return 0; //room global tooltip not supported yet

	ChatRoom *room = this->FindChatRoom((TCHAR *)wParam);
	if (room == NULL)
		return 0;  //no room found

	ChatMember *member = room->FindChatMember((TCHAR *)lParam);
	if (member == NULL)
		return 0;  //no contact found

	// ok process info output will be:
	// Skype name:	sid
	// Nick:		Nickname
	// Status:		StatusText
	// Role:		Moderator

	TCHAR outBuf[2048];
	outBuf[0]=_T('\0');

	bool bIsTipper = db_get_b(NULL, "Tab_SRMsg", "adv_TipperTooltip", 0) && ServiceExists("mToolTip/HideTip");

	//sid
	appendString(bIsTipper, _T("Skype name:"), member->GetSid(), outBuf, SIZEOF(outBuf));
	//nick
	appendString(bIsTipper, _T("Nick:"), member->GetNick(), outBuf, SIZEOF(outBuf));
	//status
	appendString(bIsTipper, _T("Status:"), (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)member->GetStatus(),GSMDF_TCHAR), outBuf, SIZEOF(outBuf));
	//role
	appendString(bIsTipper, _T("Role:"), ::TranslateW(ChatRoom::Roles[member->GetRank()]), outBuf, SIZEOF(outBuf));

	return (INT_PTR)(outBuf[0] == 0 ? NULL : mir_tstrdup(outBuf));
}