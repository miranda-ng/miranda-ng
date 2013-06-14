#pragma once

#include "common.h"

class CTransfer : public Transfer
{
public:
	typedef DRef<CTransfer, Transfer> Ref;
	typedef DRefs<CTransfer, Transfer> Refs;
  
	CTransfer(CSkypeProto*, unsigned int oid, SERootObject* p_root);

private:
	CSkypeProto* proto;

	void OnChange(int prop);
};