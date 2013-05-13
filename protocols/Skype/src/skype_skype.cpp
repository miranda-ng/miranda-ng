#include "skype_proto.h"

CAccount* CSkypeProto::newAccount(int oid) 
{ 
	return new CAccount(oid, this); 
}

CContactGroup* CSkypeProto::newContactGroup(int oid)
{ 
	return new CContactGroup(oid, this); 
}

CContact* CSkypeProto::newContact(int oid) 
{ 
	return new CContact(oid, this); 
}

CConversation* CSkypeProto::newConversation(int oid) 
{ 
	return new CConversation(oid, this); 
}

CParticipant* CSkypeProto::newParticipant(int oid) 
{ 
	return new CParticipant(oid, this); 
}

CMessage* CSkypeProto::newMessage(int oid) 
{ 
	return new CMessage(oid, this); 
}

CTransfer* CSkypeProto::newTransfer(int oid) 
{ 
	return new CTransfer(oid, this); 
}

CContactSearch* CSkypeProto::newContactSearch(int oid)
{
	return new CContactSearch(oid, this);
}

bool CSkypeProto::CreateConferenceWithConsumers(ConversationRef &conference, const SEStringList &identities)
{
	if (this->CreateConference(conference))
	{
		conference->SetOption(CConversation::P_OPT_JOINING_ENABLED, true);
		conference->SetOption(CConversation::P_OPT_ENTRY_LEVEL_RANK, CParticipant::WRITER);
		conference->SetOption(CConversation::P_OPT_DISCLOSE_HISTORY, 1);
		conference->AddConsumers(identities);

		return true;
	}

	return false;
}