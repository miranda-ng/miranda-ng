#include "stdafx.h"
#include "mirandahistory.h"

#include "mirandasettings.h"

/*
 * MirandaHistory
 */

void MirandaHistory::populateProtocols()
{
	m_Protocols.clear();

	PROTOACCOUNT **protoList;
	int protoCount;
	Proto_EnumAccounts(&protoCount, &protoList);
	upto_each_(i, protoCount) 
	{
		ext::a::string protoName = protoList[i]->szModuleName;
		m_Protocols[protoName] = protoList[i]->tszAccountName;
	}

	m_DefaultProtocol = TranslateT("(Unknown)");
}

const ext::string& MirandaHistory::getProtocol(const ext::a::string& protocol) const
{
	auto i = m_Protocols.find(protocol);
	return (i != m_Protocols.end()) ? i->second : m_DefaultProtocol;
}

void MirandaHistory::makeContactsAvailable()
{
	if (m_bContactsAvailable)
		return;

	// make protocols available
	populateProtocols();

	// first run:
	// - enum all contacts
	// - skip for ignored protocol
	// - handle meta contacts (if enabled and available)
	// - skip manually excluded
	readContacts();

	// second run:
	// - merge contacts with similar names
	mergeContacts();

	m_bContactsAvailable = true;
}

void MirandaHistory::readContacts()
{
	bool bHandleMeta = m_Settings.m_MetaContactsMode != Settings::mcmIgnoreMeta;
	ext::a::string strMetaProto = bHandleMeta ? META_PROTO : "";
	
	MirandaSettings db;
	db.setModule(con::ModCList);

	std::vector<MCONTACT> sources;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		db.setContact(hContact);

		const char* pProtoName = GetContactProto(hContact);

		// ignore because of bad or not loaded protocol?
		if (!pProtoName)
			pProtoName = con::ProtoUnknown; // MEMO: alternative would be "break;"

		ext::string curNick = pcli->pfnGetContactDisplayName(hContact, 0);

		// retrieve protocol
		const ext::a::string curProtoName = pProtoName;
		const ext::string& curProto = getProtocol(curProtoName);

		// retrieve group
		ext::string curGroup = db.readStrDirect(con::SettGroup, TranslateT("(none)"));

		// ignore because of filtered protocol?
		if (m_Settings.m_ProtosIgnore.find(curProtoName) != m_Settings.m_ProtosIgnore.end())
			continue;

		// init list of event sources
		sources.clear();
		sources.push_back(hContact);

		// handle meta-contacts
		if (bHandleMeta) {
			if (curProtoName == strMetaProto) {
				// don't include meta-contact history
				if (m_Settings.m_MetaContactsMode == Settings::mcmSubOnly)
					sources.clear();

				// include meta-contact's subcontact
				if (m_Settings.m_MetaContactsMode != Settings::mcmMetaOnly) {
					// find subcontacts to read history from
					int numSubs = db_mc_getSubCount(hContact);
					if (numSubs > 0) {
						for (int i = 0; i < numSubs; ++i) {
							MCONTACT hSubContact = db_mc_getSub(hContact, i);
							if (hSubContact)
								sources.push_back(hSubContact);
						}
					}
				}
			}
			else {
				// ignore because of meta-contact?
				if (db_mc_isMeta(hContact))
					continue;
			}
		}

		// ignore because of exclude?
		if (db_get_b(hContact, con::ModHistoryStats, con::SettExclude, false))
			continue;

		// finally add to list
		MirandaContact* pContact = MirandaContactFactory::makeMirandaContact(m_Settings.m_MergeMode, curNick, curProto, curGroup, sources);
		m_Contacts.push_back(pContact);
	}
}

void MirandaHistory::mergeContacts()
{
	if (!m_Settings.m_MergeContacts)
		return;

	for (size_t i = 0; i < m_Contacts.size(); ++i) {
		MirandaContact& cur = *m_Contacts[i];

		for (size_t j = i + 1; j < m_Contacts.size(); ++j) {
			if (m_Contacts[j]->getNick() == cur.getNick()) {
				if (!m_Settings.m_MergeContactsGroups || m_Contacts[j]->getGroup() == cur.getGroup()) {
					cur.merge(*m_Contacts[j]);
					delete m_Contacts[j];

					m_Contacts.erase(m_Contacts.begin() + j);
					--j;
				}
			}
		}
	}
}

MirandaHistory::MirandaHistory(const Settings& settings) :
	m_Settings(settings), m_bContactsAvailable(false)
{
}

MirandaHistory::~MirandaHistory()
{
	citer_each_(ContactList, i, m_Contacts)
	{
		delete *i;
	}

	m_Contacts.clear();
}

int MirandaHistory::getContactCount()
{
	if (!m_bContactsAvailable)
		makeContactsAvailable();

	return m_Contacts.size();
}

MirandaContact& MirandaHistory::getContact(int index)
{
	if (!m_bContactsAvailable)
		makeContactsAvailable();

	assert(index >= 0 && index < m_Contacts.size());

	return *m_Contacts[index];
}
