#include "_globals.h"
#include "column_words.h"

#include <algorithm>

/*
 * ColWords
 */

void ColWords::generateWords(WordMap* pWords, WordList* pWordList) const
{
	int maxCount = min(pWords->size(), m_nOffset + m_nNum);
	int curWord = 0;

	pWordList->reserve(maxCount);

	if (m_nVisMode == 0)
	{
		// most common words
		MostCommonWords pred;

		while (--maxCount >= 0)
		{
			WordMap::iterator i = std::min_element(pWords->begin(), pWords->end(), pred);

			if (++curWord > m_nOffset)
			{
				pWordList->push_back(std::make_pair(i->second, i->first));
			}

			pWords->erase(i);
		}
	}
	else if (m_nVisMode == 1)
	{
		// least common words
		LeastCommonWords pred;

		while (--maxCount >= 0)
		{
			WordMap::iterator i = std::min_element(pWords->begin(), pWords->end(), pred);

			if (++curWord > m_nOffset)
			{
				pWordList->push_back(std::make_pair(i->second, i->first));
			}

			pWords->erase(i);
		}
	}
	else // m_nVisMode == 2
	{
		// longest words
		LongestWords pred;

		while (--maxCount >= 0)
		{
			WordMap::iterator i = std::min_element(pWords->begin(), pWords->end(), pred);

			if (++curWord > m_nOffset)
			{
				pWordList->push_back(std::make_pair(i->second, i->first));
			}

			pWords->erase(i);
		}
	}

	iter_each_(WordList, j, *pWordList)
	{
		(*pWords)[j->second] = j->first;
	}
}

ColWords::ColWords()
	: m_nVisMode(0)
	, m_nNum(10)
	, m_nOffset(0)
	, m_bDetail(true)
	, m_bDetailInOut(false)
	, m_bInOutColor(false)
	, m_hVisMode(NULL)
	, m_hNum(NULL)
	, m_hOffset(NULL)
	, m_hDetail(NULL)
	, m_hDetailInOut(NULL)
	, m_hInOutColor(NULL)
{
}

void ColWords::impl_copyConfig(const Column* pSource)
{
	ColBaseWords::impl_copyConfig(pSource);

	const ColWords& src = *reinterpret_cast<const ColWords*>(pSource);

	m_nVisMode     = src.m_nVisMode;
	m_nNum         = src.m_nNum;
	m_nOffset      = src.m_nOffset;
	m_bDetail      = src.m_bDetail;
	m_bDetailInOut = src.m_bDetailInOut;
	m_bInOutColor  = src.m_bInOutColor;
}

void ColWords::impl_configRead(const SettingsTree& settings)
{
	ColBaseWords::impl_configRead(settings);

	m_nVisMode     = settings.readIntRanged(con::KeyVisMode, 0, 0, 2);
	m_nNum         = settings.readIntRanged(con::KeyNum, 10, 1, 1000);
	m_nOffset      = settings.readIntRanged(con::KeyOffset, 0, 0, 1000);
	m_bDetail      = settings.readBool(con::KeyDetail, true);
	m_bDetailInOut = settings.readBool(con::KeyDetailInOut, false);
	m_bInOutColor  = settings.readBool(con::KeyInOutColor, false);
}

void ColWords::impl_configWrite(SettingsTree& settings) const
{
	ColBaseWords::impl_configWrite(settings);

	settings.writeInt (con::KeyVisMode, m_nVisMode);
	settings.writeInt (con::KeyNum, m_nNum);
	settings.writeInt (con::KeyOffset, m_nOffset);
	settings.writeBool(con::KeyDetail, m_bDetail);
	settings.writeBool(con::KeyDetailInOut, m_bDetailInOut);
	settings.writeBool(con::KeyInOutColor, m_bInOutColor);
}

