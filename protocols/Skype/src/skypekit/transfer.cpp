#include "transfer.h"

CTransfer::CTransfer(unsigned int oid, SERootObject* root) : Transfer(oid, root) 
{
	this->proto = NULL;
	this->transferCallback = NULL;
}

void CTransfer::SetOnTransferCallback(OnTransfer callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->transferCallback = callback;
}

void CTransfer::OnChange(int prop)
{
	if (this->proto)
		(proto->*transferCallback)(this->ref(), prop);
}