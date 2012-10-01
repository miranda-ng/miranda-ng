#include "skype_subclassing.h"

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
  //(proto->*callback)(this, prop);
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

CContactGroup::CContactGroup(unsigned int oid, SERootObject* root) : ContactGroup(oid, root) 
{
}

void CContactGroup::OnChange(const ContactRef& contact)
{
}


CContact::CContact(unsigned int oid, SERootObject* root) : Contact(oid, root) 
{
}

void CAccount::SetOnChangeCallback(OnContactChangeFunc callback, CSkypeProto* proto)
{
	this->proto  = proto;
	this->callback = callback;
}

void CContact::OnChange(int prop)
{
	//(proto->*callback)(this, prop);
}