void ColWords::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	ColBaseWords::impl_configToUI(Opt, hGroup);

	OptionsCtrl::Group hTemp;

	/**/hTemp          = Opt.insertGroup(hGroup, i18n(muT("Words type")));
	/**/	m_hVisMode = Opt.insertRadio(hTemp, NULL, i18n(muT("Most common words")));
	/**/	             Opt.insertRadio(hTemp, m_hVisMode, i18n(muT("Least common words")));
	/**/			     Opt.insertRadio(hTemp, m_hVisMode, i18n(muT("Longest words")));
	/**/m_hNum         = Opt.insertEdit (hGroup, i18n(muT("Number of words")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/m_hOffset      = Opt.insertEdit (hGroup, i18n(muT("Number of words to skip in output")), muT(""), OptionsCtrl::OCF_NUMBER);
	/**/m_hDetail      = Opt.insertCheck(hGroup, i18n(muT("Word count for each word (tooltip)")), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	/**/m_hDetailInOut = Opt.insertCheck(m_hDetail, i18n(muT("Show separate counts for incoming/outgoing")));
	/**/m_hInOutColor  = Opt.insertCheck(hGroup, i18n(muT("Color words according to in/out ratio")));

	Opt.setRadioChecked(m_hVisMode    , m_nVisMode    );
	Opt.setEditNumber  (m_hNum        , m_nNum        );
	Opt.setEditNumber  (m_hOffset     , m_nOffset     );
	Opt.checkItem      (m_hDetail     , m_bDetail     );
	Opt.checkItem      (m_hDetailInOut, m_bDetailInOut);
	Opt.checkItem      (m_hInOutColor , m_bInOutColor );
}

void ColWords::impl_configFromUI(OptionsCtrl& Opt)
{
	ColBaseWords::impl_configFromUI(Opt);

	m_nVisMode     = Opt.getRadioChecked(m_hVisMode    );
	m_nNum         = Opt.getEditNumber  (m_hNum        );
	m_nOffset      = Opt.getEditNumber  (m_hOffset     );
	m_bDetail      = Opt.isItemChecked  (m_hDetail     );
	m_bDetailInOut = Opt.isItemChecked  (m_hDetailInOut);
	m_bInOutColor  = Opt.isItemChecked  (m_hInOutColor );

	// ensure constraints
	utils::ensureRange(m_nNum, 1, 1000, 10);
}

void ColWords::impl_contactDataFree(Contact& contact) const
{
	ColBaseWords::impl_contactDataFree(contact);

	WordList* pTrData = reinterpret_cast<WordList*>(contact.getSlot(contactDataTransformSlotGet()));

	if (pTrData)
	{
		delete pTrData;
		contact.setSlot(contactDataTransformSlotGet(), NULL);
	}
}

void ColWords::impl_contactDataTransform(Contact& contact) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));
	WordList* pTrData = new WordList;

	contact.setSlot(contactDataTransformSlotGet(), pTrData);

	generateWords(pData, pTrData);
}

void ColWords::impl_contactDataTransformCleanup(Contact& contact) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));

	if (pData)
	{
		pData->clear();

		delete pData;
		contact.setSlot(contactDataSlotGet(), NULL);
	}
}

Column::StyleList ColWords::impl_outputGetAdditionalStyles(IDProvider& idp)
{
	StyleList l;

	if (m_bInOutColor)
	{
		l.push_back(StylePair(muT("span.onum"), muT("color: ") + utils::colorToHTML(con::ColorOut) + muT(";")));
		l.push_back(StylePair(muT("span.inum"), muT("color: ") + utils::colorToHTML(con::ColorIn) + muT(";")));
	}

	return l;
}

void ColWords::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const mu_text* szTypeDesc[] = {
		I18N(muT("Most common words")),
		I18N(muT("Least common words")),
		I18N(muT("Longest words")),
	};

	static const mu_text* szSourceDesc[] = {
		I18N(muT("incoming messages")),
		I18N(muT("outgoing messages")),
		I18N(muT("all messages")),
	};

	if (row == 1)
	{
		ext::string strTitle = str(ext::kformat(i18n(muT("#{type} for #{data}")))
			% muT("#{type}") * i18n(szTypeDesc[m_nVisMode])
			% muT("#{data}") * i18n(szSourceDesc[m_nSource]));

		writeRowspanTD(tos, getCustomTitle(i18n(szTypeDesc[m_nVisMode]), strTitle), row, 1, rowSpan);	
	}
}

void ColWords::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	const WordList* pWords = reinterpret_cast<const WordList*>(contact.getSlot(contactDataTransformSlotGet()));

	tos << muT("<td>");

	vector_each_(j, *pWords)
	{
		const Word& w = (*pWords)[j];

		if (m_bDetail)
		{
			ext::string strTooltip;

			if (!m_bDetailInOut || m_nSource != 2)
			{
				strTooltip = utils::intToString(w.first.total());
			}
			else
			{
				strTooltip = utils::htmlEscape(ext::str(ext::kformat(i18n(muT("[Out] #{out_words} / [In] #{in_words}")))
					% muT("#{out_words}") * w.first.out
					% muT("#{in_words}") * w.first.in));
			}

			if (!m_bInOutColor || m_nSource != 2 || w.first.in == w.first.out)
			{
				tos << muT("<span title=\"") << strTooltip << muT("\">")
					<< utils::htmlEscape(w.second) << muT("</span>");
			}
			else
			{
				tos << muT("<span class=\"")
					<< ((w.first.out - w.first.in > 0) ? muT("onum") : muT("inum"))
					<< muT("\" title=\"") << strTooltip << muT("\">")
					<< utils::htmlEscape(w.second) << muT("</span>");
			}
		}
		else
		{
			if (!m_bInOutColor || m_nSource != 2 || w.first.in == w.first.out)
			{
				tos << utils::htmlEscape(w.second);
			}
			else
			{
				tos << muT("<span class=\"")
					<< ((w.first.out - w.first.in > 0) ? muT("onum") : muT("inum"))
					<< muT("\">") << utils::htmlEscape(w.second) << muT("</span>");
			}
		}
		
		if (j < pWords->size() - 1)
		{
			tos << muT(", ");
		}
	}

	tos << muT("</td>") << ext::endl;
}
