#include "search.h"

CContactSearch::CContactSearch(unsigned int oid, SERootObject* root) : ContactSearch(oid, root)
{
	this->proto = NULL;
	this->SearchCompletedCallback == NULL;
	this->ContactFindedCallback == NULL;
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
				(proto->*SearchCompletedCallback)(this->hSearch);
		}
	}
}

void CContactSearch::OnNewResult(const ContactRef &contact, const uint &rankValue)
{
	if (this->proto)
		(proto->*ContactFindedCallback)(contact, this->hSearch);
}

void CContactSearch::BlockWhileSearch()
{
	this->isSeachFinished = false;
	this->isSeachFailed = false;
	while (!this->isSeachFinished && !this->isSeachFailed) 
		Sleep(1); 
}

void CContactSearch::SetProtoInfo(CSkypeProto* proto, HANDLE hSearch)
{
	this->proto = proto;
	this->hSearch = hSearch;
}

void CContactSearch::SetOnSearchCompleatedCallback(OnSearchCompleted callback)
{
	this->SearchCompletedCallback = callback;
}

void CContactSearch::SetOnContactFindedCallback(OnContactFinded callback)
{
	this->ContactFindedCallback = callback;
}