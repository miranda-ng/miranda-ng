#ifndef _TOX_CHATROOMS_H_
#define _TOX_CHATROOMS_H_

#define WM_AUDIO_END (WM_PROTO_LAST + 100)

struct ChatRoomInviteParam
{
	CToxProto *proto;
	std::vector<MCONTACT> invitedContacts;
};

#endif //_TOX_CHATROOMS_H_