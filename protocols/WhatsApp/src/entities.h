#if !defined(ENTITIES_H)
#define ENTITIES_H

struct send_direct
{
	send_direct(MCONTACT hContact, const std::string &msg, HANDLE msgid, bool isChat = false) :
		hContact(hContact), msg(msg), msgid(msgid)
		{}

	MCONTACT hContact;
	std::string msg;
	HANDLE msgid;
};

struct input_box
{
	tstring text;
	tstring title;
	tstring defaultValue;
	int limit;

	void(__cdecl WhatsAppProto::*thread)(void*);
	WhatsAppProto *proto;
	void *userData;
};

struct input_box_ret // has to be deleted by input-box handler
{
	void *userData; // has to be deleted by input-box handler
	char *value; // mir_free() has to be called by input-box handler
};


#endif // ENTITIES_H