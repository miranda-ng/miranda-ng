#if !defined(HISTORYSTATS_GUARD_MIRANDAHISTORY_H)
#define HISTORYSTATS_GUARD_MIRANDAHISTORY_H

#include "stdafx.h"

#include <vector>

#include "mirandacontact.h"
#include "settings.h"

class MirandaHistory
	: private pattern::NotCopyable<MirandaHistory>
{
private:
	typedef std::map<ext::a::string, ext::string> ProtocolMap;
	typedef std::vector<MirandaContact*> ContactList;

private:
	const Settings& m_Settings;
	ProtocolMap m_Protocols;
	ext::string m_DefaultProtocol;
	bool m_bContactsAvailable;
	ContactList m_Contacts;

private:
	void populateProtocols();
	const ext::string& getProtocol(const ext::a::string& protocol) const;
	void makeContactsAvailable();
	void readContacts();
	void mergeContacts();

public:
	explicit MirandaHistory(const Settings& settings);
	~MirandaHistory();

	int getContactCount();
	MirandaContact& getContact(int index);
};

#endif // HISTORYSTATS_GUARD_MIRANDAHISTORY_H
