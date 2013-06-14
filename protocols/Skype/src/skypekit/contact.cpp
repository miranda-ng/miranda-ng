#include "..\skype.h"
#include "contact.h"

CContact::CContact(CSkypeProto* _ppro, unsigned int oid, SERootObject* root) :
	Contact(oid, root),
	proto(_ppro)
{
}

SEString CContact::GetSid()
{
	SEString result;
	CContact::AVAILABILITY availability;
	this->GetPropAvailability(availability);
	if (availability == CContact::SKYPEOUT)
		this->GetPropPstnnumber(result);
	else
		this->GetPropSkypename(result);
	return result;
}

SEString CContact::GetNick()
{
	SEString result;
	CContact::AVAILABILITY availability;
	this->GetPropAvailability(availability);
	if (availability == CContact::SKYPEOUT)
		this->GetPropPstnnumber(result);
	else
		this->GetPropFullname(result);
	return result;
}

bool CContact::GetFullname(SEString &firstName, SEString &lastName)
{
	SEString fullname;
	this->GetPropFullname(fullname);
	int pos = fullname.find(" ");
	if (pos != -1)
	{
		firstName = fullname.substr(0, pos - 1);
		lastName = fullname.right((int)fullname.size() - pos - 1);
	} else
		firstName = fullname;
	
	return true;
}

void CContact::OnChange(int prop)
{
	proto->OnContactChanged(this->ref(), prop);
}