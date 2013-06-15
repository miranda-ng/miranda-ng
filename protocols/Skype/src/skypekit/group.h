#pragma once

#include "common.h"
#include "contact.h"

class CContactGroup : public ContactGroup
{
public:
	typedef void (CSkypeProto::* OnContactListChanged)(CContact::Ref contact);

	typedef DRef<CContactGroup, ContactGroup> Ref;
	typedef DRefs<CContactGroup, ContactGroup> Refs;
	CContactGroup(unsigned int oid, CSkypeProto*);

private:
	CSkypeProto* proto;

	void OnChange(const ContactRef &contact);
};