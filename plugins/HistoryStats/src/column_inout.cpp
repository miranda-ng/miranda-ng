#include "stdafx.h"
#include "column_inout.h"

/*
 * ColInOut
 */

ColInOut::ColInOut()
	: m_nSource(2), m_bAbsolute(false), m_nAbsTime(1),
	m_hSource(NULL), m_hAbsolute(NULL), m_hAbsTime(NULL)
{
}

void ColInOut::impl_copyConfig(const Column* pSource)
{
	const ColInOut& src = *reinterpret_cast<const ColInOut*>(pSource);

	m_nSource   = src.m_nSource;
	m_bAbsolute = src.m_bAbsolute;
	m_nAbsTime  = src.m_nAbsTime;
}

void ColInOut::impl_configRead(const SettingsTree& settings)
{
	m_nSource   = settings.readIntRanged(con::KeySource  , 2, 0, 8);
	m_bAbsolute = settings.readBool     (con::KeyAbsolute, false);
	m_nAbsTime  = settings.readIntRanged(con::KeyAbsTime , 1, 0, 2);
}

void ColInOut::impl_configWrite(SettingsTree& settings) const
{
	settings.writeInt (con::KeySource  , m_nSource  );
	settings.writeBool(con::KeyAbsolute, m_bAbsolute);
	settings.writeInt (con::KeyAbsTime , m_nAbsTime );
}

void ColInOut::impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup)
{
	OptionsCtrl::Group hTemp;

	m_hSource   = Opt.insertCombo(hGroup, TranslateT("Data source"));
	hTemp       = Opt.insertGroup(hGroup, TranslateT("Display as"));
	m_hAbsolute = Opt.insertRadio(hTemp, NULL, TranslateT("Absolute"));
	m_hAbsolute = Opt.insertRadio(hTemp, m_hAbsolute, TranslateT("Average"), OptionsCtrl::OCF_DISABLECHILDSONUNCHECK);
	m_hAbsTime  = Opt.insertRadio(m_hAbsolute, NULL, TranslateT("Units per day"));
	              Opt.insertRadio(m_hAbsolute, m_hAbsTime, TranslateT("Units per week"));
	              Opt.insertRadio(m_hAbsolute, m_hAbsTime, TranslateT("Units per month (30 days)"));

	static const TCHAR* sourceTexts[] = {
		LPGENT("Characters (incoming)"),
		LPGENT("Characters (outgoing)"),
		LPGENT("Characters (all)"),
		LPGENT("Messages (incoming)"),
		LPGENT("Messages (outgoing)"),
		LPGENT("Messages (all)"),
		LPGENT("Chats (incoming)"),
		LPGENT("Chats (outgoing)"),
		LPGENT("Chats (all)"),
	};

	array_each_(i, sourceTexts)
	{
		Opt.addComboItem(m_hSource, TranslateTS(sourceTexts[i]));
	}

	Opt.setComboSelected(m_hSource  , m_nSource          );
	Opt.setRadioChecked (m_hAbsolute, m_bAbsolute ? 0 : 1);
	Opt.setRadioChecked (m_hAbsTime , m_nAbsTime         );
}

void ColInOut::impl_configFromUI(OptionsCtrl& Opt)
{
	m_nSource   = Opt.getComboSelected(m_hSource);
	m_bAbsolute = (Opt.getRadioChecked(m_hAbsolute) == 0);
	m_nAbsTime  = Opt.getRadioChecked(m_hAbsTime);
}

void ColInOut::impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const
{
	static const TCHAR* szShortDesc[] = {
		LPGENT("Characters"),
		LPGENT("Messages"),
		LPGENT("Chats")
	};

	static const TCHAR* szSourceDesc[] = {
		LPGENT("Incoming characters"),
		LPGENT("Outgoing characters"),
		LPGENT("Characters"),
		LPGENT("Incoming messages"),
		LPGENT("Outgoing messages"),
		LPGENT("Messages"),
		LPGENT("Incoming chats"),
		LPGENT("Outgoing chats"),
		LPGENT("Chats")
	};

	static const TCHAR* szUnitDesc[] = {
		LPGENT("day"),
		LPGENT("week"),
		LPGENT("month"),
	};

	if (row == 1)
	{
		ext::string strTitle;

		if (m_bAbsolute)
		{
			strTitle = TranslateTS(szSourceDesc[m_nSource]);
		}
		else
		{
			strTitle = str(ext::kformat(TranslateT("#{data} per #{unit}"))
				% _T("#{data}") * TranslateTS(szSourceDesc[m_nSource])
				% _T("#{unit}") * TranslateTS(szUnitDesc[m_nAbsTime]));
		}

		writeRowspanTD(tos, getCustomTitle(TranslateTS(szShortDesc[m_nSource / 3]), strTitle), row, 1, rowSpan);
	}
}

void ColInOut::impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType)
{
	if (m_bAbsolute)
	{
		static int (Contact::*getData[])() const = {
			&Contact::getInBytes,
			&Contact::getOutBytes,
			&Contact::getTotalBytes,
			&Contact::getInMessages,
			&Contact::getOutMessages,
			&Contact::getTotalMessages,
			&Contact::getInChats,
			&Contact::getOutChats,
			&Contact::getTotalChats,
		};

		tos << _T("<td class=\"num\">")
			<< utils::intToGrouped((contact.*getData[m_nSource])())
			<< _T("</td>") << ext::endl;
	}
	else
	{
		static double (Contact::*getData[])() const = {
			&Contact::getInBytesAvg,
			&Contact::getOutBytesAvg,
			&Contact::getTotalBytesAvg,
			&Contact::getInMessagesAvg,
			&Contact::getOutMessagesAvg,
			&Contact::getTotalMessagesAvg,
			&Contact::getInChatsAvg,
			&Contact::getOutChatsAvg,
			&Contact::getTotalChatsAvg,
		};

		static const double avgFactor[] = {
			60.0 * 60.0 * 24.0,
			60.0 * 60.0 * 24.0 * 7.0,
			60.0 * 60.0 * 24.0 * 30.0,
		};

		tos << _T("<td class=\"num\">")
			<< utils::floatToGrouped((contact.*getData[m_nSource])() * avgFactor[m_nAbsTime], 1)
			<< _T("</td>") << ext::endl;
	}
}
