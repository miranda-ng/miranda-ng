#pragma once

#include "common.h"

class CContact : public Contact
{
public:
	typedef void (CSkypeProto::* OnContactChanged)(CContact::Ref contact, int);

	typedef DRef<CContact, Contact> Ref;
	typedef DRefs<CContact, Contact> Refs;

	CContact(unsigned int oid, SERootObject* root);

	SEString GetSid();
	SEString GetNick();
	bool GetFullname(SEString &firstName, SEString &lastName);

	void SetOnContactChangedCallback(OnContactChanged callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnContactChanged callback;

	void OnChange(int prop);
};