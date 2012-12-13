#pragma once

#undef OCSP_REQUEST
#undef OCSP_RESPONSE

#include <skype-embedded_2.h>

struct CSkypeProto;

class CConversation : public Conversation
{
public:
	typedef void (CSkypeProto::* OnMessageReceived)(const char *sid, const char *text);

	typedef DRef<CConversation, Conversation> Ref;
	typedef DRefs<CConversation, Conversation> Refs;

	CConversation(unsigned int oid, SERootObject* root);

	void SetOnMessageReceivedCallback(OnMessageReceived callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnMessageReceived callback;
	void OnMessage(const MessageRef & message);
};

class CContact : public Contact
{
public:
	typedef void (CSkypeProto::* OnContactChanged)(CContact::Ref contact, int);

	typedef DRef<CContact, Contact> Ref;
	typedef DRefs<CContact, Contact> Refs;

	CContact(unsigned int oid, SERootObject* root);
	
	void SetOnContactChangedCallback(OnContactChanged callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnContactChanged callback;

	void OnChange(int prop);
};

class CContactSearch : public ContactSearch
{
public:
	typedef void (CSkypeProto::* OnSearchCompleted)(HANDLE hSearch);
	typedef void (CSkypeProto::* OnContactFinded)(HANDLE hSearch, CContact::Ref contact);

	typedef DRef<CContactSearch, ContactSearch> Ref;
	typedef DRefs<CContactSearch, ContactSearch> Refs;
	
	bool isSeachFinished;
	bool isSeachFailed;

	CContactSearch(unsigned int oid, SERootObject* root);

	void OnChange(int prop);
	void OnNewResult(const ContactRef& contact, const uint& rankValue);

	void SetProtoInfo(CSkypeProto* proto, HANDLE hSearch);
	void SetOnSearchCompleatedCallback(OnSearchCompleted callback);
	void SetOnContactFindedCallback(OnContactFinded callback);

	void BlockWhileSearch();
private:
	HANDLE hSearch;
	CSkypeProto* proto;
	OnSearchCompleted SearchCompletedCallback;
	OnContactFinded ContactFindedCallback;
};

class CContactGroup : public ContactGroup
{
public:
	typedef void (CSkypeProto::* OnContactListChanged)(const ContactRef& contact);

	typedef DRef<CContactGroup, ContactGroup> Ref;
	typedef DRefs<CContactGroup, ContactGroup> Refs;
	CContactGroup(unsigned int oid, SERootObject* root);

	//CContact::Refs ContactList;
	void SetOnContactListChangedCallback(OnContactListChanged callback, CSkypeProto* proto);

	//bool Contains(const ContactRef& contact);

private:
	CSkypeProto* proto;
	OnContactListChanged callback;

	void OnChange(const ContactRef& contact);
};

class CAccount : public Account
{
public:
	typedef void (CSkypeProto::* OnAccountChanged)(int);

	typedef DRef<CAccount, Account> Ref;
	typedef DRefs<CAccount, Account> Refs;
	
	CAccount(unsigned int oid, SERootObject* root);
	
	void SetOnAccountChangedCallback(OnAccountChanged callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnAccountChanged callback;
	void OnChange(int prop);
};

class CSkype : public Skype
{
public:
	typedef void (CSkypeProto::* OnConversationAdded)(CConversation::Ref conversation);

	CAccount*		newAccount(int oid);
	CContactGroup*	newContactGroup(int oid);
	CConversation*	newConversation(int oid);
	CContactSearch*	newContactSearch(int oid);
	CContact*		newContact(int oid);

	CConversation::Refs inbox;

	CSkype(int num_threads = 1);

	void SetOnConversationAddedCallback(OnConversationAdded callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnConversationAdded callback;

	void OnConversationListChange(
		const ConversationRef &conversation, 
		const Conversation::LIST_TYPE &type, 
		const bool &added);
};