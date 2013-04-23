#pragma once

#include "common.h"

class CConversation : public Conversation
{
public:
	typedef DRef<CConversation, Conversation> Ref;
	typedef DRefs<CConversation, Conversation> Refs;

	CConversation(unsigned int oid, SERootObject* root);

private:
	CSkypeProto* proto;
};