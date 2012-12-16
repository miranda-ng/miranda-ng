#include "skype_subclassing.h"

// CSkype

CSkype::CSkype(int num_threads) : Skype(num_threads)
{
	this->proto = NULL;
	this->onMessageReceivedCallback = NULL;
	this->onConversationAddedCallback = NULL;
}

CAccount* CSkype::newAccount(int oid) 
{ 
	return new CAccount(oid, this); 
}

CContactGroup* CSkype::newContactGroup(int oid)
{ 
	return new CContactGroup(oid, this); 
}

CContact* CSkype::newContact(int oid) 
{ 
	return new CContact(oid, this); 
}

CConversation* CSkype::newConversation(int oid) 
{ 
	return new CConversation(oid, this); 
}

CMessage* CSkype::newMessage(int oid) 
{ 
	return new CMessage(oid, this); 
}

CContactSearch*	CSkype::newContactSearch(int oid)
{
	return new CContactSearch(oid, this);
}

void CSkype::OnMessage (
	const MessageRef & message,
	const bool & changesInboxTimestamp,
	const MessageRef & supersedesHistoryMessage,
	const ConversationRef & conversation)
{
    /*uint now;
    skype->GetUnixTimestamp(now);
    conversation->SetConsumedHorizon(now);*/

	Message::TYPE messageType;
	message->GetPropType(messageType);

	Message::SENDING_STATUS sendingStatus;
	message->GetPropSendingStatus(sendingStatus);

	if (messageType == Message::POSTED_TEXT && !sendingStatus)
	{
		if (this->proto)
			(proto->*onMessageReceivedCallback)(message->ref());
	}
}

void CSkype::SetOnMessageReceivedCallback(OnMessageReceived callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->onMessageReceivedCallback = callback;
}

void CSkype::OnConversationListChange(
	const ConversationRef &conversation, 
	const Conversation::LIST_TYPE &type, 
	const bool &added)
{
	if ((type == Conversation::INBOX_CONVERSATIONS) && (added) && (!inbox.contains(conversation)))
	{
		conversation.fetch();
		inbox.append(conversation);
		if (this->proto)
			(proto->*onConversationAddedCallback)(conversation->ref());
	}
}

void CSkype::SetOnConversationAddedCallback(OnConversationAdded callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->onConversationAddedCallback = callback;
}

// CAccount

CAccount::CAccount(unsigned int oid, SERootObject* root) : Account(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CAccount::SetOnAccountChangedCallback(OnAccountChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

void CAccount::OnChange(int prop)
{
  if (this->proto)
	  (proto->*callback)(prop);
}

// CContactGroup

CContactGroup::CContactGroup(unsigned int oid, SERootObject* root) : ContactGroup(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CContactGroup::SetOnContactListChangedCallback(OnContactListChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

//bool CContactGroup::Contains(const ContactRef& contact)
//{
//	return this->ContactList.contains(contact);
//}

void CContactGroup::OnChange(const ContactRef& contact)
{
	if (this->proto)
		(proto->*callback)(contact);
}

// CContactSearch

CContactSearch::CContactSearch(unsigned int oid, SERootObject* root) : ContactSearch(oid, root)
{
	this->proto = NULL;
	this->SearchCompletedCallback == NULL;
	this->ContactFindedCallback == NULL;
}

void CContactSearch::OnChange(int prop)
{
	if (prop == P_CONTACT_SEARCH_STATUS)
	{
		CContactSearch::STATUS status;
		this->GetPropContactSearchStatus(status);
		if (status == FINISHED || status == FAILED)
		{
			this->isSeachFinished = true;
			if (this->proto)
				(proto->*SearchCompletedCallback)(this->hSearch);
		}
	}

	//SEString value = GetProp(prop);
	//List_String dbg = getPropDebug(prop, value);
	//fprintf(stdout,"CONTACTSEARCH.%d:%s = %s\n", getOID(), (const char*)dbg[1], (const char*)dbg[2]);
}

void CContactSearch::OnNewResult(const ContactRef& contact, const uint& rankValue)
{
	if (this->proto)
		(proto->*ContactFindedCallback)(this->hSearch, contact->ref());
}

void CContactSearch::BlockWhileSearch()
{
	this->isSeachFinished = false;
	this->isSeachFailed = false;
	while (!this->isSeachFinished && !this->isSeachFailed) 
		Sleep(1); 
}

void CContactSearch::SetProtoInfo(CSkypeProto* proto, HANDLE hSearch)
{
	this->proto = proto;
	this->hSearch = hSearch;
}

void CContactSearch::SetOnSearchCompleatedCallback(OnSearchCompleted callback)
{
	this->SearchCompletedCallback = callback;
}

void CContactSearch::SetOnContactFindedCallback(OnContactFinded callback)
{
	this->ContactFindedCallback = callback;
}

// CContact

CContact::CContact(unsigned int oid, SERootObject* root) : Contact(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CContact::SetOnContactChangedCallback(OnContactChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

void CContact::OnChange(int prop)
{
	if (this->proto)
		(proto->*callback)(this->ref(), prop);
}

// Conversation

CConversation::CConversation(unsigned int oid, SERootObject* root) : Conversation(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CConversation::OnMessage(const MessageRef & message)
{
	if (this->proto)
		(proto->*callback)(message->ref());
}

void CConversation::SetOnMessageReceivedCallback(OnMessageReceived callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

// CMessage

CMessage::CMessage(unsigned int oid, SERootObject* root) : Message(oid, root) { }