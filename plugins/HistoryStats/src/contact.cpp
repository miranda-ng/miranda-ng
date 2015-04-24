#include "stdafx.h"
#include "contact.h"
#include "utils.h"

#include <queue>
#include <cmath>

Contact::Contact(Statistic* pStatistic, int nSlots, const ext::string& nick, const ext::string& protocol, const ext::string& group, int nContacts, int nSubcontacts) :
	m_Nick(nick),
	m_Protocol(protocol),
	m_Group(group),
	m_Bytes(0, 0),
	m_Messages(0, 0),
	m_Chats(0, 0),
	m_bFirstLastTimeValid(false),
	m_FirstTime(0),
	m_LastTime(0),
	m_bChatDurValid(false),
	m_ChatDurMin(0xFFFFFFFF),
	m_ChatDurMax(0),
	m_ChatDurSum(0),
	m_Urls(0, 0),
	m_Files(0, 0),
	m_NumContacts(nContacts),
	m_NumSubcontacts(nSubcontacts),
	m_pStatistic(pStatistic)
{
	m_Slots.resize(nSlots, NULL);
}

void Contact::updateTime(DWORD msgTime)
{
	if (!m_bFirstLastTimeValid) {
		m_FirstTime = msgTime;
		m_LastTime = msgTime;
		m_bFirstLastTimeValid = true;
	}
	else {
		m_FirstTime = min(m_FirstTime, msgTime);
		m_LastTime = max(m_LastTime, msgTime);
	}
}

void Contact::updateChatDur(DWORD timeDelta)
{
	m_bChatDurValid = true;

	m_ChatDurSum += timeDelta;

	m_ChatDurMin = min(m_ChatDurMin, timeDelta);
	m_ChatDurMax = max(m_ChatDurMax, timeDelta);
}

double Contact::getAvg(int nTotal) const
{
	DWORD dwHistTime = m_pStatistic->getLastTime() - getFirstTime();
	if (dwHistTime < m_pStatistic->getAverageMinTime())
		dwHistTime = m_pStatistic->getAverageMinTime();

	return dwHistTime ? (0.0 + nTotal) / dwHistTime : 0.0;
}

void Contact::addMessage(Message& msg)
{
	if (msg.isOutgoing()) {
		m_Bytes.out += msg.getLength();
		m_Messages.out++;
	}
	else {
		m_Bytes.in += msg.getLength();
		m_Messages.in++;
	}

	updateTime(msg.getTimestamp());
}

void Contact::addChat(bool bOutgoing, DWORD, DWORD duration)
{
	if (bOutgoing)
		m_Chats.out++;
	else
		m_Chats.in++;

	updateChatDur(duration);
}

void Contact::addEvent(WORD eventType, bool bOutgoing)
{
	InOut* pIO = NULL;

	switch (eventType) {
	case EVENTTYPE_URL:
		pIO = &m_Urls;
		break;

	case EVENTTYPE_FILE:
		pIO = &m_Files;
		break;

	default:
		return;
	}

	if (bOutgoing)
		pIO->out++;
	else
		pIO->in++;
}

void Contact::merge(const Contact& other)
{
	if (m_Nick != other.m_Nick)
		m_Nick = TranslateT("(multiple)");

	if (m_Protocol != other.m_Protocol)
		m_Protocol = TranslateT("(multiple)");

	if (m_Group != other.m_Group)
		m_Group = TranslateT("(multiple)");

	m_Bytes += other.m_Bytes;
	m_Messages += other.m_Messages;
	m_Chats += other.m_Chats;

	if (other.m_bFirstLastTimeValid) {
		updateTime(other.m_FirstTime);
		updateTime(other.m_LastTime);
	}

	if (other.m_bChatDurValid) {
		m_ChatDurSum += other.m_ChatDurSum - other.m_ChatDurMin - other.m_ChatDurMax;

		updateChatDur(other.m_ChatDurMin);
		updateChatDur(other.m_ChatDurMax);
	}

	m_Files += other.m_Files;
	m_Urls += other.m_Urls;

	m_NumContacts += other.m_NumContacts;
	m_NumSubcontacts += other.m_NumSubcontacts;
}
