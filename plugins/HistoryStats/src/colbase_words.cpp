#include "stdafx.h"
#include "colbase_words.h"

/*
 * ColBaseWords
 */

void ColBaseWords::addWord(WordMap* pWords, const ext::string& word, bool bOutgoing) const
{
	// filter words
	if (m_bFilterWords)
	{
		upto_each_(i, m_ActiveWordFilter.size())
		{
			const Filter* pFilter = m_ActiveWordFilter[i];

			switch (pFilter->getMode())
			{
				case Settings::fwmWordsMatching:
					{
						citer_each_(WordSet, j, pFilter->getWords())
						{
							if (word == *j)
							{
								return;
							}
						}
					}
					break;

				case Settings::fwmWordsContaining:
					{
						citer_each_(WordSet, j, pFilter->getWords())
						{
							if (word.find(*j) != ext::string::npos)
							{
								return;
							}
						}
					}
					break;

				case Settings::fwmWordsStartingWith:
					{
						citer_each_(WordSet, j, pFilter->getWords())
						{
							if (word.length() >= j->length() && word.substr(0, j->length()) == *j)
							{
								return;
							}
						}
					}
					break;

				case Settings::fwmWordsEndingWith:
					{
						citer_each_(WordSet, j, pFilter->getWords())
						{
							if (word.length() >= j->length() && word.substr(word.length() - j->length(), j->length()) == *j)
							{
								return;
							}
						}
					}
					break;
			}
		}
	}

	// insert, if we didn't get filtered
	WordMap::iterator i = pWords->find(word);

	if (i != pWords->end())
	{
		(bOutgoing ? i->second.out : i->second.in)++;
	}
	else
	{
		pWords->insert(std::make_pair(word, bOutgoing ? InOut(0, 1) : InOut(1, 0)));
	}
}

void ColBaseWords::parseMsg(WordMap* pWords, const ext::string& msg, bool bOutgoing) const
{
	// filter messages
	if (m_bFilterMessages) {
		ext::string msgLC = utils::toLowerCase(msg);

		upto_each_(i, m_ActiveMessageFilter.size())
		{
			const Filter* pFilter = m_ActiveMessageFilter[i];

			switch (pFilter->getMode()) {
			case Settings::fwmMessagesMatching:
				citer_each_(WordSet, j, pFilter->getWords())
				{
					if (msgLC == *j) {
						return;
					}
				}
				break;

			case Settings::fwmMessagesContaining:
				citer_each_(WordSet, j, pFilter->getWords())
				{
					if (msgLC.find(*j) != ext::string::npos) {
						return;
					}
				}
				break;

			case Settings::fwmMessagesStartingWith:
				citer_each_(WordSet, j, pFilter->getWords())
				{
					if (msgLC.length() >= j->length() && msgLC.substr(0, j->length()) == *j) {
						return;
					}
				}
				break;

			case Settings::fwmMessagesEndingWith:
				citer_each_(WordSet, j, pFilter->getWords())
				{
					if (msgLC.length() >= j->length() && msgLC.substr(msgLC.length() - j->length(), j->length()) == *j) {
						return;
					}
				}
				break;
			}
		}
	}

	// start parsing into words if not already filtered
	ext::string::size_type firstChar = 0;
	ext::string::size_type nextSpace;

	while (firstChar < msg.length() && getCharMapper()->mapChar(msg[firstChar]) == ' ')
		++firstChar;

	while (firstChar < msg.length()) {
		nextSpace = firstChar + 1;

		while (nextSpace < msg.length() && getCharMapper()->mapChar(msg[nextSpace]) != ' ')
			++nextSpace;

		int wordLen = nextSpace - firstChar;

		if (wordLen >= m_nMinLength && (m_nMaxLength == 0 || wordLen <= m_nMaxLength)) {
			ext::string word(wordLen, '_');

			upto_each_(i, wordLen)
			{
				word[i] = getCharMapper()->mapChar(msg[firstChar + i]);
			}

			addWord(pWords, word, bOutgoing);
		}

		firstChar = nextSpace + 1;

		while (firstChar < msg.length() && getCharMapper()->mapChar(msg[firstChar]) == ' ')
			++firstChar;
	}
}

ColBaseWords::ColBaseWords()
	: m_nSource(2), m_nMinLength(1), m_nMaxLength(0), m_bFilterLinks(true),
	m_hSource(NULL), m_hMinLength(NULL), m_hMaxLength(NULL), m_hFilterLinks(NULL)
{
}

void ColBaseWords::impl_copyConfig(const Column* pSource)
{
	const ColBaseWords& src = *reinterpret_cast<const ColBaseWords*>(pSource);

	m_nSource      = src.m_nSource;
	m_nMinLength   = src.m_nMinLength;
	m_nMaxLength   = src.m_nMaxLength;
	m_bFilterLinks = src.m_bFilterLinks;
	m_FilterWords  = src.m_FilterWords;
}

void ColBaseWords::impl_configRead(const SettingsTree& settings)
{
	m_nSource      = settings.readIntRanged(con::KeySource, 2, 0, 2);
	m_nMinLength   = settings.readIntRanged(con::KeyMinLength, 1, 1, 1000);
	m_nMaxLength   = settings.readIntRanged(con::KeyMaxLength, 0, 0, 1000);
	m_bFilterLinks = settings.readBool(con::KeyFilterLinks, true);

	// filter words
	m_FilterWords.clear();

	int nCount = settings.readInt(con::KeyFilterWords, 0);

	upto_each_(i, nCount)
	{
		m_FilterWords.insert(settings.readStr((con::KeyFilterWords + utils::intToString(i)).c_str(), _T("")));
	}
}

