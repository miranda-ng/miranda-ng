#pragma once

#include "common.h"
#include "contact.h"

class CContactGroup : public ContactGroup
{
public:
	typedef void (CSkypeProto::* OnContactListChanged)(CContact::Ref contact);

	typedef DRef<CContactGroup, ContactGroup> Ref;
	typedef DRefs<CContactGroup, ContactGroup> Refs;
	CContactGroup(unsigned int oid, SERootObject* root);

	void SetOnContactListChangedCallback(OnContactListChanged callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnContactListChanged callback;

	void OnChange(const ContactRef &contact);
};