#include "skype_subclassing.h"

Account* CSkype::newAccount(int oid) 
{ 
	return new CAccount(oid, this); 
}

CAccount::CAccount(unsigned int oid, SERootObject* root) : Account(oid, root) 
{
	this->isLoggedOut = true;
}

void CAccount::OnChange(int prop)
{
  if (prop == Account::P_STATUS)
  {
	  Account::STATUS loginStatus;
	  this->GetPropStatus(loginStatus);
	  if (loginStatus == Account::LOGGED_IN)  
		  this->isLoggedOut = false;
		
		if (loginStatus == Account::LOGGED_OUT) 
		{ 
			this->isLoggedOut = true; 
			Account::LOGOUTREASON whyLogout;
			this->GetPropLogoutreason(whyLogout);
			/*if (whyLogout != Account::LOGOUT_CALLED)
			{
				printf("%s\n", (const char*)tostring(whyLogout));
			}*/
		}
	}
};

void CAccount::BlockWhileLoggingIn()
{
  while (this->isLoggedOut) 
	  Sleep(1); 
}

void CAccount::BlockWhileLoggingOut()
{
  while ( !this->isLoggedOut) 
	  Sleep(1);
};