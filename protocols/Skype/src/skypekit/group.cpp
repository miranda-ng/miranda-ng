#include "group.h"

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

void CContactGroup::OnChange(const ContactRef &contact)
{
	if (this->proto)
		(proto->*callback)(contact);
}