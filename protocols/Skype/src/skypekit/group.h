#pragma once

#include "common.h"
#include "contact.h"

class CContactGroup : public ContactGroup
{
public:
	typedef void (CSkypeProto::* OnContactListChanged)(CContact::Ref contact);

	typedef DRef<CContactGroup, ContactGroup> Ref;
	typedef DRefs<CContactGroup, ContactGroup> Refs;
	CContactGroup(CSkypeProto*, unsigned int oid, SERootObject* root);

private:
	CSkypeProto* proto;

	void OnChange(const ContactRef &contact);
};