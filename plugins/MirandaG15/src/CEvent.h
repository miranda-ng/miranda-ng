#ifndef _CEVENT_H_
#define _CEVENT_H_

enum EventType 
{
	EVENT_MSG_RECEIVED = 0,
	EVENT_MSG_SENT = 3,
	EVENT_MESSAGE_ACK = 4,
	EVENT_STATUS = 1,
	EVENT_MSG_ACK = 2,

	EVENT_FILE = 6,
	EVENT_AUTHREQUEST = 7,
	EVENT_CONTACTS = 8,
	EVENT_ADDED = 9,

	EVENT_CONTACT_ADDED = 10,
	EVENT_CONTACT_DELETED = 11,
	EVENT_CONTACT_NICK = 12,
	EVENT_CONTACT_HIDDEN = 13,
	EVENT_CONTACT_GROUP = 14,

	EVENT_SIGNED_ON = 15,
	EVENT_SIGNED_OFF = 16,

	EVENT_PROTO_STATUS = 17,
	EVENT_PROTO_CONNECTED = 18,
	EVENT_PROTO_DISCONNECTED = 19,

	EVENT_TYPING_NOTIFICATION = 20,
	EVENT_MESSAGEWINDOW = 21,
	EVENT_IRC_SENT = 22,
	EVENT_IRC_RECEIVED = 23
};

#define MSG_READ 0
#define MSG_UNREAD 1 

struct CEvent
{
	enum EventType	eType;
	uint32_t dwFlags = 0;
	MCONTACT hContact = 0;
	struct tm Time = {};
	bool bTime = false;

	bool bNotification = false;
	bool bLog = true;

	tstring strDescription;
	tstring strSummary;

	MEVENT hValue = 0;
	int iValue = 0;
	tstring strValue;
};

#endif