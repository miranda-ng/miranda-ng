#pragma once

#include "common.h"

class CAccount : public Account
{
public:
	typedef void (CSkypeProto::* OnAccountChanged)(int);

	typedef DRef<CAccount, Account> Ref;
	typedef DRefs<CAccount, Account> Refs;
	
	CAccount(CSkypeProto*, unsigned int oid, SERootObject* root);

	bool IsOnline();
	bool SetAvatar(SEBinary avatar, Skype::VALIDATERESULT &result);

private:
	CSkypeProto* ppro;

	void OnChange(int prop);
};