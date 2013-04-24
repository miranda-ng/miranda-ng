#pragma once

#include "common.h"

#include "group.h"
#include "search.h"
#include "account.h"
#include "contact.h"
#include "message.h"
#include "transfer.h"
#include "participant.h"
#include "conversation.h"

class CSkype : public Skype
{
public:
	typedef void (CSkypeProto::* OnMessaged)(CConversation::Ref conversation, CMessage::Ref message);

	CAccount*		newAccount(int oid);
	CContactGroup*	newContactGroup(int oid);
	CConversation*	newConversation(int oid);
	CContactSearch*	newContactSearch(int oid);
	CParticipant*	newParticipant(int oid);
	CContact*		newContact(int oid);	
	CMessage*		newMessage(int oid);
	CTransfer*		newTransfer(int oid);

	CSkype(int num_threads = 1);

	void SetOnMessageCallback(OnMessaged callback, CSkypeProto* proto);

private:
	CSkypeProto*	proto;
	OnMessaged		onMessagedCallback;

	void OnMessage(
		const MessageRef & message,
		const bool & changesInboxTimestamp,
		const MessageRef & supersedesHistoryMessage,
		const ConversationRef & conversation);
};