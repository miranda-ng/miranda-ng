#ifndef _CEVENT_H_
#define _CEVENT_H_

enum EventType {	EVENT_MSG_RECEIVED = 0,
					EVENT_MSG_SENT = 3,
					EVENT_MESSAGE_ACK = 4,
					EVENT_STATUS = 1,
					EVENT_MSG_ACK = 2,

					EVENT_URL = 5,
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
					EVENT_IRC_RECEIVED = 23};

#define MSG_READ 0
#define MSG_UNREAD 1 

class CEvent 
{
public:
	CEvent()
	{
		bTime = false;
		dwFlags = NULL;
		hContact = NULL;
		bNotification = false;
		bLog = true;
		strDescription = _T("");

		iValue = NULL;
		hValue = NULL;
		strValue = _T("");
	}

	enum EventType	eType;
	DWORD dwFlags;
	MCONTACT hContact;
	tm	Time;
	bool bTime;

	bool bNotification;
	bool bLog;

	tstring strDescription;
	tstring strSummary;

	MEVENT hValue;
	int iValue;
	tstring strValue;
};

#endif