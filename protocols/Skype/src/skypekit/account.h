#pragma once

#include "common.h"

class CAccount : public Account
{
public:
	typedef void (CSkypeProto::* OnAccountChanged)(int);

	typedef DRef<CAccount, Account> Ref;
	typedef DRefs<CAccount, Account> Refs;
	
	CAccount(unsigned int oid, SERootObject* root);

	bool IsOnline();
	bool SetAvatar(SEBinary avatar, Skype::VALIDATERESULT &result);
	
	void SetOnAccountChangedCallback(OnAccountChanged callback, CSkypeProto *ppro);

private:
	CSkypeProto* ppro;
	OnAccountChanged callback;

	void OnChange(int prop);
};