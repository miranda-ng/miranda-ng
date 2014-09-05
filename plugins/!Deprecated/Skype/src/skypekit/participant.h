#pragma once

#include "common.h"

class ChatRoom;

class CParticipant : public Participant
{
public:
	//typedef void (ChatRoom::* OnChanged)(const ParticipantRef &participant, int);

	typedef DRef<CParticipant, Participant> Ref;
	typedef DRefs<CParticipant, Participant> Refs;

	CParticipant(unsigned int oid, SERootObject* root);

	//void SetOnChangedCallback(OnChanged callback, ChatRoom *room);
	void SetChatRoom(ChatRoom *room);

private:
	ChatRoom *room;
	//OnChanged callback;

	void OnChange(int prop);
};