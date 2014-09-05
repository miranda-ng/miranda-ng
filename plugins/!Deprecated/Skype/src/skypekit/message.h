#pragma once

#include "common.h"

class CMessage : public Message
{
public:
	typedef DRef<CMessage, Message> Ref;
	typedef DRefs<CMessage, Message> Refs;

	CMessage(unsigned int oid, SERootObject* root);
};