void ColBaseWords::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeySource, m_nSource);
	settings.writeInt (con::KeyMinLength, m_nMinLength);
	settings.writeInt (con::KeyMaxLength, m_nMaxLength);
	settings.writeBool(con::KeyFilterLinks, m_bFilterLinks);

	// filter words
	settings.writeInt(con::KeyFilterWords, m_FilterWords.size());

	int nFilterNr = 0;

	citer_each_(ColFilterSet, i, m_FilterWords)
	{
		settings.writeStr((con::KeyFilterWords + utils::intToString(nFilterNr++)).c_str(), i->c_str());
	}
}

void ColBaseWords::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	OptionsCtrl::Group hTemp;

	hTemp = Opt.insertGroup (hGroup, TranslateT("Extract words from"));
	m_hSource = Opt.insertRadio (hTemp, NULL, TranslateT("Incoming messages"));
	             Opt.insertRadio (hTemp, m_hSource, TranslateT("Outgoing messages"));
	              Opt.insertRadio (hTemp, m_hSource, TranslateT("All messages"));
	m_hMinLength = Opt.insertEdit(hGroup, TranslateT("Ignore words shorter than (chars)"), _T(""), OptionsCtrl::OCF_NUMBER);
	m_hMaxLength = Opt.insertEdit(hGroup, TranslateT("Ignore words longer than (chars, 0=no limit)"), _T(""), OptionsCtrl::OCF_NUMBER);
	m_hFilterLinks = Opt.insertCheck(hGroup, TranslateT("Filter URLs/e-mail addresses"));
	                 Opt.insertButton(hGroup, TranslateT("Filter words/messages"), TranslateT("Define..."), 0, Settings::biFilterWords);

	Opt.setRadioChecked(m_hSource     , m_nSource     );
	Opt.setEditNumber  (m_hMinLength  , m_nMinLength  );
	Opt.setEditNumber  (m_hMaxLength  , m_nMaxLength  );
	Opt.checkItem      (m_hFilterLinks, m_bFilterLinks);
}

void ColBaseWords::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource = Opt.getRadioChecked(m_hSource);
	m_nMinLength = Opt.getEditNumber(m_hMinLength);
	m_nMaxLength = Opt.getEditNumber(m_hMaxLength);
	m_bFilterLinks = Opt.isItemChecked(m_hFilterLinks);

	// ensure constraints
	utils::ensureRange(m_nMinLength, 1, 1000, 1);
	utils::ensureRange(m_nMaxLength, 0, 1000, 0);
}

ext::string ColBaseWords::impl_contactDataGetUID() const
{
	ext::string strUID = ext::str(ext::format(_T("words-|-|-|-|"))
											% m_nSource
											% m_nMinLength
											% m_nMaxLength
											% (m_bFilterLinks ? 1 : 0));

	citer_each_(ColFilterSet, i, m_FilterWords)
	{
		strUID += _T("-");
		strUID += *i;
	}

	return strUID;
}

void ColBaseWords::impl_contactDataBeginAcquire()
{
	m_bFilterMessages = false;
	m_bFilterWords = false;
	m_ActiveMessageFilter.clear();
	m_ActiveWordFilter.clear();

	citer_each_(ColFilterSet, i, m_FilterWords)
	{
		const Filter* pFilter = getSettings()->getFilter(*i);

		if (pFilter && !pFilter->getWords().empty()) {
			switch (pFilter->getMode()) {
			case Settings::fwmMessagesMatching:
			case Settings::fwmMessagesContaining:
			case Settings::fwmMessagesStartingWith:
			case Settings::fwmMessagesEndingWith:
				m_ActiveMessageFilter.push_back(pFilter);
				m_bFilterMessages = true;
				break;

			case Settings::fwmWordsMatching:
			case Settings::fwmWordsContaining:
			case Settings::fwmWordsStartingWith:
			case Settings::fwmWordsEndingWith:
				m_ActiveWordFilter.push_back(pFilter);
				m_bFilterWords = true;
				break;
			}
		}
	}
}

void ColBaseWords::impl_contactDataPrepare(Contact& contact) const
{
	WordMap* pData = new WordMap;
	contact.setSlot(contactDataSlotGet(), pData);
}

void ColBaseWords::impl_contactDataFree(Contact& contact) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));
	if (pData) {
		delete pData;
		contact.setSlot(contactDataSlotGet(), NULL);
	}
}

void ColBaseWords::impl_contactDataAcquireMessage(Contact& contact, Message& msg)
{
	if (m_nSource == 2 || m_nSource == 1 && msg.isOutgoing() || m_nSource == 0 && !msg.isOutgoing()) {
		WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));
		parseMsg(pData, m_bFilterLinks ? msg.getWithoutLinks() : msg.getRaw(), msg.isOutgoing());
	}
}

void ColBaseWords::impl_contactDataMerge(Contact& contact, const Contact& include) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));
	const WordMap* pIncData = reinterpret_cast<const WordMap*>(include.getSlot(contactDataSlotGet()));

	citer_each_(WordMap, j, *pIncData)
	{
		if (pData->find(j->first) != pData->end())
			(*pData)[j->first] += j->second;
		else
			pData->insert(*j);
	}
}
