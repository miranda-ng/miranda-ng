#include "stdafx.h"
#include "mirandacontact.h"

#include "_consts.h"

#include "utils.h"
#include "main.h"

/*
 * MirandaContact
 */

void MirandaContact::fetchSlot(int i)
{
	ContactInfo& ci = m_CIs[i];

	if (!ci.hEvent) {
		free(ci.ei.dbe.pBlob);

		m_CIs.erase(m_CIs.begin() + i);
		return;
	}

	EventInfo& ei = ci.ei;

	ei.hContact = ci.hContact;
	ei.dbe.cbBlob = db_event_getBlobSize(ci.hEvent);
	ei.dbe.cbSize = sizeof(ei.dbe);

	if (ei.dbe.cbBlob > ei.nAllocated) {
		ei.nAllocated = ei.dbe.cbBlob;
		ei.dbe.pBlob = reinterpret_cast<PBYTE>(realloc(ei.dbe.pBlob, ei.dbe.cbBlob + 1));
	}

	db_event_get(ci.hEvent, &ei.dbe);

	stripMetaID(ei.dbe);

	ci.hEvent = db_event_next(ci.hContact, ci.hEvent);
}

void MirandaContact::stripMetaID(DBEVENTINFO& dbe)
{
	if (strcmp(dbe.szModule,META_PROTO)==0) {
		char* pTextBegin = reinterpret_cast<char*>(dbe.pBlob);

		if (dbe.cbBlob >= 6 && !pTextBegin[dbe.cbBlob - 1]) {
			char* pIDEnd = pTextBegin + dbe.cbBlob - 1;
			char* pIDBegin = pIDEnd;
			char* pIDSep = NULL;

			while (pIDBegin >= pTextBegin + 2 && *--pIDBegin)
				if (*pIDBegin == '*')
					pIDSep = pIDBegin;

			++pIDBegin;

			if (pIDSep && pIDBegin < pIDSep && !*(pIDBegin - 1)) {
				dbe.cbBlob = pIDBegin - pTextBegin;
			}
		}
	}
}

MirandaContact::MirandaContact(const ext::string& strNick, const ext::string& strProtocol, const ext::string& strGroup, const SourceHandles& sources) :
	m_strNick(strNick),
	m_strProtocol(strProtocol),
	m_strGroup(strGroup),
	m_Sources(sources)
{
}

MirandaContact::~MirandaContact()
{
	endRead();
}

void MirandaContact::merge(const MirandaContact& other)
{
	if (m_strNick != other.m_strNick)
		m_strNick = TranslateT("(multiple)");

	if (m_strProtocol != other.m_strProtocol)
		m_strProtocol = TranslateT("(multiple)");

	if (m_strGroup != other.m_strGroup)
		m_strGroup = TranslateT("(multiple)");

	citer_each_(SourceHandles, i, other.m_Sources)
	{
		m_Sources.push_back(*i);
	}
}

void MirandaContact::beginRead()
{
	// clean up first
	endRead();

	// allocate required data
	m_CIs.resize(m_Sources.size());

	for (int j = m_Sources.size() - 1; j >= 0; --j) {
		ContactInfo& ci = m_CIs[j];

		ci.hContact = m_Sources[j];
		ci.hEvent = db_event_first(ci.hContact);
		ci.ei.dbe.pBlob = NULL;
		ci.ei.nAllocated = 0;

		fetchSlot(j);
	}

	fillQueue();
}

void MirandaContact::endRead()
{
#if defined(_DEBUG)
	if (m_CIs.size() + m_EIs.size() + m_SpareEIs.size() > 0) {
		ext::string strLog = ext::str(ext::format(_T("Freeing | CIs and |+| EIs...\n")) % m_CIs.size() % m_EIs.size() % m_SpareEIs.size());

		OutputDebugString(strLog.c_str());
	}
#endif // _DEBUG

	citer_each_(std::vector<ContactInfo>, i, m_CIs)
	{
		free(i->ei.dbe.pBlob);
	}

	citer_each_(std::list<EventInfo>, i, m_EIs)
	{
		free(i->dbe.pBlob);
	}

	citer_each_(std::list<EventInfo>, i, m_SpareEIs)
	{
		free(i->dbe.pBlob);
	}

	m_CIs.clear();
	m_EIs.clear();
	m_SpareEIs.clear();
}

void MirandaContact::readNext()
{
	if (!m_EIs.empty()) {
		m_SpareEIs.push_back(m_EIs.front());
		m_EIs.pop_front();
	}

	fillQueue();
}

/*
 * MirandaContactTolerantMerge
 */

