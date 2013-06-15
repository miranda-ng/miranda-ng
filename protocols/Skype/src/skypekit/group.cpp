#include "..\skype.h"
#include "group.h"

CContactGroup::CContactGroup(unsigned int oid, CSkypeProto* _ppro) :
	ContactGroup(oid, _ppro),
	proto(_ppro)
{
}

void CContactGroup::OnChange(const ContactRef &contact)
{
	proto->OnContactListChanged(contact);
}