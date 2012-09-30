#pragma once

#include <skype-embedded_2.h>

class CContact : public Contact
{
public:
	typedef DRef<CContact, Contact> Ref;
	typedef DRefs<CContact, Contact> Refs;

	CContact(unsigned int oid, SERootObject* root);

protected:
	void OnChange(int prop);
};

class CContactGroup : public ContactGroup
{
public:
	typedef DRef<CContactGroup, ContactGroup> Ref;
	typedef DRefs<CContactGroup, ContactGroup> Refs;
	CContactGroup(unsigned int oid, SERootObject* root);

	CContact::Refs ContactList;

protected:
	void OnChange(const ContactRef& contact);
};

class CAccount : public Account
{
public:
	typedef DRef<CAccount, Account> Ref;
	typedef DRefs<CAccount, Account> Refs;
	
	bool isLoggedOut;
	
	CAccount(unsigned int oid, SERootObject* root);
	
	void BlockWhileLoggingIn();
	void BlockWhileLoggingOut();

protected:
	void OnChange(int prop);
};

class CSkype : public Skype
{
public:
	CAccount*		newAccount(int oid);
	CContactGroup*	newContactGroup(int oid);
	CContact*		newContact(int oid);
};