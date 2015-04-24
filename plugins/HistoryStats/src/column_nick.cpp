#include "stdafx.h"
#include "column_nick.h"

/*
 * ColNick
 */

ColNick::ColNick()
	: m_bDetail(true)
	, m_bContactCount(true)
	, m_hDetail(NULL)
	, m_hContactCount(NULL)
{
}

void ColNick::impl_copyConfig(const Column* pSource)
{
	const ColNick& src = *reinterpret_cast<const ColNick*>(pSource);

	m_bDetail       = src.m_bDetail;
	m_bContactCount = src.m_bContactCount;
}

void ColNick::impl_configRead(const SettingsTree& settings)
{
	m_bDetail       = settings.readBool(con::KeyDetail, true);
	m_bContactCount = settings.readBool(con::KeyContactCount, true);
}

void ColNick::impl_configWrite(SettingsTree& settings) const
{
	settings.writeBool(con::KeyDetail, m_bDetail);
	settings.writeBool(con::KeyContactCount, m_bContactCount);
}

void ColNick::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	m_hDetail       = Opt.insertCheck(hGroup, TranslateT("First/last message time (tooltip)"));
	m_hContactCount = Opt.insertCheck(hGroup, TranslateT("Show contact count for omitted/totals (tooltip)"));

	Opt.checkItem(m_hDetail      , m_bDetail      );
	Opt.checkItem(m_hContactCount, m_bContactCount);
}

void ColNick::impl_configFromUI(OptionsCtrl& Opt)
{
	m_bDetail       = Opt.isItemChecked(m_hDetail);
	m_bContactCount = Opt.isItemChecked(m_hContactCount);
}

void ColNick::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	if (row == 1)
	{
		writeRowspanTD(tos, getCustomTitle(TranslateT("Nick"), TranslateT("Nick")), row, 1, rowSpan);
	}
}

void ColNick::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display)
{
	tos << _T("<td");

	if (m_bDetail || (m_bContactCount && display != asContact))
	{
		ext::string strFirstTime = TranslateT("(unknown)");
		ext::string strLastTime = TranslateT("(unknown)");

		if (contact.isFirstLastTimeValid())
		{
			strFirstTime = utils::timestampToDateTime(contact.getFirstTime());
			strLastTime = utils::timestampToDateTime(contact.getLastTime());
		}

		ext::string strTooltip;

		if (m_bDetail && !(m_bContactCount && display != asContact))
		{
			strTooltip = utils::htmlEscape(ext::str(ext::kformat(TranslateT("[First] #{first_time} / [Last] #{last_time}"))
				% _T("#{first_time}") * strFirstTime
				% _T("#{last_time}") * strLastTime));
		}
		else if (m_bDetail && (m_bContactCount && display != asContact))
		{
			strTooltip = utils::htmlEscape(ext::str(ext::kformat(TranslateT("#{count} contacts / [First] #{first_time} / [Last] #{last_time}"))
				% _T("#{count}") * contact.getNumContacts()
				% _T("#{first_time}") * strFirstTime
				% _T("#{last_time}") * strLastTime));
		}
		else // if (!m_bDetail && (m_bContactCount && display != asContact))
		{
			strTooltip = utils::htmlEscape(ext::str(ext::kformat(TranslateT("#{count} contacts"))
				% _T("#{count}") * contact.getNumContacts()));
		}

		tos << _T(" title=\"") << strTooltip << _T("\">");
	}
	else tos << _T(">");		

	if (display == asContact)
		tos << utils::htmlEscape(contact.getNick()) << _T("</td>") << ext::endl;
	else if (display == asOmitted)
		tos << TranslateT("Omitted") << _T("</td>") << ext::endl;
	else
		tos << TranslateT("Totals") << _T("</td>") << ext::endl;
}
