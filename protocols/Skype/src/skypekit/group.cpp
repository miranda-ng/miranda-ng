#include "..\skype.h"
#include "group.h"

CContactGroup::CContactGroup(CSkypeProto* _ppro, unsigned int oid, SERootObject* root) :
	ContactGroup(oid, root),
	proto(_ppro)
{
}

void CContactGroup::OnChange(const ContactRef &contact)
{
	proto->OnContactListChanged(contact);
}