#include "stdafx.h"
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
	, m_hVisMode(nullptr)
	, m_hNum(nullptr)
	, m_hOffset(nullptr)
	, m_hDetail(nullptr)
	, m_hDetailInOut(nullptr)
	, m_hInOutColor(nullptr)
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

	hTemp          = Opt.insertGroup(hGroup, TranslateT("Words type"));
		m_hVisMode  = Opt.insertRadio(hTemp, nullptr, TranslateT("Most common words"));
		              Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Least common words"));
				        Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Longest words"));
	m_hNum         = Opt.insertEdit (hGroup, TranslateT("Number of words"), L"", OptionsCtrl::OCF_NUMBER);
	m_hOffset      = Opt.insertEdit (hGroup, TranslateT("Number of words to skip in output"), L"", OptionsCtrl::OCF_NUMBER);
	m_hDetail      = Opt.insertCheck(hGroup, TranslateT("Word count for each word (tooltip)"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hDetailInOut = Opt.insertCheck(m_hDetail, TranslateT("Show separate counts for incoming/outgoing"));
	m_hInOutColor  = Opt.insertCheck(hGroup, TranslateT("Color words according to in/out ratio"));

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

void ColWords::impl_contactDataFree(CContact& contact) const
{
	ColBaseWords::impl_contactDataFree(contact);

	WordList* pTrData = reinterpret_cast<WordList*>(contact.getSlot(contactDataTransformSlotGet()));

	if (pTrData)
	{
		delete pTrData;
		contact.setSlot(contactDataTransformSlotGet(), nullptr);
	}
}

void ColWords::impl_contactDataTransform(CContact& contact) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));
	WordList* pTrData = new WordList;

	contact.setSlot(contactDataTransformSlotGet(), pTrData);

	generateWords(pData, pTrData);
}

void ColWords::impl_contactDataTransformCleanup(CContact& contact) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));

	if (pData)
	{
		pData->clear();

		delete pData;
		contact.setSlot(contactDataSlotGet(), nullptr);
	}
}

Column::StyleList ColWords::impl_outputGetAdditionalStyles(IDProvider&)
{
	StyleList l;

	if (m_bInOutColor)
	{
		l.push_back(StylePair(L"span.onum", L"color: " + utils::colorToHTML(con::ColorOut) + L";"));
		l.push_back(StylePair(L"span.inum", L"color: " + utils::colorToHTML(con::ColorIn) + L";"));
	}

	return l;
}

void ColWords::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szTypeDesc[] = {
		LPGENW("Most common words"),
		LPGENW("Least common words"),
		LPGENW("Longest words"),
	};

	static const wchar_t* szSourceDesc[] = {
		LPGENW("incoming messages"),
		LPGENW("outgoing messages"),
		LPGENW("all messages"),
	};

	if (row == 1)
	{
		ext::string strTitle = str(ext::kformat(TranslateT("#{type} for #{data}"))
			% L"#{type}" * TranslateW(szTypeDesc[m_nVisMode])
			% L"#{data}" * TranslateW(szSourceDesc[m_nSource]));

		writeRowspanTD(tos, getCustomTitle(TranslateW(szTypeDesc[m_nVisMode]), strTitle), row, 1, rowSpan);	
	}
}

void ColWords::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType)
{
	const WordList* pWords = reinterpret_cast<const WordList*>(contact.getSlot(contactDataTransformSlotGet()));

	tos << L"<td>";

	vector_each_(j, *pWords)
	{
		const Word& w = (*pWords)[j];

		if (m_bDetail)
		{
			ext::string strTooltip;

			if (!m_bDetailInOut || m_nSource != 2)
				strTooltip = utils::intToString(w.first.total());
			else
			{
				strTooltip = utils::htmlEscape(ext::str(ext::kformat(TranslateT("[Out] #{out_words} / [In] #{in_words}"))
					% L"#{out_words}" * w.first.out
					% L"#{in_words}" * w.first.in));
			}

			if (!m_bInOutColor || m_nSource != 2 || w.first.in == w.first.out)
			{
				tos << L"<span title=\"" << strTooltip << L"\">"
					<< utils::htmlEscape(w.second) << L"</span>";
			}
			else
			{
				tos << L"<span class=\""
					<< ((w.first.out - w.first.in > 0) ? L"onum" : L"inum")
					<< L"\" title=\"" << strTooltip << L"\">"
					<< utils::htmlEscape(w.second) << L"</span>";
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
				tos << L"<span class=\""
					<< ((w.first.out - w.first.in > 0) ? L"onum" : L"inum")
					<< L"\">" << utils::htmlEscape(w.second) << L"</span>";
			}
		}
		
		if (j < pWords->size() - 1)
		{
			tos << L", ";
		}
	}

	tos << L"</td>" << ext::endl;
}