void MirandaContactTolerantMerge::fillQueue()
{
	// assume that items with +/- 30 seconds may be equal
	static const int timestampTol = 30;

	while (!m_CIs.empty() && (m_EIs.size() < 2 || (m_EIs.back().dbe.timestamp - m_EIs.front().dbe.timestamp) <= timestampTol)) {
		// find oldest next event in chains
		int nNext = 0;
		DWORD timestampFirst = m_CIs.front().ei.dbe.timestamp;

		for (int i = 1; i < m_CIs.size(); ++i) {
			if (m_CIs[i].ei.dbe.timestamp < timestampFirst) {
				timestampFirst = m_CIs[i].ei.dbe.timestamp;
				nNext = i;
			}
		}

		// insert the fetched at correct position or throw away if duplicate
		ContactInfo& ci = m_CIs[nNext];

		std::list<EventInfo>::iterator insPos = m_EIs.end();
		bool bIsDuplicate = false;

		iter_each_(std::list<EventInfo>, j, m_EIs)
		{
			EventInfo& j_ei = *j;
			int timestampDelta = j_ei.dbe.timestamp - ci.ei.dbe.timestamp;

			if (timestampDelta > 0) {
				insPos = j;
			}

			if (j_ei.hContact != ci.ei.hContact &&
				 timestampDelta >= -timestampTol && timestampDelta <= timestampTol &&
				 j_ei.dbe.eventType == ci.ei.dbe.eventType &&
				 (j_ei.dbe.flags & ~DBEF_READ) == (ci.ei.dbe.flags & ~DBEF_READ) &&
				 j_ei.dbe.cbBlob == ci.ei.dbe.cbBlob &&
				 memcmp(j_ei.dbe.pBlob, ci.ei.dbe.pBlob, j_ei.dbe.cbBlob) == 0) {
				bIsDuplicate = true;

				break;
			}
		}

		if (!bIsDuplicate) {
			m_EIs.insert(insPos, ci.ei);

			if (!m_SpareEIs.empty()) {
				ci.ei = m_SpareEIs.front();
				m_SpareEIs.pop_front();
			}
			else {
				ci.ei.dbe.pBlob = NULL;
				ci.ei.nAllocated = 0;
			}
		}

		fetchSlot(nNext);
	}
}

/*
 * MirandaContactStrictMerge
 */

void MirandaContactStrictMerge::fillQueue()
{
	// assume that items with +/- 30 seconds may be equal
	static const int timestampTol = 0;

	while (!m_CIs.empty() && (m_EIs.size() < 2 || (m_EIs.back().dbe.timestamp - m_EIs.front().dbe.timestamp) <= timestampTol)) {
		// find oldest next event in chains
		int nNext = 0;
		DWORD timestampFirst = m_CIs.front().ei.dbe.timestamp;

		for (int i = 1; i < m_CIs.size(); ++i) {
			if (m_CIs[i].ei.dbe.timestamp < timestampFirst) {
				timestampFirst = m_CIs[i].ei.dbe.timestamp;
				nNext = i;
			}
		}

		// insert the fetched at correct position or throw away if duplicate
		ContactInfo& ci = m_CIs[nNext];

		std::list<EventInfo>::iterator insPos = m_EIs.end();
		bool bIsDuplicate = false;

		iter_each_(std::list<EventInfo>, j, m_EIs)
		{
			EventInfo& j_ei = *j;
			int timestampDelta = j_ei.dbe.timestamp - ci.ei.dbe.timestamp;

			if (timestampDelta > 0)
				insPos = j;

			if (j_ei.hContact != ci.ei.hContact
				 && timestampDelta >= -timestampTol && timestampDelta <= timestampTol
				 && j_ei.dbe.eventType == ci.ei.dbe.eventType
				 && (j_ei.dbe.flags & ~DBEF_READ) == (ci.ei.dbe.flags & ~DBEF_READ)
				 && j_ei.dbe.cbBlob == ci.ei.dbe.cbBlob
				 && memcmp(j_ei.dbe.pBlob, ci.ei.dbe.pBlob, j_ei.dbe.cbBlob) == 0)
			{
				bIsDuplicate = true;
				break;
			}
		}

		if (!bIsDuplicate) {
			m_EIs.insert(insPos, ci.ei);

			if (!m_SpareEIs.empty()) {
				ci.ei = m_SpareEIs.front();
				m_SpareEIs.pop_front();
			}
			else {
				ci.ei.dbe.pBlob = NULL;
				ci.ei.nAllocated = 0;
			}
		}

		fetchSlot(nNext);
	}
}

/*
 * MirandaContactNoMerge
 */

void MirandaContactNoMerge::fillQueue()
{
	while (!m_CIs.empty() && m_EIs.size() < 1) {
		// find oldest next event in chains
		int nNext = 0;
		DWORD timestampFirst = m_CIs.front().ei.dbe.timestamp;

		for (int i = 1; i < m_CIs.size(); ++i) {
			if (m_CIs[i].ei.dbe.timestamp < timestampFirst) {
				timestampFirst = m_CIs[i].ei.dbe.timestamp;
				nNext = i;
			}
		}

		// insert the fetched at correct position or throw away if duplicate
		ContactInfo& ci = m_CIs[nNext];

		m_EIs.push_back(ci.ei);

		if (!m_SpareEIs.empty()) {
			ci.ei = m_SpareEIs.front();
			m_SpareEIs.pop_front();
		}
		else {
			ci.ei.dbe.pBlob = NULL;
			ci.ei.nAllocated = 0;
		}

		fetchSlot(nNext);
	}
}

/*
 * MirandaContactFactory
 */

MirandaContact* MirandaContactFactory::makeMirandaContact(int MergeMode, const ext::string& strNick, const ext::string& strProtocol, const ext::string& strGroup, const MirandaContact::SourceHandles& sources)
{
	switch (MergeMode) {
	case Settings::mmTolerantMerge:
		return new MirandaContactTolerantMerge(strNick, strProtocol, strGroup, sources);

	case Settings::mmStrictMerge:
		return new MirandaContactStrictMerge(strNick, strProtocol, strGroup, sources);

	case Settings::mmNoMerge:
		return new MirandaContactNoMerge(strNick, strProtocol, strGroup, sources);

	default:
		return 0;
	}
}
