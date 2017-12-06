#ifndef _TOX_CHATROOMS_H_
#define _TOX_CHATROOMS_H_

struct ChatRoomInviteParam
{
	CToxProto *proto;
	std::vector<MCONTACT> invitedContacts;
};

#endif //_TOX_CHATROOMS_H_