#include "..\skype.h"
#include "transfer.h"

CTransfer::CTransfer(unsigned int oid, CSkypeProto* _ppro) :
	Transfer(oid, _ppro),
	proto(_ppro)
{
}

void CTransfer::OnChange(int prop)
{
	proto->OnTransferChanged(this->ref(), prop);
}