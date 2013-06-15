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

	CContactSearch(unsigned int oid, CSkypeProto* _ppro);

	void OnChange(int prop);
	void OnNewResult(const ContactRef &contact, const uint &rankValue);

	__forceinline void SetProtoInfo(HANDLE _hSearch) {
		hSearch = _hSearch;
	}

	void BlockWhileSearch();

private:
	HANDLE hSearch;
	CSkypeProto* proto;
};