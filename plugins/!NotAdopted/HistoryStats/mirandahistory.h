#if !defined(HISTORYSTATS_GUARD_MIRANDAHISTORY_H)
#define HISTORYSTATS_GUARD_MIRANDAHISTORY_H

#include "_globals.h"

#include <vector>

#include "mirandacontact.h"
#include "protocol.h"
#include "settings.h"

class MirandaHistory
	: private pattern::NotCopyable<MirandaHistory>
{
private:
	typedef std::map<ext::a::string, Protocol> ProtocolMap;
	typedef std::vector<MirandaContact*> ContactList;

private:
	const Settings& m_Settings;
	ProtocolMap m_Protocols;
	Protocol m_DefaultProtocol;
	bool m_bContactsAvailable;
	ContactList m_Contacts;

private:
	void populateProtocols();
	const Protocol& getProtocol(const ext::a::string& protocol) const;
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
