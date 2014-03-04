#include "_globals.h"
#include "mirandahistory.h"

#include "mirandasettings.h"

/*
 * MirandaHistory
 */

void MirandaHistory::populateProtocols()
{
	m_Protocols.clear();

	PROTOCOLDESCRIPTOR **protoList;
	int protoCount;

	if (mu::proto::enumProtocols(&protoCount, &protoList) == 0)
	{				
		upto_each_(i, protoCount) 
		{
			if (protoList[i]->type != PROTOTYPE_PROTOCOL)
			{
				continue;
			}

			ext::a::string protoName = protoList[i]->szName;

			Protocol& curProto = m_Protocols[protoName];

			curProto.displayName = Protocol::getDisplayName(protoName);
		}
	}

	m_DefaultProtocol.displayName = i18n(muT("(Unknown)"));
}

const Protocol& MirandaHistory::getProtocol(const ext::a::string& protocol) const
{
	std::map<ext::a::string, Protocol>::const_iterator i = m_Protocols.find(protocol);

	return (i != m_Protocols.end()) ? i->second : m_DefaultProtocol;
}

void MirandaHistory::makeContactsAvailable()
{
	if (m_bContactsAvailable)
	{
		return;
	}

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
	bool bHandleMeta = mu::metacontacts::_available() && m_Settings.m_MetaContactsMode != Settings::mcmIgnoreMeta;
	ext::a::string strMetaProto = bHandleMeta ? mu::metacontacts::getProtocolName() : muA("");
	MirandaSettings db;

	HANDLE hContact = mu::db_contact::findFirst();
	std::vector<HANDLE> sources;
	
	while (hContact)
	{
		db.setContact(hContact);

		const mu_ansi* pProtoName = mu::proto::getContactBaseProto(hContact);

		// if something leads to ignorance of conact jump to end of
		// processing this contact via 'break'
		do
		{
			// ignore because of bad or not loaded protocol?
			if (!pProtoName)
			{
				pProtoName = con::ProtoUnknown; // MEMO: alternative would be "break;"
			}

			ext::string curNick = mu::clist::getContactDisplayName(hContact);
			
			// retrieve protocol
			const ext::a::string curProtoName = pProtoName;
			const Protocol& curProto = getProtocol(curProtoName);

			// retrieve group
			db.setModule(con::ModCList);
			ext::string curGroup = db.readStrDirect(con::SettGroup, i18n(muT("(none)")));

			// ignore because of filtered protocol?
			if (m_Settings.m_ProtosIgnore.find(curProtoName) != m_Settings.m_ProtosIgnore.end())
			{
				break;
			}

			// init list of event sources
			sources.clear();
			sources.push_back(hContact);

			// handle meta-contacts
			if (bHandleMeta)
			{
				if (curProtoName == strMetaProto)
				{
					// don't include meta-contact history
					if (m_Settings.m_MetaContactsMode == Settings::mcmSubOnly)
					{
						sources.clear();
					}

					// include meta-contact's subcontact
					if (m_Settings.m_MetaContactsMode != Settings::mcmMetaOnly)
					{
						// find subcontacts to read history from
						int numSubs = mu::metacontacts::getNumContacts(hContact);

						if (numSubs > 0)
						{
							for (int i = 0; i < numSubs; ++i)
							{
								HANDLE hSubContact = mu::metacontacts::getSubContact(hContact, i);

								if (hSubContact)
								{
									sources.push_back(hSubContact);
								}
							}
						}
					}
				}
				else
				{
					// ignore because of meta-contact?
					if (mu::metacontacts::getMetaContact(hContact))
					{
						break;
					}
				}
			}

			// ignore because of exclude?
			db.setModule(con::ModHistoryStats);

			if (db.readBool(con::SettExclude, false))
			{
				break;
			}

			// finally add to list
			MirandaContact* pContact = MirandaContactFactory::makeMirandaContact(m_Settings.m_MergeMode, curNick, curProto.displayName, curGroup, sources);

			m_Contacts.push_back(pContact);
		} while (false);

		hContact = mu::db_contact::findNext(hContact);
	}
}

void MirandaHistory::mergeContacts()
{
	if (!m_Settings.m_MergeContacts)
	{
		return;
	}

	for (ContactList::size_type i = 0; i < m_Contacts.size(); ++i)
	{
		MirandaContact& cur = *m_Contacts[i];

		for (ContactList::size_type j = i + 1; j < m_Contacts.size(); ++j)
		{
			if (m_Contacts[j]->getNick() == cur.getNick())
			{
				if (!m_Settings.m_MergeContactsGroups || m_Contacts[j]->getGroup() == cur.getGroup())
				{
					cur.merge(*m_Contacts[j]);
					delete m_Contacts[j];

					m_Contacts.erase(m_Contacts.begin() + j);
					--j;
				}
			}
		}
	}
}

MirandaHistory::MirandaHistory(const Settings& settings)
	: m_Settings(settings), m_bContactsAvailable(false)
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
	{
		makeContactsAvailable();
	}

	return m_Contacts.size();
}

MirandaContact& MirandaHistory::getContact(int index)
{
	if (!m_bContactsAvailable)
	{
		makeContactsAvailable();
	}

	assert(index >= 0 && index < m_Contacts.size());

	return *m_Contacts[index];
}
