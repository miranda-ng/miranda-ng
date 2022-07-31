#if !defined(HISTORYSTATS_GUARD_CONTACT_H)
#define HISTORYSTATS_GUARD_CONTACT_H

#include "stdafx.h"
#include "_consts.h"

#include <string>
#include <map>
#include <vector>
#include <ctime>

#include "inout.h"
#include "settings.h"
#include "message.h"
#include "statistic.h"

/*
 * CContact
 */

class CContact
	: private pattern::NotCopyable<CContact>
{
private:
	ext::string m_Nick;
	ext::string m_Protocol;
	ext::string m_Group;
	InOut m_Bytes;
	InOut m_Messages;
	InOut m_Chats;
	bool m_bChatDurValid;
	uint32_t m_ChatDurMin;
	uint32_t m_ChatDurMax;
	uint32_t m_ChatDurSum;
	bool m_bFirstLastTimeValid;
	uint32_t m_FirstTime;
	uint32_t m_LastTime;
	InOut m_Files;
	int m_NumContacts;
	int m_NumSubcontacts;

	std::vector<void*> m_Slots;
	Statistic* m_pStatistic;

private:
	void updateTime(uint32_t msgTime);
	void updateChatDur(uint32_t timeDelta);
	double getAvg(int nTotal) const;

public:
	explicit CContact(Statistic* pStatistic, int nSlots, const ext::string& nick, const ext::string& protocol, const ext::string& group, int nContacts, int nSubcontacts);
	
	// basic contact info
	const ext::string& getNick() const { return m_Nick; }
	const ext::string& getProtocol() const { return m_Protocol; }
	const ext::string& getGroup() const { return m_Group; }

	// characters/messages/bytes (absolute)
	int getOutBytes() const { return m_Bytes.out; }
	int getInBytes() const { return m_Bytes.in; }
	int getTotalBytes() const { return m_Bytes.total(); }
	int getOutMessages() const { return m_Messages.out; }
	int getInMessages() const { return m_Messages.in; }
	int getTotalMessages() const { return m_Messages.total(); }
	int getOutChats() const { return m_Chats.out; }
	int getInChats() const { return m_Chats.in; }
	int getTotalChats() const { return m_Chats.total(); }

	// characters/messages/bytes (average)
	double getOutBytesAvg() const { return getAvg(m_Bytes.out); }
	double getInBytesAvg() const { return getAvg(m_Bytes.in); }
	double getTotalBytesAvg() const { return getAvg(m_Bytes.total()); }
	double getOutMessagesAvg() const { return getAvg(m_Messages.out); }
	double getInMessagesAvg() const { return getAvg(m_Messages.in); }
	double getTotalMessagesAvg() const { return getAvg(m_Messages.total()); }
	double getOutChatsAvg() const { return getAvg(m_Chats.out); }
	double getInChatsAvg() const { return getAvg(m_Chats.in); }
	double getTotalChatsAvg() const { return getAvg(m_Chats.total()); }

	// chat duration
	bool isChatDurValid() const { return m_bChatDurValid; }
	uint32_t getChatDurMin() const { return m_ChatDurMin; }
	uint32_t getChatDurAvg() const { return getTotalChats() ? m_ChatDurSum / getTotalChats() : 0; }
	uint32_t getChatDurMax() const { return m_ChatDurMax; }
	int getChatDurMinForSort() const { return m_bChatDurValid ? getChatDurMin() : -1; }
	int getChatDurAvgForSort() const { return m_bChatDurValid ? getChatDurAvg() : -1; }
	int getChatDurMaxForSort() const { return m_bChatDurValid ? getChatDurMax() : -1; }
	uint32_t getChatDurSum() const { return m_ChatDurSum; }

	// first/last time
	bool isFirstLastTimeValid() const { return m_bFirstLastTimeValid; }
	uint32_t getFirstTime() const { return m_FirstTime; }
	uint32_t getLastTime() const { return m_LastTime; }

	// files and URLs
	int getOutFiles() const { return m_Files.out; }
	int getInFiles() const { return m_Files.in; }
	int getTotalFiles() const { return m_Files.total(); }

	// (sub)contact counts
	int getNumContacts() const { return m_NumContacts; }
	int getNumSubcontacts() const { return m_NumSubcontacts; }

	// stuff for reading history
	void beginMessages() { }
	void endMessages() { }
	void addMessage(Message& msg);
	void addChat(bool bOutgoing, uint32_t localTimestampStarted, uint32_t duration);
	void addEvent(uint16_t eventType, bool bOutgoing);
	void merge(const CContact& other);

	// slot stuff
	int countSlot() const { return m_Slots.size(); }
	const void* getSlot(int index) const { return m_Slots[index]; }
	void* getSlot(int index) { return m_Slots[index]; }
	void setSlot(int index, void* pData) { m_Slots[index] = pData; }
};

/*
 * ContactCompareBase
 */

class ContactCompareBase
{
protected:
	bool m_bAsc;

public:
	virtual bool cmp(const CContact&, const CContact&) { return m_bAsc; }
	void setDir(bool bAsc) { m_bAsc = bAsc; }
	explicit ContactCompareBase() : m_bAsc(true) { }
};

/*
 * ContactCompare<T_>
 */

template<typename T_>
class ContactCompare
	: public ContactCompareBase
{
private:
	T_ (CContact::*m_getData)() const;
	ContactCompareBase* m_pNextCmp;

public:
	virtual bool cmp(const CContact& first, const CContact& second)
	{
		T_ firstVal = (first.*m_getData)();
		T_ secondVal = (second.*m_getData)();

		if (firstVal == secondVal)
		{
			return m_pNextCmp->cmp(first, second);
		}
		else
		{
			return (m_bAsc ? (firstVal < secondVal) : (firstVal > secondVal));
		}
	}

public:
	explicit ContactCompare(ContactCompareBase* pNextCmp, T_ (CContact::*getData)() const)
		: m_pNextCmp(pNextCmp)
	{
		m_getData = getData;
	}
};

/*
 * ContactCompareStr
 */

class ContactCompareStr
	: public ContactCompareBase
{
private:
	const ext::string& (CContact::*m_getData)() const;
	ContactCompareBase* m_pNextCmp;

public:
	virtual bool cmp(const CContact& first, const CContact& second)
	{
		const ext::string& firstVal = (first.*m_getData)();
		const ext::string& secondVal = (second.*m_getData)();

		// int cmpRes = ext::strfunc::icmp(firstVal.c_str(), secondVal.c_str());
		int cmpRes = ext::strfunc::icoll(firstVal.c_str(), secondVal.c_str());

		if (cmpRes == 0)
		{
			return m_pNextCmp->cmp(first, second);
		}
		else
		{
			return (m_bAsc ? (cmpRes < 0) : (cmpRes > 0));
		}
	}

public:
	explicit ContactCompareStr(ContactCompareBase* pNextCmp, const ext::string& (CContact::*getData)() const)
		: m_pNextCmp(pNextCmp)
	{
		m_getData = getData;
	}
};

/*
 * ContactCompareOp
 */

class ContactCompareOp
{
private:
	ContactCompareBase* m_pCmp;

public:
	bool operator ()(const CContact* first, const CContact* second) { return m_pCmp->cmp(*first, *second); }
	explicit ContactCompareOp(ContactCompareBase* pCmp) : m_pCmp(pCmp) { }
};

#endif // HISTORYSTATS_GUARD_CONTACT_H