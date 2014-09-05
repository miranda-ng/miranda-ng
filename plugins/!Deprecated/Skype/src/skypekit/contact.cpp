#include "..\skype.h"
#include "contact.h"

CContact::CContact(unsigned int oid, CSkypeProto* _ppro) :
	Contact(oid, _ppro),
	proto(_ppro)
{
}

SEString CContact::GetSid()
{
	SEString result;
	CContact::AVAILABILITY availability;
	this->GetPropAvailability(availability);
	if (availability == Contact::SKYPEOUT || availability == Contact::BLOCKED_SKYPEOUT)
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
	{
		SEString sid;
		this->GetIdentity(sid);

		this->GetPropDisplayname(result);
		if (this->proto && this->proto->login)
		{
			if (sid.equals(result))
				this->GetPropFullname(result);
		}

		if (result.size() == 0)
			result = sid;
	}
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