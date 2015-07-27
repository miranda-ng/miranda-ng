#ifndef _SKYPE_TROUTER_H_
#define _SKYPE_TROUTER_H_

struct TRInfo
{
	std::string socketIo,
		connId,
		st,
		se,
		instance,
		ccid,
		sessId,
		sig,
		url;
	time_t lastRegistrationTime;
};

#endif //_SKYPE_TROUTER_H_