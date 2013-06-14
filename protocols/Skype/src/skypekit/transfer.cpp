#include "..\skype.h"
#include "transfer.h"

CTransfer::CTransfer(CSkypeProto* _ppro, unsigned int oid, SERootObject* root) :
	Transfer(oid, root),
	proto(_ppro)
{
}

void CTransfer::OnChange(int prop)
{
	proto->OnTransferChanged(this->ref(), prop);
}