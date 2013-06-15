#pragma once

#include "common.h"

class CAccount : public Account
{
public:
	typedef DRef<CAccount, Account> Ref;
	typedef DRefs<CAccount, Account> Refs;
	
	CAccount(unsigned int oid, CSkypeProto*);

	bool IsOnline();
	bool SetAvatar(SEBinary avatar, Skype::VALIDATERESULT &result);

private:
	CSkypeProto* ppro;

	void OnChange(int prop);
};