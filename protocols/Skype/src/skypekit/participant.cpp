#include "participant.h"

CParticipant::CParticipant(unsigned int oid, SERootObject* root) : Participant(oid, root)
{
	this->room = NULL;
}

void CParticipant::SetOnChangedCallback(OnChanged callback, ChatRoom *room)
{
	this->room = room;
	this->callback = callback;
}

void CParticipant::OnChange(int prop)
{
	if (this->room != NULL)
		(room->*callback)(this->ref(), prop);
}