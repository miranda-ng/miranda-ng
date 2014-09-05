#include "..\skype.h"
#include "search.h"

CContactSearch::CContactSearch(unsigned int oid, CSkypeProto* _ppro) :
	ContactSearch(oid, _ppro),
	proto(_ppro)
{
}

void CContactSearch::OnChange(int prop)
{
	if (prop == P_CONTACT_SEARCH_STATUS)
	{
		CContactSearch::STATUS status;
		this->GetPropContactSearchStatus(status);
		if (status == FINISHED || status == FAILED)
		{
			this->isSeachFinished = true;
			if (this->proto)
				proto->OnSearchCompleted(this->hSearch);
		}
	}
}

void CContactSearch::OnNewResult(const ContactRef &contact, const uint &rankValue)
{
	proto->OnContactFinded(contact, this->hSearch);
}

void CContactSearch::BlockWhileSearch()
{
	this->isSeachFinished = false;
	this->isSeachFailed = false;
	while (!this->isSeachFinished && !this->isSeachFailed) 
		Sleep(1); 
}
