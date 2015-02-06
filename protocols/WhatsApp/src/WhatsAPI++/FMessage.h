/*
 * FMessage.h
 *
 *  Created on: 02/07/2012
 *      Author: Antonio
 */



#ifndef FMESSAGE_H_
#define FMESSAGE_H_

#include <string>
//#include <SDL.h>
#include <time.h>
#include "IMutex.h"

struct Key
{
	std::string remote_jid;
	bool from_me;
	std::string id;

	Key(const std::string &remote_jid, bool from_me, const std::string &id);
	std::string toString();

};

struct FMessage
{
	Key key;
	unsigned char media_wa_type;
	std::string data;
	long long timestamp;
	std::string remote_resource;
	bool wants_receipt;
	unsigned char status;
	std::string notifyname;
	bool offline;
	std::string media_url;
	std::string media_name;
	
	long long media_size;
	int media_duration_seconds;
	double latitude;
	double longitude;

	enum {
		WA_TYPE_UNDEFINED = 0,
		WA_TYPE_IMAGE = 1,
		WA_TYPE_AUDIO = 2,
		WA_TYPE_VIDEO = 3,
		WA_TYPE_CONTACT = 4,
		WA_TYPE_LOCATION = 5,
		WA_TYPE_SYSTEM = 7
	};

	enum {
		STATUS_UNSENT = 0,
		STATUS_UPLOADING = 1,
		STATUS_UPLOADED = 2,
		STATUS_SENT_BY_CLIENT = 3,
		STATUS_RECEIVED_BY_SERVER = 4,
		STATUS_RECEIVED_BY_TARGET = 5,
		STATUS_NEVER_SEND = 6,
		STATUS_SERVER_BOUNCE = 7,

		STATUS_USER_ADDED = 191,
		STATUS_USER_REMOVED = 192,
		STATUS_SUBJECT_CHANGED = 193,
		STATUS_PICTURE_CHANGED_SET = 194,
		STATUS_PICTURE_CHANGED_DELETE = 195
	};

	static std::string getMessage_WA_Type_StrValue(unsigned char type);
	static unsigned char getMessage_WA_Type(const std::string &typeString);

	FMessage();
	FMessage(const std::string &remote_jid, bool from_me, const std::string &id);
	virtual ~FMessage();
};

#endif /* FMESSAGE_H_ */
