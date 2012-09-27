#pragma once

#include <skype-embedded_2.h>

class CAccount;

class CSkype : public Skype
{
public:
	Account* newAccount(int oid);
};

class CAccount : public Account
{
public:
	typedef DRef<CAccount, Account> Ref;
	typedef DRefs<CAccount, Account> Refs;
	
	bool isLoggedOut;
	
	CAccount(unsigned int oid, SERootObject* root);
	void OnChange(int prop);
	
	void BlockWhileLoggingIn();
	void BlockWhileLoggingOut();
};

class CContact : public Contact
{
};

