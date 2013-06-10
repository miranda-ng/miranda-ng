#include "conversation.h"

CConversation::CConversation(unsigned int oid, SERootObject* root) : Conversation(oid, root) { }

void CConversation::SetOnConvoChangedCallback(OnConvoChanged callback)
{
	this->callback = callback;
}

void CConversation::OnChange(int prop)
{
	//(((CSkypeProto*)this->root)->*callback)(this->ref(), prop);
}