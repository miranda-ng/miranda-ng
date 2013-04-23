#pragma once

#include "common.h"

class CTransfer : public Transfer
{
public:
	typedef void (CSkypeProto::* OnTransfer)(CTransfer::Ref transfer, int);

	typedef DRef<CTransfer, Transfer> Ref;
	typedef DRefs<CTransfer, Transfer> Refs;
  
	CTransfer(unsigned int oid, SERootObject* p_root);

	void SetOnTransferCallback(OnTransfer callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnTransfer transferCallback;

	void OnChange(int prop);
};