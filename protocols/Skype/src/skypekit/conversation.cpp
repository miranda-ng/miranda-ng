#include "conversation.h"

CConversation::CConversation(unsigned int oid, SERootObject* root) : Conversation(oid, root)
{
}

void CConversation::OnParticipantListChange()
{
	this->GetParticipants(this->participants, CConversation::OTHER_CONSUMERS);
	fetch(this->participants);
}
