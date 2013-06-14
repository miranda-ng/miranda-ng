#include "skype.h"

CAccount* CSkypeProto::newAccount(int oid) 
{ 
	return new CAccount(oid, this); 
}

CContactGroup* CSkypeProto::newContactGroup(int oid)
{ 
	return new CContactGroup(oid, this); 
}

CContact* CSkypeProto::newContact(int oid) 
{ 
	return new CContact(oid, this); 
}

CConversation* CSkypeProto::newConversation(int oid) 
{ 
	return new CConversation(oid, this); 
}

CParticipant* CSkypeProto::newParticipant(int oid) 
{ 
	return new CParticipant(oid, this); 
}

CMessage* CSkypeProto::newMessage(int oid) 
{ 
	return new CMessage(oid, this); 
}

CTransfer* CSkypeProto::newTransfer(int oid) 
{ 
	return new CTransfer(oid, this); 
}

CContactSearch* CSkypeProto::newContactSearch(int oid)
{
	return new CContactSearch(oid, this);
}