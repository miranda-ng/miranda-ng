#ifndef _CIRCHISTORY_H_
#define _CIRCHISTORY_H_

struct SIRCMessage
{
	tm Time;
	bool bIsMe;
	tstring strMessage;
};


class CIRCHistory
{
public:
	tstring strChannel;
	tstring strProtocol;
	MCONTACT hContact;
	list<SIRCMessage> LMessages;
	list<tstring> LUsers;
};


#endif