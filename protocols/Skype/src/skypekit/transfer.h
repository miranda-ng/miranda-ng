#pragma once

#include "common.h"

class CTransfer : public Transfer
{
public:
	typedef DRef<CTransfer, Transfer> Ref;
	typedef DRefs<CTransfer, Transfer> Refs;
  
	CTransfer(unsigned int oid, CSkypeProto*);

private:
	CSkypeProto* proto;

	void OnChange(int prop);
};