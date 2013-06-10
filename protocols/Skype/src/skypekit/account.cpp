#include "account.h"

CAccount::CAccount(unsigned int oid, SERootObject* root) : Account(oid, root) 
{
	this->ppro = NULL;
}

void CAccount::SetOnAccountChangedCallback(OnAccountChanged callback, CSkypeProto *ppro)
{
	this->ppro = ppro;
	this->callback = callback;
}

bool CAccount::IsOnline()
{
	CAccount::STATUS status;
	this->GetPropStatus(status);
	return status == CAccount::LOGGED_IN;
}

bool CAccount::SetAvatar(SEBinary avatar, Skype::VALIDATERESULT &result)
{
	int fbl;
	if (!((Skype*)this->root)->ValidateAvatar(avatar, result, fbl) || result != Skype::VALIDATED_OK)
		return false;

	if (!this->SetBinProperty(Account::P_AVATAR_IMAGE, avatar))
		return false;

	return true;
}

void CAccount::OnChange(int prop)
{
	if (this->ppro != NULL)
		(ppro->*callback)(prop);
}