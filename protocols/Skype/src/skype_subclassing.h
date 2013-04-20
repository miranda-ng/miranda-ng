#pragma once

#undef OCSP_REQUEST
#undef OCSP_RESPONSE

#include <skype-embedded_2.h>

struct CSkypeProto;

class CSkype;

class CMessage : public Message
{
public:
	typedef DRef<CMessage, Message> Ref;
	typedef DRefs<CMessage, Message> Refs;

	CMessage(unsigned int oid, SERootObject* root);
};

class CTransfer : public Transfer
{
public:
	typedef void (CSkypeProto::* OnTransfer)(int prop, CTransfer::Ref transfer);

	typedef DRef<CTransfer, Transfer> Ref;
	typedef DRefs<CTransfer, Transfer> Refs;
  
	CTransfer(unsigned int oid, SERootObject* p_root);

	void SetOnTransferCallback(OnTransfer callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnTransfer transferCallback;

	void OnChange(int prop);
};

class CParticipant : public Participant
{
public:
	typedef DRef<CParticipant, Participant> Ref;
	typedef DRefs<CParticipant, Participant> Refs;

	CParticipant(unsigned int oid, SERootObject* root);

	static SEString GetRankName(CParticipant::RANK rank);
};

class CConversation : public Conversation
{
public:
	typedef void (CSkypeProto::* OnMessageReceived)(CMessage::Ref message);

	typedef DRef<CConversation, Conversation> Ref;
	typedef DRefs<CConversation, Conversation> Refs;

	CConversation(unsigned int oid, SERootObject* root);

	static CConversation::Ref FindBySid(CSkype *skype, const wchar_t *sid);

	void SetOnMessageReceivedCallback(OnMessageReceived callback, CSkypeProto* proto);

private:
	CSkypeProto* proto;
	OnMessageReceived messageReceivedCallback;
	
	void OnMessage(const MessageRef & message);
	void OnChange(int prop);
};

class CContact : public Contact
{
public:
	typedef void (CSkypeProto::* OnContactChanged)(CContact::Ref contact, int);

	typedef DRef<CContact, Contact> Ref;
	typedef DRefs<CContact, Contact> Refs;

	CContact(unsigned int oid, SERootObject* root);

	/*bool SentAuthRequest(SEString message);*/

	SEString GetSid();
	SEString GetNick();
	bool GetFullname(SEString &firstName, SEString &lastName);

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

	void SetOnContactListChangedCallback(OnContactListChanged callback, CSkypeProto* proto);

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

	bool SetAvatar(SEBinary avatar, Skype::VALIDATERESULT &result);
	
	void SetOnAccountChangedCallback(OnAccountChanged callback, CSkypeProto* proto);

private:
	CSkype *skype;
	CSkypeProto* proto;
	OnAccountChanged callback;
	void OnChange(int prop);
};

class CSkype : public Skype
{
public:
	typedef void (CSkypeProto::* OnMessaged)(CConversation::Ref conversation, CMessage::Ref message);

	CAccount*		newAccount(int oid);
	CContactGroup*	newContactGroup(int oid);
	CConversation*	newConversation(int oid);
	CContactSearch*	newContactSearch(int oid);
	CParticipant*	newParticipant(int oid);
	CContact*		newContact(int oid);	
	CMessage*		newMessage(int oid);
	CTransfer*		newTransfer(int oid);

	CConversation::Refs inbox;

	CSkype(int num_threads = 1);

	void SetOnMessageCallback(OnMessaged callback, CSkypeProto* proto);

	static CSkype *GetInstance(HINSTANCE hInstance, const wchar_t *profileName, const wchar_t *dbPath);

private:
	CSkypeProto*	proto;
	OnMessaged		onMessagedCallback;

	void OnMessage(
		const MessageRef & message,
		const bool & changesInboxTimestamp,
		const MessageRef & supersedesHistoryMessage,
		const ConversationRef & conversation);

	static BOOL IsRunAsAdmin();
	static char *LoadKeyPair(HINSTANCE hInstance);
	static int	StartSkypeRuntime(HINSTANCE hInstance, const wchar_t *profileName, int &port, const wchar_t *dbPath);
};