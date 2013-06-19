#include "..\skype.h"
#include "participant.h"
#include "..\skype_chat.h"

CParticipant::CParticipant(unsigned int oid, SERootObject* root) :
	Participant(oid, root)
{
	this->room = NULL;
}

//void CParticipant::SetOnChangedCallback(OnChanged callback, ChatRoom *room)
//{
//	this->room = room;
//	this->callback = callback;
//}

void CParticipant::SetChatRoom(ChatRoom *room)
{
	this->room = room;
}

void CParticipant::OnChange(int prop)
{
	if (this->room != NULL)
		this->room->OnParticipantChanged(this->ref(), prop);
		//(room->*callback)(this->ref(), prop);
}