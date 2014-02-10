#if !defined(ENTITIES_H)
#define ENTITIES_H

struct send_direct
{
	send_direct(HCONTACT hContact,const std::string &msg, HANDLE msgid, bool isChat = false)
      : hContact(hContact), msg(msg), msgid(msgid)
      {}
	HCONTACT hContact;
	std::string msg;
	HANDLE msgid;
};

struct send_typing
{
	send_typing(HCONTACT hContact,const int status) : hContact(hContact), status(status) {}
	HCONTACT hContact;
	int status;
};

struct input_box
{
   tstring text;
   tstring title;
   tstring defaultValue;
   int limit;

   void (__cdecl WhatsAppProto::*thread)(void*);
   WhatsAppProto* proto;
   void* userData;
};

struct input_box_ret // has to be deleted by input-box handler
{
   void* userData; // has to be deleted by input-box handler
   char* value; // mir_free() has to be called by input-box handler
};



#endif // ENTITIES_H