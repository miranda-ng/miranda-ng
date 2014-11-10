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

class Key {
public:
	std::string remote_jid;
	bool from_me;
	std::string id;

	Key(const std::string& remote_jid, bool from_me, const std::string& id);
	std::string toString();

};

class FMessage {
private:
	static int generating_id;
	static std::string generating_header;


public:
   static IMutex* generating_lock; // #WORKAROUND

	Key* key;
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

	static const unsigned char WA_TYPE_UNDEFINED = 0;
	static const unsigned char WA_TYPE_IMAGE = 1;
	static const unsigned char WA_TYPE_AUDIO = 2;
	static const unsigned char WA_TYPE_VIDEO = 3;
	static const unsigned char WA_TYPE_CONTACT = 4;
	static const unsigned char WA_TYPE_LOCATION = 5;
	static const unsigned char WA_TYPE_SYSTEM = 7;

	static const int STATUS_UNSENT = 0;
	static const int STATUS_UPLOADING = 1;
	static const int STATUS_UPLOADED = 2;
	static const int STATUS_SENT_BY_CLIENT = 3;
	static const int STATUS_RECEIVED_BY_SERVER = 4;
	static const int STATUS_RECEIVED_BY_TARGET = 5;
	static const int STATUS_NEVER_SEND = 6;
	static const int STATUS_SERVER_BOUNCE = 7;

	static const int STATUS_USER_ADDED = 191;
	static const int STATUS_USER_REMOVED = 192;
	static const int STATUS_SUBJECT_CHANGED = 193;
	static const int STATUS_PICTURE_CHANGED_SET = 194;
	static const int STATUS_PICTURE_CHANGED_DELETE = 195;


	static std::string getMessage_WA_Type_StrValue(unsigned char type);
	static std::string nextKeyIdNumber();
	static unsigned char getMessage_WA_Type(const std::string& typeString);

	FMessage();
	FMessage(const std::string& remote_jid, bool from_me = true, const std::string& data = "");
	FMessage(Key* key);
	virtual ~FMessage();
};

#endif /* FMESSAGE_H_ */
