#include "..\skype.h"
#include "conversation.h"
#include "..\skype_chat.h"

CConversation::CConversation(unsigned int oid, SERootObject* root) :
	Conversation(oid, root)
{
	this->room = NULL;
}

void CConversation::SetChatRoom(ChatRoom *room)
{
	this->room = room;
}

void CConversation::OnChange(int prop)
{
	if (this->room != NULL)
		this->room->OnChange(this->ref(), prop);
}