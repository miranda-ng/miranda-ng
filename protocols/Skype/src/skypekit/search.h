#pragma once

#include "common.h"
#include "contact.h"

class CContactSearch : public ContactSearch
{
public:
	typedef DRef<CContactSearch, ContactSearch> Ref;
	typedef DRefs<CContactSearch, ContactSearch> Refs;
	
	bool isSeachFinished;
	bool isSeachFailed;

	CContactSearch(unsigned int oid, SERootObject* root);

	void OnChange(int prop);
	void OnNewResult(const ContactRef &contact, const uint &rankValue);

	void SetProtoInfo(CSkypeProto* proto, HANDLE hSearch);

	void BlockWhileSearch();

private:
	HANDLE hSearch;
	CSkypeProto* proto;
};