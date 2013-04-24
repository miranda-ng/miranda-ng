#include "account.h"
#include "skypekit.h"

CAccount::CAccount(unsigned int oid, SERootObject* root) : Account(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CAccount::SetOnAccountChangedCallback(OnAccountChanged callback, CSkypeProto* proto)
{
	this->skype = (CSkype *)root;

	this->proto = proto;
	this->callback = callback;
}

bool CAccount::SetAvatar(SEBinary avatar, Skype::VALIDATERESULT &result)
{
	int fbl;
	if (!this->skype->ValidateAvatar(avatar, result, fbl) || result != Skype::VALIDATED_OK)
		return false;

	if (!this->SetBinProperty(Account::P_AVATAR_IMAGE, avatar))
		return false;

	return true;
}

void CAccount::OnChange(int prop)
{
  if (this->proto)
	  (proto->*callback)(prop);
}