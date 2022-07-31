#include "stdafx.h"
#include "column_wordcount.h"

#include <algorithm>

/*
 * ColWordCount
 */

ColWordCount::ColWordCount()
	: m_nVisMode(0), m_bDetail(true),
	m_hVisMode(nullptr), m_hDetail(nullptr)
{
}

void ColWordCount::impl_copyConfig(const Column* pSource)
{
	ColBaseWords::impl_copyConfig(pSource);

	const ColWordCount& src = *reinterpret_cast<const ColWordCount*>(pSource);

	m_nVisMode = src.m_nVisMode;
	m_bDetail  = src.m_bDetail;
}

void ColWordCount::impl_configRead(const SettingsTree& settings)
{
	ColBaseWords::impl_configRead(settings);

	m_nVisMode = settings.readIntRanged(con::KeyVisMode, 0, 0, 2);
	m_bDetail  = settings.readBool(con::KeyDetail, true);
}

void ColWordCount::impl_configWrite(SettingsTree& settings) const
{
	ColBaseWords::impl_configWrite(settings);

	settings.writeInt (con::KeyVisMode, m_nVisMode);
	settings.writeBool(con::KeyDetail, m_bDetail);
}

void ColWordCount::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	ColBaseWords::impl_configToUI(Opt, hGroup);

	OptionsCtrl::Group hTemp;
	
	hTemp         = Opt.insertGroup(hGroup, TranslateT("Word count type"));
		m_hVisMode = Opt.insertRadio(hTemp, nullptr, TranslateT("Total words"));
		             Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Distinct words"));
		             Opt.insertRadio(hTemp, m_hVisMode, TranslateT("Ratio total/distinct words"));
	m_hDetail     = Opt.insertCheck(hGroup, TranslateT("Additional info in tooltip (depends on type)"));

	Opt.setRadioChecked(m_hVisMode, m_nVisMode);
	Opt.checkItem      (m_hDetail , m_bDetail );
}

void ColWordCount::impl_configFromUI(OptionsCtrl& Opt)
{
	ColBaseWords::impl_configFromUI(Opt);

	m_nVisMode = Opt.getRadioChecked(m_hVisMode);
	m_bDetail  = Opt.isItemChecked  (m_hDetail );
}

void ColWordCount::impl_contactDataFree(CContact& contact) const
{
	ColBaseWords::impl_contactDataFree(contact);

	size_t* pTrData = reinterpret_cast<size_t*>(contact.getSlot(contactDataTransformSlotGet()));

	if (pTrData)
	{
		delete pTrData;
		contact.setSlot(contactDataTransformSlotGet(), nullptr);
	}
}

void ColWordCount::impl_contactDataTransform(CContact& contact) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));
	size_t* pTrData = new size_t[2];

	contact.setSlot(contactDataTransformSlotGet(), pTrData);

	// total words
	pTrData[0] = 0;

	citer_each_(WordMap, word, *pData)
	{
		pTrData[0] += word->second.total();
	}

	// distinct words
	pTrData[1] = pData->size();
}

void ColWordCount::impl_contactDataTransformCleanup(CContact& contact) const
{
	WordMap* pData = reinterpret_cast<WordMap*>(contact.getSlot(contactDataSlotGet()));

	if (pData)
	{
		pData->clear();

		delete[] pData;
		contact.setSlot(contactDataSlotGet(), nullptr);
	}
}

void ColWordCount::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const wchar_t* szTypeDesc[] = {
		LPGENW("Total word count"),
		LPGENW("Distinct word count"),
		LPGENW("Ratio total/distinct words")
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

void ColWordCount::impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType)
{
	const size_t* pWordCount = reinterpret_cast<const size_t*>(contact.getSlot(contactDataTransformSlotGet()));

	switch (m_nVisMode)
	{
		case 0:
			{
				if (!m_bDetail)
				{
					tos << L"<td class=\"num\">"
						<< utils::intToGrouped(pWordCount[0])
						<< L"</td>" << ext::endl;
				}
				else
				{
					tos << L"<td class=\"num\" title=\""
						<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("#{distict_words} distinct"))
							% L"#{distict_words}" * utils::intToGrouped(pWordCount[1])))
						<< L"\">"
						<< utils::intToGrouped(pWordCount[0])
						<< L"</td>" << ext::endl;
				}
			}
			break;

		case 1:
			{
				if (!m_bDetail)
				{
					tos << L"<td class=\"num\">"
						<< utils::intToGrouped(pWordCount[1])
						<< L"</td>" << ext::endl;
				}
				else
				{
					tos << L"<td class=\"num\" title=\""
						<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("#{words} total"))
							% L"#{words}" * utils::intToGrouped(pWordCount[0])))
						<< L"\">"
						<< utils::intToGrouped(pWordCount[1])
						<< L"</td>" << ext::endl;
				}
			}
			break;

		default:
			{
				if (!m_bDetail)
				{
					tos << L"<td class=\"num\">"
						<< utils::ratioToString(pWordCount[0], pWordCount[1], 2)
						<< L"</td>" << ext::endl;
				}
				else
				{
					tos << L"<td class=\"num\" title=\""
						<< utils::htmlEscape(ext::str(ext::kformat(TranslateT("#{words} total / #{distict_words} distinct"))
							% L"#{words}" * utils::intToGrouped(pWordCount[0])
							% L"#{distict_words}" * utils::intToGrouped(pWordCount[1])))
						<< L"\">"
						<< utils::ratioToString(pWordCount[0], pWordCount[1], 2)
						<< L"</td>" << ext::endl;
				}
			}
			break;
	}
}
