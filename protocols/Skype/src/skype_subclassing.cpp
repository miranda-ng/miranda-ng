#include "skype_subclassing.h"

// CSkype

CAccount* CSkype::newAccount(int oid) 
{ 
	return new CAccount(oid, this); 
}

CContactGroup* CSkype::newContactGroup(int oid)
{ 
	return new CContactGroup(oid, this); 
}

CContact* CSkype::newContact(int oid) 
{ 
	return new CContact(oid, this); 
}

CConversation* CSkype::newConversation(int oid) 
{ 
	return new CConversation(oid, this); 
}

// CAccount

CAccount::CAccount(unsigned int oid, SERootObject* root) : Account(oid, root) 
{
	this->isLoggedOut = true;
}

void CAccount::OnChange(int prop)
{
  if (prop == CAccount::P_STATUS)
  {
	  CAccount::STATUS loginStatus;
	  this->GetPropStatus(loginStatus);
	  if (loginStatus == CAccount::LOGGED_IN)  
		  this->isLoggedOut = false;
		
		if (loginStatus == CAccount::LOGGED_OUT) 
		{ 
			this->isLoggedOut = true; 
			CAccount::LOGOUTREASON whyLogout;
			this->GetPropLogoutreason(whyLogout);
			if (whyLogout != Account::LOGOUT_CALLED)
			{
				printf("%s\n", (const char*)tostring(whyLogout));
			}
		}
	}

}

void CAccount::BlockWhileLoggingIn()
{
  while (this->isLoggedOut) 
	  Sleep(1); 
}

void CAccount::BlockWhileLoggingOut()
{
  while ( !this->isLoggedOut) 
	  Sleep(1);
}

// CContactGroup

CContactGroup::CContactGroup(unsigned int oid, SERootObject* root) : ContactGroup(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CContactGroup::SetOnContactListChangedCallback(OnContactListChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

bool CContactGroup::Contains(const ContactRef& contact)
{
	return this->ContactList.contains(contact);
}

void CContactGroup::OnChange(const ContactRef& contact)
{
	if (this->proto)
		(proto->*callback)(contact);
}

// CContact

CContact::CContact(unsigned int oid, SERootObject* root) : Contact(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CContact::SetOnContactChangedCallback(OnContactChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

void CContact::OnChange(int prop)
{
	if (this->proto)
		(proto->*callback)(this->ref(), prop);
}

// Conversation

CConversation::CConversation(unsigned int oid, SERootObject* root) : Conversation(oid, root) {}

void CConversation::OnMessage(const MessageRef & message)
{
  // Message handling goes here
}