#if !defined(HISTORYSTATS_GUARD_MIRANDACONTACT_H)
#define HISTORYSTATS_GUARD_MIRANDACONTACT_H

#include "stdafx.h"

#include <vector>
#include <list>

/*
 * MirandaContact
 */

class MirandaContact
	: private pattern::NotCopyable<MirandaContact>
{
public:
	struct EventInfo {
		MCONTACT hContact;
		DBEVENTINFO dbe;
		int nAllocated;
//		int nDuplicates;
	};

	struct ContactInfo {
		MCONTACT hContact;
		MEVENT hEvent;
		EventInfo ei;
	};

	typedef std::vector<MCONTACT> SourceHandles;

private:
	// general info
	ext::string m_strNick;
	ext::string m_strProtocol;
	ext::string m_strGroup;
	SourceHandles m_Sources;

protected:
	// reading messages
	std::vector<ContactInfo> m_CIs;
	std::list<EventInfo> m_EIs;
	std::list<EventInfo> m_SpareEIs;

private:
	// reading messages
	void stripMetaID(DBEVENTINFO& dbe);

protected:
	// reading messages
	void fetchSlot(int i);
	virtual void fillQueue() = 0;

public:
	explicit MirandaContact(const ext::string& strNick, const ext::string& strProtocol, const ext::string& strGroup, const SourceHandles& sources);
	virtual ~MirandaContact();

	// general info
	const ext::string& getNick() const { return m_strNick; }
	const ext::string& getProtocol() const { return m_strProtocol; }
	const ext::string& getGroup() const { return m_strGroup; }
	const SourceHandles& getSources() const { return m_Sources; }

	// merge
	void merge(const MirandaContact& other);

	// reading messages
	void beginRead();
	void endRead();
	bool hasNext() { return !m_EIs.empty(); }
	const DBEVENTINFO& getNext() { return m_EIs.front().dbe; }
	void readNext();
};

/*
 * MirandaContactTolerantMerge
 */

class MirandaContactTolerantMerge
	: private pattern::NotCopyable<MirandaContactTolerantMerge>
	, public MirandaContact
{
protected:
	// reading message
	virtual void fillQueue();

public:
	explicit MirandaContactTolerantMerge(const ext::string& strNick, const ext::string& strProtocol, const ext::string& strGroup, const SourceHandles& sources)
		: MirandaContact(strNick, strProtocol, strGroup, sources)
	{
	}
};

/*
 * MirandaContactStrictMerge
 */

class MirandaContactStrictMerge
	: private pattern::NotCopyable<MirandaContactStrictMerge>
	, public MirandaContact
{
protected:
	// reading message
	virtual void fillQueue();

public:
	explicit MirandaContactStrictMerge(const ext::string& strNick, const ext::string& strProtocol, const ext::string& strGroup, const SourceHandles& sources)
		: MirandaContact(strNick, strProtocol, strGroup, sources)
	{
	}
};

/*
 * MirandaContactNoMerge
 */

class MirandaContactNoMerge
	: private pattern::NotCopyable<MirandaContactNoMerge>
	, public MirandaContact
{
protected:
	// reading message
	virtual void fillQueue();

public:
	explicit MirandaContactNoMerge(const ext::string& strNick, const ext::string& strProtocol, const ext::string& strGroup, const SourceHandles& sources)
		: MirandaContact(strNick, strProtocol, strGroup, sources)
	{
	}
};

/*
 * MirandaContactFactory
 */

class MirandaContactFactory
	: public pattern::NotInstantiable<MirandaContactFactory>
{
public:
	static MirandaContact* makeMirandaContact(int MergeMode, const ext::string& strNick, const ext::string& strProtocol, const ext::string& strGroup, const MirandaContact::SourceHandles& sources);
};

#endif // HISTORYSTATS_GUARD_MIRANDACONTACT_H
